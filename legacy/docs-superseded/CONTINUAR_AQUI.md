# 🔄 CONTINUAR AQUÍ — postmarketOS en BQ Aquaris E4.5 (krillin)

> Documento de traspaso entre sesiones. Última actualización: **2026-06-10 (tarde)**.
> Lee esto primero, luego el `README.md` (la documentación técnica completa).

---

## ⚡ TL;DR — dónde lo dejamos

- ✅ **RECONSTRUIDO DESDE CERO en la Pi nueva** (la vieja 192.168.0.112 quedó
  inaccesible y no había backups): pmbootstrap + aports + config del kernel
  regenerado + kernel compilado + rootfs nueva. El bootloop antiguo quedó
  resuelto de paso (rootfs nueva = raíz limpia, sin el conf malo de NM).
- ✅ **El teléfono arranca estable**: kernel 3.10.107 propio, OpenRC, sshd,
  NetworkManager, fb-refresh en autostart, autologin tty1, internet por NAT.
- 🔄 Instalando **Sxmo** (dwm/X11) + táctil evdev (en curso al cierre de sesión).
- 🆕 **LECCIÓN CRÍTICA**: en este kernel, escribir a `fb0/blank` = **pánico del
  kernel** → NO usar el viejo `99-display-kick.start` (causó un bootloop hoy;
  ver §1bis). Los writes a `fb0/pan` (fb-refresh) son seguros.

---

## 1. 🚑 LO PRIMERO: recuperar el teléfono del bootloop

1. **Sacar la batería** (el E4.5 la tiene extraíble) ~5 s y volver a ponerla →
   corta el bootloop.
2. **Entrar en fastboot**: mantener **Vol+** y conectar el USB a la Pi (o
   Power+Vol+).
3. Desde la Pi, **flashear el boot de diagnóstico** (lleva al debug-shell, que
   recupera la red USB SIN NetworkManager):
   ```sh
   # en la Pi (juan@192.168.0.112):
   sudo fastboot flash boot /tmp/boot-debug2.img    # si ya no existe, regenerar (§5)
   sudo fastboot reboot
   ```
4. Esperar al debug-shell y entrar por telnet:
   ```sh
   sudo ip addr add 172.16.42.2/24 dev usb0
   telnet 172.16.42.1        # puerto 23 = debug-shell del initramfs
   ```
5. **Reparar la raíz** (desde el debug-shell del teléfono):
   ```sh
   e2fsck -fy /dev/mmcblk0p5
   # montar y borrar el archivo de NM que cerró el acceso USB:
   mount /dev/mmcblk0p5 /sysroot
   rm -f /sysroot/etc/NetworkManager/conf.d/99-usb-unmanaged.conf
   umount /sysroot
   ```
6. **Si e2fsck no la arregla** → retransferir la raíz limpia (§6).
7. **Restaurar el boot normal** y reiniciar:
   ```sh
   sudo fastboot flash boot ~/.local/var/pmbootstrap/chroot_rootfs_bq-krillin/boot/boot.img
   sudo fastboot reboot
   ```

---

## 2. 🔑 Datos de acceso

| Qué | Valor |
|---|---|
| Máquina de construcción | Raspberry Pi NUEVA, `ssh cpcd@192.168.0.123` (clave del Mac instalada; passwordless sudo) |
| Pi vieja (OBSOLETA) | `juan@192.168.0.112` — apagada/inaccesible desde 2026-06-10 |
| Teléfono por USB | `ssh user@172.16.42.1` — **usuario `user`** (contraseña `147147`) |
| Teléfono: copiar archivos | **NO tiene scp** — usar `cat` sobre ssh (`ssh user@... "cat > /tmp/f" < f`) |
| IP de la Pi en usb0 | `172.16.42.2/24` (ponerla a mano tras cada reconexión) |
| Debug-shell initramfs | telnet a `172.16.42.1` puerto 23 (solo con boot-debug o fallo de montaje) |
| Partición raíz en el tel. | `/dev/mmcblk0p5` (= partición `android`/`system`) |
| Imágenes en la Pi | `/tmp/pmos-root.img`, `/tmp/boot-clean.img`, `/tmp/boot-debug.img`, `/tmp/zeros4m.img` (⚠️ /tmp se borra al reiniciar la Pi) |
| Config kernel | `~/.local/var/pmbootstrap/cache_git/pmaports/device/testing/linux-bq-krillin/config-bq-krillin.armv7` — REGENERADO, hacer backup |

