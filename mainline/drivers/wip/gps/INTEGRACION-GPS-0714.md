# Integración del GPS en pmOS (0714) — cadena mnld → gpsd → geoclue/Phosh

> El GPS funciona end-to-end (ver `HANDOFF-CHIPDETECTOR-0714.md` §RESOLUCIÓN FINAL). Esto es el **pulido**
> para que arranque solo al boot y llegue a geoclue/Phosh. Fix real = cielo despejado (interior=0 satélites).

## La cadena
```
DSP (combo MT6582)
  └─ /dev/stpgps  (driver mt6582-btif.c, built-in; STP GPS; frames 0xAAF0)
       └─ mnld  (stock, bajo shim bionic libxlogshim.so)  →  calcula NMEA con libmnl
            └─ /dev/gps  (driver mt6582-gpsdrv.ko, loopback write→read)
                 └─ socat -u /dev/gps PTY,link=/run/gpsnmea   (gpsd exige apertura EXCLUSIVA)
                      └─ gpsd -N -n /run/gpsnmea   (parsea NMEA → JSON/TPV, puerto 2947)
                           └─ /var/run/gps-share.sock  (socat UNIX-LISTEN → gpspipe -r)
                                └─ geoclue (network-nmea)  →  Phosh / apps
```

## Ficheros / pasos (todo automatizado por `etc/local.d/zzz-gps.start`)
1. **Boot script**: `mainline/rootfs/etc/local.d/zzz-gps.start` (en el móvil `/etc/local.d/zzz-gps.start`,
   +x). Reemplaza el viejo (que usaba el bridge `mtkgps_aaf0` superado). Hace los 6 pasos de la cadena.
2. **Módulo gpsdrv persistente**: `mt6582-gpsdrv.ko` en `/lib/modules/$(uname -r)/` del móvil (fuente
   `~/gpsdrv-mod/` en la Pi). El script lo `insmod`-ea al boot (btif=/dev/stpgps es built-in, ya está).
3. **Shim**: `/system/lib/libxlogshim.so` (v3, fuente `gps-bionic-shim-v2.c`). Config: `/data/misc/mnl.prop`
   (`dev.dsp=/dev/stpgps dev.gps=/dev/gps pmtk.conn=serial`).
4. **geoclue.conf** (`/etc/geoclue/geoclue.conf`): `[network-nmea] enable=true nmea-socket=/var/run/gps-share.sock`
   (ya venía) + **`[modem-gps] enable=false`** ⚠️ CAMBIO NECESARIO: si modem-gps está `true` es el proveedor
   EXCLUSIVO y tapa network-nmea (geoclue.conf lín ~115).
5. **Servicio OpenRC gpsd DESACTIVADO** (`rc-update del gpsd`): arrancaba otro gpsd (en /dev/gps0 del bridge
   viejo) que competía por el puerto 2947. Nuestro `zzz-gps.start` gestiona gpsd.

## Verificación (interior, sin fix)
```
lsmod|grep gpsdrv            # cargado
pgrep -f /xbin/mnld          # mnld corriendo
ls /run/gpsnmea              # PTY del relay
pgrep -f sbin/gpsd           # gpsd corriendo
gpspipe -w | grep mode       # "mode":1  (1=sin fix; 2/3=fix, con cielo)
socat -u UNIX-CONNECT:/var/run/gps-share.sock -   # sirve $GPGGA/$GPGSV... a geoclue
```
Con `mode:1` y `$GPGSV,1,1,0` (0 satélites) = correcto en interior. **Para un FIX: exterior + varios
minutos (cold start).** Entonces `mode` pasa a 2/3, la posición se puebla, y geoclue/Phosh la reciben.

## Apps para PROBAR el GPS en Phosh (0714)
No hay app de mapas gráfica en los repos habilitados (solo Alpine main+community; **falta el repo de
pmOS** → no hay gnome-maps/pure-maps). Opciones que SÍ funcionan:
- **`xgps` (Satélites GPS)** — cliente gpsd GRÁFICO (Python GTK3, corre en Wayland sin Xwayland). Muestra
  el cielo con los satélites, barras de señal y el fix/posición. **La mejor para la prueba de exterior**
  (ves los satélites aparecer). Añadido al menú con `/usr/share/applications/xgps.desktop`
  (`Exec=xgps localhost`). Requirió instalar `py3-cairo` + `py3-gpsd` (dan el módulo python `gps`).
- **"Where am I?"** — ya venía (`geoclue-where-am-i.desktop`, `/usr/libexec/geoclue-2.0/demos/where-am-i`).
  Demo de geoclue: abre terminal y muestra la ubicación que geoclue entrega (prueba la cadena hasta las apps).
- **`cgps` / `gpsmon`** (terminal, `gpsd-clients`) — vista de texto de fix/satélites/posición en vivo.
Todas leen del gpsd que monta `zzz-gps.start`. Interior = 0 satélites / sin fix; **exterior = satélites y fix**.

## Pendiente / mejoras (no bloqueante)
- **AGPS/almanaque** para TTFF más rápido (el primer fix en frío tarda; con asistencia = segundos). El
  stock inyectaba efemérides por SUPL (`/data/agps_supl/`, PMTK710) — ver `STOCK-mnld-live.strace`.
- Un applet/indicador de GPS en Phosh (estado fix/satélites).
- Verificación end-to-end de geoclue→Phosh requiere un cliente pidiendo ubicación + fix real (exterior).

*Casa (Opus 4.8), 2026-07-14.*
