# HITO — Rootfs Alpine moderno + mainline en eMMC (postmarketOS base)

**Estado: FUNCIONA** (2026-06-16). El BQ Aquaris E4.5 arranca **Alpine Linux 3.24.1**
(la última) sobre **kernel mainline 7.0.12**, con rootfs **persistente en eMMC (p7)**,
gestionado por **OpenRC**, con **OpenSSH** (root por clave) y **apk** (gestor de
paquetes) operativo. Es un postmarketOS moderno en su núcleo — y más moderno que el
pmOS "oficial" del E4.5, que usa el kernel downstream 3.10.

## Verificado
```
hostname: krillin-mainline
Linux krillin-mainline 7.0.12 SMP armv7l
Alpine 3.24.1   /  / = /dev/mmcblk0p7 ext4 rw (persistente)
OpenRC default: sshd [started] local [started]
sshd (OpenSSH) LISTEN :22  ·  1 GB RAM  ·  eMMC p1..p7 visible
```

## Mapa de particiones (del blkdevparts que pasa el LK)
`/proc/cmdline` del LK:
`blkdevparts=mmcblk0:512K@512K(ebr1),10240K@9216K(protect_f),10240K@19456K(protect_s),6144K@71168K(sec_ro),1048576K@91136K(android),716800K@1140736K(cache),-@1856512K(usrdata)`
→ **p5 = android(system 1GB), p6 = cache, p7 = usrdata (~5.6 GB)**. Mainline usa ese
blkdevparts (no hay GPT/MBR). El rootfs Alpine va en **p7** (usrdata; android/p5 lo
sobrescribe cualquier reinstalación de Android).

## Receta (reproducible)
1. **Rootfs**: `alpine-minirootfs-3.24.1-armhf.tar.gz` → imagen ext4 (`mkfs.ext4`,
   instalar `e2fsprogs` en la Pi) → extraer → `dd` a p7 (gzip por SSH/usb0).
2. **Módulos**: `make ... INSTALL_MOD_PATH=<rootfs> modules_install` (opcional; los
   drivers clave son built-in).
3. **Config (chroot en la Pi vía binfmt/qemu, o en vivo por SSH con NAT)**:
   `apk add openrc openssh util-linux ifupdown-ng ca-certificates`;
   `rc-update add devfs/procfs/sysfs (sysinit), hostname/bootmisc/networking (boot),
   sshd/local (default)`; `/etc/network/interfaces` usb0 estático 172.16.42.1;
   inittab OpenRC estándar; `authorized_keys` de la Pi; PermitRootLogin prohibit-password.
4. **Internet para apk** (NAT en la Pi): `sysctl net.ipv4.ip_forward=1` +
   `iptables -t nat -A POSTROUTING -o wlan0 -j MASQUERADE` + FORWARD usb0<->wlan0;
   en el teléfono `ip route add default via 172.16.42.2` + DNS. (repos a http:// o
   `apk add ca-certificates` para https).

## Arranque (stage-1 initramfs → switch_root)
`boot-mainline-rootfs.img` = kernel mainline + initramfs `init-switchroot.sh`, que:
monta `/dev/mmcblk0p7` y hace `exec switch_root /newroot /sbin/init` (Alpine/OpenRC).
Si p7 falla, FALLBACK de emergencia: levanta SSH (busybox+dropbear) para no perder
acceso. `boot-mainline-ssh.img` = initramfs-only de rescate (no monta p7).

## Cómo conectar
`ssh -i ~/.ssh/id_ed25519 root@172.16.42.1` (desde la Pi, por USB).

## Pendiente
GUI/periféricos (ver HITO-I2C-TOUCH.md y el roadmap). El sistema base CLI está sólido.
