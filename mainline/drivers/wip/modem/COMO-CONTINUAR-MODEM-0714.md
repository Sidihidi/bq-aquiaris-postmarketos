# Cómo continuar el port del módem (H3 handshake) — guía autocontenida — 0714

> Todo lo necesario para retomar el arranque del MD sin re-derivar nada. Estado: **H0-H2 ✅, H3
> scaffolding ✅ (load+remap+release ejecutan y el AP sobrevive), falta el handshake CCCI.**
> Docs de detalle: `H0/H1/H2/H3-*.md` en este dir. Fórmulas y direcciones: `H2-MAPA-REGISTROS-0713.md`.

## 1. Resumen de dónde estamos
| Hito | Estado | Prueba |
|---|---|---|
| H0 firmware | ✅ | `modem.img` = `MOLY.WR8.W1449.MD.WG.MP.V1` extraído |
| H1 MTCMOS del MD | ✅ | `PWR_STATUS bit0=1` |
| H2a CCIF+BANK | ✅ | accesibles (reads=0, no 0xffffffff) |
| H2b carveout 24MB | ✅ | `OF: reserved mem: 0xb8000000..0xb97fffff` |
| H3 load/remap/release | ✅ | firmware colocado (w0=0xe59ff018), remap OK, AP sobrevive al release |
| **H3 handshake CCCI** | ❌ **AQUÍ SEGUIMOS** | CON/RCHNUM=0, el MD no levanta canal |

## 2. Artefactos (en la Pi `cpcd@192.168.0.38` y el móvil `root@172.16.42.1` por usb0)
**Pi:**
- Source (H1+H2+H3): `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c`
  (backups `.bak-pre-h1-md` = limpio, `.bak-pre-h3` = con H1+H2). Copia en repo: `mt6582-spm-H1.c`.
- **Config LIMPIO obligatorio**: `~/mainline/linux-7.0.12/build-krillin/.config.bak-robustez-0711`
  (SIN `CONFIG_EXTRA_FIRMWARE` — hornear CONNSYS rompe el boot). `cp` a `.config` antes de compilar.
- Firmware: `~/mainline/downstream/stock-firmware-0713/modem.img` (md5 `0414422b`). Ya en el móvil en
  `/lib/firmware/modem.img`.
- **DTBs (críticos, ver §5)**: `~/mainline/pkg/ganador.dtb` (base buena, SIN carveout),
  `ganador-h2b.dtb` (CON carveout modem-region@b8000000), `ganador.dts` (editable, = decompilado + carveout).
- Imágenes: `~/mainline/pkg/boot-menupick23.img` (H3 test actual), `boot-menupick13-DSIFIX-GANADOR.img`
  (**daily driver bueno** — para recuperar).
- Downstream de referencia: `~/mainline/downstream/drivers/misc/mediatek/dual_ccci/mt6582/`.

## 3. Ciclo de build+flash (reproducible)
```bash
# EN LA PI:
cd ~/mainline/linux-7.0.12
cp build-krillin/.config.bak-robustez-0711 build-krillin/.config     # config LIMPIO (¡clave!)
# ...editar drivers/soc/mediatek/mt6582-spm.c...
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc) zImage
cd ~/mainline/pkg
cat build-krillin/.../zImage ganador-h2b.dtb > zimage-dtb           # ¡ganador-h2b.dtb (CON carveout)!
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk
abootimg --create boot-test.img -f bootimg-mp7.cfg -k zimage-dtb-mtk -r initrd-menupick-mtk
# STAGE-FLASH desde pmOS viva (móvil en pmOS):
scp boot-test.img root@172.16.42.1:/root/b.img
ssh root@172.16.42.1 'dd if=/root/b.img of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync'   # verificar md5 readback
ssh root@172.16.42.1 'reboot'    # el usuario elige pmOS en el menú
```
`bootimg-mp7.cfg` y `mtk_hdr.py` están en `~/mainline/pkg/`. Límite de partición boot: `0x1400000` (20MB).

