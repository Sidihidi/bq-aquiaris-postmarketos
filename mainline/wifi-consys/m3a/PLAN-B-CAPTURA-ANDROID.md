# PLAN B — Capturar la secuencia CONSYS real de Android (receta)

Objetivo: ver los **bytes STP/BTIF exactos** que el kernel stock (3.10, que SÍ funciona) intercambia
con el CONSYS al arrancar — sobre todo el **handshake que "abre" el canal WMT** (lo que nos falta:
con type=4 el MCU no engancha; eso es firmware, no está en el source). Es la fuente definitiva.

## Mecanismo (investigado)
- Android arranca el CONSYS con userspace **`wmt_loader` + `6620_launcher`** (stock `/system/bin/`,
  daemon WMT genérico MTK). El launcher abre `/dev/stpwmt` y hace ioctls:
  `WMT_IOCTL_SET_PATCH_NAME` (4), `WMT_IOCTL_SET_STP_MODE` (5), `WMT_IOCTL_FUNC_ONOFF_CTRL` (6)
  (`WMT_IOC_MAGIC=0xa0`). → el **kernel** (wmt_lib/wmt_ic_soc) hace la **init del chip** (handshake
  STP/BTIF + **descarga del patch** `mt6572_82_patch`) **al arrancar el launcher**, ANTES de tocar
  ninguna radio. (Encender WiFi/BT luego = `func_on(type)` por el char dev `/dev/wmtWifi` etc.)
- Cada paquete TX/RX se registra en el **ring `stp_dbg`** (`stp_dbg.c:stp_dbg_dump_data`,
  `STP_DBG_LOG_ENTRY_NUM` entradas, marca dir Tx/Rx). El chip-id se lee con GEN_HCR (reg-read),
  luego QUERY/SET baud, RESET, patch_dwn, WIFI_RAM_CODE...

## Pasos de ejecución
0. **Estado**: stock `boot.img` + `system.img` (sparse) en Pi `~/wifi-fw/` y Mac
   `~/Downloads/1.5.2_krillin/`. `system.raw` (1GB) en Pi, montable en `/mnt/stocksys`.
   **REVERSIBLE**: el rootfs pmOS está en p7 (no se toca); restaurar = `fastboot flash boot
   boot-color1.img`. El stock system va a `android`/p5 (no a p7).

1. **Mecanismo de captura: CONFIRMADO = dmesg.** `stp_dbg_dump_data` usa **`printk`** (vuelca los
   bytes Tx/Rx al kernel log) y los `STP_DBG_INFO_FUNC`/`WMT_INFO_FUNC` también printean (gated por
   `gStpDbgDbgLevel` / `gWmtDbgLvl`). → **subir el nivel de debug + leer `dmesg`/last_kmsg.**
   - El log por-paquete va a un ring (`stp_dbg_log_pkt`); el VOLCADO a dmesg lo hace `stp_dbg_dump_data`
     (en coredump o por trigger). Pero la init del chip (chip-id, baud, reset, patch) ya emite INFO.
   - Subir nivel: `gStpDbgDbgLevel`/`gWmtDbgLvl` por su proc (`/proc/driver/wmt_dbg` o similar) o
     module param. Con nivel alto → cada paso de la init del WMT/STP printea a kmsg.
   - **Ruta más simple sin modificar nada**: si el stock tiene `last_kmsg`/pstore, bootear stock →
     dejar que el launcher haga la init (printk) → bootear pmOS → leer `/proc/last_kmsg` o el ramoops.
     (Confirmar que el stock kernel tiene pstore/last_kmsg; si no, usar el volcado a /cache.)

2. **Modificar para auto-capturar (idea del usuario)** — dos sub-opciones:
   - **(a) Modificar el RAMDISK del boot.img** (más fácil para meter un servicio init; init.rc vive
     ahí, no en /system). El ramdisk stock está en formato MTK (header + comprimido; desempaquetar
     con `abootimg`/el header MTK + gunzip/lz4). Añadir un servicio que tras el boot haga:
     `dmesg > /cache/wmt.log` (o el dump del stp_dbg) `; sync`.
   - **(b) Modificar system.img** (lo elegido): montar rw, pero en KitKat init NO importa init.rc de
     /system fácil → hace falta un hook (p.ej. pisar un binario/script que init ya ejecute, o
     `install-recovery.sh`). Más enrevesado que (a). **Recomendado: combinar — ramdisk para el
     servicio + system para los binarios del launcher (que ya están en stock).**
   - **Destino del volcado**: partición **`cache`/p6** (Android la monta rw; NO es p7/pmOS).
     Luego bootear pmOS y `mount /dev/mmcblk0p6 /mnt && cat /mnt/wmt.log`.

3. **Flashear**: `fastboot flash boot <boot-stock-o-modificado>` + `fastboot flash system
   <system-stock-o-modificado>` (fastboot traga sparse). Bootear. Esperar a que el launcher haga
   la init (unos segundos). El volcado queda en /cache.

4. **Leer la captura**: `fastboot flash boot boot-color1.img` (vuelve pmOS) → montar p6 → leer el
   log = **la secuencia STP/BTIF exacta** (resync, GEN_HCR, baud, RESET, patch...) + las RESPUESTAS
   del CONSYS. Eso nos da el handshake "abrir canal WMT" que nos falta.

5. **Restaurar pmOS** (ya hecho en 4 con boot-color1; el rootfs p7 está intacto).

## Qué buscar en la captura (la respuesta)
- El **primer byte/frame** que el kernel manda tras el power (¿resync? ¿un enable de STP?) y el
  **type** del canal (¿0, 4, otro?) en los frames WMT que SÍ obtienen respuesta.
- La **secuencia de "enable STP"** / cómo se abre el canal WMT en el MCU (lo que falta).
- Las **respuestas del CONSYS** (los EVT) → confirman que el RX-DMA nuestro debería captarlas.
- El **timing** (esperas entre pasos).

## Mutualizar
El patrón (cargar FW a un coprocesador + handshake) sirve de plantilla para otros bloques con
firmware del MT6582. El otro grande es el **módem (MD)**, pero ese es Halium-only (fuera de mainline).
El CONSYS desbloquea WiFi+BT+GPS+FM de golpe (ver [[HITO-WIFI-CONSYS]]).