### §1bis 🆕 Bootloop por display-kick (2026-06-10) y cómo se rescató

El viejo `99-display-kick.start` (`echo 1 > /sys/class/graphics/fb0/blank`)
**panica el kernel recompilado** → puesto en `/etc/local.d/` causó bootloop.
Rescate SIN tocar hardware: el sistema arranca sshd ANTES que el servicio
`local` → hay ventana de ~10 s. Polling rápido de ssh desde la Pi y, al entrar,
`rm -f /etc/local.d/98-* /etc/local.d/99-*`. El daemon `fb-refresh` (pan cada
250 ms) es seguro y queda en autostart (`98-fb-refresh.start`).

**Nota Windows/PowerShell:** los comandos con comillas/`()`/`&` se rompen al pasar
por SSH desde PowerShell. **Método que funciona:** escribir un script `.sh` en
`C:\Users\jferr\Desktop\pmos-krillin\`, hacer `scp` a la Pi, `sed -i 's/\r$//'`
(quitar CRLF) y ejecutarlo con `bash`. Todos los scripts de la sesión están ahí.

---

## 3. 🧠 Conocimiento clave (los muros que ya rompimos)

1. **Kernel** = LineageOS **3.10.107** (`Pablito2020/android_kernel_bq_krillin`,
   rama `lineage-15.0`), compilado con **gcc6** (GCC moderno no compila kernels de
   2017). Parche obligatorio: `linux3.4-ARM-8933-1` (ensamblador GNU moderno).
2. **OpenRC, NO systemd** — systemd se cuelga sobre el kernel 3.10.
3. **ext4: quitar features modernas** (`metadata_csum`, `orphan_file`, journal
   checksum v3) con `tune2fs` o el kernel 3.10 no monta la raíz.
4. **Flasheo de la rootfs**: fastboot NO puede con imágenes grandes en este LK
   (sin soporte sparse). Truco: flashear 4 MB de ceros a `system` → arranca al
   debug-shell → transferir por `nc`/`dd` (§6). `fastboot erase` SE CUELGA.
5. **bootimg MTK**: base `0x10000000`, labels `KERNEL`/`ROOTFS`, pagesize 2048.
6. **Táctil**: panel **multitouch tipo A** → libinput NO lo soporta (da "double
   tracking ID" y descarta todo). Solución: forzar driver **evdev** en X
   (`/etc/X11/xorg.conf.d/99-touchscreen.conf`, el `99-` debe ganar al `40-libinput`).
7. **Pantalla**: el driver `mtkfb` arranca en modo "decouple" y no presenta el
   framebuffer sin disparo. Workaround: demonio `/usr/local/bin/fb-refresh` que
   hace `echo 0,0 > /sys/class/graphics/fb0/pan` cada 100 ms.

---

## 4. ⚠️ EL ERROR QUE NOS CERRÓ EL ACCESO (no repetir)

En este port **es NetworkManager quien asigna la IP `172.16.42.1` a `usb0`**.
Marcar `usb0` como `unmanaged-devices` en NM (para frenar las caídas periódicas
de la conexión USB) le quita la IP y **corta TODO el acceso por USB**.

- **Solución correcta** (pendiente): crear un **perfil de conexión estático** de
  NM para usb0 (IP fija), NO excluirla.
- El archivo culpable a borrar: `/etc/NetworkManager/conf.d/99-usb-unmanaged.conf`

---

## 5. 🛠️ Regenerar artefactos si se perdieron

Los `/tmp/*` de la Pi pueden borrarse entre sesiones. Para regenerar:

```sh
# en la Pi:
export PATH=~/.local/bin:$PATH

# boot.img de diagnóstico (debug-shell forzado):
B=~/.local/var/pmbootstrap/chroot_rootfs_bq-krillin/boot/boot.img
cp "$B" /tmp/boot-debug2.img
abootimg -u /tmp/boot-debug2.img -c \
  "cmdline=pmos_boot_uuid=<BOOT_UUID> pmos_root_uuid=<ROOT_UUID> pmos_rootfsopts=defaults pmos.debug-shell"
# (los UUID se leen con: abootimg -i $B | grep cmdline)

# 4 MB de ceros para forzar fallo de montaje:
dd if=/dev/zero of=/tmp/zeros4m.img bs=1M count=4

# regenerar la imagen entera desde cero (si hiciera falta):
pmbootstrap config systemd never
pmbootstrap install --no-fde --add openssh-server --password 147147
# luego extraer la partición raíz y QUITAR features ext4 (ver README §6.1)
```

---

## 6. 📤 Transferir la rootfs por nc (cuando fastboot no puede)

```sh
# 1) en fastboot, forzar fallo de montaje:
sudo fastboot flash system /tmp/zeros4m.img && sudo fastboot reboot
# 2) esperar al debug-shell, IP en la Pi:
sudo ip addr add 172.16.42.2/24 dev usb0
# 3) receptor en el teléfono + envío (script /tmp/transfer_root.sh en la Pi):
{ echo 'nc -l -p 9999 | dd of=/dev/mmcblk0p5 bs=1M'; sleep 3; } | telnet 172.16.42.1 &
nc -N 172.16.42.1 9999 < /tmp/pmos-root.img      # ~70 s
# 4) en el debug-shell: reboot -f
```

**OJO**: cada vez que se regenera `/tmp/pmos-root.img` hay que volver a quitarle
las features ext4 (`tune2fs -O ^metadata_csum,^orphan_file,^has_journal` + e2fsck
+ `tune2fs -j`). Ver `README.md` §6.1.

---

## 6bis. 🚀 Proyecto mainline — **¡M1 CONSEGUIDO 2026-06-10!**

**Linux 7.0.12 ARRANCA en el krillin**: pingüinos + consola visibles en pantalla
vía simple-framebuffer. Confirmado en hardware real. Detalles del primer boot:
- Pantalla amarillenta = canales R/B intercambiados → v2 usa `format = "a8b8g8r8"`
- Texto blanco con "Aquaris" (model del DT) + letras rojas (por identificar, foto pendiente)
- `boot-mainline-v2.img` en `~/mainline/pkg/` de la Pi: colores corregidos,
  init estático que auto-rebootea a los 45 s (reboot caliente → DRAM viva →
  leer ramoops en 0xBFF00000 vía /dev/mem desde pmOS), `panic=15` (el wdt
  mt6582 de mainline registra restart handler → un panic también auto-rebootea)
- **Protocolo de iteración v2**: mantener pmOS flasheado en `bootimg` y probar
  mainline con `fastboot boot boot-mainline-vN.img` (carga en RAM, NO flashea).
  Fallo = sacar batería y arranque normal a pmOS. Coste por iteración: 1 Vol+.
- Flasheo por dd desde pmOS corriendo (sin botones): `pkg/flash_boot_dd.sh`
  (verifica magic ANDROID! en sector 60416 antes de escribir)
- Compilación: `~/mainline/linux-7.0.12`, `make O=build-krillin ARCH=arm
  CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs` (~35 min en la Pi)
- dts: `arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts`; ensamblado: `pkg/assemble.sh`
- **Siguiente (M2)**: nodo musb/phy USB en el dts (mirar bases en el árbol
  downstream — re-extraer tarball de distfiles) → gadget serie/red → SSH en mainline

### Descubrimientos clave de la tarde (2026-06-10)

1. **El LK IGNORA el cmdline del boot.img** — siempre pasa el suyo (ATAGs). Consecuencias:
   mainline necesita `ARM_ATAG_DTB_COMPAT_CMDLINE_EXTEND` (no FROM_BOOTLOADER) para que
   los bootargs del DT lleguen; downstream no puede recibir params nuevos sin recompilar
   con CONFIG_CMDLINE.
2. **Mapa de memoria real** (de /proc/iomem + FBIOGET_FSCREENINFO):
   System RAM = 0x80000000-0xBBFFFFFF + 0xBD800000-0xBF2FFFFF;
   mtkfb usa 0xBF400000 + 0x5FA000 (fb activo; la cola de la región 12MB TAMBIÉN
   lleva píxeles — NO poner ramoops dentro de 0xBF400000-0xBFFFFFFF).
   **HUECO VERIFICADO: 0xBF300000 (1 MB)** — ni System RAM ni mtkfb; los datos
   SOBREVIVEN al reboot caliente (marcador comprobado). Canal post-mortem oficial.
3. **/dev/mem con read() falla en highmem (Bad address)** — usar mmap: herramientas
   `memdump`/`memwrite`/`fbinfo` (estáticas armv7) en ~/mainline/pkg de la Pi y /tmp del teléfono.
