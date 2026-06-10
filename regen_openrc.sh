#!/bin/bash
set -e
export PATH=~/.local/bin:$PATH
echo "=== cambiando a OpenRC (systemd never) ==="
pmbootstrap config systemd never
pmbootstrap config systemd
echo "=== install OpenRC + openssh ==="
pmbootstrap install --no-fde --add openssh-server 2>&1 | tail -5
echo "=== verificando contenido ANTES de tocar el telefono ==="
IMG=~/.local/var/pmbootstrap/chroot_native/home/pmos/rootfs/bq-krillin.img
# leer offset de la particion root (p2) dinamicamente
read START SIZE < <(/sbin/fdisk -l "$IMG" 2>/dev/null | awk '/Linux root/{print $2, $4}')
echo "root part: start=$START sectores=$SIZE"
dd if="$IMG" of=/tmp/pmos-root.img bs=512 skip=$START count=$SIZE status=none
M=/mnt/pmoschk; sudo mkdir -p $M; sudo mount -o loop /tmp/pmos-root.img $M
echo "--- init system ---"; ls $M/sbin/init $M/lib/systemd/systemd 2>/dev/null
echo "--- sshd? ---"; ls -la $M/usr/sbin/sshd 2>/dev/null || echo SSHD_FALTA
echo "--- runlevels default ---"; ls $M/etc/runlevels/default/ 2>/dev/null
echo "--- runlevels boot ---"; ls $M/etc/runlevels/boot/ 2>/dev/null
sudo umount $M
echo FIN
