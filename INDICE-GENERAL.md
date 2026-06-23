# Índice general — BQ Aquaris E4.5 (`krillin`, MT6582) · Linux mainline + Phosh

> Índice navegable para retomar el proyecto. La visión general, el estado y el arranque rápido están
> en **[README.md](README.md)** — empieza por ahí. Última actualización: **2026-06-23**.

## Estado actual (resumen)
Linux **7.0.12 mainline + Alpine** en hardware real, arrancando desde **SD** (`mmcblk1p1`, boot en
sector 83968), con **Phosh acelerado (lima/Mali-400)**, **táctil**, **carga + batería%**, **PMIC
MT6323** en DT, **Bluetooth** (empareja + toggle en Phosh), **WiFi** (escanea redes reales; `.connect`
softMAC en progreso) y **GPS** (protocolo `0xAAF0` decodificado; cadena a Phosh validada).

| Subsistema | Estado | Dónde |
|---|---|---|
| Boot mainline + Alpine (desde SD) | ✅ | [mainline/HITOS.md](mainline/HITOS.md) §1, §5 |
| eMMC / USB-gadget / SSH | ✅ | [HITOS.md](mainline/HITOS.md) §2, §3 |
| **Display DRM** (mediatek-drm, color OK) | ✅ | [HITOS.md](mainline/HITOS.md) §4, §13, §19 · `mainline/disp-drm/` |
| **GPU lima / Mali-400** | ✅ | [HITOS.md](mainline/HITOS.md) §18 |
| **Phosh** (Wayland) | ✅ | [HITOS.md](mainline/HITOS.md) §14, §15 · `mainline/phosh/` |
| **Dual-boot SD** (pmOS-SD / Android-interna) | ✅ | [mainline/DUAL-BOOT-SD.md](mainline/DUAL-BOOT-SD.md) |
| Táctil FT5336 + EINT + botones | ✅ (Vol↑ pend.) | [HITOS.md](mainline/HITOS.md) §6, §9, §16, §17 |
| Carga FAN5405 + Batería % | ✅ | [HITOS.md](mainline/HITOS.md) §11, §12 |
| PMIC MT6323 (hub) | ✅ | [HITOS.md](mainline/HITOS.md) §7 |
| **Bluetooth** (hci0 + BlueZ) | ✅ empareja | [HITOS.md](mainline/HITOS.md) §19 · [HITO-WIFI-M3A](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md) |
| **WiFi — scan** (`wlan0`, cfg80211) | ✅ | [HITOS.md](mainline/HITOS.md) §19 · `mainline/wifi-consys/wifi/` |
| **WiFi — connect / WPA2 / data-path** | 🟡 en progreso | [FASE2-CONNECT](mainline/wifi-consys/wifi/FASE2-CONNECT.md) · [WIFI-ROADMAP](mainline/wifi-consys/wifi/WIFI-ROADMAP.md) |
| **GPS** (protocolo `0xAAF0`) | 🟡 falta arrancar el motor | [GPS-CONSYS](mainline/wifi-consys/gps/GPS-CONSYS.md) |
| Brillo · botones power/vol · sensores · audio · FM · módem | ⬜ | [ROADMAP-FINAL.md](ROADMAP-FINAL.md) (F3–F5) |

## Documentos de la raíz
- **[README.md](README.md)** — entrada: estado por subsistema, estructura del repo, cómo trabajar, recuperación, índice de docs.
- **[ROADMAP-FINAL.md](ROADMAP-FINAL.md)** — **roadmap maestro F0–F5 + TODO** (documento único de plan; supersede los roadmaps dispersos).
- **[CONTINUAR-AQUI.md](CONTINUAR-AQUI.md)** — estado de la última sesión + próximos pasos inmediatos.
- **[GUIA-MTKCLIENT.md](GUIA-MTKCLIENT.md)** — recuperación por BROM con mtkclient (red de seguridad).
- **[ESTRATEGIA-CUSTOM-BOOTLOADER.md](ESTRATEGIA-CUSTOM-BOOTLOADER.md)** — notas sobre el LK / bootloader / dual-boot.
- **[PLAN-MAESTRO-IMPLEMENTACION.md](PLAN-MAESTRO-IMPLEMENTACION.md)** — plan de implementación de los drivers que faltan.
- **[README-PMOS-3.10.md](README-PMOS-3.10.md)** — el port PREVIO sobre kernel 3.10 (Sxmo/X11), archivado (no es la vía actual).

