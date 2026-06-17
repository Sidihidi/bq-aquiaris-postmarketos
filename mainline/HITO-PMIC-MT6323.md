# 🏆 HITO: PMIC MT6323 al DeviceTree (el "hub") — RESUELTO (2026-06-17)

Kernel **#22** mainline 7.0.12. El MT6323 (PMIC del MT6582) entra entero por el
DeviceTree vía el PMIC wrapper (pwrap). **Es el prerequisito de batería%, WiFi,
audio y de la ruta a Phosh** (gestiona los rails de GPU/display/RF).

## Resultado verificado (en hardware)
```
probe of 1000d000.pwrap returned 0            # pwrap OK (antes daba -2)
probe of 1000d000.pwrap:mt6323 returned 0     # MFD OK (antes -ENXIO)
mt6323-regulator: Chip ID = 0x2023            # chip correcto
31 reguladores: VA VCAMA VCAMAF VCAMD VCAMIO VCN18 VCN28 VCN33_BT VCN33_WIFI
  VEMC3V3 VGP2 VGP3 VIBR VIO18 VIO28 VM VMC VMCH VPA VPROC VRF18 VRTC VSIM1
  VSIM2 VSYS VTCXO VUSB vgp1 (+ dummy/vmmc/vqmmc)
vgp1: 2800000uV state=enabled                 # alimentación del táctil FT5336
storm "unexpected interrupt": 0
```

## Las 3 piezas (drivers) + el DT
Ver `patches/0002`, `0003` (pwrap) y `0004` (mfd). DT en `dts/mt6582-bq-krillin.dts`
(nodo `pwrap@1000d000` compatible **`mediatek,mt6582-pwrap`** → `mt6323` →
`mt6323-regulator` → `ldo_vgp1`).

1. **pwrap_mt6582 con `caps=0`** — la variante mt2701 tiene `PWRAP_CAP_RESET` →
   pide un reset-controller que el MT6582 no tiene → probe fallaba con **-2**.
2. **`int_en_all=0` + saltar `request_irq`** — con la IRQ del pwrap activa había
   una **tormenta infinita** `unexpected interrupt int=0x2` que inundaba la consola
   framebuffer y colgaba el arranque. Poner INT_EN=0 NO bastó (la escritura va al
   offset INT_EN de mt2701, que no enmascara la línea SPI en el MT6582) → la
   solución fue **no registrar el handler** (usamos el pwrap por polling).
3. **MFD `mt6397-core`: IRQ del PMIC opcional** — el probe exigía `interrupts` en
   el nodo mt6323 (`-ENXIO: IRQ index 0 not found`). No cableamos la IRQ del PMIC
   (no la necesitamos: reguladores y AUXADC van por polling) →
   `platform_get_irq_optional` + saltar `irq_init` si no hay IRQ.

Config: `CONFIG_MFD_MT6397=y`, `CONFIG_REGULATOR_MT6323=y` (ya estaban).

## ⚠️ LECCIONES DE ORO de mtkclient en este equipo (¡no repetir el sufrimiento!)

Esta sesión "rompió fastboot" tras flashear un kernel con storm; se recuperó por
**BROM** (no hizo falta reescribir Android). Aprendido:

- **`wf --offset` ESTÁ ROTO en el DA legacy**: IGNORA el offset y escribe en
  **sector 0** (machaca MBR/proinfo). El log lo delata: `Wrote ... to sector 0`.
  → **USAR `wo <offset> <length> <fichero>`** (posicional) → `writeflash(addr=offset)`.
  Ej.: `sudo venv/bin/python mtk.py wo 0x2900000 0xD18800 boot.img`
  El log correcto dice `Writing offset 0x2900000 ... Wrote ... to offset 0x2900000`.
- **Los offsets de mtkclient son HEX** (`rf`/`ro`/`wo`). Pasar con `0x...`.
- **Offsets de partición (espacio mtkclient, = dumchar + 0xB80000):**
  `seccfg 0x2880000` · `uboot(LK) 0x28A0000` · `boot 0x2900000` · (boot = uboot+0x60000).
  Verificar firma al leer: uboot → `88 16 88 58` ("LK"); boot → `ANDROID!`.
- **Setup en la Pi**: `sudo systemctl stop ModemManager` (roba el VCOM 0e8d:2000),
  instalar `~/mtkclient/Setup/Linux/*.rules` en /etc/udev/rules.d, y que
  `~/mtkclient/hwparam.json` sea de `cpcd` (no root) o `wo`/`rf` peta con
  `PermissionError`. Correr `~/mtkclient/venv/bin/python mtk.py ...` (venv).
- **BROM en Pi5 = interactivo + reconnect-on-cue**: lanzar el comando y, cuando
  imprime el bucle de espera, batería fuera→dentro + mantener Vol− + conectar USB.
  El DA legacy (`MTK_AllInOne_DA_mt6590.bin`) sube y conecta a stage2 fiable.
- **El LK golden es de LOLLIPOP (verifica firma → bootloop con kernels propios).**
  El LK que tiene el equipo es el **KitKat 1.5.2 (permisivo)** y los primeros
  247184 B == `stock-1.5.2/lk.bin` (md5 `f0f3a93e…`). **NO restaurar el LK golden.**
- **Tamaño de boot-pmic.img = 0xD18800** (13731840 B), no 0xD18000.

## Estado tras el hito
- **fastboot VOLVIÓ** tras arrancar un kernel limpio (estaba colgado por el storm).
  Ya hay dos vías de flasheo: `fastboot flash boot` y `mtk wo 0x2900000 …` por BROM.
- **sshd** no arrancó solo este boot (está en runlevel `default`; `rc-service sshd
  restart` lo levanta). Pendiente: hacerlo robusto al orden de la red USB.

## Siguiente
- Migrar el táctil del poke late_initcall de VGP1 a `vcc-supply=<&mt6323_vgp1_reg>`.
- Batería % (AUXADC, BATSNS ch7) — ahora factible por el hub.
- WiFi (VCN33_WIFI ya disponible), Audio (VA/VRF18), EINT.
