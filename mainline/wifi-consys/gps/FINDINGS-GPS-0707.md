# GPS del krillin — diagnóstico (2026-07-07)

## Estado
El GPS **NO emite NMEA todavía**. El radio se enciende (WMT `func_on[GPS]` al abrir `/dev/stpgps`),
pero el DSP de navegación **no arranca** → 0 bytes en `/dev/stpgps`.

## Arquitectura (confirmada)
El GPS del combo MTK habla un **protocolo binario propietario 0xAAF0** (NO NMEA):
frames `AA F0 <LEN> 00 <TYPE> FE <payload> AA 0F` (LEN = payload+2). El bridge
`mtkgps_aaf0.c` convierte esos frames a NMEA para gpsd/geoclue. El kernel expone `/dev/stpgps`
(STP, `GPS_TASK_INDX`); **NO** hay `/sys/class/gpsdrv/gps/*` (eso era downstream; aquí el power va
por el open de `/dev/stpgps`).

Cadena: `/dev/stpgps` → `mtk-gps-bridge` → `/dev/gps0` (pty) → gpsd → socat → geoclue.

## EL BLOQUEO (diagnosticado 0707)
1. **El bridge DESPLEGADO (`/usr/local/bin/mtk-gps-bridge`) es un STUB**: strace muestra que abre
   `/dev/stpgps`, imprime "START_SEQ vacía: solo bombeo", **NO envía el START_BURST** y sale. El bridge
   COMPLETO (`~/gps/mtkgps_aaf0.c` + compilado) nunca se desplegó al móvil.
2. **El bridge completo SÍ envía el START_BURST** (`write(3, "\xaa\xf0\x08...")`) pero el **GPS NO
   responde** (`read(3) = ERESTARTSYS`, bloqueado sin datos). → el burst no arranca el DSP.
3. **Causa**: el **frame de START `t=0x05`** del burst está **adivinado** (`AA F0 06 00 05 FE 00 00 00 00
   AA 0F`) porque la captura antigua (`~/gps/mnld-live.strace` línea 3, `write(13,...)=206`) **truncó
   los últimos ~6 bytes** (el display de strace acaba justo en `AA F0 06 00 05 FE`, sin el payload+footer).
   Los frames se construyen dinámicamente en mnld → NO están literales en el binario (`~/gps/*/mnld`).

## LO QUE FALTA (para rematar)
**Re-capturar el START_BURST completo** del `mnld` de stock con `strace -s 512`:
- Infra lista en `~/android-cap/`: `cap.sh` (adb + logs) y `dev.sh` (instala un **wrapper strace de
  mnld** con `-s 512` en `/system/xbin/mnld` → captura a `/data/local/tmp/mnld.strace`).
- Dual-boot: flashear `~/android-cap/boot.img` (LineageOS) a mmcblk0 seek=83968; restaurar pmOS con
  `~/mainline/pkg/boot-diag.img` (#243).
- Retos: interacción Android a ciegas (solo adb), y **disparar una sesión GPS** para que mnld envíe el
  burst (el burst está en la fase "live", no en el init). Frames periódicos `t=0x05` ya capturados
  enteros: `AA F0 09 00 05 FE 19 00 00 03 00 28 01 AA 0F` y `...34 00 00 00 00 40 01...` (referencia).
- Luego: completar `START_BURST[]` en `mtkgps_aaf0.c`, recompilar (`arm-linux-gnueabihf-gcc -static -lm`),
  desplegar como `/usr/local/bin/mtk-gps-bridge`, verificar frames por strace, y **validar un fix real en
  exterior** (con vista al cielo).

## Reproducir el diagnóstico
`strace -s 200 -x -e read,write /tmp/gpsfull` (bridge completo armv7 en `/tmp/gpsfull`): se ve el
`write(3, burst)` y el `read(3)` bloqueado sin respuesta.
