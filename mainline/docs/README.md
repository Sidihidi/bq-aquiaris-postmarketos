# postmarketOS en el BQ Aquaris E4.5 ("krillin")

Port **funcional** de postmarketOS para el BQ Aquaris E4.5 (MediaTek MT6582,
codename `krillin`, el primer Ubuntu Phone, 2014). Construido desde cero — no
existía port previo en pmaports.

> **Estado:** arranca, da consola en pantalla + SSH por USB, interfaz gráfica
> Sxmo (X11), pantalla táctil y WiFi funcionando. Sin aceleración GPU
> (limitación del kernel 3.10, ver §8). Probado en hardware real.

---

## 1. Hardware y punto de partida

| Dato | Valor |
|---|---|
| Dispositivo | BQ Aquaris E4.5 (`krillin`) |
| SoC | MediaTek MT6582 (Cortex-A7 ×4, **armv7**) |
| GPU | ARM Mali-400 MP2 |
| RAM | 1 GB |
| Pantalla | 540×960 (qHD), panel táctil multitouch **tipo A** |
| eMMC | 8 GB |
| Estado de fábrica del equipo | LineageOS 14.1 (Android 7.1.2), bootloader **desbloqueado**, fastboot disponible |

**Entorno de construcción:** una Raspberry Pi (Debian 13 `trixie`, aarch64, 8 GB
RAM). Vale cualquier Linux; la clave es que `pmbootstrap` necesita un host Linux.
El teléfono se conecta por USB a la máquina de construcción.

---

## 2. Resumen de la arquitectura del port

```
   [Máquina de construcción: Raspberry Pi / Linux]
       pmbootstrap  ──compila──►  kernel + rootfs (armv7)
            │
            │ USB
            ▼
   [BQ Aquaris E4.5]
       partición boot   ← boot.img (kernel 3.10 + initramfs, formato MTK)
       partición system ← rootfs ext4 "pmOS_root" (OpenRC + Sxmo)
```

- **Kernel:** LineageOS 3.10.107 (rama `lineage-15.0` de
  `Pablito2020/android_kernel_bq_krillin`), compilado con **gcc6**.
- **Init:** **OpenRC** (NO systemd — ver §7, fue una de las decisiones clave).
- **Rootfs:** ext4 con features modernas **deshabilitadas** (el kernel 3.10 no
  las monta — ver §7).

---

## 3. Requisitos en la máquina de construcción

```sh
# pmbootstrap desde git (el de los repos de distro suele ir desfasado para 'edge')
git clone https://gitlab.postmarketos.org/postmarketOS/pmbootstrap.git ~/pmbootstrap
mkdir -p ~/.local/bin
ln -sf ~/pmbootstrap/pmbootstrap.py ~/.local/bin/pmbootstrap
export PATH=~/.local/bin:$PATH

# herramientas auxiliares usadas en esta guía
sudo apt-get install -y fastboot abootimg e2fsprogs telnet netcat-openbsd

pmbootstrap --version   # debe ser >= 3.x
pmbootstrap init        # arch: armv7 ya viene del deviceinfo; UI: console; systemd: never
```

---

## 4. Los archivos del port (aports)

Se crean en el árbol de pmaports que clona `pmbootstrap`
(`~/.local/var/pmbootstrap/cache_git/pmaports/`), bajo
`device/testing/device-bq-krillin/` y `device/testing/linux-bq-krillin/`.
Copias maestras de todos ellos están en este directorio (`aports/`).

### 4.1 `device-bq-krillin/deviceinfo`
Parámetros del equipo + cabecera del boot.img. **Los offsets MTK son críticos**
(salieron de los `BoardConfig.mk` de los device trees de CyanogenMod del krillin,
verificados contra el boot.img que el propio equipo ya arrancaba):

```
deviceinfo_flash_offset_base="0x10000000"
deviceinfo_flash_offset_kernel="0x00008000"
deviceinfo_flash_offset_ramdisk="0x01000000"
deviceinfo_flash_offset_second="0x00f00000"
deviceinfo_flash_offset_tags="0x00000100"
deviceinfo_flash_pagesize="2048"
deviceinfo_bootimg_mtk_label_kernel="KERNEL"   # cabeceras MTK obligatorias
deviceinfo_bootimg_mtk_label_ramdisk="ROOTFS"  # o el bootloader rechaza la imagen
```

### 4.2 `linux-bq-krillin/` (el kernel)
- `APKBUILD`: descarga el kernel de LineageOS, fuerza **gcc6** (el GCC moderno no
  compila kernels de 2017), y aplica el parche compartido
  `linux3.4-ARM-8933-1-replace-Sun-Solaris-style-flag-on-section.patch`
  (symlink a `device/.shared-patches/linux/`).
