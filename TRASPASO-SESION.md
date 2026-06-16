# 🔄 TRASPASO DE SESIÓN — continuar en casa

> Lee esto primero, luego **[INDICE.md](INDICE.md)** (mapa + accesos).
> Última sesión: **2026-06-16**. Antes: 2026-06-10 (ver git history).

---

## 🟢 ESTADO AL CIERRE (2026-06-16)

El teléfono está **sano y arranca Android stock**. Posición inmejorable:

- 🔓 **Bootloader DESBLOQUEADO** (`unlocked: yes`, `secure: no`). Kernels custom
  por fastboot **sin TWRP/ROM/seccfg-unlock**. (El miedo del seccfg era infundado.)
- 🛟 **Golden backup COMPLETO** (incluye preloader): se restaura cualquier brick.
- 💿 **Firmware stock 2.1.0 + scatter** (mapa exacto de particiones).
- 🔧 **TWRP 3.0.2.0 instalado y PERMANENTE** en `recovery` (Android ya no lo borra).
- 📱 **Android stock arranca** (sistema en p5).
- 🐧 **mainline Linux 7.0.12**: M1 (arranca+consola) y M2 (eMMC+ext4) ✅; M2b
  (driver USB MUSB propio) escrito y compila, llegó a v25. Pendiente probar USB + M3.
- ⏸️ **pmOS**: la raíz en p5 fue sobrescrita por Android → reinstalar (ver abajo).

**Nada urgente ni roto.** Todo lo de abajo es opcional y con red de seguridad.

---

## 📍 DÓNDE ESTÁ CADA ARTEFACTO

**Pi de construcción** — `ssh cpcd@192.168.0.123` (Raspberry Pi 5; clave del Mac; sudo sin pass):
| Ruta | Qué |
|---|---|
| `~/golden/` | Backup de particiones: preloader, boot, recovery, seccfg, uboot(lk), nvram, proinfo, protect_f/s (+ printgpt.txt, gettargetconfig.txt). Restaurar: ver guía. |
| `~/pmos-artifacts/` | `boot-pstore.img` (pmOS), `boot-debug.img` (debug-shell), `boot-stock-android.img`, `pmos-root.img` (raíz pmOS lista), `twrp-recovery.img`, `boot-clean.img`, `zeros4m.img`, `config-bq-krillin.armv7` |
| `~/mainline/linux-7.0.12/` | Árbol mainline compilado (build-krillin), dts, driver M2b parcheado |
| `~/mainline/pkg/` | Herramientas MTK (mtk_hdr.py, assemble.sh, flash_boot_dd.sh, memdump/etc) + `boot-mainline-v24/v25.img` |
| `~/mtkclient/` | mtkclient (funciona interactivo, ver guía) |

**Mac (este equipo)** — `/Users/cpcd/Desktop/pmos-krillin/`:
| Ruta | Qué |
|---|---|
| `artifacts/golden/` | Copia del golden backup (incl. `preloader.bin`) — **fuera de la Pi** |
| `artifacts/stock-firmware/` | scatter + lk + boot + recovery stock + `twrp-3.0.2.0-recovery.img` |
| `artifacts/` | `pmos-root.img`, `boot-*.img` |
| `~/Downloads/BQ Aquaris E4.5 ... Lollipop/` | **Firmware stock COMPLETO** (system.img 924MB, userdata, preloader, scatter, todo para SP Flash Tool) |
| `~/Downloads/TWRPv3.0.2.0/` | TWRP recovery + su scatter |

**GitHub**: https://github.com/Sidihidi/bq-aquiaris-postmarketos (docs + scatter + parches;
los `*.img`/`*.bin` NO se suben — `nvram` lleva IDs únicos. Los artefactos viven en Pi+Mac.)

---

## ⚡ HECHOS CLAVE / TRAMPAS (aprendidos a base de golpes)

