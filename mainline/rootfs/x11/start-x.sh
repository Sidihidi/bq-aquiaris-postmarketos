#!/bin/sh
# Arrancar X desde SSH en el BQ E4.5 (consola en fbcon). Ejecutar EN el telefono.
# X toma /dev/fb0 (simplefb). vt1 + -keeptty para arrancar desde sesion SSH (sin tty propio).
pkill -x jwm 2>/dev/null; pkill Xorg 2>/dev/null; sleep 1
export DISPLAY=:0
X :0 vt1 -keeptty -nolisten tcp -config /etc/X11/xorg.conf > /tmp/xorg.log 2>&1 &
sleep 4
xsetroot -solid "#2a4d69" 2>/dev/null
matchbox-keyboard >/tmp/mkb.log 2>&1 &
jwm >/tmp/jwm.log 2>&1 &
sleep 1
echo "X arrancado (DISPLAY=:0). Logs: /tmp/xorg.log /tmp/jwm.log"
