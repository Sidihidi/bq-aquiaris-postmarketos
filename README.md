# BQ Aquaris E4.5 (`krillin`) — Linux MAINLINE en un MediaTek MT6582 de 2014

Reviviendo el **BQ Aquaris E4.5** (MediaTek MT6582, Cortex-A7 ×4 armv7, Mali-400 MP2, 1 GB RAM,
540×960; *el primer Ubuntu Phone, 2014*) con **Linux mainline 7.0.12 + postmarketOS/Alpine + Phosh**,
escribiendo y portando los drivers que upstream nunca tuvo para este SoC.

> **Estado (2026-07-02):** un teléfono de 2014 con **Linux mainline 7.0.12 + Alpine** y **Phosh
> acelerado por GPU (lima/Mali-400)**, arrancando desde **SD**, con **táctil multitouch (kernel),
> carga, batería en la UI, brillo por slider, Bluetooth y WiFi (red abierta navega)**. En curso:
> **WPA2** (portando el driver stock) y una **tanda de drivers nuevos** (vibrador, LEDs, botón
> power, RTC, thermal, audio).

📍 **Puntos de entrada:** historia técnica → [mainline/HITOS.md](mainline/HITOS.md) · plan de drivers
vigente → [PLAN-PORTS-DRIVERS.md](PLAN-PORTS-DRIVERS.md).
Repo: [github.com/Sidihidi/bq-aquiaris-postmarketos](https://github.com/Sidihidi/bq-aquiaris-postmarketos).

---

## ✅ Estado por subsistema

**✅** funciona en HW · **🟡** parcial / en progreso · **⬜** sin empezar.

| Subsistema | Estado | Detalle |
|---|---|---|
| Boot mainline (SMP 4×A7) | ✅ | Arranca desde **SD** (`mmcblk1p1`), sector **83968**; LK KitKat carga zImage+dtb. |
| Display DRM + GPU | ✅ | `mediatek-drm` (DSI) + **lima / Mali-400**. |
| Phosh (Wayland) | ✅ | phoc + phosh + squeekboard sobre GLES2/lima. |
| **Táctil** | ✅ | **Multitouch por kernel** (`edt-ft5x06` + chunked-read I2C, EINT117, 0 CPU en reposo). |
| PMIC MT6323 | ✅ | En el DeviceTree (pwrap + MFD + reguladores). |
| Batería % + carga | ✅ | VBAT por AUXADC + indicador en Phosh; cargador FAN5405. |
| Brillo (slider) | ✅ | Slider de Phosh → logind → `/sys/class/backlight` → daemon PWM. |
| Bluetooth (hci0) | ✅ | Empareja (probado con un S24) + toggle en Phosh. Vía CONSYS/BTIF. |
| WiFi — scan + red abierta | ✅ | Escanea redes reales; **red abierta navega** (lease + ping). |
| **WiFi — WPA2** | 🟡 | Asocia y completa el 4-way, pero el FW no cifra el data-path (DHCP falla). **Se está portando el driver stock `mt_wifi`** (8 hipótesis de parcheo agotadas). |
| GPS | 🟡 | Protocolo `0xAAF0` decodificado; cadena gpsd→geoclue→Phosh validada. Falta el `START_SEQ` de `mnld`. |
| **Vibrador · LEDs RGB · botón power · RTC** | 🟡 | **Code-complete** (drivers mainline + DT+config); compilan, **falta 1 flash de verificación**. |
| **Thermal** | 🟡 | AUXADC del SoC **validado en HW**; falta el rail del NTC (lee 0 mV). |
| **STP (robustez BT)** | 🟡 | Analizado; parche de resync RX listo (handoff), sin integrar (no romper el BT que ya va). |
| **Audio** | 🟡 | Mapeado + esqueleto AFE compilando; port de ~6-8 sem (ALSA SoC de cero). |
| Botones vol | ✅ | Keypad matriz (`mt6779-keypad`). |
| FM · cámara · módem 3G | ⬜ | FM = mucho por poco; cámara/módem = propietarios (muy difícil). |

*(El **CONSYS** — combo WiFi/BT/GPS/FM del MT6582, que mainline nunca soportó — se levanta entero por
drivers propios en `mainline/wifi-consys/`. Es la frontera técnica del proyecto.)*

---

## 🔀 Dos frentes en paralelo

El trabajo va en dos sesiones coordinadas por este repo:

1. **WiFi WPA2** — port del driver stock `mt_wifi` a 7.0.12
   (`mainline/wifi-consys/wifi/mt_wifi_port/`, Fase 1 KAL). Plan y estado:
   [HANDOFF-MTWIFI-PORT-PLAN-0702.md](mainline/wifi-consys/wifi/HANDOFF-MTWIFI-PORT-PLAN-0702.md) ·
   [HANDOFF-MTWIFI-PORT-STATE-0702.md](mainline/wifi-consys/wifi/HANDOFF-MTWIFI-PORT-STATE-0702.md).
   Por qué se abandonó el parcheo del driver A:
   [HANDOFF-BLINDPOKE-REFUTADO-0702.md](mainline/wifi-consys/wifi/HANDOFF-BLINDPOKE-REFUTADO-0702.md).
2. **Resto de drivers** — vibrador/LEDs/power/RTC/thermal/STP/audio, plan e estado en
   [PLAN-PORTS-DRIVERS.md](PLAN-PORTS-DRIVERS.md).

---

## 🗺️ Estructura del repo

```
README.md                     ← este (punto de entrada)
PLAN-PORTS-DRIVERS.md         ← plan maestro de drivers (estado + recetas de test)
mainline/
  HITOS.md                    ← historia unificada del proyecto (todos los hitos)
  dts/mt6582-bq-krillin.dts   ← el DeviceTree canónico del krillin
  audio/                      ← port de audio (AFE) — esqueleto + handoff
  wifi-consys/                ← CONSYS WiFi/BT/GPS/FM (la frontera)
    wifi/                     ← driver A (mt6582-wifi.c) + handoffs + mt_wifi_port/ (port stock)
    HANDOFF-STP-CORE-PORT-0702.md
  rootfs/ · phosh/ · userspace/  ← configs, stack Phosh, bridges de userspace
fw-analysis/                  ← firmware WiFi nds32 descompilado (Ghidra) + herramientas RE
docs/archive/                 ← documentación histórica superada (ver su README)
```

> **Nota:** el árbol de build vive en la Pi (`~/mainline/linux-7.0.12`), no en el repo. Aquí van las
> **fuentes canónicas** (drivers propios, DTS, configs, handoffs). Sincronizar con `scp` + `md5`.

---

## 🔧 Cómo se trabaja

Flujo: **editar → compilar (cross armhf) en la Pi → flashear por USB → verificar en HW.**

- **Pi de build** — `ssh cpcd@192.168.0.123` (sudo NOPASSWD). Árbol: `~/mainline/linux-7.0.12`
  (build dir `O=build-krillin`). *(Hay una 2ª Pi, `.38`, con el otro frente; no confundirlas.)*
- **Teléfono** (pmOS, desde la Pi) — `ssh root@172.16.42.1`. USB gadget: la Pi es `172.16.42.2/24`
  en `usb0` → reconectar con `sudo ip addr replace 172.16.42.2/24 dev usb0`.
- **Build** (en la Pi): `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 zImage dtbs`
  (**`-j2`, no `-j4`** — el pico de CPU reinicia la Pi). Empaquetar+flashear: `~/wifi-iter-w.sh`
  (dd a sector 83968 con verificación md5) o `fastboot flash boot boot-*-sd.img`.
- **Disparar el CONSYS** (BT/WiFi/GPS): `echo 1 > /sys/kernel/debug/mt6582_wifi/bringup`.
- **Tras un crash**: el pstore se salva en cada boot en `/var/log/pstore/boot-*` (además de
  `/sys/fs/pstore/console-ramoops-0`).

> ⚠️ Repo (Mac) y árbol de build (Pi) pueden divergir: sincronizar y comparar `md5` antes de editar.
> ⚠️ No martillear el SSH anidado Pi→teléfono (satura sshd/musb). Agrupar comandos. Si el teléfono no
> responde → power-cycle físico.

---

## 🛟 Recuperación — difícil de brickear

- **Regla de oro MTK:** NUNCA flashear `preloader`; **nunca** restaurar el LK de **Lollipop**
  (bootloop; el bueno es **KitKat 1.5.2**). Lo demás es recuperable.
- **fastboot** (Pi): `fastboot flash boot boot-*-sd.img`. Entrar: Power ~10 s → Power + Vol↑.
  Usar `flash boot`, **nunca** `boot`.
- **mtkclient / BROM** (red de seguridad): backup golden en `artifacts/golden/` (fuera de git).
  Usar **`wo <off> <len> <img>`** (offsets HEX, boot=`0x2900000`), **nunca `wf`** (machaca el sector 0).
- **Dual-boot:** pmOS vive en la **SD**; la interna es Android (`/data` = `mmcblk0p7`).

---

*Proyecto de aficionado, bring-up en hardware real. Historial completo en
[mainline/HITOS.md](mainline/HITOS.md); documentación superada en
[docs/archive/](docs/archive/README.md).*
