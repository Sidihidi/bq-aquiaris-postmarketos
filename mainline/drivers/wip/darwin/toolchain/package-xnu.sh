#!/bin/sh
# Empaqueta el XNU recien compilado en una imagen de boot MTK para el krillin.
# Cadena: mach_kernel -> Mach.img3 -> Mach.o(.mach) -> vmlinux(GenericBooter) ->
#         +ganador-h2b.dtb -> mtk_hdr KERNEL -> abootimg -> boot-xnu.img
set -e
DK=$HOME/darwin-krillin
GB=$DK/GenericBooter
PKG=$HOME/mainline/pkg
IMG3=/opt/Developer/Toolchains/DarwinARM.toolchain/usr/bin/image3maker
CC=arm-none-eabi-

echo "[1] copiar kernel nuevo"
cp $DK/xnu/BUILD/obj/DEBUG_ARM_MT6582/mach_kernel $DK/mach_kernel-mt6582
ls -la $DK/mach_kernel-mt6582

echo "[2] Mach.img3 (tag krnl) desde el kernel nuevo"
$IMG3 -f $DK/mach_kernel-mt6582 -t krnl -o $GB/images/Mach.img3

echo "[3] images/Mach.o con seccion .mach (DeviceTree/Ramdisk sin cambios)"
${CC}ld -r -b binary -o $GB/images/Mach.o $GB/images/Mach.img3
${CC}objcopy --rename-section .data=.mach $GB/images/Mach.o $GB/images/Mach.o

echo "[4] reconstruir GenericBooter (vmlinux embebe images/Mach.o)"
cd $GB
rm -f vmlinux vmlinux.raw
make ARCH=arm CROSS_COMPILE=$CC 2>&1 | tail -8
cp vmlinux.raw $GB/GenericBooter-xnu.bin
ls -la $GB/GenericBooter-xnu.bin

echo "[5] append dtb + cabecera MTK KERNEL"
cat $GB/GenericBooter-xnu.bin $PKG/ganador-h2b.dtb > $PKG/gb-xnu-dtb
python3 $PKG/mtk_hdr.py KERNEL $PKG/gb-xnu-dtb $PKG/gb-xnu-dtb-mtk

echo "[6] abootimg -> boot-xnu.img"
abootimg --create $PKG/boot-xnu.img -f $PKG/bootimg.cfg -k $PKG/gb-xnu-dtb-mtk -r $PKG/initrd-menupick-mtk 2>&1 | tail -5
ls -la $PKG/boot-xnu.img
echo "LISTO: $PKG/boot-xnu.img"