- `config-bq-krillin.armv7`: la `lineage_krillin_defconfig` del árbol del kernel,
  ajustada para postmarketOS (ver `pmbootstrap kconfig check`):
  `DEVTMPFS`, `DEVTMPFS_MOUNT`, `VT`, `VT_CONSOLE`, `DEVPTS_MULTIPLE_INSTANCES`,
  `CRYPTO_XTS`, `FRAMEBUFFER_CONSOLE` activados; `ANDROID_PARANOID_NETWORK`
  desactivado.

### 4.3 `device-bq-krillin/APKBUILD`
Metapaquete del dispositivo. Depende de `linux-bq-krillin`, `mkbootimg`,
`mtk-mkimage`, `postmarketos-base`.

---

## 5. Construcción

```sh
pmbootstrap checksum linux-bq-krillin
pmbootstrap checksum device-bq-krillin
pmbootstrap kconfig check bq-krillin       # debe pasar
pmbootstrap build linux-bq-krillin         # ~5 min (gcc6, chroot armv7 nativo)

# rootfs OpenRC + openssh + (opcional) GUI:
pmbootstrap config device bq-krillin
pmbootstrap config systemd never           # IMPRESCINDIBLE (ver §7)
pmbootstrap install --no-fde --add openssh-server --password TUCLAVE
```

Esto genera `~/.local/var/pmbootstrap/chroot_native/home/pmos/rootfs/bq-krillin.img`
(imagen GPT con partición EFI + raíz ext4) y, dentro del chroot del rootfs, el
`boot.img` ya en formato MTK.

---

## 6. Instalación en el teléfono

El bootloader (LK) de este MTK es **quisquilloso**: acepta imágenes pequeñas
(≤~14 MB) por fastboot, pero **falla** al transferir la rootfs grande
(`Protocol error` / `Unknown chunk type` — el LK no soporta sparse, y satura su
búfer). La solución es un canal propio vía el debug-shell del initramfs.

### 6.1 Preparar la imagen de raíz (en la máquina de construcción)

```sh
IMG=~/.local/var/pmbootstrap/chroot_native/home/pmos/rootfs/bq-krillin.img
# extraer SOLO la partición raíz (mira los sectores con: fdisk -l "$IMG")
dd if="$IMG" of=/tmp/pmos-root.img bs=512 skip=<START> count=<COUNT>

# ¡CRÍTICO! quitar features ext4 que el kernel 3.10 no monta:
sudo tune2fs -O ^metadata_csum,^metadata_csum_seed,^orphan_file /tmp/pmos-root.img
sudo tune2fs -O ^has_journal /tmp/pmos-root.img
sudo e2fsck -fy /tmp/pmos-root.img
sudo tune2fs -j /tmp/pmos-root.img        # journal nuevo, sin checksums v3
```

### 6.2 Flashear el kernel (sí cabe por fastboot)

```sh
# teléfono en fastboot: apagar, y conectar USB manteniendo Vol+
sudo fastboot flash boot ~/.local/var/pmbootstrap/chroot_rootfs_bq-krillin/boot/boot.img
```

### 6.3 Meter la rootfs vía debug-shell (el truco)

```sh
# forzar fallo de montaje para caer al debug-shell:
sudo fastboot flash system /tmp/zeros4m.img   # 4 MB de ceros (dd if=/dev/zero ...)
sudo fastboot reboot
# NOTA: 'fastboot erase system' se CUELGA en este LK — usar los ceros, no erase.

# el equipo arranca, no puede montar root, y levanta un debug-shell por
# red-USB (RNDIS). En la máquina de construcción:
sudo ip addr add 172.16.42.2/24 dev usb0
# receptor en el teléfono + envío desde el host (nc), escribe directo a la partición:
{ echo 'nc -l -p 9999 | dd of=/dev/mmcblk0p5 bs=1M'; sleep 3; } | telnet 172.16.42.1 &
nc -N 172.16.42.1 9999 < /tmp/pmos-root.img      # ~70 s para 600 MB
# luego, en el debug-shell:
#   reboot -f
```

`/dev/mmcblk0p5` = partición `system` (`android`) del krillin. La etiqueta
`pmOS_root` hace que el initramfs la encuentre por UUID/label.

---

## 7. Problemas resueltos (lecciones del port)

1. **El kernel no compilaba** (`proc-v7.S: junk at end of line #`): el ensamblador
   GNU moderno no acepta las flags de sección estilo Solaris de 2014. → parche
   compartido `linux3.4-ARM-8933-1`.
2. **systemd no arranca sobre kernel 3.10**: se queda colgado. → reconstruir el
   rootfs con **OpenRC** (`pmbootstrap config systemd never`).
3. **`mount: Invalid argument` al montar la raíz**: el `mkfs.ext4` moderno crea
   features (`metadata_csum`, `orphan_file`, journal checksum v3) que el kernel
   3.10 desconoce. → cirugía con `tune2fs` (§6.1).
