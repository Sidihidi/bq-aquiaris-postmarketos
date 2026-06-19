# GPS por el CONSYS (MT6582) — /dev/stpgps → gpsd → Phosh

> Estado 2026-06-19: **Stage 1 (kernel) HECHO y compila limpio** (objeto + zImage, linux-7.0.12,
> build-krillin). Falta capturar la secuencia de arranque de `mnld` desde Android (Stage 2) y
> escribir el daemon de userspace (Stage 3) + gpsd/geoclue (Stage 4).

## El descubrimiento (por qué esto es fácil en el kernel y el trabajo está en userspace)

El char dev del GPS del downstream (`stp_chrdev_gps.c`) es un **tubo crudo del canal STP 2**:

- `GPS_write()` → `mtk_wcn_stp_send_data(buf, count, GPS_TASK_INDX=2)` (sin tocar los bytes).
- `GPS_read()` → bloquea en `wait_event(GPS_wq, flag)` y devuelve `mtk_wcn_stp_receive_data(..., 2)`.
- `GPS_open()` → `mtk_wcn_wmt_func_on(WMTDRV_TYPE_GPS=2)`.

**No hay `$PMTK` ni lógica de GPS en el kernel.** Todo el protocolo del MT3332 (arranque del DSP,
calibración de reloj, "start positioning", AGPS) vive en el daemon de Android **`mnld`** (userspace).
El kernel solo entrega/recoge el payload STP ya desensamblado.

**Consecuencia clave:** nuestro `/dev/stpgps` tiene el **mismo contrato byte-a-byte** que el del
downstream. Una secuencia capturada de `mnld` en Android se **reproduce tal cual** sobre mainline.

## Stage 1 — kernel: `/dev/stpgps` (HECHO)

En `mt6582-btif.c` (el driver que ya hace BTIF-DMA + STP + hci0), añadido:

- `#define STP_TYPE_GPS 2` + un **kfifo** (16 KB) + waitqueue `gps_wq`.
- **func_on(GPS)** en `bring_up_chip()`, justo tras func_on(BT) (síncrono, **antes** del kthread RX,
  para no competir por `rxbuf`). El streaming NMEA NO arranca aquí; lo dispara userspace.
- El **kthread RX** rutea las tramas `type==2` al kfifo y hace `wake_up(gps_wq)`.
- **miscdevice `/dev/stpgps`**:
  - `open` → si el chip no está levantado, dispara el bring-up completo (patch + func_on BT/GPS +
    hci0); resetea el kfifo.
  - `read` → bloqueante (o `O_NONBLOCK`), `kfifo_to_user` cuando hay NMEA.
  - `write` → `stp_send(STP_TYPE_GPS, ...)` bajo `tx_lock` (comparte el pipe con el BT).
  - `poll` → `EPOLLIN` cuando hay datos.

Compila sin warnings con la config real (`O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-`).
Driver = `mainline/wifi-consys/m3a/mt6582-btif.c`. Backup del previo: `*.bak-pre-gps` en el Pi.

### LNA del GPS — un problema menos
`wmt_plat_gps_lna_ctrl` está bajo `#ifdef GPIO_GPS_LNA_PIN`, **no definido** en la config de este SoC
→ no hay LNA externo que conmutar (antena interna). No necesitamos GPIO de LNA.

## Stage 2 — CAPTURAR la secuencia de arranque de `mnld` (Android + adb root) ★ CRÍTICO

Es el método Plan B que destrabó el WiFi: arrancar Android, espiar el tráfico real, reproducirlo.

1. **Arrancar Android con adb-root**: `fastboot flash boot boot-stockadb2.img` + reboot. `adb root`.
2. **strace disponible?** `adb shell which strace toybox`. Si no:
   subir un `strace` estático armv7: `adb push strace-armv7 /data/local/tmp/strace && adb shell chmod 755 /data/local/tmp/strace`.
3. **Espiar mnld** (ya corre lanzado por init):
   ```
   adb shell 'PID=$(pidof mnld); echo mnld=$PID; \
     strace -f -tt -y -x -s 2048 -e trace=read,write,openat,ioctl -p $PID -o /data/local/tmp/mnld.strace' &
   ```
   `-y` imprime la ruta de cada fd (para localizar el de `/dev/stpgps`), `-x` vuelca los bytes en hex.
