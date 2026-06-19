#!/bin/bash
IMG=/tmp/pmos-root.img
M=/mnt/pmoschk
sudo mkdir -p $M
echo "=== 1. verificar contenido OpenRC ==="
sudo mount -o loop "$IMG" $M
echo "--- init: ---"
ls -la $M/sbin/init 2>/dev/null
echo "--- sshd: ---"
ls -la $M/usr/sbin/sshd 2>/dev/null || echo SSHD_FALTA
echo "--- runlevel default: ---"
ls $M/etc/runlevels/default/ 2>/dev/null
echo "--- runlevel boot (resumen): ---"
ls $M/etc/runlevels/boot/ 2>/dev/null | head -8
sudo umount $M
echo "=== 2. cirugia ext4 para kernel 3.10 ==="
sudo tune2fs -O ^orphan_file "$IMG" >/dev/null 2>&1
sudo tune2fs -O ^metadata_csum "$IMG" >/dev/null 2>&1
sudo tune2fs -O ^metadata_csum_seed "$IMG" >/dev/null 2>&1
sudo tune2fs -O ^has_journal "$IMG" >/dev/null 2>&1
sudo e2fsck -fy "$IMG" 2>&1 | tail -1
sudo tune2fs -j "$IMG" >/dev/null 2>&1
echo "--- features finales: ---"
sudo dumpe2fs -h "$IMG" 2>/dev/null | grep -i 'features'
echo "=== LISTO_PARA_FLASHEAR ==="
