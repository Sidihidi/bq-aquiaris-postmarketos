# Índice general — BQ Aquaris E4.5 (`krillin`, MT6582) · Linux mainline + Phosh

> Índice detallado para retomar el proyecto. La visión general, el estado y el arranque rápido
> están en **[README.md](README.md)** — empieza por ahí. Última actualización: **2026-06-19**.

## Estado actual (resumen)
Linux **7.0.12 mainline + Alpine** en hardware real, con **Phosh acelerado (lima/Mali-400)**,
**dual-boot pmOS-SD/Android-interna**, y el **CONSYS (WiFi/BT/GPS/FM) levantado** — **Bluetooth
funcionando** (hci0 + BlueZ, escanea/visible/vinculable).

| Subsistema | Estado | Doc |
|---|---|---|
| Boot mainline + Alpine | ✅ | [mainline/HITO-M1.md](mainline/HITO-M1.md), [HITO-ROOTFS-ALPINE](mainline/HITO-ROOTFS-ALPINE.md) |
| eMMC / USB-SSH | ✅ | [HITO-M2b-USB-RESUELTO](mainline/HITO-M2b-USB-RESUELTO.md), [HITO-M2d-SSH-RESUELTO](mainline/HITO-M2d-SSH-RESUELTO.md) |
| **Display DRM** (mtk_drm, color OK) | ✅ | [HITO-M3-DISPLAY-RESUELTO](mainline/HITO-M3-DISPLAY-RESUELTO.md), `mainline/disp-drm/` |
| **GPU lima / Mali-400** | ✅ | [HITO-LIMA-MFG](mainline/HITO-LIMA-MFG.md) |
| **Phosh** (Wayland, fluido) | ✅ | [HITO-WAYLAND](mainline/HITO-WAYLAND.md), `mainline/phosh/` |
| **Dual-boot SD** | ✅ | [DUAL-BOOT-SD](mainline/DUAL-BOOT-SD.md) |
| Táctil FT5336 + EINT + botones | ✅ | [HITO-TOUCH-DRIVER](mainline/HITO-TOUCH-DRIVER.md), [HITO-EINT-RESUELTO](mainline/HITO-EINT-RESUELTO.md), [HITO-KEYPAD](mainline/HITO-KEYPAD.md) |
| Carga FAN5405 + Batería % | ✅ | [HITO-CARGA-FAN5405](mainline/HITO-CARGA-FAN5405.md), [HITO-BATERIA-WIP](mainline/HITO-BATERIA-WIP.md) |
| PMIC MT6323 (hub) | ✅ | [HITO-PMIC-MT6323](mainline/HITO-PMIC-MT6323.md) |
| **CONSYS WiFi/BT/GPS/FM** | ✅ bring-up | [HITO-WIFI-CONSYS](mainline/HITO-WIFI-CONSYS.md) + **[HITO-WIFI-M3A](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md)** |
| **Bluetooth** (hci0 + BlueZ) | ✅ funciona | [HITO-WIFI-M3A](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md) |
| WiFi-netdev (802.11) · GPS/FM userspace · Audio · Módem | ⬜ | [ROADMAP-DRIVERS](ROADMAP-DRIVERS.md) |

## Dónde está cada cosa (docs clave)
- **[README.md](README.md)** — entrada: qué es, estado, mapa del repo, cómo trabajar, recuperación.
- **[mainline/README.md](mainline/README.md)** — cómo compilar/flashear/depurar el kernel mainline.
- **[mainline/wifi-consys/m3a/HITO-WIFI-M3A.md](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md)** — la frontera:
  el bring-up completo del CONSYS (BTIF-DMA, patch, func_on, Bluetooth) + blueprint + scripts.
- **[ROADMAP-DRIVERS.md](ROADMAP-DRIVERS.md)** — lo que queda (WiFi 802.11, GPS/FM, audio, módem).
- **[GUIA-MTKCLIENT.md](GUIA-MTKCLIENT.md)** / **[ESTRATEGIA-CUSTOM-BOOTLOADER.md](ESTRATEGIA-CUSTOM-BOOTLOADER.md)** — BROM/recuperación/bootloader.
- **[README-PMOS-3.10.md](README-PMOS-3.10.md)** — el port previo sobre kernel 3.10 (Sxmo/X11).
- `mainline/HITO-*.md` — un doc por subsistema (ver tabla arriba).
- `legacy/` — scripts y docs de sesiones tempranas (3.10), archivados (no son la vía actual).

## Entorno / acceso (para casa)
- **Pi de build** (todo el trabajo real vive aquí): `ssh cpcd@192.168.0.123` (pass `cpcdupct`, sudo NOPASSWD).
  Kernel `~/mainline/linux-7.0.12` (build `O=build-krillin`). Imágenes/backups `~/mainline/pkg/`.
  Scripts de iteración WiFi: `~/wifi-iter.sh`, `~/wifi-bringup.sh` (copia en `mainline/wifi-consys/m3a/scripts/`).
- **Teléfono mainline desde la Pi**: `ssh root@172.16.42.1`. Red USB: Pi = `172.16.42.2/24` en `usb0`.
- **Flasheo rápido sin fastboot**: `wifi-iter.sh` hace `dd` a `/dev/mmcblk0` sector **83968 (0x2900000)**
  (la partición boot real; el offset 0x1D80000 del dumchar es erróneo, el real = +0xB80000).
- ⚠️ No martillear SSH anidado Pi→teléfono.

## ⚠️ Reglas de seguridad (CRÍTICO)
1. Softbrick → SP Flash Tool **"Download Only"**, **NUNCA "Format All"** (borra IMEI/NVRAM).
2. **NUNCA** restaurar el LK de **Lollipop** (verifica firma → bootloop). El bueno es **KitKat 1.5.2**.
3. `mtkclient`: **`wo <off> <len> <img>`**, **NUNCA `wf`** (machaca el sector 0).
4. Fastboot: **`fastboot flash boot`**, **NUNCA `fastboot boot`**. Recuperar pmOS = `fastboot flash boot boot-color1.img`.
5. Entrar en fastboot = manual: Power ~10s → **Power + Vol↑**.
6. **`/data` (Android) = mmcblk0p7**. Con dual-boot, pmOS vive en la **SD**; la interna es para Android.
