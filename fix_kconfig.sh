#!/bin/bash
set -e
CFG=~/.local/var/pmbootstrap/cache_git/pmaports/device/testing/linux-bq-krillin/config-bq-krillin.armv7
for o in ANDROID_PARANOID_NETWORK CRYPTO_XTS DEVTMPFS DEVTMPFS_MOUNT VT DEVPTS_MULTIPLE_INSTANCES FRAMEBUFFER_CONSOLE VT_CONSOLE; do
    sed -i "/CONFIG_$o[ =]/d" "$CFG"
done
cat >> "$CFG" <<'EOF'
# CONFIG_ANDROID_PARANOID_NETWORK is not set
CONFIG_CRYPTO_XTS=y
CONFIG_DEVTMPFS=y
CONFIG_DEVTMPFS_MOUNT=y
CONFIG_VT=y
CONFIG_VT_CONSOLE=y
CONFIG_DEVPTS_MULTIPLE_INSTANCES=y
CONFIG_FRAMEBUFFER_CONSOLE=y
EOF
export PATH=~/.local/bin:$PATH
pmbootstrap checksum linux-bq-krillin 2>&1 | tail -1
pmbootstrap kconfig check bq-krillin 2>&1 | tail -5
