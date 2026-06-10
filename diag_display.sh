#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
echo "=== framebuffers ==="
ls -la /dev/fb* 2>/dev/null; cat /proc/fb 2>/dev/null
echo "=== fbcon en dmesg ==="
dmesg | grep -iE "frame buffer|fbcon|switching to colour|mtkfb|DISP" | head -12
echo "=== vtconsole (fbcon bound?) ==="
for v in /sys/class/vtconsole/vtcon*; do echo "$v: $(cat $v/name) bind=$(cat $v/bind)"; done
echo "=== resolucion fb0 ==="
cat /sys/class/graphics/fb0/virtual_size 2>/dev/null
cat /sys/class/graphics/fb0/bits_per_pixel 2>/dev/null
echo "=== getty en tty1? ==="
ps | grep -E "getty" | grep -v grep
echo "=== test: ruido a fb0 (mira la pantalla!) ==="
echo 147147 | sudo -S sh -c "cat /dev/urandom > /dev/fb0" 2>&1 | tail -1
echo HECHO_RUIDO
'
