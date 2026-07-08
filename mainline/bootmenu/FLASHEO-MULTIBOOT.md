# ⚠️ Cómo flashear ahora (con menú multiboot) — NO PISAR EL SECTOR 83968

## ARQUITECTURA NUEVA (con menú multiboot)

```
eMMC sector 83968 = boot-menupick2.img  ← FIJO, NO TOCAR
                     (kernel mainline + initramfs menupick)
                         │
                    MENÚ EN PANTALLA
                    (Vol+/- + Power)
                         │
           ┌─────────────┴─────────────┐
           │                           │
      pmOS (switch_root)         Android (kexec)
      → mmcblk1p1 (SD)           → /boot/android-boot.img
```

## ❌ LO QUE NO HAY QUE HACER NUNCA MÁS

**NO flashear boot.img en el sector 83968 del eMMC:**

```sh
# ❌ ESTO PISARÍA EL MENÚ MULTIBOOT:
dd if=boot-pmos-nuevo.img of=/dev/mmcblk0 bs=512 seek=83968
fastboot flash boot boot-pmos-nuevo.img
```

El sector 83968 ahora contiene el **boot.img del menú** (kernel + initramfs menupick).
Si lo pisas, el móvil volverá a arrancar SOLO pmOS sin menú.

## ✅ CÓMO ACTUALIZAR pmOS AHORA

### Caso 1: Actualizar el rootfs de pmOS (paquetes, configs, etc.)
**No tocar el boot.img.** El rootfs vive en la SD (`mmcblk1p1`), independiente del boot:
```sh
# En el móvil, actualizar Alpine como siempre:
apk update && apk upgrade
# O copiar ficheros por scp al rootfs de la SD
```

### Caso 2: Actualizar el KERNEL de pmOS (zImage)
El kernel está DENTRO del boot-menupick2.img. Para actualizarlo:
1. Recompilar el kernel en la Pi
2. Reconstruir el boot.img del menú con el nuevo zImage + el initramfs menupick
3. Flashear el **boot-menupick-NUEVO.img** en el sector 83968 (reemplaza el menú viejo por uno con kernel nuevo, pero sigue siendo el menú)

```sh
# En la Pi:
cd ~/mainline/pkg
Z=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/zImage
D=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dtb
cat "$Z" "$D" > zimage-dtb
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk >/dev/null
abootimg --create boot-menupick2.img -f bootimg-sd.cfg -k zimage-dtb-mtk -r initrd-menupick-mtk
# Flashear (reemplaza el menú viejo por el nuevo con kernel actualizado):
dd if=boot-menupick2.img of=/dev/mmcblk0 bs=512 seek=83968  # DESDE EL MÓVIL
# o: fastboot flash boot boot-menupick2.img                 # DESDE FASTBOOT
```

### Caso 3: Actualizar el initramfs menupick (cambiar entradas del menú)
Reconstruir el initramfs y reempaquetar el boot.img del menú (igual que Caso 2).

### Caso 4: Añadir Maemo Leste al menú
1. Crear partición `mmcblk1p3` en la SD para el rootfs de Maemo
2. Reconstruir el initramfs menupick con la tercera entrada
3. Reempaquetar y flashear el boot.img del menú

## 📁 DÓNDE ESTÁ CADA COSA

| Qué | Dónde | Se toca? |
|---|---|---|
| **Boot.img del menú** | eMMC sector 83968 (`boot-menupick2.img`) | Solo para actualizar kernel/initramfs del menú |
| **Rootfs pmOS** | SD `mmcblk1p1` | Sí, libremente (apk upgrade, etc.) |
| **Kernel mainline** | Dentro del boot.img del menú (zimage-dtb-mtk) | Solo al recompilar |
| **DTB** | Dentro del boot.img del menú (concatenado con zImage) | Solo al cambiar el DTS |
| **Initramfs menupick** | Dentro del boot.img del menú (initrd-menupick-mtk) | Solo al cambiar entradas del menú |
| **android-boot.img** | SD `/boot/android-boot.img` | Para actualizar Android (LineageOS) |
| **eMMC particiones Android** | mmcblk0p5 (sistema) + mmcblk0p7 (usrdata) | NUNCA (Android intacto) |

## 🔧 SCRIPT DE FLASHEO SEGURO

Usa `~/mainline/pkg/flash_menu.sh` (crear):
```sh
#!/bin/bash
# Flashea el boot.img del MENÚ MULTIBOOT (no un boot.img de pmOS suelto).
# Uso: bash flash_menu.sh [boot-menupick2.img]
set -e
IMG=${1:-boot-menupick2.img}
cd ~/mainline/pkg
PH=root@172.16.42.1
sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null || true
SIZE=$(wc -c < "$IMG"); MD5SRC=$(md5sum "$IMG" | cut -d' ' -f1)
echo "Flasheando MENÚ MULTIBOOT: $IMG ($SIZE bytes)"
cat "$IMG" | ssh $PH "cat > /tmp/b.img"
ssh $PH "SEC=83968; M=\$(dd if=/dev/mmcblk0 bs=512 skip=\$SEC count=1 2>/dev/null|head -c 8);
  dd if=/tmp/b.img of=/dev/mmcblk0 bs=512 seek=\$SEC conv=fsync 2>/dev/null; sync;
  DD=\$(dd if=/dev/mmcblk0 bs=512 skip=\$SEC count=\$(((SIZE+511)/512)) 2>/dev/null|head -c \$SIZE|md5sum|cut -d' ' -f1);
  [ \"\$DD\" = '$MD5SRC' ] && echo FLASH_OK || echo FLASH_BAD"
```

## ⚠️ REGLA DE ORO

**El sector 83968 del eMMC es ahora el MENÚ MULTIBOOT.** Solo se flashea con un
`boot-menupick*.img`. Los boot.img de pmOS sueltos (que antes iban ahí) ya NO
se flashean en el eMMC — el rootfs de pmOS está en la SD y se actualiza con `apk`.

Para recuperar el estado anterior (sin menú, solo pmOS directo):
flashear un `boot-*.img` normal en el sector 83968.