## El port mainline (`mainline/`)
- **[mainline/HITOS.md](mainline/HITOS.md)** — **historia unificada**: un solo doc con todos los hitos (causa raíz, registros, secuencias y ficheros). Sustituye a los antiguos `HITO-*.md` sueltos.
- **[mainline/README.md](mainline/README.md)** — cómo compilar / flashear / depurar el kernel mainline.
- **[mainline/DUAL-BOOT-SD.md](mainline/DUAL-BOOT-SD.md)** — pmOS en SD + Android en la interna.
- `mainline/dts/` — el DeviceTree del krillin (`mt6582-bq-krillin.dts`).
- `mainline/drivers/` — drivers del kernel (`mt6582-*.c`: btif, wifi, mfg-power, …).
- `mainline/disp-drm/` — display DRM (mtk_drm) + mapeo de registros DISP.
- `mainline/userspace/` · `mainline/rootfs/` — bridges/daemons + configs de userspace (battery, charge, bluetooth, x11, sshd).
- `mainline/phosh/` · `mainline/quickwins/` — stack Phosh + apps.
- `mainline/pkg/` — ensamblado del boot.img (`assemble.sh`, `mtk_hdr.py`, cfgs).
- `mainline/estrategia/` · `mainline/docs/` — estrategia de drivers + referencias técnicas únicas.

## CONSYS — la frontera (`mainline/wifi-consys/`)
El combo WiFi/BT/GPS/FM del MT6582, sin soporte mainline. Levantado por un driver propio.
- **[m3a/HITO-WIFI-M3A.md](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md)** — bring-up completo del CONSYS (BTIF-DMA, patch, func_on, Bluetooth) + scripts + captura.
- **[m3a/SECUENCIA-ARRANQUE-CONSYS.md](mainline/wifi-consys/m3a/SECUENCIA-ARRANQUE-CONSYS.md)** — la secuencia de arranque del chip.
- `wifi/` — WiFi 802.11: `mt6582-wifi.c` + `mt6582-wifi-reg.h`; docs [WIFI-DESIGN](mainline/wifi-consys/wifi/WIFI-DESIGN.md), [WIFI-ROADMAP](mainline/wifi-consys/wifi/WIFI-ROADMAP.md), [FASE2-CONNECT](mainline/wifi-consys/wifi/FASE2-CONNECT.md).
- `gps/` — bridge GPS (`mtk-gps-bridge.c`, protocolo `0xAAF0`); doc [GPS-CONSYS](mainline/wifi-consys/gps/GPS-CONSYS.md).

## Lo antiguo / archivado
- `legacy/` — scripts y docs de sesiones tempranas (`docs-superseded/`, `scripts-early/`).
- `aports/` · `device-bq-krillin/` · `linux-bq-krillin/` — el path ANTIGUO (pmbootstrap, kernel 3.10 downstream).
- `artifacts/` — backups golden (BROM) + firmware stock (no en git; los `.img` están en `.gitignore`).

## Entorno / acceso (para casa)
- **Pi de build** (todo el trabajo real vive aquí): `ssh cpcd@192.168.0.123` (sudo NOPASSWD).
  Kernel `~/mainline/linux-7.0.12` (build `O=build-krillin`). Imágenes/backups en `~/mainline/pkg/`.
- **Teléfono mainline desde la Pi**: `ssh root@172.16.42.1`. Red USB: la Pi es `172.16.42.2/24` en `usb0`
  (`sudo ip addr replace 172.16.42.2/24 dev usb0`).
- **Build + flash del kernel**: editar en la Pi → `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs`
  → empaquetar (`mtk_hdr.py` + `abootimg`) → `dd` a sector **83968** **desde pmOS viva**, o `fastboot flash boot boot-*-sd.img`.
- **Disparar el CONSYS** (BT/WiFi/GPS): `echo 1 > /sys/kernel/debug/mt6582_wifi/bringup`.
- ⚠️ Repo (Mac) y fuente de build (Pi) pueden DIVERGIR — sincronizar con `scp` + comparar `md5`.
- ⚠️ No martillear SSH anidado Pi→teléfono.

## ⚠️ Reglas de seguridad (CRÍTICO)
1. Softbrick → SP Flash Tool **"Download Only"**, **NUNCA "Format All"** (borra IMEI/NVRAM).
2. **NUNCA** restaurar el LK de **Lollipop** (verifica firma → bootloop). El bueno es **KitKat 1.5.2**.
3. `mtkclient`: **`wo <off> <len> <img>`** (offsets HEX, boot = `0x2900000`), **NUNCA `wf`** (machaca el sector 0).
4. Fastboot: **`fastboot flash boot`**, **NUNCA `fastboot boot`**. Entrar en fastboot = manual (Power ~10 s → Power + Vol↑).
5. **NUNCA** flashear `preloader`.
6. **`/data` (Android) = `mmcblk0p7`**. Con dual-boot, **pmOS vive en la SD**; la interna es para Android.
