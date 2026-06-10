#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== InputClass: forzar evdev en el tactil tipo-A ==="
S "cat > /etc/X11/xorg.conf.d/20-touchscreen.conf << \"EOF\"
Section \"InputClass\"
    Identifier \"mtk touchscreen (type A)\"
    MatchProduct \"mtk-tpd\"
    MatchIsTouchscreen \"on\"
    MatchDevicePath \"/dev/input/event*\"
    Driver \"evdev\"
EndSection
EOF"
echo TOUCH_CONF_OK
echo "=== reiniciar sxmo (tinydm) ==="
S "rc-service tinydm restart 2>&1 | tail -1"
sleep 8
echo "=== verificar driver del tactil en el nuevo Xorg.0.log ==="
grep -E "mtk-tpd.*Applying|Using input driver.*mtk-tpd|double tracking" /var/log/Xorg.0.log 2>/dev/null | tail -6
echo "=== reactivar refresco ==="
S "(setsid /usr/local/bin/fb-refresh </dev/null >/dev/null 2>&1 &)"
echo "instancias refresh: $(ps | grep fb-refre[s]h | wc -l)"
echo "=== MIRA Y TOCA LA PANTALLA ==="
'
