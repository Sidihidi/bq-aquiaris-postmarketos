#!/bin/sh
# Flashea una imagen a la particion bootimg (offset 0x1D80000 de mmcblk0)
# con verificacion del magic ANDROID! antes de escribir. USO: flash_boot_dd.sh <img>
set -e
IMG="$1"
OFF=$((0x1D80000))
SEC=$((OFF / 512))   # 60416
MAGIC=$(dd if=/dev/mmcblk0 bs=512 skip=$SEC count=1 2>/dev/null | head -c 8)
[ "$MAGIC" = "ANDROID!" ] || { echo "ABORT: magic '$MAGIC' != ANDROID! en offset bootimg"; exit 1; }
SIZE=$(wc -c < "$IMG")
[ $SIZE -le $((0x1400000)) ] || { echo "ABORT: imagen mayor que la particion"; exit 1; }
dd if="$IMG" of=/dev/mmcblk0 bs=512 seek=$SEC conv=fsync 2>/dev/null
echo "FLASHEADO ($SIZE bytes en sector $SEC). Verificando..."
NEW=$(dd if=/dev/mmcblk0 bs=512 skip=$SEC count=1 2>/dev/null | head -c 8)
[ "$NEW" = "ANDROID!" ] && echo "VERIFICADO: cabecera ANDROID! presente" || echo "OJO: cabecera rara tras escribir"
