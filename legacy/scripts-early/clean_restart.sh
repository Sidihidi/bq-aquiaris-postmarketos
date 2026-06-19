#!/bin/bash
ssh -o ConnectTimeout=8 -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== matar Xorg zombis y rotar log ==="
S "pkill -9 Xorg; pkill -9 xinit; pkill -9 dwm; pkill -9 dbus-run-session"
sleep 2
S "mv /var/log/Xorg.0.log /var/log/Xorg.0.log.old 2>/dev/null"
echo "X restantes: $(ps | grep -c [X]org)"
echo "=== confirmar que mi conf 99 existe y mira bien ==="
cat /etc/X11/xorg.conf.d/99-touchscreen.conf
echo "=== arrancar tinydm ==="
S "rc-service tinydm start 2>&1 | tail -1"
sleep 10
echo "=== driver del tactil en log NUEVO ==="
grep -E "Using input driver.*mtk-tpd|Applying InputClass.*mtk-tpd" /var/log/Xorg.0.log 2>/dev/null
echo "=== double tracking en log nuevo ==="
grep -c "double tracking" /var/log/Xorg.0.log 2>/dev/null
echo "=== refresco ==="
ps | grep -c fb-refre[s]h
'