4. **fastboot no puede con la rootfs grande**: LK sin soporte sparse. → canal
   `nc`/`dd` por el debug-shell (§6.3).
5. **Arranca pero sin SSH**: faltaba `openssh-server`. → `--add openssh-server`
   en `pmbootstrap install`.

---

## 8. Estado de los subsistemas

| Subsistema | Estado | Notas |
|---|---|---|
| Arranque + initramfs | ✅ | al primer intento en hardware real |
| Consola en pantalla | ✅⚠️ | funciona con *workaround* de refresco (ver §9) |
| SSH por USB | ✅ | `ssh user@172.16.42.1` |
| Pantalla táctil | ✅ | requiere driver `evdev`, no libinput (panel tipo-A; ver §9) |
| WiFi | ✅ | (firmware MTK; ver sesión de WiFi) |
| GUI (Sxmo / dwm / X11) | ✅⚠️ | usable; lenta (render por software) |
| Aceleración GPU | ❌ | Mali-400 + Lima exige kernel ≥5.2; inviable en 3.10 |
| Módem (llamadas/datos) | ⬜ | no abordado; vía CCCI userspace (investigado, ver notas) |

---

## 9. Workarounds activos en el rootfs

Estos ajustes se aplican *dentro* del sistema instalado (no en la construcción):

- **Refresco de pantalla** (`/usr/local/bin/fb-refresh` + `/etc/local.d/`): el
  driver `mtkfb` arranca en modo "decouple" y no presenta el framebuffer sin un
  disparo. Un bucle que hace `echo 0,0 > /sys/class/graphics/fb0/pan` cada 100 ms
  mantiene la pantalla viva. *Mejora pendiente: parchear `mtkfb` en el kernel para
  presentación directa — eliminaría este hack.*
- **Táctil con evdev** (`/etc/X11/xorg.conf.d/99-touchscreen.conf`): el panel usa
  multitouch **tipo A** (`SYN_MT_REPORT`), que **libinput no soporta** (da
  `double tracking ID` y descarta los eventos). Se fuerza el driver `evdev`.
  El nombre `99-` es importante: debe ganar al `40-libinput.conf` de pmOS.
- **Autologin en tty1** (`/etc/inittab`): `login -f user` para usar la consola sin
  teclado físico.
- **Estabilidad del SSH por USB** (⚠️ *pendiente de solución correcta*): la
  conexión RNDIS se cae periódicamente porque NetworkManager recicla `usb0`.
  **NO** marcar `usb0` como `unmanaged-devices` en NM: en este port es **NM quien
  asigna la IP `172.16.42.1`** a la interfaz USB, así que marcarla "no gestionada"
  deja al teléfono sin IP y **corta todo el acceso por USB** (lección aprendida a
  base de cerrarnos la puerta). La solución correcta es definir un *perfil de
  conexión estático* de NM para `usb0` (IP fija, sin DHCP ni reconexiones), no
  excluirla.

---

## 10. Acceso al sistema

```sh
# SSH por USB (red RNDIS):
sudo ip addr add 172.16.42.2/24 dev usb0       # en la máquina host
ssh user@172.16.42.1                            # usuario: user

# dar internet al teléfono (NAT desde el host; Debian usa nftables):
sudo sysctl -w net.ipv4.ip_forward=1
sudo nft add table ip pmosnat
sudo nft 'add chain ip pmosnat post { type nat hook postrouting priority 100 ; }'
sudo nft add rule ip pmosnat post ip saddr 172.16.42.0/24 oifname "wlan0" masquerade
# en el teléfono: ip route add default via 172.16.42.2 ; echo nameserver 1.1.1.1 > /etc/resolv.conf
```

---

## 11. Recuperar el teléfono / volver a LineageOS

Nunca se tocaron `preloader`, `nvram`, `protect_*` ni `recovery` — el equipo es
recuperable. Para volver a LineageOS: entrar en recovery (Power+Vol+) y reinstalar
el zip de Lineage, que reescribe `boot` y `system`.

> ⚠️ **Regla de oro MTK:** no flashear NUNCA la partición `preloader`. Es lo único
> que convierte el equipo en un pisapapeles irrecuperable sin herramientas de bajo
> nivel.

---

## 12. Pendiente / hoja de ruta

- [ ] Parchear `mtkfb` en el kernel para salir del modo decouple (matar el hack de refresco).
- [ ] Empaquetar el port como aport limpio y enviarlo a pmaports (`device/testing`).
- [ ] Expandir la raíz a la partición grande (5,7 GB) para tener espacio holgado.
- [ ] Módem: daemon AT en userspace sobre los canales CCCI del kernel (datos/SMS).
- [ ] (largo plazo) Bringup a kernel mainline (mt6582 tiene soporte inicial) → Lima → GPU real.
