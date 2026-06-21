# 🔄 CONTINUAR AQUÍ — postmarketOS en BQ Aquaris E4.5 (krillin)

> Documento de traspaso entre sesiones. Última actualización: **2026-06-10**.
> Lee esto primero, luego el `README.md` (la documentación técnica completa).

---

## ⚡ TL;DR — dónde lo dejamos

- ✅ **El port FUNCIONA**: postmarketOS arranca en el E4.5 real, con consola en
  pantalla, SSH por USB, GUI Sxmo, **pantalla táctil** y WiFi.
- 🔴 **AHORA MISMO el teléfono está en BOOTLOOP** (se queda en *"linux default
  SW Flow Control"*). Causa probable: **partición raíz ext4 corrupta** tras un
  apagado forzado en caliente. **Es recuperable** (nunca se tocó el preloader).
- ⚠️ Antes del bootloop me cerré el acceso por USB con un cambio malo de
  NetworkManager (ver §4). Hay que deshacerlo durante la recuperación.

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
| Máquina de construcción | Raspberry Pi, `ssh juan@192.168.0.112` (clave instalada) |
| Pi: contraseña sudo | `123456` (passwordless sudo activo) |
| Teléfono por USB | `ssh user@172.16.42.1` — **usuario `user`**, NO juan |
| Teléfono: contraseña | `147147` |
| IP de la Pi en usb0 | `172.16.42.2/24` (ponerla a mano tras cada reconexión) |
| Debug-shell initramfs | telnet a `172.16.42.1` puerto 23 (pone la IP sin NM) |
| Partición raíz en el tel. | `/dev/mmcblk0p5` (= partición `android`/`system`) |

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
