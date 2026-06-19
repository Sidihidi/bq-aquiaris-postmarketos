# BQ Aquaris E4.5 (`krillin`) — Linux MAINLINE en un MediaTek MT6582 de 2014

Reviviendo el **BQ Aquaris E4.5** (MediaTek MT6582, Cortex-A7 ×4 armv7, Mali-400 MP2, 1 GB RAM,
540×960; *el primer Ubuntu Phone, 2014*) con **Linux mainline moderno**, escribiendo y portando
los drivers que upstream nunca tuvo para este SoC.

> **Estado (2026-06-19):** un teléfono de 2014 corriendo **Linux mainline 7.0.12 + Alpine** con
> **Phosh acelerado por GPU**, **dual-boot pmOS(SD)/Android(interna)**, y **Bluetooth funcionando**
> (escanea y encuentra dispositivos reales) — además de display DRM, táctil, carga, batería y PMIC.

📍 **Entra por aquí** (este README) y para el detalle por subsistema, [INDICE-GENERAL.md](INDICE-GENERAL.md).
Repo: [github.com/Sidihidi/bq-aquiaris-postmarketos](https://github.com/Sidihidi/bq-aquiaris-postmarketos).

---

## ✅ Qué FUNCIONA (Linux 7.0.12 mainline + Alpine 3.24)

| Subsistema | Estado | Cómo |
|---|---|---|
| Arranque kernel (SMP 4 CPUs) | ✅ | boot.img MTK; el LK carga zImage+dtb |
| **Display DRM** (color OK) | ✅ | `mtk_drm` pinta el panel HX8389 (DSI), card1=mediatek-drm |
| **GPU acelerada** | ✅ | **lima / Mali-400** (`mt6582-mfg-power.c` enciende el MFG por SPM) |
| **Phosh** (Wayland, fluido) | ✅ | phoc+phosh+squeekboard, WLR_RENDERER=gles2 |
| **Dual-boot SD** | ✅ | **pmOS en la SD** (mmcblk1) + **Android intacto** en la interna. [DUAL-BOOT-SD](mainline/DUAL-BOOT-SD.md) |
| eMMC / USB-SSH / I2C / GPIO | ✅ | `mtk-sd` + `blkdevparts`; `mt6582-musb.c`; `i2c-mt65xx` |
| Táctil (FT5336) | ✅ | VGP1 (PMIC) + I2C0@0x38 + uinput → `/dev/input/event0` |
| EINT + botones (vol) | ✅ | `gpio-mt6582-eint.c` (169 EINTs) + `mt6779-keypad` |
| Carga USB + Batería % | ✅ | FAN5405 (I2C0@0x6a) 4.2V/800mA; VBAT por AUXADC MT6323 |
| PMIC MT6323 (hub) | ✅ | pwrap + MFD + 31 reguladores. [HITO](mainline/HITO-PMIC-MT6323.md) |
| **WiFi/BT/GPS/FM — bring-up del CONSYS** | ✅ | **patch + las 4 radios encendidas** (ver abajo) |
| **Bluetooth** (hci0 + BlueZ) | ✅ | **escanea/visible/vinculable**; Phosh por D-Bus. [HITO](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md) |
| WiFi-netdev (802.11) · GPS/FM userspace | ⬜ | el CONSYS está vivo; falta el puente a userspace (ver Roadmap) |
| Audio · Módem 3G | ⬜ | módem = camino Halium |

*(También funciona una GUI X11 ligera y el port previo sobre kernel 3.10 — ver
[README-PMOS-3.10.md](README-PMOS-3.10.md). Phosh+mainline es la vía principal hoy.)*

---

## 🚀 Lo gordo de la sesión 2026-06-19 — el CONSYS (WiFi/BT/GPS/FM)

El **CONSYS MT6582** es el combo de radio (WiFi+BT+GPS+FM) que mainline nunca soportó. Lo levantamos
entero por un driver propio (`mainline/wifi-consys/`):

1. **BTIF-DMA** — transporte AP↔CONSYS en modo DMA/VFF (el bug que lo desbloqueó: el TX-DMA necesita
   un **FLUSH** para expulsar la cola parcial). El CONSYS **responde** STP/WMT.
2. **Patch download** — los 2 patches (`mt6572_82_patch_e1_*`, 62 fragmentos) descargados por WMT.
3. **func_on** — **las 4 radios encendidas** a nivel de chip (BT/FM/GPS/WiFi, status=0).
4. **Bluetooth completo** — `hci0` real (`hci_register_dev`) → **BlueZ escanea y encuentra dispositivos**.

Guía técnica viva y el blueprint completo: [mainline/wifi-consys/m3a/HITO-WIFI-M3A.md](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md).

---

## 🗺️ Mapa del repo

```
README.md              ← este (punto de entrada)
INDICE-GENERAL.md      ← índice detallado de todos los docs
mainline/              ← EL PORT MAINLINE (lo principal)
  README.md            ← cómo compilar/flashear/depurar el kernel
  HITO-*.md            ← un doc por subsistema resuelto (PMIC, EINT, táctil, carga, display…)
  DUAL-BOOT-SD.md      ← pmOS en SD + Android en interna
  disp-drm/            ← display DRM (mtk_drm) + el DTS del krillin (code/mt6582-bq-krillin.dts)
  wifi-consys/         ← WiFi/BT/GPS/FM (CONSYS) — la frontera actual
    m3a/               ← drivers (mt6582-consys.c, mt6582-btif.c) + HITO-WIFI-M3A.md + captura + scripts
  rootfs/              ← configs de userspace (battery, charge, x11, sshd)
  pkg/                 ← ensamblado del boot.img (assemble.sh, mtk_hdr.py)
artifacts/             ← backups golden (BROM) + firmware stock (no en git por privacidad)
device-bq-krillin/, aports/  ← aports de pmbootstrap (port 3.10)
legacy/                ← scripts/docs de sesiones tempranas (3.10), archivados
```

---

## 🔧 Cómo se trabaja (para retomar en casa)

- **Raspberry Pi** `ssh cpcd@192.168.0.123` (sudo NOPASSWD) — compila (cross armhf) + habla con el
  teléfono por USB. Árbol del kernel: `~/mainline/linux-7.0.12` (build dir `O=build-krillin`).
- **Teléfono** (pmOS mainline, desde la Pi): `ssh root@172.16.42.1`. Red USB: la Pi es
  `172.16.42.2/24` en `usb0` (`sudo ip addr replace 172.16.42.2/24 dev usb0`).
- **Ciclo de iteración rápida** (scripts en la Pi, copia en `mainline/wifi-consys/m3a/scripts/`):
  - `~/wifi-iter.sh` = build zImage + empaqueta + **flashea por dd** (sector 83968 = 0x2900000, la
    partición boot real) + reinicia. Sin fastboot.
  - `~/wifi-check.sh` / `~/wifi-bringup.sh` = espera el arranque y dispara/lee el bring-up del CONSYS
    (`echo 1 > /sys/kernel/debug/mt6582_btif/bringup`).
- **Imágenes** en `~/mainline/pkg/`: `boot-pmos-sd.img` (pmOS desde SD), `boot-color1.img` (pmOS
  interna), `boot-btifDMA-sd.img` (el de WiFi/BT actual).

> ⚠️ No martillear SSH anidado Pi→teléfono (satura el sshd / el musb). Agrupar comandos.

---

## 🛟 Recuperación — difícil de brickear

- **Regla de oro MTK:** NUNCA flashear `preloader`. Lo demás es recuperable.
- **fastboot** (en la Pi): `fastboot flash boot boot-color1.img` → pmOS de vuelta (p7 intacto).
- **mtkclient/BROM** (red de seguridad): backup golden en `artifacts/golden/`. Guía:
  [GUIA-MTKCLIENT.md](GUIA-MTKCLIENT.md) — usar `wo <off> <len> <img>` (offsets hex, boot=0x2900000).
- **Softbrick** → SP Flash Tool "Download Only" KitKat (NUNCA "Format All": borra IMEI/NVRAM).

---

## ▶️ Roadmap (lo que queda)

1. **Bluetooth — pulido**: bdaddr real de NVRAM + auto-bring-up al arranque + el panel de Phosh.
2. **GPS** (`/dev/stpgps`→gpsd) y **FM** — **mismo patrón que el BT**, casi gratis.
3. **WiFi de verdad** — el netdev 802.11 (HifAhbProbe + WIFI_RAM_CODE): ~133K líneas, el gigante.
4. **Menú de boot** (boot/recovery por combo del LK) para elegir pmOS-SD vs Android sin reflashear.
5. **Audio**; **Módem 3G** (Halium sobre 3.10).

---

*Proyecto de aficionado, bring-up en hardware real. El historial completo está en los `HITO-*.md`
y en [INDICE-GENERAL.md](INDICE-GENERAL.md).*