4. **Disparar un fix** (para que mnld haga el "start"): abrir una app GPS (**GPS Test**, **GPS Status**,
   o Google Maps) y dejar el teléfono junto a una ventana ~60–90 s. Alternativa por shell:
   `adb shell dumpsys location` / `cmd location ...` o el menu de prueba `*#*#3646633#*#*` (EngineerMode → Location).
5. **Parar y recoger**: matar el strace, `adb pull /data/local/tmp/mnld.strace`.
6. **Extraer** del log:
   - Los `write(<fd de /dev/stpgps>, "\x..\x..", N)` en orden = **la secuencia a reproducir** (init del
     DSP + clock + start). El primer write tras `openat(...stpgps...)` es el arranque.
   - Los `read(<fd>, "\x..", N)` = lo que el GPS devuelve. **Confirmar que aparece NMEA** (`$GPGGA`,
     `$GPGSV`, `$GPRMC`, `$GNGGA`...). Si el read trae binario MNL + NMEA mezclado, anotar el framing.

   Si `mnld` no aparece (build user sin él activo): arrancarlo a mano bajo strace:
   `adb shell 'stop mnld 2>/dev/null; /data/local/tmp/strace -f -tt -y -x -s 2048 -o /data/local/tmp/mnld.strace /system/bin/mnld'`.

**Sin strace (fallback):** ftrace de `mtk_wcn_stp_send_data`/`mtk_wcn_stp_receive_data` da timing pero
no los bytes; mejor un LD_PRELOAD que envuelva `write/read` y filtre el fd de `/dev/stpgps`.

Guardar la captura en el repo: `mainline/wifi-consys/gps/captura/mnld-stpgps.strace` (+ el hex decodificado).

## Stage 3 — daemon `mtk-gps-bridge` (userspace, reemplaza a mnld)

Tiny daemon (C o Python) que:
1. `open("/dev/stpgps", O_RDWR)`  → dispara bring-up + func_on(GPS) en el kernel.
2. **escribe la secuencia capturada** (init DSP + clock + "start positioning").
3. bucle: `read(/dev/stpgps)` → si es NMEA, lo **vuelca a un PTY**; `write` periódico de "keep-alive"
   si la captura lo muestra.
4. expone el PTY como `/dev/gps0` (con `socat PTY,link=/dev/gps0` o creando el pty en el propio daemon).

Esqueleto:
```c
int fd = open("/dev/stpgps", O_RDWR);
write(fd, START_SEQ, sizeof START_SEQ);     // <- bytes de la captura
for (;;) { n = read(fd, buf, sizeof buf); write(pty, buf, n); }  // NMEA -> gpsd
```
Riesgo a vigilar en la captura: campos **dinámicos** (timestamps, datos AGPS, checksums sobre datos
variables). El "start" del MT3332 suele ser config estática → replay literal funciona; si hay AGPS,
saltarlo (fix más lento, pero funciona en frío).

## Stage 4 — gpsd → prueba → geoclue → Phosh

1. **Prueba cruda** (antes de la GUI): `apk add gpsd gpsd-clients`; `gpsd -N -n /dev/gps0`;
   `cgps` o `gpsmon` → si muestra satélites/fix, **el GPS funciona**. Este es el hito "hacerlo funcionar".
2. **Phosh/GNOME**: `apk add geoclue`. geoclue es la fuente de localización de GNOME Maps / Pure Maps.
   El puente **gpsd→geoclue** es el detalle de integración a cerrar una vez fluya NMEA (geoclue no usa
   gpsd por defecto; opciones: source NMEA por socket, o Pure Maps que habla gpsd directo). Recomendado:
   primero `cgps` (Stage 4.1), luego decidir el puente para Phosh.
3. **Autostart**: servicio OpenRC `gps-bridge.start` en `/etc/local.d/` (abre /dev/stpgps + lanza el
   bridge + gpsd), análogo al `zz-consys-bt.start` del Bluetooth.

## Orden de trabajo en la próxima sesión con el teléfono
1. Flashear el zImage nuevo (ya construido) → `ls /dev/stpgps` debe existir tras el bring-up del BT.
2. Capturar `mnld` en Android (Stage 2) → obtener `START_SEQ` + confirmar NMEA.
3. Escribir el bridge con esos bytes → `cgps` con fix.
4. Conectar a Phosh por geoclue.
