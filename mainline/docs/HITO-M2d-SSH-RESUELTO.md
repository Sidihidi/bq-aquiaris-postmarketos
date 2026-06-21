# HITO M2d — SSH por USB en mainline (shell root interactiva)

**Estado: RESUELTO** (2026-06-16). `ssh root@172.16.42.1` desde el host (Pi) entra a
una shell root en mainline Linux 7.0.12, por USB. telnet (:23) como fallback.
Bonus verificado en la sesión: 1 GB RAM, eMMC completa visible (mmcblk0 + boot0/boot1
+ p1..p7 + rpmb) -> base lista para el rootfs.

## Arquitectura
initramfs = mini-rootfs en RAM con:
- **busybox 1.36.1** estático armhf (/bin/sh + utilidades) — defconfig + CONFIG_STATIC=y
- **dropbear 2022.83** estático armhf (dropbearmulti) — auth por clave
- init de shell (`init-ssh.sh`) que levanta usb0 + telnetd + dropbear
- `/root/.ssh/authorized_keys` = clave pública de la Pi
El display lo arregla el kernel (late_initcall krillin_dispfix), el init no lo toca.

## Recetas de compilación (cross armhf, en la Pi)
```
# busybox
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- defconfig
sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/; s/CONFIG_TC=y/# CONFIG_TC is not set/' .config
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc)

# dropbear (pubkey-only, estatico)
printf '#define DROPBEAR_SVR_PASSWORD_AUTH 0\n#define DROPBEAR_CLI_PASSWORD_AUTH 0\n' > localoptions.h
./configure --host=arm-linux-gnueabihf CC=arm-linux-gnueabihf-gcc \
   --disable-zlib --disable-syslog --disable-lastlog --disable-utmp --disable-wtmp --disable-harden
make PROGRAMS="dropbear dbclient dropbearkey scp" MULTI=1 STATIC=1 -j$(nproc)
```

## Los 4 fixes que costaron (todos resueltos)
1. **dropbear no compilaba**: `crypt() required` -> deshabilitar password auth
   (`DROPBEAR_SVR_PASSWORD_AUTH 0`); usamos solo pubkey.
2. **dropbear no linkaba estatico**: `-fPIE + -static` -> "dangerous relocation".
   Fix: `--disable-harden` (quita -fPIE/-pie).
3. **telnetd/dropbear morian al instante**: faltaba **devpts** (`/dev/pts`) para los
   PTYs. Fix: `mount -t devpts devpts /dev/pts` + symlink /dev/ptmx en el init.
4. **dropbear arrancaba pero no escuchaba / rechazaba la clave**:
   - `Invalid option -E`: esta build (sin syslog) NO tiene `-E` (ya loguea a stderr).
     Fix: quitar `-E`.
   - `/root must be owned by user or root, and not writable by group or others`: el
     cpio creo los ficheros con uid 1000 y /root como 775. Fix: cpio `--owner=0:0`
     + el init hace `chown -R 0:0 /root /etc/dropbear; chmod 700 /root /root/.ssh;
     chmod 600 authorized_keys`.
   NOTA: NO hubo problema de NSS/getpwnam (el aviso del linker era inofensivo: glibc
   estatica resolvio /etc/passwd con nsswitch "files").

## Cómo conectar (desde la Pi, clave ya autorizada)
```
ssh -i ~/.ssh/id_ed25519 root@172.16.42.1     # SSH (puerto 22)
telnet 172.16.42.1                            # fallback (puerto 23, sin auth)
```

## Pendiente / siguiente
Con SSH + eMMC visible (p5 = rootfs de pmOS), el siguiente hito es arrancar/montar un
rootfs real con mainline. Recomendado: generar un rootfs *mainline* (pmbootstrap o
debootstrap armhf) en vez de reutilizar el de pmOS 3.10 (módulos/OpenRC de 3.10 no
encajan con 7.0.12). Desde la shell SSH se puede `mount /dev/mmcblk0p5 /mnt`,
inspeccionar y hacer chroot para iterar.
