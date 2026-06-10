#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== renombrar a 99- (mayor prioridad que 40-libinput) ==="
S "mv /etc/X11/xorg.conf.d/20-touchscreen.conf /etc/X11/xorg.conf.d/99-touchscreen.conf"
S "ls /usr/share/X11/xorg.conf.d/ /etc/X11/xorg.conf.d/ 2>/dev/null | grep -iE \"libinput|touch\""
echo "=== parar todo X y limpiar refresco duplicado ==="
S "rc-service tinydm stop 2>&1 | tail -1"
S "pkill -9 Xorg; pkill -9 xinit; pkill -9 dwm"
for p in $(ps | grep "fb-refre[s]h" | awk "{print \$1}"); do S "kill $p"; done
sleep 3
echo "X vivos: $(ps | grep -c [X]org)  refresh: $(ps | grep -c fb-refre[s]h)"
echo "=== arrancar de nuevo ==="
S "(setsid /usr/local/bin/fb-refresh </dev/null >/dev/null 2>&1 &)"
S "rc-service tinydm start 2>&1 | tail -1"
sleep 10
echo "=== driver del tactil AHORA ==="
grep -E "Using input driver.*mtk-tpd|mtk-tpd.*Applying InputClass" /var/log/Xorg.0.log | tail -5
echo "=== errores libevdev recientes? ==="
grep -c "double tracking" /var/log/Xorg.0.log
echo "=== TOCA LA PANTALLA AHORA ==="
'
