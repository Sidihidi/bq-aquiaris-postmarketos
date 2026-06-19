#!/bin/bash
set -e
export PATH=~/.local/bin:$PATH
echo "=== zap (conservando paquetes compilados) ==="
yes | pmbootstrap zap 2>&1 | tail -2
echo "=== config actual ==="
pmbootstrap config systemd
echo "=== install desde cero con OpenRC + openssh ==="
pmbootstrap install --no-fde --add openssh-server --password 147147 2>&1 | tail -6
echo "=== extraer particion raiz ==="
IMG=$HOME/.local/var/pmbootstrap/chroot_native/home/pmos/rootfs/bq-krillin.img
/sbin/fdisk -l "$IMG" 2>/dev/null | grep -E 'Linux root|EFI'
START=$(/sbin/fdisk -l "$IMG" 2>/dev/null | grep 'Linux root' | awk '{print $2}')
COUNT=$(/sbin/fdisk -l "$IMG" 2>/dev/null | grep 'Linux root' | awk '{print $4}')
echo "start=$START count=$COUNT"
dd if="$IMG" of=/tmp/pmos-root.img bs=512 skip=$START count=$COUNT status=none
ls -la /tmp/pmos-root.img
echo "=== ZAP_REGEN_COMPLETO ==="