## 4. Cómo PROBAR (los triggers sysfs, granular y seguro)
Con el móvil en pmOS (kernel H3), en orden (los primeros son seguros, el release es el que suelta el MD):
```bash
ssh root@172.16.42.1
echo 1 > /sys/module/mt6582_spm/parameters/spm_md_dump      # lee CCIF+BANK (baseline, seguro)
echo 1 > /sys/module/mt6582_spm/parameters/spm_md_load      # carga modem.img al carveout (seguro)
echo 1 > /sys/module/mt6582_spm/parameters/spm_md_remap     # escribe BANK-remap (seguro)
echo 1 > /sys/module/mt6582_spm/parameters/spm_md_release   # SUELTA EL MD + sondea CCIF 5s (arriesgado)
dmesg | grep -aiE 'H3|H2 CCIF'                               # ver resultados
```
**Éxito del handshake** = tras el release, `RCHNUM` o `CON` del CCIF (dump) cambian de 0 → el MD levantó un
canal (`MD_INIT_START_BOOT`). Ahora mismo siguen a 0.

## 5. ⚠️ GOTCHAS que cuestan horas (NO repetir)
1. **DTB del árbol compartido roto por la sesión Mac (.123)**: modifica `mt6582-bq-krillin.dts` de forma
   que hace BOOTLOOP en nuestro árbol (quita el interrupt-controller del PMIC, etc.). **NO uses el dtb del
   build-dir.** Usa SIEMPRE `ganador-h2b.dtb` (extraído de la imagen buena + carveout). Para regenerarlo:
   `dtc -I dtb -O dts ganador.dtb > ganador.dts` (con `/usr/bin/dtc`, el del kernel no está compilado) →
   añadir el nodo carveout ANCLANDO en `framebuffer-region@bf400000` (NO en `ranges;`) → `dtc -I dts -O dtb`.
2. **Carveout NO a 0xbc000000** (cae dentro del CMA 0xbb-0xbf → ignorado). Va a **0xb8000000** (bajo CMA).
3. **Config con EXTRA_FIRMWARE = bootloop**. Usar `.config.bak-robustez-0711`.
4. **Recuperar bootloop**: usuario pone el móvil en fastboot →
   `fastboot flash boot ~/mainline/pkg/boot-menupick13-DSIFIX-GANADOR.img && fastboot continue` (desde la Pi).
   NUNCA forzar apagados; el power-cycle lo hace el usuario.

## 6. Siguiente iteración: el handshake CCCI (candidatos, con RE pendiente)
El MD no levanta canal. NO es la EMI-MPU (bring-up = memoria abierta). A investigar/probar en el downstream
`dual_ccci/mt6582/src/ccci_platform.c` + `ccci_md_main.c`:
1. **CCIF init**: ¿hay que configurar el CCIF (CON a modo arbitración `CCIF_CON_ARB=0x1`) antes del release?
   Buscar dónde el downstream inicializa el CCIF (`ccif_ctl_init`, escritura a `CCIF_CON`).
2. **Run-time data / shared-mem**: `set_md_runtime`/`ccci_send_run_time_data` — el MD puede esperar el
   bloque de geometría de shared-mem antes de hablar. Ver qué se escribe al smem ANTES de `let_md_go`.
3. **Secuencia completa**: revisar `md_env_setup_before_boot` → todo lo que ocurre entre el remap y
   `let_md_go` (`ccci_start_modem`, `boot_md`). Puede faltar un paso (init del CCIF, IRQ 132 registrada,
   escribir la misc_info al smem, etc.).
4. **Layout de memoria del MD**: cómo el reset vector del MD (arranca en su addr 0x0) alcanza el firmware
   en DRAM. El BANK-remap mapea bank4 (vista 0x40000000). Verificar si el firmware debe estar en otro offset
   o si falta apuntar el vector inicial del MD al DRAM.
5. **Registrar la IRQ 132 (CCIF0_AP)** y manejar la RX: aunque el MD levante el canal, sin el handler no
   avanza el protocolo. Para solo DETECTAR HS1 basta el poll (ya está); para responder hace falta el IRQ.

Cuando el dump muestre `RCHNUM`/`CON` ≠ 0 tras el release = **HS1 conseguido** (el MD ejecuta MOLY y habla).

## 7. Estado del móvil ahora
En `menupick23` (test H3). Daily bueno = GANADOR. Batería: cargar antes de sesiones largas (la Pi carga
lento; para recuperar rápido, cargador de pared — el daemon `fan5405-charge` mantiene la carga).

*Guía 2026-07-14. Andamiaje H3 completo; el handshake CCCI es la iteración que sigue.*
