#!/bin/bash
IMG=/tmp/pmos-root.img
echo "=== fsck inicial ==="
sudo e2fsck -fy "$IMG" 2>&1 | tail -1
echo "=== quitar metadata_csum ==="
sudo tune2fs -O ^metadata_csum "$IMG" 2>&1
sudo e2fsck -fy "$IMG" 2>&1 | tail -1
echo "=== quitar metadata_csum_seed y orphan_file ==="
sudo tune2fs -O ^metadata_csum_seed "$IMG" 2>&1
sudo tune2fs -O ^orphan_file "$IMG" 2>&1
sudo e2fsck -fy "$IMG" 2>&1 | tail -1
echo "=== regenerar journal limpio ==="
sudo tune2fs -O ^has_journal "$IMG" 2>&1
sudo e2fsck -fy "$IMG" 2>&1 | tail -1
sudo tune2fs -j "$IMG" 2>&1 | tail -1
echo "=== FEATURES FINALES ==="
sudo dumpe2fs -h "$IMG" 2>/dev/null | grep -i features
