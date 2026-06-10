#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== A. protocolo MT del tactil (A vs B) ==="
S "timeout 1 evtest /dev/input/event3 2>/dev/null | grep -E \"ABS_MT_SLOT|SYN_MT_REPORT\"" | sort -u
echo "(SYN_MT_REPORT = tipo A legacy / ABS_MT_SLOT = tipo B moderno)"
echo "=== B. que driver de input cargo Xorg para el tactil ==="
grep -iE "mtk-tpd|input|libinput|evdev|Using input driver" /var/log/Xorg.0.log 2>/dev/null | head -15
echo "=== C. tiene libinput instalado? (no soporta tipo A) ==="
S "apk info -e xf86-input-libinput" && echo LIBINPUT_PRESENTE || echo SIN_LIBINPUT
S "apk info -e xf86-input-evdev" && echo EVDEV_PRESENTE || echo SIN_EVDEV
echo "=== D. buffers del fb (por que imagen estatica) ==="
cat /sys/class/graphics/fb0/virtual_size
cat /sys/class/graphics/fb0/pan 2>/dev/null
echo "=== E. errores de Xorg ==="
grep -iE "\(EE\)|\(WW\).*fb|no devices detected" /var/log/Xorg.0.log 2>/dev/null | head -10
'
