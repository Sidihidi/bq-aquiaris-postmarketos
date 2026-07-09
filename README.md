# BQ Aquaris E4.5 (`krillin`) — Linux MAINLINE en un MediaTek MT6582 de 2014

Reviviendo el **BQ Aquaris E4.5** (MediaTek MT6582, Cortex-A7 ×4 armv7, Mali-400 MP2, 1 GB RAM,
540×960; *el primer Ubuntu Phone, 2014*) con **Linux mainline 7.0.12**, portando desde cero los
drivers que upstream nunca tuvo para este SoC.

> **Estado (2026-07-09):** un teléfono de 2014 con **Linux mainline 7.0.12 + Alpine/Phosh**
> completamente funcional: WiFi WPA2+DHCP, Bluetooth, **Radio FM que suena**, audio, vídeo,
> sensores, suspend profundo, y **menú multiboot** que elige entre pmOS, Android y Maemo Leste.
> GPS y módem en investigación.

---

## ✅ Estado por subsistema

| Subsistema | Estado | Detalle |
|---|---|---|
| Boot SMP 4×A7 | ✅ | Arranca desde SD; LK KitKat → boot.img sector 83968 |
| Display DRM + GPU | ✅ | `mediatek-drm` (DSI hx8389) + **lima / Mali-400** |
| Phosh (Wayland) | ✅ | phoc + phosh + squeekboard, GLES2/lima |
| Táctil multitouch | ✅ | `edt-ft5x06` + chunked-read (5 dedos, EINT117) |
| **WiFi WPA2+DHCP+datos** | ✅ | Port del stock `mt_wifi` = `mtk_mtwifi`. HTTPS real. |
| **Bluetooth (hci0)** | ✅ | Empareja + toggle. Vía CONSYS/BTIF. |
| **Radio FM** | ✅ | **¡SUENA!** Port del stock MT6627 + app GTK4. `/dev/fm` + I2S analógico. |
| **Audio completo** | ✅ | `aplay`/mpv/PulseAudio. Altavoz + auriculares. AFE MT6323. |
| Batería % + carga | ✅ | VBAT por AUXADC (hwmon kernel) + FAN5405. |
| Botón power | ✅ | Toggle INT_SEL + daemon helper (2 toques tras idle). |
| Auto-rotación + ALS | ✅ | LSM330 + TMD2772 por IIO + iio-sensor-proxy |
| Vibración + LEDs RGB | ✅ | regulator-haptic + leds-mt6323 + feedbackd |
| Brillo (slider) | ✅ | Phosh → logind → sysfs → daemon PWM |
| Vídeo (YouTube) | ✅ | mpv/Livi h264 360p fluido, lima/GLES2 |
| Suspend | ✅ | s2idle + autosuspend híbrido + SPM M3 (deep sleep CPU0) |
| **Menú multiboot** | ✅ | Selección visual: pmOS / Android / Maemo Leste (Vol+/- + Power) |
| USB gadget + SSH | ✅ | g_ether (mt6582-musb) + SSH por USB y WiFi |
| **GPS** | 🟡 | Fase A: runner libmnl enlaza. Falta Fase B (HW). |
| **Magnetómetro** | 🟡 | MMC3516x driver IIO compila. Falta flash de validación. |
| **Thermal** | 🟡 | auxadc_thermal con entrada mt6582 compila. Falta flash. |
| **Accdet (jack)** | 🟡 | mt6323-accdet compila. Botones inline pendientes. |
| **Módem 2G/3G** | 🔬 | M1 (arrancar baseband) = GO. Moonshot completo. |
| **Cámara** | ❌ | NO-GO: HAL cerrada imprescindible, kernel es passthrough. |
| **Android 13** | ❌ | NO-GO: RAM (1<2GB) + armv7 deprecado. |

---

## 🗺️ Estructura del repo

