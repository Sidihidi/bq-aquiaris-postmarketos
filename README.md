# BQ Aquaris E4.5 (`krillin`) — Linux MAINLINE en un MediaTek MT6582 de 2014

Reviviendo el **BQ Aquaris E4.5** (MediaTek MT6582, Cortex-A7 ×4 armv7, Mali-400 MP2, 1 GB RAM,
540×960; *el primer Ubuntu Phone, 2014*) con **Linux mainline moderno** + **postmarketOS/Alpine**,
escribiendo y portando los drivers que upstream nunca tuvo para este SoC.

> **Estado (2026-06-25):** un teléfono de 2014 corriendo **Linux mainline 7.0.12 + Alpine** con
> **Phosh acelerado por GPU (lima/Mali-400)**, arrancando desde **SD**, con **táctil, carga,
> batería en la UI (% + cargando/descargando), Bluetooth (empareja), WiFi (red abierta navega) y
> GPS (protocolo decodificado)** — más **17 plugins de Phosh** y **sesión elogind activa**.

📍 **Empieza por aquí.** Para el detalle por subsistema y el historial completo:
[mainline/HITOS.md](mainline/HITOS.md). Para el plan de trabajo: [ROADMAP-FINAL.md](ROADMAP-FINAL.md).
Repo: [github.com/Sidihidi/bq-aquiaris-postmarketos](https://github.com/Sidihidi/bq-aquiaris-postmarketos).

---

## ✅ Estado por subsistema (Linux 7.0.12 mainline + Alpine)

Honesto sobre lo que funciona vs lo que está en progreso. **✅** = funciona en HW · **🟡** = parcial /
en progreso · **⬜** = no empezado.

| Subsistema | Estado | Detalle |
|---|---|---|
| **Boot mainline (SMP 4×A7)** | ✅ | Arranca desde **SD** (`mmcblk1p1`), boot en **sector 83968**. El LK (KitKat) carga zImage+dtb. |
| **Display DRM** | ✅ | `mediatek-drm` pinta el panel (DSI), `card1` = mediatek-drm. Color OK. |
| **GPU acelerada** | ✅ | **lima / Mali-400** (`mt6582-mfg-power.c` enciende el MFG por SPM). |
| **Phosh** (Wayland) | ✅ | phoc + phosh + squeekboard sobre GLES2/lima. |
| **Táctil FT5336** | ✅ | I2C0@0x38 + EINT117; alimentado por VGP1 (PMIC). |
| **PMIC MT6323** | ✅ | En el DeviceTree (pwrap + MFD + 31 reguladores) — el "hub" de rails. |
| **Batería %** | ✅ | VBAT por **AUXADC** del MT6323 (canal BATSNS) + **indicador en Phosh**: % real + **cargando/descargando** (rayo) vía `test_power`→UPower. |
| **Carga USB** | ✅ | Cargador FAN5405 (I2C0@0x6a), 4.2 V / 800 mA, con kick del watchdog. |
| **Bluetooth (hci0)** | ✅ | Empareja (probado con un S24) + **toggle en Phosh**. Vía el CONSYS. |
| **WiFi — scan** | ✅ | Escanea **redes reales** (`iw dev wlan0 scan` lista decenas de APs). cfg80211/`wlan0` registrados. |
| **WiFi — connect** | 🟡 | `.connect` **softMAC** implementado (Fase 2: AUTH/ASSOC + STA-record + CH_PRIVILEGE conducidos por el host). **Falta confirmar asociación + WPA2 + data-path.** |
| **GPS** | 🟡 | Protocolo **`0xAAF0` decodificado**; cadena **gpsd→geoclue→Phosh validada**. Falta el `START_SEQ` de `mnld` (arrancar el motor). |
| **Stack Phosh** (plugins + sesión) | 🟡 | **17 plugins** (toggles del panel: datos/hotspot/dark-mode/etc. + widgets de lockscreen); **sesión elogind activa** (base de suspend/power). |
| Brillo | 🟡 | Por comando **`bl 0-100`** (PWM_DUTY). El *slider* = problema del source de Phosh (gsd-power delega a `org.gnome.Shell.Brightness`, ausente). |
| Botones power/vol · sensores · audio | ⬜ | Fase 3 (Phosh 100%) — ver roadmap. |
| FM · vibrador · cámara · módem 3G | ⬜ | Fases 4-5 (cámara/módem = propietarios, muy difícil). |

> **Estabilidad del boot:** el baseline es **estable**. En ~1/3 de los arranques la GUI tarda o no
> sube (se afinará con `supervise-daemon` de OpenRC). Recuperación = power-cycle o `reboot -f`.

*(El **CONSYS** — combo WiFi/BT/GPS/FM del MT6582, que mainline nunca soportó — se levanta entero por
un driver propio en `mainline/wifi-consys/`. Es la frontera técnica del proyecto. Bring-up,
transporte BTIF-DMA, descarga de patches y RF-cal: ver [HITOS.md](mainline/HITOS.md) §19.)*

---

## 🗺️ Estructura del repo

```
README.md                       ← este (punto de entrada)
ROADMAP-FINAL.md                ← roadmap maestro F0–F5 + TODO (documento único de plan)
CONTINUAR-AQUI.md               ← estado de la última sesión + próximos pasos
INDICE-GENERAL.md               ← índice navegable de todos los docs
GUIA-MTKCLIENT.md               ← recuperación por BROM (mtkclient)
ESTRATEGIA-CUSTOM-BOOTLOADER.md ← notas sobre el LK / bootloader
PLAN-MAESTRO-IMPLEMENTACION.md  ← plan de implementación de drivers
README-PMOS-3.10.md             ← el port PREVIO sobre kernel 3.10 (Sxmo/X11), archivado

mainline/                       ← EL PORT MAINLINE (lo principal)
  HITOS.md                      ← historia unificada del proyecto (1 doc, todos los hitos)
  README.md                     ← cómo compilar / flashear / depurar el kernel
  DUAL-BOOT-SD.md               ← pmOS en SD + Android en la interna
  dts/                          ← el DeviceTree del krillin (mt6582-bq-krillin.dts)
  drivers/                      ← drivers del kernel (mt6582-*.c: btif, wifi, mfg-power…)
  disp-drm/                     ← display DRM (mtk_drm) + mapeo de registros
  wifi-consys/                  ← CONSYS WiFi/BT/GPS/FM — la frontera
    m3a/                        ← bring-up + transporte BTIF (mt6582-btif.c) + HITO-WIFI-M3A.md
    wifi/                       ← WiFi 802.11 (mt6582-wifi.c + reg.h) + FASE2-CONNECT.md
    gps/                        ← bridge GPS (mtk-gps-bridge.c, protocolo 0xAAF0)
  userspace/                    ← bridges/daemons de userspace (mtkgps_aaf0.c, etc.)
  rootfs/                       ← configs de userspace (battery, charge, bluetooth, x11, sshd)
  phosh/ · quickwins/           ← stack Phosh + apps
  pkg/                          ← ensamblado del boot.img (assemble.sh, mtk_hdr.py)
  estrategia/ · docs/           ← estrategia de drivers + referencias técnicas únicas
  golden/                       ← lk.img + seccfg.img golden (los .img no van a git)

legacy/                         ← scripts y docs de sesiones tempranas, archivados
aports/ · device-bq-krillin/ · linux-bq-krillin/  ← el path ANTIGUO (pmbootstrap, kernel 3.10)
artifacts/                      ← backups golden (BROM) + firmware stock (no en git)
```

> **Limpieza (2026-06-23):** la doc dispersa se unificó. Los antiguos `mainline/HITO-*.md` sueltos
> ahora viven en un solo **`mainline/HITOS.md`**; los roadmaps dispersos los supersede
> **`ROADMAP-FINAL.md`**. `mainline/docs/` guarda solo referencias técnicas únicas.

---

## 🔧 Cómo se trabaja (para retomar en casa)

El flujo es: **editar en la Pi → compilar (cross armhf) → flashear por USB → verificar en HW.**

- **Raspberry Pi de build** — `ssh cpcd@192.168.0.123` (sudo NOPASSWD). Compila el kernel y habla con
  el teléfono por USB. Árbol del kernel: `~/mainline/linux-7.0.12` (build dir `O=build-krillin`).
- **Teléfono** (pmOS mainline, desde la Pi) — `ssh root@172.16.42.1`. Red USB gadget: la Pi es
  `172.16.42.2/24` en `usb0` → reconectar con
  `sudo ip addr replace 172.16.42.2/24 dev usb0; ssh root@172.16.42.1`.
- **Build + flash del kernel** (en la Pi):
  ```sh
  cd ~/mainline/linux-7.0.12
  # editar drivers/soc/mediatek/mt6582-*.c (o el dts)
  make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs
  # empaquetar (en ~/mainline/pkg): cat zImage dtb > z; python3 mtk_hdr.py KERNEL z z-mtk;
  #   abootimg --create boot.img -f <cfg> -k z-mtk -r initrd
  # flashear: dd a sector 83968 DESDE pmOS viva, o fastboot flash boot boot-*-sd.img
  ```
- **Disparar el CONSYS** (BT/WiFi/GPS): `echo 1 > /sys/kernel/debug/mt6582_wifi/bringup`
  (un mutex serializa el bring-up; sin carrera).

> ⚠️ **El repo (Mac) y la fuente de build (Pi) pueden DIVERGIR** en algunos ficheros — sincronizar con
> `scp` y comparar `md5` antes de editar.
> ⚠️ No martillear SSH anidado Pi→teléfono (satura el sshd / el musb). Agrupar comandos.

---

## 🛟 Recuperación — difícil de brickear

- **Regla de oro MTK:** NUNCA flashear `preloader`. **NUNCA** restaurar el LK de **Lollipop** (verifica
  firma → bootloop; el bueno es **KitKat 1.5.2**). Lo demás es recuperable.
- **fastboot** (en la Pi): `fastboot flash boot boot-*-sd.img` → pmOS de vuelta.
  Entrar en fastboot = manual: Power ~10 s → **Power + Vol↑**. Usar `flash boot`, **nunca** `boot`.
- **mtkclient / BROM** (red de seguridad): backup golden en `artifacts/golden/`. Usar
  **`wo <off> <len> <img>`** (offsets en HEX, boot = `0x2900000`), **NUNCA `wf`** (machaca el sector 0).
  Guía: [GUIA-MTKCLIENT.md](GUIA-MTKCLIENT.md).
- **Softbrick** → SP Flash Tool **"Download Only"** (KitKat). **NUNCA "Format All"** (borra IMEI/NVRAM).
- **`/data` (Android) = `mmcblk0p7`.** Con dual-boot, **pmOS vive en la SD**; la interna es Android.

---

## ▶️ Roadmap (resumen)

Plan maestro completo en **[ROADMAP-FINAL.md](ROADMAP-FINAL.md)**. Una fase a la vez, verificando en HW:

- **F0 — Boot estable:** endurecer arranque (sshd/Phosh siempre) con `supervise-daemon`.
- **F1 — WiFi 100%:** confirmar asociación (red abierta) → data-path RX/TX → WPA2 → navegar.
- **F2 — GPS 100%:** capturar el `START_SEQ` de `mnld` → fix real → geoclue → Phosh.
- **F3 — Phosh 100%:** brillo, botones power/vol, sensores + autorrotación, audio, suspend/wake.
- **F4 — Periféricos:** FM, vibrador (cámara/módem = propietarios, muy difícil).
- **F5 — (futuro):** estudio de viabilidad de Android 12 (el muro histórico cae al tener drivers mainline).

---

## 📚 Índice de docs

- **[INDICE-GENERAL.md](INDICE-GENERAL.md)** — índice navegable de todo (empieza ahí si te pierdes).
- **[mainline/HITOS.md](mainline/HITOS.md)** — la historia: cada hito, causa raíz, registros y ficheros.
- **[ROADMAP-FINAL.md](ROADMAP-FINAL.md)** — el plan maestro F0–F5 + recetas clave.
- **[CONTINUAR-AQUI.md](CONTINUAR-AQUI.md)** — estado de la última sesión + próximos pasos.
- **[mainline/README.md](mainline/README.md)** — compilar / flashear / depurar el kernel.
- **[mainline/wifi-consys/m3a/HITO-WIFI-M3A.md](mainline/wifi-consys/m3a/HITO-WIFI-M3A.md)** — el bring-up del CONSYS al detalle.
- **[GUIA-MTKCLIENT.md](GUIA-MTKCLIENT.md)** · **[ESTRATEGIA-CUSTOM-BOOTLOADER.md](ESTRATEGIA-CUSTOM-BOOTLOADER.md)** — BROM / recuperación / bootloader.
- **[README-PMOS-3.10.md](README-PMOS-3.10.md)** — el port previo sobre kernel 3.10 (archivado).

---

*Proyecto de aficionado, bring-up en hardware real. El historial completo está en
[mainline/HITOS.md](mainline/HITOS.md).*
