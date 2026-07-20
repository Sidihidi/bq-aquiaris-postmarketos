#!/bin/sh
# Build XNU para el krillin (MT6582). Toolchain DarwinARM via xcrun/mig en PATH.
set -e
cd ~/darwin-krillin/xnu
export PATH=/opt/Developer/Toolchains/DarwinARM.toolchain/usr/bin:$PATH
make ARCH_CONFIGS=ARM KERNEL_CONFIGS=DEBUG MACHINE_CONFIG=MT6582 "$@" 2>&1
