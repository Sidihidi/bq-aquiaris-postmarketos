# Dual-boot pmOS + Maemo Leste en UNA sola SD — VALIDADO (2026-07-10)

> El BQ krillin arranca pmOS **y** Maemo Leste desde una única tarjeta SD, con menú en pantalla.
> Validado en HW: ambos SO arrancan de forma fiable tras el init robusto v2.

## Layout de la SD (64 GB, GPT/MBR — 3 particiones)
| Part | Device | Tamaño | FS | Label | Contenido |
|------|--------|--------|-----|-------|-----------|
| 1 | `mmcblk1p1` | ~22 GB | ext4 | `pmos` | rootfs postmarketOS (Alpine + Phosh) |
| 2 | `mmcblk1p2` | ~7.5 GB | vfat | `SHARED` | datos compartidos entre ambos SO |
| 3 | `mmcblk1p3` | ~30 GB | ext4 | `maemo` | rootfs Maemo Leste (Devuan + Hildon) |

Particionado en el Mac (SD de 64 GB conectada) + poblado ext4 en la Pi clonando las dos SD originales
(pmOS de 64 GB `sde`, Maemo de 32 GB `sdf`) a las particiones destino.

## Arranque: un solo boot.img en el eMMC + menú en pantalla
- **`boot-menupick4.img`** (eMMC `mmcblk0` sector 83968): kernel mainline 7.0.12 + **initramfs con
  `menupick`** (menú gráfico en el framebuffer, Vol+/− navega, Power selecciona; auto-boot pmOS a 10s).
- El init hace `switch_root` a la partición del SO elegido: **0=pmOS → `mmcblk1p1`**, **2=Maemo →
  `mmcblk1p3`**, **1=Android → kexec de `mmcblk1p1:/boot/android-boot.img`**.

## FIX v2 (0710): init robusto — por qué antes caía a emergencia
El init v1 mandaba a pmOS al dropbear de emergencia de forma intermitente. **3 causas, todas en el check
del rootfs**, arregladas con una función `boot_rootfs()`:
1. **`[ -e /newroot/sbin/init ]` con un symlink daba FALSO NEGATIVO** — `/sbin/init` en el rootfs suele
   ser un symlink (p.ej. `→ /bin/busybox` o systemd); `-e` sigue el enlace contra el root del *initramfs*
   (donde el target no existe) → "no hay init" → emergencia espuria. **Fix:** aceptar `-e` **o** `-L`
   (symlink) **o** `/newroot/init`.
2. **Sin reintento de mount** — la SD a veces tarda en asentar / el ext4 tiene el journal sucio tras un
   apagado abrupto. **Fix:** 3 reintentos por opción + fallback `-o noload` (monta ignorando el journal)
   + `remount,rw` tras montar.
3. **Espera del block device corta.** **Fix:** hasta 25 s esperando `/dev/mmcblk1pN`.

El init v2 unifica los 3 caminos (pmOS/Android/Maemo) en `boot_rootfs()` y solo cae a EMERGENCIA
(dropbear en usb0 172.16.42.1) si de verdad no hay rootfs arrancable. Fuente: `init-menupick.sh`.

## Receta de flasheo (por ssh anidado Pi→móvil)
El boot.img se transfiere **en base64** (el PTY del ssh anidado corrompe binario crudo) + verificación
md5 origen→móvil→readback antes de dar por bueno el `dd`:
```
base64 boot-menupick4.img | ssh root@movil 'base64 -d > /tmp/m.img'
# verificar md5, luego:
dd if=/tmp/m.img of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync ; sync
# readback: dd skip=83968 count=<secs> | md5sum  == origen
```
GOTCHA: al alternar entre el dropbear de emergencia y pmOS, la host-key ssh del móvil cambia → limpiar
con `ssh-keygen -R 172.16.42.1` en la Pi (los warnings no bloquean el comando, pero ensucian el log).

## Estado
- ✅ **pmOS arranca fiable** (entrada 0 / auto).
- ✅ **Maemo Leste arranca** (entrada 2 → `mmcblk1p3`).
- ⏳ **SIGUIENTE: que Maemo bootee BIEN del todo** (llegar a Hildon con display/táctil/servicios) — el
  `switch_root` funciona; falta pulir el arranque del rootfs de Maemo en sí.

## FIX v3 + aliases mmc (0710 tarde): la flakiness REAL era un swap de controladores
Con el init v2 seguía cayendo a emergencia a menudo ("`Can't lookup blockdev`"). El pstore del boot
fallido reveló la causa: **el orden de probe de los dos controladores MSDC es aleatorio** — a veces la
**SD enumera como `mmcblk0` y el eMMC como `mmcblk1`**. Consecuencias en un boot intercambiado:
1. `mmcblk1p1/p3` (SD esperada) no existe → emergencia tras 25 s.
2. **Peor**: el `blkdevparts=mmcblk0:...` del LK (tabla del eMMC) se aplica a la **SD**, enmascarando
   su tabla real con particiones falsas (`p1(ebr1) p2(protect_f)...`) → riesgo de corrupción.

**Fix doble (boot-menupick7.img, md5 `fe292daf…`):**
- **DTS `aliases { mmc0 = &mmc0; mmc1 = &mmc1; }`** — clava eMMC=mmcblk0 y SD=mmcblk1 (el core mmc
  respeta los aliases del DT). Esta es la solución de fondo.
- **init v3**: `resolve_dev()` resuelve el rootfs por **LABEL** (`findfs LABEL=pmos|maemo`, el busybox
  del initrd trae `findfs`/`blkid`) con fallback al device fijo. Cinturón y tirantes.

*Sesión Mac (Fable 5), 2026-07-10. Ambos SO arrancan desde una SD; v2 robusteció el mount, v3+aliases
matan la causa raíz de las caídas a emergencia.*