```
mainline/
├── drivers/
│   ├── done/           ← ✅ Drivers terminados y probados en HW
│   │   ├── wifi/       (mt6582-wifi + mt_wifi_port stock + mtk-pmic-keys)
│   │   ├── bt/         (mt6582-btif)
│   │   ├── fm/         (fm_glue + shims + app GTK4 + groundtruth)
│   │   ├── audio/      (afe-pcm + codec-sequence)
│   │   ├── battery/    (mt6323-auxadc hwmon)
│   │   ├── spm/        (mt6582-spm + suspend)
│   │   └── sensors/    (LSM330 + TMD2772)
│   ├── wip/            ← 🟡 En progreso
│   │   ├── powerkey/   (mtk-pmic-keys: 2 toques tras idle)
│   │   ├── gps/        (Fase A: runner libmnl enlaza)
│   │   ├── magnetometer/ (MMC3516x: compila)
│   │   ├── thermal/    (auxadc_thermal: compila)
│   │   ├── accdet/     (mt6323-accdet: compila)
│   │   └── modem/      (CCCI: M1 GO, moonshot)
│   └── nogo/           ← ❌ Descartados (cámara, Android 13)
├── bootmenu/           ← Menú multiboot (menupick.c + init-menupick.sh)
├── maemo-leste/        ← Port de Maemo Leste (rootfs construido)
├── pmos/               ← Configs/scripts del rootfs pmOS
├── investigations/     ← Estudios de viabilidad (bootloader, Android, etc.)
├── dts/                ← DeviceTree canónico del krillin
└── userspace/          ← Daemons y configs de userspace (backlight, autosuspend, etc.)

docs/
├── bitacora/           ← Diario de sesiones (ordenado por fecha)
└── archive/            ← Documentación histórica superada

fw-analysis/            ← Firmware NDS32 descompilado (Ghidra)
```

---

## 🔀 Multiboot (3 SOs)

El móvil tiene un **menú de arranque visual** (navegable con Vol+/- y Power):

| Opción | SO | Dónde | Cómo arranca |
|---|---|---|---|
| 0 | **postmarketOS** | SD `mmcblk1p1` | switch_root a Alpine |
| 1 | **Android** (LineageOS 13) | eMMC (intacto) | kexec del kernel 3.10 |
| 2 | **Maemo Leste** | SD `mmcblk1p3` | switch_root a Devuan |

El boot.img del menú va en el **sector 83968 del eMMC** (fijo, no se toca).
Ver [cómo flashear](mainline/bootmenu/FLASHEO-MULTIBOOT.md).

---

## 🔧 Cómo se trabaja

**Flujo:** editar → compilar (cross armhf) en la Pi → flashear → verificar en HW.

- **Pi de build** — `cpcd@192.168.0.38` (Debian trixie, sudo NOPASSWD). Árbol: `~/mainline/linux-7.0.12`
- **Teléfono** (pmOS) — `ssh root@172.16.42.1` (USB) o `ssh root@192.168.0.x` (WiFi)
- **Build kernel**: `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 zImage dtbs`
- **⚠️ MULTIBOOT**: el sector 83968 tiene el boot.img del MENÚ. NO flashear boot.img de pmOS allí.
  Ver [FLASHEO-MULTIBOOT.md](mainline/bootmenu/FLASHEO-MULTIBOOT.md).
- **Reglas de oro MTK**: NUNCA flashear preloader. NUNCA restaurar LK Lollipop (bueno = KitKat 1.5.2).
  sysrq reboot deja el PMIC en mal estado → power-cycle físico.

---

## 📓 Documentación

- [Bitácora de sesiones](docs/bitacora/README.md) — qué se hizo y cómo seguir, por sesión
- [Estado del proyecto](mainline/ESTADO-PROYECTO-0709.md) — coordinación entre sesiones paralelas
- [Historia de hitos](mainline/HITOS.md) — cronología completa del proyecto
- [Estudios de viabilidad](mainline/investigations/) — bootloader, Android 13, cámara/módem/FM

---

*Proyecto de aficionado, bring-up en hardware real. Histórico completo en [HITOS.md](mainline/HITOS.md).*
