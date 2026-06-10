#!/bin/bash
IMG=/tmp/pmos-root.img
echo "=== features actuales ==="
sudo dumpe2fs -h "$IMG" 2>/dev/null | grep -i 'features'
echo "=== quitando features modernas (kernel 3.10) ==="
sudo tune2fs -O ^orphan_file "$IMG" 2>&1 | tail -1
sudo tune2fs -O ^metadata_csum "$IMG" 2>&1 | tail -1
sudo tune2fs -O ^metadata_csum_seed "$IMG" 2>&1 | tail -1
echo "=== e2fsck ==="
sudo e2fsck -fy "$IMG" 2>&1 | tail -4
echo "=== features finales ==="
sudo dumpe2fs -h "$IMG" 2>/dev/null | grep -i 'features'
