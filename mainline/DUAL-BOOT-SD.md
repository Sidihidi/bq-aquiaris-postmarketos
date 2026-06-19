# Dual-boot: pmOS en SD + Android en la interna (2026-06-19) — FUNCIONANDO

## Estado: ✅ pmOS arranca desde la tarjeta SD, interna (Android) intacta
- Root = `/dev/mmcblk1p1` (SD 32GB), Phosh + lima + mtk_drm OK. `mmcblk0p7` (interna) NO se toca.

## Cómo se montó
1. **DT**: nodo `mmc1: mmc@11240000` (mtk-msdc.1 = ranura SD), IRQ `GIC_SPI 40`, bus 4-bit,
   `broken-cd`, supplies PMIC `ldo_vmch` (VDD 3.3V) + `ldo_vmc` (I/O) — añadidos a `mt6323regulator`.
   (Base MSDC1 0x11240000, del downstream; eMMC interna = msdc0 0x11230000.) La SD se detecta sola.
2. **SD**: `mkfs.ext4 -F /dev/mmcblk1p1` (formatea, borra el FAT) → `mount` → copiar rootfs con
   `cp -a` de los dirs reales (bin boot etc home lib opt root sbin srv usr var; busybox tar NO
   soporta --one-file-system) + crear mountpoints (dev proc sys run tmp mnt) → **arreglar
   `/etc/fstab`**: `mmcblk0p7`→`mmcblk1p1` y quitar la línea de swap (el /swap de 1.6G no se copia).
3. **boot.img**: `boot-pmos-sd.img` = zImage + dtb(con mmc1) + **initrd con switch_root a
   `mmcblk1p1`** (de `init-switchroot.sh`, `sed mmcblk0p7→mmcblk1p1`). En `~/mainline/pkg`.

## Cómo elegir SO (de momento: reflashear boot)
- **pmOS-SD**: `fastboot flash boot boot-pmos-sd.img` (+ SD puesta).
- **pmOS-interna**: `fastboot flash boot boot-color1.img`.
- **Android stock**: `fastboot flash boot boot-stockadb2.img` (adb-root debug) o el stock limpio.

## Menú de boot (paso futuro, sin tocar el bootloader)
Plan: `boot` = pmOS-SD, `recovery` = Android stock → el combo de teclas del LK elige (Vol↑+Power =
recovery). + `Vol↓+Power` = fastboot (del LK). Opcional: initramfs que lea una tecla → SD vs interna.
4-en-1: fastboot / recovery(Android) / pmOS-SD / pmOS-interna. (U-Boot mt6582 existe pero es UART-only
sin display → impráctico ahora.)

## Pendiente menor: la SD va a 12MHz (max-frequency conservador) → lenta. Subir a 50MHz
(cap-sd-highspeed) cuando se confirme estable.
