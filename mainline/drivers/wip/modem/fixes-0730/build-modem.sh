#!/bin/bash
set -e
K=$HOME/mainline/linux-7.0.12; P=$HOME/mainline/pkg
LOG=/tmp/build-m1.log; : > $LOG
say(){ echo "$@" | tee -a $LOG; }
say "[1] compilar kernel"
cd $K
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc) zImage >>$LOG 2>&1
say "  zImage = $(stat -c%s build-krillin/arch/arm/boot/zImage) B"
say "[2] zImage + dtb -> cabecera MTK"
cat build-krillin/arch/arm/boot/zImage $P/ganador-h2b.dtb > $P/zimage-dtb-m1
python3 $P/mtk_hdr.py KERNEL $P/zimage-dtb-m1 $P/zimage-dtb-m1-mtk >>$LOG 2>&1
say "[3] abootimg (con salida visible: falla si se redirige a /dev/null)"
rm -f $P/boot-modem-m1.img
abootimg --create $P/boot-modem-m1.img -f $P/bootimg.cfg -k $P/zimage-dtb-m1-mtk -r $P/initrd-menupick-mtk | tee -a $LOG
[ -s $P/boot-modem-m1.img ] || { say "FALLO: imagen no creada"; exit 1; }
say "  IMG = $(stat -c%s $P/boot-modem-m1.img) B"
md5sum $P/boot-modem-m1.img | tee -a $LOG
say FIN
