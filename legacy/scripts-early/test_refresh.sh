#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "--- 1. llenar fb de ruido ---"
S "cat /dev/urandom > /dev/fb0"
echo "--- 2. trigger PUT_VSCREENINFO (fbset) ---"
S "fbset -depth 32"
sleep 2
echo "--- 3. pan por sysfs ---"
ls /sys/class/graphics/fb0/
S "echo 0,960 > /sys/class/graphics/fb0/pan" && echo PAN_960_OK
sleep 2
S "echo 0,0 > /sys/class/graphics/fb0/pan" && echo PAN_0_OK
echo "--- 4. blank/unblank ---"
S "echo 1 > /sys/class/graphics/fb0/blank"; sleep 1
S "echo 0 > /sys/class/graphics/fb0/blank" && echo UNBLANK_OK
echo "--- FIN: dime que viste en pantalla ---"
'
