#!/bin/bash
set -e
which abootimg >/dev/null || sudo apt-get install -y -q abootimg >/dev/null 2>&1
B=~/.local/var/pmbootstrap/chroot_rootfs_bq-krillin/boot/boot.img
cp "$B" /tmp/boot-debug.img
CMD="console=tty0 pmos_boot_uuid=4d25dffd-5eac-41a9-8c6a-e890229536c8 pmos_root_uuid=03802229-e07a-42e2-9c90-d623b30c3651 pmos_rootfsopts=defaults pmos.debug-shell"
abootimg -u /tmp/boot-debug.img -c "cmdline=$CMD"
echo "=== verificacion ==="
abootimg -i /tmp/boot-debug.img | grep -A1 cmdline
ls -la /tmp/boot-debug.img
