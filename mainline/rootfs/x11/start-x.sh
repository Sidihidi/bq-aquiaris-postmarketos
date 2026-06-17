#!/bin/sh
# Arrancar la GUI X11 en el BQ E4.5. Espera al tactil (event0) y lanza X+jwm+teclado.
for i in $(seq 1 20); do [ -e /dev/input/event0 ] && break; sleep 1; done
pkill -x jwm 2>/dev/null; pkill Xorg 2>/dev/null; sleep 1
export DISPLAY=:0
X :0 vt1 -keeptty -nolisten tcp -config /etc/X11/xorg.conf > /tmp/xorg.log 2>&1 &
sleep 4
xsetroot -solid "#2a4d69" 2>/dev/null
matchbox-keyboard >/tmp/mkb.log 2>&1 &
jwm >/tmp/jwm.log 2>&1 &
echo "X arrancado (DISPLAY=:0)."
