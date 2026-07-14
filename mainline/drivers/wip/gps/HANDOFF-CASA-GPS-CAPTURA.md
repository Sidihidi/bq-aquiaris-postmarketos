# HANDOFF a casa — captura GPS con fix real (VÍA B) — 0714

## Objetivo
Rematar la **vía B** del GPS: el motor bionic (`libmnlp_mt6582`) corre en pmOS y produce sus reportes
en el stream binario **`0xAAF0`**, pero **standalone no serializa el NMEA** y **desde la fuente no se puede
decodificar** (todos los binarios de LineageOS son otra versión que nuestras fuentes). La única vía es
**empírica**: capturar el stream **con un fix REAL** (cielo despejado) y localizar lat/lon en los bytes.
Contexto completo: `RECETA-BIONIC-VIA1-0714.md` (STATUS noche-4d/4e).

## Qué correr (en el MÓVIL, con VISTA AL CIELO)
```sh
ssh root@172.16.42.1        # (o como accedas al móvil desde casa)
sh /data/gps-capture.sh     # ya desplegado y persistente en /data/
```
- Mantén el móvil con **vista al cielo** (ventana orientada al exterior o en el alféizar) unos **minutos**.
- El script muestra feedback en vivo:
  - `report.bin: N bytes, M frames` + inventario por tipo (`0xFE05` status, `0xFE08` reporte).
  - **`actividad 0xFE08: X frames, Y payloads DISTINTOS`** → en interior `Y=1` (dummy); **al adquirir
    satélites `Y` sube** (los reportes cambian). Es el indicador de que está entrando señal.
  - **`>>> NMEA DETECTADO`** → jackpot: si con el fix el motor loguea sentencias `$GP` (vía ALOGD, con
    `debug_nmea=1`), salen aquí directas y **no hay que decodificar nada**.
- **Ctrl-C** para parar cuando lleves unos minutos o tengas fix. Deja los ficheros en `/data/gps_mnl/`:
  - `report.bin` — el stream 0xAAF0 (lo importante).
  - `logd.cap` — log ALOGD (posible NMEA directo).
  - `xlog.txt` — narración del motor.

⚠️ Si sale `ERROR: /dev/stpgps no existe` → el DSP GPS no está listo (falta el bring-up del CONSYS/radio).
   Revisar dmesg / que el servicio de bring-up GPS haya corrido. El prefijo bionic `/system/*` debe existir
   (si no, restaurar `~/gps-bionic-prefix.tar.gz`).

## Qué hacer con los ficheros (análisis)
1. **Si apareció NMEA en el feedback / `logd.cap`** → ¡ya está! Extraer las sentencias:
   ```sh
   strings /data/gps_mnl/logd.cap | grep -aoE '\$G[PN][A-Z]{3}[^*]*\*[0-9A-Fa-f]{2}'
   ```
   → alimentar a gpsd (o cablear al `/dev/gps` / socket) → geoclue → Phosh. FIN.
2. **Si NO hay NMEA pero `report.bin` creció / `payloads distintos` subió** → decodificar el 0xFE08:
   ```sh
   python3 aaf0-parse.py /data/gps_mnl/report.bin
   ```
   El parser tabula tipos y **busca candidatos a `double` lat/lon** en los frames grandes. Con las
   **coordenadas aproximadas conocidas** del sitio (p.ej. lat 40.4°, lon -3.7° para Madrid), localizar el
   `double` (8 bytes IEEE-754) que las contenga dentro del `0xFE08` → ese offset = el campo de posición.
   De ahí mapear el resto (los structs de referencia: `mtk_gps_type.h` `double LLH[4]`, `dfLat/dfLon`).
3. Con el offset de lat/lon confirmado, escribir el decodificador `0xAAF0 → NMEA` ($GPGGA/$GPRMC/$GPGSA)
   y cablearlo a `/dev/gps` → gpsd → geoclue → Phosh.

## Estado de las otras vías (por si acaso)
- **Vía A (mnld)**: bloqueada — el binario `mnld` de LineageOS ≠ la fuente `nu3001`; `launch_daemon_thread`
  no llega al `fork()` por una razón que solo se ve **desensamblando el binario** (Ghidra) o con `strace`
  del mnld de LineageOS en un fix real. El gate del chipid ya está resuelto (área de properties del shim).
- **Recuperación Pi**: `~/mainline/linux-7.0.12` INTACTO (verificado). `~/mainline/{downstream,pkg}`
  borrados → `downstream` se re-clona de `github.com/bq/aquaris-E4.5`; `pkg` se regenera.

## Ficheros (en el repo)
- `gps-capture.sh` — el capturador (copia en `/data/gps-capture.sh` del móvil).
- `aaf0-parse.py` — el parser/analizador del stream.
- `gps-bionic-shim.c` — el shim (xlog + property-area del chipid).
- `mt6582-gpsdrv.c` — el driver `/dev/gps` (loadable).
