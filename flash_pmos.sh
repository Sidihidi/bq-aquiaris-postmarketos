#!/bin/bash
set -e
B=~/.local/var/pmbootstrap/chroot_rootfs_bq-krillin/boot/boot.img
echo "=== FLASH BOOT ==="
sudo fastboot flash boot "$B" 2>&1
echo "=== FLASH SYSTEM (pmOS root) ==="
sudo fastboot flash system /tmp/pmos-root.img 2>&1
echo "=== TODO FLASHEADO ==="
