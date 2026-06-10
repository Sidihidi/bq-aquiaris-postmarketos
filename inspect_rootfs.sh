#!/bin/bash
IMG=/tmp/pmos-root.img
M=/mnt/pmosroot
sudo mkdir -p $M
sudo mount -o loop $IMG $M || { echo NO_MONTA; exit 1; }
echo "=== sshd instalado? ==="
ls -la $M/usr/sbin/sshd 2>/dev/null || echo SSHD_NO_EXISTE
echo "=== servicios en runlevel default ==="
ls $M/etc/runlevels/default/ 2>/dev/null
echo "=== servicios en runlevel boot ==="
ls $M/etc/runlevels/boot/ 2>/dev/null
echo "=== claves de host ==="
ls -la $M/etc/ssh/ssh_host_* 2>/dev/null || echo SIN_CLAVES_HOST
echo "=== sshd_config relevantes ==="
grep -E '^(PasswordAuth|PermitRoot|Port|ListenAddress)' $M/etc/ssh/sshd_config 2>/dev/null
echo "=== inittab ==="
cat $M/etc/inittab 2>/dev/null | grep -v '^#'
sudo umount $M
echo "=== FIN ==="
