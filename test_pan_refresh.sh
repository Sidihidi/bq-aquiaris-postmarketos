#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== escribir algo nuevo en tty1 y kick ==="
S "echo \"*** HOLA JUAN - $(date) ***\" > /dev/tty1"
S "echo 1 > /sys/class/graphics/fb0/blank; sleep 1; echo 0 > /sys/class/graphics/fb0/blank"
echo KICK1_HECHO
sleep 3
echo "=== ahora escribir otra linea y solo PAN (sin blank) ==="
S "echo \"*** SEGUNDA LINEA - solo pan ***\" > /dev/tty1"
S "echo 0,0 > /sys/class/graphics/fb0/pan"
echo PAN_HECHO
'
