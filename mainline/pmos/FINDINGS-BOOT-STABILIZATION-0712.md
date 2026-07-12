# Estabilización del boot (pmOS) — 0712: raíz del "boot parcial" encontrada y arreglada

> El boot se había desestabilizado (sshd flaky, arranques parciales, sensores/red caídos). Raíz
> encontrada y arreglada + verificada en vivo. La config persiste en `/etc/runlevels/` (durable en
> esta instalación) y se reaplica con `setup-boot-runlevels.sh`.

## Raíz: al runlevel `boot` le faltaban servicios esenciales
`rc-update show` reveló un runlevel `boot` **pelado**: solo `bootmisc haveged hostname networking
seedrng`. Faltaban **`root`, `localmount`, `swap`, `sysctl`, `modules`, `hwclock`**. Y `dbus` no
estaba en ningún runlevel.

Cadena de dependencias que rompía todo:
```
dbus / networkmanager / polkit / bluetooth / chronyd / avahi   (need localmount)
        -> localmount   (need root)
                -> root  <-- NO estaba en ningun runlevel => nunca arrancaba
```
Síntoma diagnóstico exacto: `rc-service dbus start` → `ERROR: localmount needs service(s) root`.
Resultado: sin system D-Bus (→ sin auto-rotación/ALS/proximidad, ver
`../drivers/done/sensors/FINDINGS-SENSOR-PROXY-0712.md`), sin NetworkManager, sin polkit, con clock
skew. El sistema limpiaba a medias (sshd + phosh por caminos propios) = el "arranque parcial".

## Fix (aplicado + verificado en vivo)
1. **Restaurar runlevels** (`setup-boot-runlevels.sh`, idempotente):
   `rc-update add {root,localmount,swap,sysctl,modules,hwclock} boot` + `rc-update add dbus default`.
2. **Reloj**: el RTC mt6323 daba hora basura (2073) → clock skew en cada boot. Se puso a la hora real
   (`date -u -s` + `hwclock -w`) y se añadió `hwclock` a `boot` (lo restaura del RTC). `chronyd`
   (default) lo afina por NTP una vez hay red. Con la batería cargada el RTC mantiene la hora.
3. **Verificación en vivo** (sin reiniciar): tras regenerar el deptree de OpenRC
   (`Caching service dependencies`), `openrc boot` + `openrc default` levantaron TODO limpio:
   `localmount, dbus, bluetooth, networkmanager (RUNNING), chronyd, iio-sensor-proxy, polkit,
   modemmanager` = **started**; socket dbus presente; hora correcta. La cascada sana end-to-end.

> Por qué la prueba en vivo falló al principio y luego funcionó: el **deptree cacheado** del boot
> degradado no incluía los servicios recién añadidos. En un boot LIMPIO el deptree se construye
> DESPUÉS de leer los runlevels → incluye root/localmount/dbus → la cascada arranca bien. Por eso el
> próximo arranque debe venir limpio.

## Pendiente / seguimiento (menor)
- **`local` reporta `[ !! ]`**: los lanzadores de daemons en `/etc/local.d/*.start` usan
  `start-stop-daemon`, que devuelve 1 si el daemon YA corre (falso fallo al re-ejecutar). En boot
  limpio la mayoría arranca a 0. ⚠️ **Bug latente real**: `zzv-battery.start` usa
  `start-stop-daemon --exec /usr/bin/python3` → matchea CUALQUIER python3; con varios daemons python
  puede fallar/no-arrancar en boot real. Fix: matchear por pidfile o por el script concreto, y/o
  `--oknodo`. (No bloquea: los daemons que pueden arrancar, arrancan.)
- **`sysctl`**: claves `net.ipv4.tcp_syncookies` / `kernel.unprivileged_bpf_disabled` desconocidas
  para nuestro kernel minimal → warnings inocuos; limpiar de sysctl.conf si molesta.
- **`sshd` "already running"**: OpenRC intenta arrancar un sshd ya vivo (supervise-daemon) = no es
  fallo real.
- **Verificar en reboot real** cuando la batería esté segura (config correcta + verificada en vivo,
  pero el boot fresco es la prueba definitiva).

## ✅ VERIFICADO EN REBOOT REAL (0712)
Reboot limpio → **boot LIMPIO**: uptime 58s, **hora correcta sin skew** (fix hwclock OK),
`rc.log` sin un solo ERROR/cascada/segfault, y `localmount, dbus, networkmanager (RUNNING),
bluetooth, chronyd, local` = **started solos**. dbus socket, iio-sensor-proxy, carga y phosh vivos.
- **Única fragilidad restante**: `polkit` (need dbus) perdió una **carrera de arranque** contra el
  system bus → falló en OpenRC. Pero polkit es **dbus-activatable** → `polkitd` acaba corriendo
  activado por D-Bus bajo demanda (el claim de sensores de monitor-sensor funciona = accel+luz+prox).
- **Blindaje añadido**: `etc/local.d/zzz-boot-heal.start` — red de seguridad que en 2º plano
  reintenta dbus/polkit chequeando el PROCESO real (`pgrep -x polkitd`, no la contabilidad de OpenRC,
  para no pelearse con la instancia dbus-activada). Cubre esta y futuras carreras contra dbus.

## Para Maemo (el otro rootfs del dual-boot)
Este fix es de OpenRC (pmOS). Maemo tiene su propio init → necesita su propia auditoría. **Lo
compartido que ya mejora en ambos**: el RTC mt6323 quedó puesto a hora real (hardware común), y el
initrd de menupick es el mismo. El clock skew afectaba a los dos.

Ficheros: `mainline/pmos/setup-boot-runlevels.sh`.

*Sesión principal (Fable 5), 2026-07-12.*