1. **mtkclient SÍ funciona en la Pi 5** — pero INTERACTIVO: lanzar el comando y
   **reconectar el móvil cuando diga "please reconnect to brom mode"**. En
   background/automático da "Couldn't get device configuration" (no es hardware).
2. **Batería FUERA = el móvil cicla en preloader** (0e8d:2000), no engancha.
   Para fastboot/recovery: **batería DENTRO** + combo.
3. **mtkclient no lee la GPT** (bug MBR MT6582) → para volcar/recortar particiones,
   **offset real = offset_scatter/dumchar + 0xB80000** (verificado por firmas).
4. **TWRP "se lo traga"** = Android restaura el stock vía `/system/recovery-from-boot.p`.
   NO es el método de flasheo. Arreglo: arrancar TWRP sin pasar por Android
   (`fastboot reboot recovery`, que SÍ funciona aquí) y renombrar ese archivo a `.bak`.
5. **El LK ignora el cmdline del boot.img** → params por bootargs del DT (mainline: `CMDLINE_EXTEND`).
6. **fastboot funciona perfecto en la Pi 5** (a diferencia de BROM). Recuperación
   rápida: `fastboot flash boot ~/pmos-artifacts/boot-stock-android.img` → Android.
7. **Kernel 3.10**: NUNCA escribir a `/sys/class/graphics/fb0/blank` (pánico).
8. **Scripting**: `$(())` y `{}` se rompen en zsh(Mac)→ssh→bash; usar heredoc `<<'EOF'`
   + `iflag=skip_bytes,count_bytes` con decimales.

---

## ▶️ OPCIONES PARA CONTINUAR (sin prisa, con red de seguridad)

**A) Verificar TWRP permanente** (rápido): arrancar Android → apagar → Power+Vol+ →
debe salir TWRP (no el stock). Confirma que la desactivación funcionó.

**B) Reinstalar pmOS** (raíz lista en la Pi):
```sh
# móvil en fastboot:
sudo fastboot flash boot ~/pmos-artifacts/boot-debug.img && sudo fastboot reboot recovery
# si levanta debug-shell con red: transferir raíz a p5 por nc (ver mainline/README §)
{ echo 'nc -l -p 9999 | dd of=/dev/mmcblk0p5 bs=1M'; sleep 3; } | telnet 172.16.42.1 &
nc -N 172.16.42.1 9999 < ~/pmos-artifacts/pmos-root.img
sudo fastboot flash boot ~/pmos-artifacts/boot-pstore.img && sudo fastboot reboot
# luego re-aplicar Sxmo/táctil/refresco (scripts/ del repo)
```

**C) Dual-boot Android + pmOS** (el proyecto grande): ya tenemos recovery=TWRP.
Falta diseñar el **reparto de almacenamiento** (Android en p5/system, pmOS en una
porción de usrdata/p7 5.7 GB) usando el scatter de mapa. Es el trabajo de diseño.

**D) Seguir mainline** (M2b USB / M3 display): flashear `~/mainline/pkg/boot-mainline-v25.img`
(autocontenido, no necesita raíz). Ver `mainline/README.md` y `mainline/HITO-M2b-WIP.md`.

---

## 🔌 ACCESOS

| Qué | Valor |
|---|---|
| Pi | `ssh cpcd@192.168.0.123` (Raspberry Pi 5, sudo sin pass) |
| Teléfono USB (pmOS) | `ssh user@172.16.42.1` / pass `147147` ; IP Pi en usb0: `172.16.42.2/24` |
| Teléfono | unlocked, secure off; serial JB053237 ; ME_ID 4040D87ADB20ECB4CCF07147B1F82E46 |
| mtkclient (Pi) | `~/mtkclient/venv/bin/python ~/mtkclient/mtk.py ...` (interactivo, reconnect-on-cue) |
| GitHub | https://github.com/Sidihidi/bq-aquiaris-postmarketos |

> ⚠️ Si "casa" es otra máquina: clona el repo (docs) y entra por ssh a la Pi
> (artefactos). Las imágenes no están en GitHub a propósito.
