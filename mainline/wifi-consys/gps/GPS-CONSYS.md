# GPS por el CONSYS (MT6582) — /dev/stpgps → gpsd → Phosh

> Estado 2026-06-19: **Stage 1 (kernel) DESPLEGADO EN HARDWARE** (#79: /dev/stpgps vivo,
> func_on(GPS)=RADIO ENCENDIDO, BT intacto). **Stage 3+4 (userspace) DESPLEGADO Y VALIDADO**: bridge +
> gpsd + autostart corriendo en el teléfono; cadena gpsd→cliente probada con NMEA enlatada (TPV/fix
> real). **Solo falta el dato que vive en Android: el `START_SEQ` de `mnld`** (Stage 2). Y el último
> cable geoclue→Phosh.

## Estado en HARDWARE (2026-06-19) — lo que YA corre en el teléfono
- **Kernel #79** flasheado desde la pmOS viva (dd al sector 83968; ver nota de flasheo abajo). Tras
  boot: `/dev/stpgps` (char 10,259), dmesg `func_on[GPS]: *** RADIO ENCENDIDO ***`, BT (hci0) intacto.
- **gpsd 3.27.3 + gpsd-clients + socat** instalados (`apk`, internet por NAT de la Pi).
- **Bridge desplegado** en `/usr/local/bin/mtk-gps-bridge` — OJO: compilar **`-static`** (la Pi tiene
  `arm-linux-gnueabihf-gcc` = glibc, pero Alpine es **musl** → un binario dinámico da "not found" por
  `/lib/ld-linux-armhf.so.3`; el estático corre en musl). Abre /dev/stpgps + crea `/dev/gps0` (pty).
- **Autostart** `/etc/local.d/zzz-gps.start` (corre TRAS `zz-consys-bt.start`, sin carrera de bring-up):
  lanza el bridge, espera a `/dev/gps0`, y `gpsd -N -n /dev/gps0`. Verificado: bridge+gpsd vivos, gpsd
  sirve /dev/gps0.
- **Cadena gpsd→cliente VALIDADA** con NMEA enlatada: `gpspipe -w` devolvió `TPV mode:3
  lat=53.3613 lon=-6.5056 alt=61.7` + `SKY uSat:8`. Todo el receptor funciona; falta el emisor (NMEA real).
- **Flasheo desde pmOS:** la partición boot REAL es **sector 83968 (0x2900000)**, NO el 0x1D80000 de
  `flash_boot_dd*.sh` (a ceros). `dd ... seek=83968 conv=fsync` + verificación md5 + rollback `/tmp/b.img`.

## GUI EN PHOSH — geoclue (GPS) + Bluetooth: CABLEADO Y VALIDADO (2026-06-19)
**GPS → geoclue → Phosh/Maps (VALIDADO end-to-end con NMEA enlatada):**
- geoclue ya instalado; su fuente **network-nmea** lee NMEA de un **unix socket** (sin avahi):
  `geoclue.conf [network-nmea] enable=true` + `nmea-socket=/var/run/gps-share.sock`.
- El feed: `socat UNIX-LISTEN:/var/run/gps-share.sock,fork,mode=0666 EXEC:"gpspipe -r"` (NMEA crudo de
  gpsd). Añadido al autostart `zzz-gps.start`. (El banner JSON de gpsd lo ignora geoclue, parsea el NMEA.)
- **PRUEBA:** con NMEA enlatada, `/usr/libexec/geoclue-2.0/demos/where-am-i` (cliente D-Bus, = lo que usan
  Phosh/GNOME Maps) imprimió `Latitude 53.361337 / Longitude -6.505620 / Accuracy 1 m`. geoclue logueó
  `GClueNMEASource now active` → `New location available`. **Toda la cadena GPS→geoclue funciona.**
  (Aviso cosmético: el NMEA da timestamp "en el futuro" porque el RTC está en 1970 — no afecta lat/lon.)

**Bluetooth → Phosh:** Phosh ya está compilado con soporte BT (enlaza `libgnome-bluetooth-3.0`, símbolo
`PhoshBt`). Instalado `gnome-bluetooth` 47.2 (+GIR `GnomeBluetooth-3.0.typelib` +udev). bluetoothd
(BlueZ 5.86) up, controlador `00:00:46:65:82:01` Powered → el **toggle de Bluetooth sale en Phosh** tras
recargar la sesión. (Emparejar: gnome-control-center → Bluetooth.)

**Reboot-persistente:** tras reiniciar, arrancan solos bridge+gpsd+gps-share (zzz-gps.start), BT up,
geoclue configurado. **Solo falta el `START_SEQ` real (Stage 2) para que fluya NMEA de verdad** y la
posición salga en GNOME Maps/Phosh sin tocar nada más.

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
