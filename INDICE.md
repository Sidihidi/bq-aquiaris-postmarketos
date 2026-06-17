# 📍 ÍNDICE MAESTRO — postmarketOS + mainline en BQ Aquaris E4.5 (krillin)

> Punto de entrada único. Última actualización: **2026-06-17 (tarde)**.
> Proyecto: revivir el BQ Aquaris E4.5 (MediaTek MT6582, 2014) con Linux moderno.

---

## 🎯 ESTADO ACTUAL (2026-06-17 tarde)

**Sistema principal = Linux MAINLINE 7.0.12 + Alpine 3.24, kernel #22.** Funciona en hardware:

| Tema | Estado |
|---|---|
| Arranque SMP, display color, eMMC, USB-gadget+**SSH**, I2C/GPIO | ✅ |
| **Táctil** FT5336 (userspace→event0) + **GUI X11** (jwm) | ✅ |
| **Carga** (daemon FAN5405) | ✅ |
| 🆕 **PMIC MT6323** al DT (pwrap + MFD + **31 reguladores**, vgp1@2.8V) | ✅ `mainline/HITO-PMIC-MT6323.md` |
| 🆕 **Batería %** (`battery`: VBAT por AUXADC, ~3.7V) | ✅ `mainline/HITO-BATERIA-WIP.md` |
| 🆕 **sshd al boot** (levanta solo) | ✅ `mainline/rootfs/sshd/` |
| **Bootloader** | 🔓 DESBLOQUEADO; golden backup completo; TWRP 3.0.2.0 |
| **Recuperación** | fastboot (va) + BROM/mtkclient (`wo`, NO `wf`) |

**Próximo paso inmediato: EINT** (driver gpio+eint MT6582) — investigación completa y plan en
**`mainline/HITO-EINT-PLAN.md`**. Desbloquea táctil-por-IRQ + botones + driver de kernel del táctil.
Luego: WiFi, Audio, ruta a Phosh (simpledrm→lima).

---

## 📚 DOCUMENTOS (leer en este orden)

1. **[INDICE.md](INDICE.md)** — este archivo (estado + mapa).
2. **[TRASPASO-SESION.md](TRASPASO-SESION.md)** — traspaso entre sesiones/máquinas.
3. **[CONTINUAR_AQUI.md](CONTINUAR_AQUI.md)** — historia, accesos, recuperación, hoja de ruta.
4. **[README.md](README.md)** — documentación técnica completa del port pmOS (kernel 3.10).
5. **[ESTRATEGIA-CUSTOM-BOOTLOADER.md](ESTRATEGIA-CUSTOM-BOOTLOADER.md)** — root/unlock/BROM, dual-boot LK, bootloader propio.
6. **[GUIA-MTKCLIENT.md](GUIA-MTKCLIENT.md)** — BROM paso a paso, golden backup, seccfg, flasheo.
7. **[ROADMAP-DRIVERS.md](ROADMAP-DRIVERS.md)** — 🗺️ roadmap priorizado de drivers (PMIC/batería/carga, GUI X11, ruta a Phosh: simpledrm→lima→wlroots, WiFi, audio, EINT, sensores, módem) con dependencias, fases y quick-wins. Síntesis de 12 informes de investigación.
8. **[mainline/README.md](mainline/README.md)** — compilar/flashear/depurar el kernel mainline.
8. **[mainline/HITO-M2b-WIP.md](mainline/HITO-M2b-WIP.md)** — driver USB MUSB del MT6582 (WIP).

## 🗂️ CARPETAS

| Carpeta | Qué hay |
|---|---|
| `aports/` | Paquetes pmOS: `device-bq-krillin` + `linux-bq-krillin` (kernel 3.10.107, config). |
| `mainline/` | Bringup Linux 7.x: `dts/`, `patches/`, `pkg/` (herramientas MTK), `drivers-musb/`, configs, dmesg de cada iteración. |
| `scripts/` | `capa0-backup.sh` (golden backup) + los ~55 scripts de sesión (flasheo, ext4, táctil, NAT, refresco). |
| `artifacts/` | Imágenes: `pmos-root.img`, `boot-*.img`, **`golden/`** (backup de particiones). ⚠️ `*.img` están gitignored (no van a GitHub; `nvram` lleva IDs únicos). |

## 🔌 ACCESOS

| Qué | Valor |
|---|---|
| Pi de construcción | `ssh cpcd@192.168.0.123` (clave del Mac; sudo sin contraseña). **Raspberry Pi 5.** |
| Teléfono por USB (pmOS) | `ssh user@172.16.42.1`, pass `147147`. IP Pi: `172.16.42.2/24`. |
| GitHub | https://github.com/Sidihidi/bq-aquiaris-postmarketos |
| Móvil | unlocked, secure boot off, serial JB053237 / ME_ID 4040D87ADB20ECB4CCF07147B1F82E46 |

## ⚡ HECHOS CLAVE / TRAMPAS (no repetir)

- **El LK ignora el cmdline del boot.img** → params por bootargs del DT (mainline: `CMDLINE_EXTEND`).
- **mtkclient no lee la GPT** (bug MBR MT6582) → volcar flash + recortar; **offset real = dumchar + 0xB80000**.
- **mtkclient en Pi 5**: interactivo + reconectar cuando pida "please reconnect to brom mode".
- **Recuperación siempre**: `fastboot flash boot artifacts/boot-pstore.img` (pmOS) o `mtk w boot golden/boot.img`.
- **Kernel 3.10**: NUNCA escribir a `/sys/class/graphics/fb0/blank` (pánico). `pan` es seguro.
- **fastboot NO** puede con imágenes grandes (rootfs) en este LK → canal nc por el debug-shell.
- **Scripting**: `$(())` y `{}` se rompen en zsh(Mac)→ssh→bash; usar heredoc `<<'EOF'`.

---

## ▶️ REINSTALAR pmOS (procedimiento)

```sh
# (la raíz pmos-root.img ya está en la Pi: ~/pmos-artifacts/pmos-root.img)
# 1) móvil en fastboot. En la Pi: forzar el debug-shell del initramfs
sudo fastboot flash boot ~/pmos-artifacts/boot-debug.img && sudo fastboot reboot
# 2) esperar al debug-shell (telnet en 172.16.42.1) y dar IP en la Pi
sudo ip addr add 172.16.42.2/24 dev usb0
# 3) transferir la raíz a p5 por nc (receptor en el tel + envío):
{ echo 'nc -l -p 9999 | dd of=/dev/mmcblk0p5 bs=1M'; sleep 3; } | telnet 172.16.42.1 &
nc -N 172.16.42.1 9999 < ~/pmos-artifacts/pmos-root.img
# 4) arranque normal de pmOS:
sudo fastboot flash boot ~/pmos-artifacts/boot-pstore.img && sudo fastboot reboot
# 5) re-aplicar Sxmo + táctil + refresco (ver scripts/ e install_sxmo.sh)
```
