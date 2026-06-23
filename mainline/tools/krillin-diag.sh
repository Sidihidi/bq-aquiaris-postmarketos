#!/bin/bash
# krillin-diag.sh — diagnostico completo del telefono (pmOS mainline). Ejecutar EN LA PI.
# Reconecta usb0 + reporta: boot, GUI, CONSYS (BT/WiFi/GPS), memoria, errores dmesg, pstore.
# Uso: bash krillin-diag.sh
PH=root@172.16.42.1
O="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=4"
echo "=== krillin-diag $(date +%H:%M:%S) ==="
UP=0
for i in $(seq 1 20); do
  sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
  ssh -n $O $PH true 2>/dev/null && { UP=1; break; }
  sleep 3
done
if [ $UP = 0 ]; then
  echo "[!] SIN SSH tras ~60s."
  ping -c1 -W2 172.16.42.1 >/dev/null 2>&1 \
    && echo "    (ping OK -> arrancando o init atascado; espera o mira la pantalla)" \
    || echo "    (sin ping -> apagado/colgado -> usa: krillin-recover.sh forcereboot)"
  exit 1
fi
ssh -n $O $PH '
  echo "--- BOOT ---"
  echo "uptime: $(cut -d. -f1 /proc/uptime)s   kernel: $(uname -r)   root: $(findmnt -no SOURCE / 2>/dev/null)"
  echo "--- GUI / SSH ---"
  echo "sshd=$(pidof sshd|wc -w)  phoc=$(pidof phoc|wc -w)  phosh=$(pidof phosh|wc -w)   (pidof, NO pgrep -x)"
  [ "$(pidof phoc|wc -w)" = 0 ] && echo "  [!] GUI no arriba -> tail /tmp/phosh-boot.log"
  echo "--- CONSYS (BT / WiFi / GPS) ---"
  echo "hci0  : $(ls /sys/class/bluetooth/ 2>/dev/null | tr "\n" " ")"
  echo "wlan0 : $([ -d /sys/class/net/wlan0 ] && echo presente || echo ausente)"
  echo "gps   : stpgps=$([ -e /dev/stpgps ] && echo ok || echo no)  gps0=$([ -e /dev/gps0 ] && echo ok || echo no)"
  echo "bring-up CONSYS: $(dmesg 2>/dev/null | grep -c "BRING-UP del CONSYS") vez/veces (1=ok, 2+=carrera)"
  echo "--- MEMORIA ---"
  free -m | awk "/Mem:/{print \"  RAM: \"\$3\"/\"\$2\" MB usado\"}"
  echo "--- ERRORES dmesg ---"
  ERR=$(dmesg 2>/dev/null | grep -iE "Kernel panic|Oops:|BUG:|Call Trace|segfault|Out of memory|I/O error|EXT4-fs error|task.*blocked|soft lockup" | grep -vi "command line" | sed -E "s/^\[[0-9.]+\] //" | tail -6)
  [ -n "$ERR" ] && echo "$ERR" || echo "  (sin errores graves)"
  echo "--- PSTORE (crash del boot anterior) ---"
  if ls /sys/fs/pstore/ 2>/dev/null | grep -qi "dmesg"; then echo "  [!] crash previo (dmesg-ramoops) -> krillin-logs.sh"; else echo "  sin crash (solo console-ramoops = normal)"; fi
'
echo "=== fin ==="
