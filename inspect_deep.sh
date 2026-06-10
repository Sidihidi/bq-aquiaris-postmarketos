#!/bin/bash
IMG=/tmp/pmos-root.img
M=/mnt/pmosroot
sudo mkdir -p $M
sudo mount -o loop $IMG $M || { echo NO_MONTA; exit 1; }
echo "=== TODOS los runlevels ==="
for rl in $M/etc/runlevels/*/; do echo "[$rl]"; ls "$rl"; done
echo "=== /etc/init.d (cuantos servicios) ==="
ls $M/etc/init.d/ 2>/dev/null | wc -l
ls $M/etc/init.d/ 2>/dev/null | grep -iE 'ssh|udev|getty|network|usb'
echo "=== paquetes instalados (apk world) ==="
cat $M/etc/apk/world 2>/dev/null
echo "=== sshd binario en cualquier sitio ==="
sudo find $M -name 'sshd' 2>/dev/null | head
echo "=== lost+found ==="
sudo ls $M/lost+found 2>/dev/null | head
echo "=== tamano usado real ==="
sudo du -sh $M 2>/dev/null
sudo umount $M
echo FIN
