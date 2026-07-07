# launch_phosh.sh — lanza Phosh (Wayland) con SUPERVISION.
# v2: antes era un 'exec' one-shot: si phoc moria al arrancar (carrera DRM/seatd) NO habia GUI
# y nadie reintentaba (uno de los dos sintomas del boot inestable). Ahora: si la sesion muere
# antes de 60s se cuenta como fallo de arranque (max 5 seguidos); si duro >60s (crash tardio)
# el contador se resetea y la GUI SIEMPRE vuelve. Log: /tmp/phosh-boot.log (via zzzz-phosh.start).
export XDG_RUNTIME_DIR=/run/user/1000
export WLR_RENDERER=gles2 LIBSEAT_BACKEND=seatd
export XDG_DATA_DIRS=/usr/local/share:/usr/share
export PATH=/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin
n=0
while :; do
  t0=$(cut -d. -f1 /proc/uptime)
  # Limpiar sockets dbus huerfanos antes de (re)lanzar phoc. Cada dbus-run-session
  # crea un socket /tmp/dbus-*; si la sesion muere y se relanza, el socket viejo
  # queda huerfanos. Visto en HW: 328 acumulados tras dias de crashes/reboots del
  # WiFi -> phosh_env() del mt6582-powerkey los escanea y la GUI degrada. Solo
  # borramos los que NINGUN proceso tiene abierto como fd (los vivos se quedan).
  find /tmp -maxdepth 1 -name 'dbus-*' -type s 2>/dev/null | while read s; do
    alive=$(find /proc/[0-9]*/fd -lname "$s" 2>/dev/null | head -1)
    [ -z "$alive" ] && rm -f "$s"
  done
  echo "[launch_phosh] lanzando phoc (intento tras $n fallos, uptime ${t0}s)"
  dbus-run-session phoc -E /usr/local/bin/phosh-session.sh
  rc=$?
  t1=$(cut -d. -f1 /proc/uptime)
  if [ $((t1 - t0)) -ge 60 ]; then
    n=0        # sesion larga que murio -> relanzar siempre (la GUI vuelve sola)
  else
    n=$((n + 1))   # murio al arrancar -> fallo de boot
  fi
  echo "[launch_phosh] phoc salio rc=$rc tras $((t1 - t0))s (fallos seguidos: $n)"
  [ $n -ge 5 ] && break
  sleep 3
done
echo "[launch_phosh] 5 fallos de arranque seguidos; me rindo"