4. **`reboot2 bootloader` FUNCIONA** (syscall RESTART2) → entrada a fastboot por software
   desde pmOS, sin botones. Binario en /tmp/reboot2 del teléfono (rehacer tras reinstalar rootfs).
5. **`fastboot boot` (arrancar desde RAM) NO lo soporta este LK** (envía OKAY y se queda).
   Protocolo real: dd-flash de bootimg desde pmOS corriendo + combo Power+Vol+ solo para recuperar.
6. **DEVMEM ya activo** en el kernel 3.10 actual (boot-pstore.img flasheado; pstore-ram
   del downstream NO sirve por el punto 1 — leer ramoops con memdump y punto).
7. v3 mainline: ramoops@0xBF300000 (DT), bootargs "panic=15 clk_ignore_unused" (con EXTEND).

### Contexto del proyecto mainline (datos de partida)

Objetivo: kernel moderno (7.x soporta armv7) → DRM + Lima (GPU) → Phosh posible.
Datos clave extraídos del kernel vivo:
- **FB físico `0xBF400000`** (región 12 MB `mtkfb.0` en /proc/iomem) → nodo `simple-framebuffer`
- Panel `hx8389_qhd_dsi_vdo_truly` — DSI **modo vídeo** (se auto-refresca → simplefb viable)
- stride 2176 (544×4), 540×960
- UART `ttyMT0` @ 921600 existe (LK lo desactiva; testpoints si hiciera falta)
- `androidboot.hardware=sprout` → plataforma **Android One MT6582** (buscar mainline previo de sprout)
- `mt6582.dtsi` existe en mainline desde 2014 (CPUs/GIC/timer/UART)
- Depuración sin cables: simplefb (consola en pantalla) + ramoops/pstore (DRAM sobrevive reboot)
- Preparado en la Pi: toolchain cross (`gcc-arm-linux-gnueabihf`), downstream en `~/mainline/downstream`, mainline 7.0.12 en `~/mainline/`
- Hitos: M1 consola simplefb → M2 USB gadget/SSH → M3 eMMC → M4 DRM+Lima → Phosh
- **Recuperación siempre**: `sudo fastboot flash boot ~/pmos-artifacts/boot-clean.img` (solo se toca `boot`)

