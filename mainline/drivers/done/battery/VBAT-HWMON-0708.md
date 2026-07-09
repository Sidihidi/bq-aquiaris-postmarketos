# VBAT por driver kernel (hwmon mt6323-auxadc) — retirada de pwrap_poke del camino periódico (2026-07-08)

## Por qué

El wedge del bloque de interrupciones del PMIC (0708: eint 25 / mt6397-pmic congelado hasta
rebind de mt6323-keys) tiene como sospechoso principal a `pwrap_poke`: MMIO crudo al pwrap
@0x1000d000 desde userspace, SIN lock contra el driver pwrap del kernel. El único usuario
*periódico* era `/usr/local/bin/battery` (llamado cada 15 s por `battery-upower`): cada
lectura de VBAT hacía 2 lecturas + 2 escrituras + 8 lecturas por pwrap_poke, 24/7, en carrera
con keys/rtc/regulator/codec que van por el regmap del kernel.

## Qué

- **`mt6323-auxadc.c`** → `drivers/hwmon/mt6323-auxadc.c`. Driver hwmon mínimo que expone
  VBAT (BATSNS, canal 7 del AUXADC del MT6323). Receta **idéntica** a la validada por
  userspace, pero por el **regmap del mt6397** (= pwrap con lock):
  - `CON11 (0x0758) bit4` VBUF enable; `CON22 (0x076E) bit7` request canal 7 — se
    re-asegura en cada lectura (aguanta pérdida de estado tras s2idle);
  - poll ready en `ADC0 (0x0714) bit15` (timeout 200 ms), promedio de 8 muestras
    (bits 14:0), `mV = raw*4*1800/32768`.
  - Sysfs: `/sys/class/hwmon/hwmonN/` con `name=mt6323_auxadc`, `in0_input` (mV),
    `in0_label=vbat`. Probe loguea `VBAT xxxx mV` en dmesg.
  - regmap por phandle DT `mediatek,pmic = <&pmic>` → `mt6397_chip->regmap`, patrón
    calcado de `mt6582-afe-pcm.c` (EPROBE_DEFER si el MT6323 aún no está).
  - **hwmon y no power_supply** a propósito: un segundo power_supply tipo battery
    aparecería en UPower y competiría con `test_power` (el que alimenta Phosh).
- **DTS**: nodo raíz `pmic_vbat: pmic-vbat { compatible = "mediatek,mt6323-auxadc";
  mediatek,pmic = <&pmic>; }` (añadido a `mainline/dts/mt6582-bq-krillin.dts`).
- **Kconfig/Makefile**: `SENSORS_MT6323_AUXADC` (depends MFD_MT6397), `=y` en build-krillin.
- **`rootfs/battery` reescrito**: lee `in0_input` del hwmon (localizado por `name`);
  conserva TAL CUAL la compensación I·R (cargando OCV≈VBAT−100 mV / descargando +40 mV),
  la curva OCV piecewise y el **formato de salida** que parsea `battery-upower`
  (`VBAT: X mV (~Y%)`). Si no hay hwmon → stderr + exit 1 (SIN fallback silencioso a
  pwrap: no reintroducir el wedge por accidente). El script viejo queda como
  `/usr/local/bin/battery-pwrap` SOLO para diagnóstico manual.
- `battery-upower`, `charge-status` (i2c, no toca pwrap) y `zzv-battery.start`: sin cambios.

## Deploy

1. En la Pi: copiar `mt6323-auxadc.c` + `deploy-vbat-hwmon.sh` (base64/tar, nunca heredoc)
   y `sh deploy-vbat-hwmon.sh` (idempotente: fuente + Kconfig + Makefile + DTS + config).
2. `~/build-audio.sh` → empaquetar (`mtk_hdr` + `abootimg`) → flashear (`wifi-iter-w.sh`
   si el móvil está en userspace; fastboot si bootloop).
3. Script al móvil (desde la Pi):
   ```sh
   base64 < battery | ssh root@172.16.42.1 'base64 -d > /tmp/battery.new && chmod +x /tmp/battery.new \
     && cp /usr/local/bin/battery /usr/local/bin/battery-pwrap \
     && mv /tmp/battery.new /usr/local/bin/battery'
   ssh root@172.16.42.1 'pkill -f battery-upower; sh /etc/local.d/zzv-battery.start'
   ```

## Validación

1. `dmesg | grep mt6323-auxadc` → `VBAT xxxx mV` en el probe (3400–4350 plausible).
2. `cat /sys/class/hwmon/hwmon*/in0_input` estable y coherente; una única comparación
   manual con `battery-pwrap` (aceptable como diagnóstico puntual, NO periódico).
3. `/usr/local/bin/battery` → línea con % ; `upower -d` / icono de Phosh siguen vivos
   (`/sys/module/test_power/parameters/battery_voltage` se actualiza cada ~15 s).
4. **Anti-wedge** (el criterio de éxito real): tras horas de uso, las pulsaciones físicas
   del botón siguen incrementando el contador eint 25 / mt6397 en `/proc/interrupts`
   y el botón v6 responde. Antes, con pwrap_poke periódico, el bloque de INT del PMIC
   acababa congelándose (ver [[krillin-boot-power-userspace]]).
