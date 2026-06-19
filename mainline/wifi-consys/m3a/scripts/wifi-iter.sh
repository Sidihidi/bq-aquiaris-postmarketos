#!/bin/bash
set -e
cd ~/mainline/linux-7.0.12
echo "=== build zImage ==="
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage 2>&1 | grep -iE "error:|warning:.*btif|zImage is ready" | tail -10
[ build-krillin/arch/arm/boot/zImage -nt drivers/soc/mediatek/mt6582-btif.c ] || { echo "BUILD FALLO (zImage no recompilado) — ABORTO"; exit 1; }
cd ~/mainline/pkg
Z=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/zImage
D=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dtb
cat "$Z" "$D" > zimage-dtb
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk >/dev/null
abootimg --create boot-btifDMA-sd.img -f bootimg-sd.cfg -k zimage-dtb-mtk -r initrd-sd-mtk 2>&1 | tail -1
echo "=== flash + reboot ==="
PH=root@172.16.42.1
sudo systemctl stop ModemManager 2>/dev/null
sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
IMG=boot-btifDMA-sd.img; SIZE=$(wc -c < "$IMG"); MD5SRC=$(md5sum "$IMG" | cut -d' ' -f1)
cat "$IMG" | ssh -o StrictHostKeyChecking=no $PH "cat > /tmp/b.img"
ssh -n -o StrictHostKeyChecking=no $PH "
  SEC=83968; SIZE=$SIZE
  M=\$(dd if=/dev/mmcblk0 bs=512 skip=\$SEC count=1 2>/dev/null | head -c 8)
  [ \"\$M\" = 'ANDROID!' ] || { echo ABORT-magic; exit 1; }
  dd if=/tmp/b.img of=/dev/mmcblk0 bs=512 seek=\$SEC conv=fsync 2>/dev/null; sync
  DD=\$(dd if=/dev/mmcblk0 bs=512 skip=\$SEC count=\$(((SIZE+511)/512)) 2>/dev/null | head -c \$SIZE | md5sum | cut -d' ' -f1)
  [ \"\$DD\" = '$MD5SRC' ] && echo FLASH_OK_MD5 || { echo FLASH_BAD; exit 1; }
"
ssh -n -o StrictHostKeyChecking=no $PH "sync; sleep 1; reboot" 2>/dev/null || true
echo REBOOTED
