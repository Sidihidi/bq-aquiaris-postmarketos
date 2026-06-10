#!/bin/bash
set -e
cd ~/mainline/pkg
Z=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/zImage
D=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dtb
[ -f "$Z" ] && [ -f "$D" ] || { echo "falta zImage o dtb"; exit 1; }
cat "$Z" "$D" > zimage-dtb
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk
python3 mtk_hdr.py ROOTFS initrd-tiny.cpio.gz initrd-tiny-mtk
# cfg sin bootsize (el nuestro será mayor) y cmdline determinista
grep -v "^bootsize" bootimg.cfg > bootimg-mainline.cfg
sed -i 's/^cmdline = .*/cmdline = console=tty0 clk_ignore_unused ignore_loglevel/' bootimg-mainline.cfg
rm -f boot-mainline.img
abootimg --create boot-mainline.img -f bootimg-mainline.cfg -k zimage-dtb-mtk -r initrd-tiny-mtk
ls -la boot-mainline.img zimage-dtb-mtk initrd-tiny-mtk
SIZE=$(stat -c%s boot-mainline.img)
MAX=$((0x1400000))
echo "tamaño: $SIZE / $MAX ($(( SIZE * 100 / MAX ))% de la partición bootimg)"
[ $SIZE -le $MAX ] && echo "CABE-EN-PARTICION" || echo "DEMASIADO-GRANDE"