## 7. 🗺️ Próximos pasos (en orden)

1. **[BLOQUEANTE]** Recuperar del bootloop (§1).
2. **Verificar el táctil arreglado**: tras un arranque limpio, comprobar en
   `/var/log/Xorg.0.log` que pone `Using input driver 'evdev' for 'mtk-tpd'` (ya
   no `libinput`) y que NO hay "double tracking". Probar tocar la GUI de Sxmo.
3. **Estabilizar SSH por USB**: perfil estático de NM para usb0 (§4).
4. **Servicio de refresco + despertar pantalla**: confirmar que arrancan solos
   (`/etc/local.d/98-fb-refresh.start` y `99-display-kick.start`).
5. **Expandir la raíz** a la partición grande (5,7 GB) — ahora va al 79-93%.
6. **(opcional) GPU**: blob Mali r3pX sobre el kernel 3.10 + gcompat. NO intentar
   kernel 5.2/mainline (MT6582 sin display driver mainline = inviable; confirmado
   junio 2026).
7. **Mejora real de pantalla**: parchear `mtkfb` en el kernel para salir del modo
   decouple (mataría el hack del demonio de refresco). Requiere recompilar kernel.
8. **Módem** (datos/SMS): daemon AT en userspace sobre canales CCCI del kernel.
9. **Limpiar y enviar el port** a pmaports (`device/testing/device-bq-krillin`).

---

## 8. 📁 Archivos de esta sesión (en `C:\Users\jferr\Desktop\pmos-krillin\`)

- `README.md` — documentación técnica completa del port (LÉELA).
- `CONTINUAR_AQUI.md` — este archivo.
- `aports/` (si se exportó) — copias de deviceinfo/APKBUILD/config del port.
- Scripts `.sh` — todos los usados (flasheo, ext4, táctil, refresco, NAT, etc.).

> 💡 La memoria persistente de Claude también guarda el estado en
> `project_pmos_bq_e45.md` — al abrir nueva sesión, recordará el contexto.
