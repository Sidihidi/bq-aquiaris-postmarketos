# 🗺️ ROADMAP — BQ Aquaris E4.5 "krillin" en postmarketOS mainline

Estado a **2026-07-05** (kernel mainline **7.0.12 #236**, Phosh 0.55 build propio). Este documento es
la foto completa: qué está al 100%, qué está a medias, y qué falta — con el orden de ataque sugerido.

---

## ✅ FUNCIONAL AL 100% (verificado en hardware)

| Área | Estado | Notas / dónde está |
|---|---|---|
| **Boot estable** | ✅ | ~35s hasta GUI, crng <10s, nada crashed. Entropía auto-reparable, usb0 con guardia, sshd supervisado, local.d asíncrono, Phosh supervisado (relanzado si muere phoc O phosh). `mainline/userspace/BOOT-ESTABILIZACION-0705.md` |
| **WiFi WPA2 + DHCP** | ✅ | Port del stock `mt_wifi` (`mtk_mtwifi`, built-in, driver por defecto). 2 redes reales validadas (grupo CCMP), NM/GUI, autoconnect. OOPS de RX-mgmt arreglado (#236). `mainline/wifi-consys/wifi/MILESTONE-WIFI-MAINLINE-COMPLETO-0704.md` |
| **SSH de rescate** | ✅ | usb0 (172.16.42.1, con guardia) + **WiFi nativo** (`root@192.168.0.109`, autoconnect prio 10) |
| **Pantalla (panel+backlight+brillo)** | ✅ | DSI hx8389 + backlight PWM (fix EN-bit: duty=0 no corta la luz solo) + slider de brillo (shim D-Bus) |
| **Táctil FT5336** | ✅ | Power-cycle VGP1+reset al boot (`touch-power.start`); pendiente de pasar al DT (abajo) |
| **Botón de encendido** | ✅ | Corto: bloquear+apagar / despertar→lockscreen (passcode). Largo: menú Apagar/Reiniciar. Daemon `mt6582-powerkey` (EVIOCGRAB dueño único) + parche phosh no-DPMS (`phosh-patches/`) + método `Lock` (no SetActive: no bloqueaba con lock-enabled=false) |
| **GUI Phosh** | ✅ | 0.55 build propio (en el móvil: `/root/build/phosh`), con parche `0001-monitor-no-dpms-off.patch`. Supervisión completa de sesión |
| **Bluetooth (bring-up + pairing)** | ✅ | hci0 vía btif/WMT (bring-up diferido post-Phosh), agente auto-pairing. Audio BT = ver Audio |
| **Carga (mantenedor FAN5405)** | ✅ | `fan5405-charge` daemon; con el móvil apagado carga el PMIC en HW |
| **Radio FM** | ✅ **SUENA (0708)** | `fmradio 1023 &` + `amixer cset name='FM Radio Route' 1` (auriculares = antena). Chip MT6627 on-die por `/dev/fm` + path digital I2S→ASRC→DAC con conexión directa en `mt6582-afe-pcm.c`. Causa histórica del silencio: RESTORE_SEARCH hacía RampDown del DSP + GAIN1/0x448 inexistente. Ground truth dual-boot en `mainline/wifi-consys/fm/groundtruth-0708/` |

## 🟡 PARCIAL / POR VALIDAR

| Área | Qué falta | Siguiente paso |
|---|---|---|
| **Batería %** | ✅ **FUNCIONA (0706)**: `battery` lee VBAT por AUXADC del MT6323 (canal 7, `raw*4*1800/32768`, validado 4088mV) + `charge-status` (FAN5405 i2c) → `battery-upower` alimenta `test_power.ko` → UPower → Phosh (%, voltaje, icono de carga). UPower decide cargando/descargando por el SIGNO de `current_now`. **Precisión mejorada (0706)**: compensación I·R de carga (OCV≈VBAT−100mV cargando / +40mV descargando) → cargando muestra ~78% en vez del 88% inflado. Scripts versionados en `mainline/rootfs/` | **Gauge preciso = trabajo futuro**: tabla ZCV real (`krilin_cust_battery_meter_table.h`) + coulomb-counting del HW FG del MT6323 (leer corriente de batería por pwrap) para compensar la curva cargada |
| **GPS** | 🔄 **REFRAME (0707)**: es **host-based** — el DSP solo da medidas crudas 0xAAF0; la POSICIÓN la calcula `libmnl` en la CPU. → capturar el START de LineageOS es **insuficiente** (sin libmnl no hay fix). Cadena bridge→gpsd→geoclue lista pero el bridge no puede calcular PVT | **CONDITIONAL-GO (~3-6 sem)**: portar/correr la pila stock `libmnl` (playbook WiFi) sobre `/dev/stpgps` — linkar `libmnl_6628.a` (AOSP) + glue abierto + shims KAL. Plan: `PORT-STRATEGY-DRIVERS-0707.md` §3.1 |
| **Botón: fluidez** | ✅ **RESUELTO (0708, Ola 1)**: `mt6582-session-helper` persistente en la sesión (socket `powerkey.sock`, D-Bus ya conectado) → lock/unlock en ~0ms (fire-and-forget; el Lock de phosh tarda ~1.3s pero la pantalla ya está apagada). Fallback su+gdbus intacto | — |
| **Auto-brillo (ALS)** | ✅ **FUNCIONA (0708, Ola 1)**: `mt6582-autobrightness` lee lux del TMD2772 por /sys (699ms+gain16; de fábrica 2.73ms/gain1 = 0 lux en interior) → curva log → `/run/mt6582-bl-pct` (canal del slider, respeta ajuste manual hasta que la luz cambie 3x) | Afinar curva con uso real |
| **STP resync-RX** | ✅ **INTEGRADO (0708, kernel #258)**: `stp_pop_frame` descarta bytes hasta header plausible; contador `debugfs/mt6582_btif/resync_drops` (=0 en tráfico normal). BT+FM+WiFi validados sin regresión | — |
| **Bloqueo por inactividad** | Con el parche no-DPMS, el idle-blank de phosh ya no oscurece (solo marca estado); idle-delay dejado a 0 | Mini-listener de `ActiveChanged`→flag del backlight → idle-off completo |
| **Vibración** | ✅ **RESUELTO (0706)**: causa = perfil de feedbackd en `silent`. Fix: `gsettings set org.sigxcpu.feedbackd profile quiet` (en `phosh-session.sh`). Motor `regulator-haptic` FF (event2), sxmo en grupo `input`, tema `default.json`. **Validado vibrando en HW.** `full` añade háptico al pulsar botones + sonidos | — |
| **Vídeo/YouTube** | ✅ FUNCIONA (0705): `mpv <url>` o **Livi** (GUI móvil) reproducen YouTube 360p h264 fluido (yt-dlp + nodejs + gst completo instalados; captura de prueba verificada). SIN SONIDO hasta que el driver de audio esté. El player web de YouTube en Epiphany sigue siendo demasiado pesado para esta CPU — usar Livi/mpv | Al llegar el audio: quitar `--no-audio` y validar A/V sync |

## ❌ PENDIENTE (por portear/implementar)

En orden de ataque sugerido (impacto/dificultad):

1. **Audio (ALSA para MT6582)** — **✅✅✅ FUNCIONA Y FORMALIZADO EN EL DRIVER (0706, kernel #241):
   `aplay`/mpv suenan SOLOS (auriculares + altavoz), sin scripts.** `mt6582-afe-pcm.c` hace el codec
   MT6323 en `.prepare`/`.close` (regmap del pwrap vía phandle `mediatek,pmic`, con el `usleep(10ms)`
   de bias-settle — la clave del RE del HAL) + amp externo GPIO118. Hito en
   `MILESTONE-AUDIO-FUNCIONA-0706.md`. **Audio-GUI ✅ RESUELTO (0706)**: PulseAudio SÍ crea el sink
   (`alsa_output.platform-11220000...stereo-fallback`); el problema era solo que PA no arrancaba en la
   sesión. `paplay`/apps por PulseAudio suenan (validado). `phosh-session.sh` arranca `pulseaudio
   --start` al boot + autospawn=yes para las apps (Livi/YouTube-en-navegador por pulsesink). El aviso
   de callaudiod ("lacks speaker/earpiece port") es solo para el ROUTING DE LLAMADAS (sin módem);
   un perfil UCM con puertos con nombre queda pendiente SOLO para eso.
2. **Suspend/resume (s2idle)** — 🟢 **FUNCIONA + AUTO-SUSPEND ACTIVO (0706)**. s2idle suspende/resume
   de punta a punta (todos los resume callbacks completan, confirmado por ramoops); RTC y **botón**
   despiertan; **WiFi reconecta** tras resume. usb0/musb no re-enumera solo → wrapper `mt6582-suspend`
   lo repara (unbind/rebind). Daemon `mt6582-autosuspend` (boot: `zzy-autosuspend.start`) suspende
   con la pantalla 60s apagada salvo inhibidores (audio/SSH/flag `/run/mt6582-no-suspend`); wake por
   botón. Validado end-to-end en HW (usuario confirmó: suspende→botón→pantalla). **PENDIENTE**: medir
   el ahorro real (necesita batería) para decidir si s2idle basta; el nit de latencia al desbloquear
   es la fluidez del botón (ya listada). **Deep suspend** (power-gating del SoC, ahorro grande) =
   falta el SPM del mt6582 en mainline → proyecto propio. Detalle: `mainline/suspend/FINDINGS-SUSPEND-0706.md`.
3. **Módem 2G/3G (llamadas/SMS/datos)** — 🔴 **NO-GO (estudio 0707)**: MD en core separado con firmware
   MOLY propietario + CCCI (memoria compartida); **cero soporte en mainline** para esta clase de módem MTK
   (el único WWAN MTK mainline, t7xx, es 5G-PCIe, no aplica); **ningún MT65xx tiene llamadas en mainline**.
   Requiere port del eccci + boot del MD + puente RIL↔ModemManager (sin framework destino) = moonshot de
   meses, baja probabilidad. Límite conocido, como la cámara. Detalle: `mainline/modem/FEASIBILITY-MODEM-0707.md`.
4. **Sensores** (I2C) — 🟢 **AUTO-ROTACIÓN FUNCIONA (0706)**: acelerómetro **LSM330** @0x1d por IIO
   `st_accel` (=y) + nodo DT con **mount-matrix** calibrada (`[[0,1,0],[1,0,0],[0,0,-1]]`, de
   `direction=7`) + `iio-sensor-proxy` (rc default) + **regla polkit** que autoriza a sxmo a reclamar
   sensores (sin ella la sesión aislada da AccessDenied). Validado en HW. **Añadidos #243**: giroscopio
   LSM330 @0x6b (`st_gyro`) + luz/proximidad TMD2772 @0x39 (`tsl2772`) — la **luz ambiente ya la detecta
   iio-sensor-proxy**. **Pendiente**: auto-brillo (que Phosh use la ALS con el backlight custom), proximidad
   en el proxy, magnetómetro (MMC3516x @0x30 sin driver mainline), lockscreen que no rota (Phosh lo fija a
   portrait). Detalle: `mainline/sensors/FINDINGS-SENSORS-0706.md`.
5. **Cámaras** — muy abajo en prioridad (ISP MTK complejo).
   - **Waydroid (apps Android)** — 🟡 VIABLE pero APARCADO por RAM (0706). Existe imagen armv7
     (`waydroid_arm`, LineageOS 20/Android 13 en SourceForge); pmOS empaqueta `waydroid` (falta añadir
     el repo pmOS — el móvil solo apunta a Alpine v3.24 main/community). Kernel necesita ~10 configs:
     `ANDROID_BINDER_IPC`, `ANDROID_BINDERFS`, `FUSE_FS`, `OVERLAY_FS`, `TUN`, `BRIDGE`, `VETH`,
     `MEMCG`, `CGROUP_PIDS`, `PSI` (namespaces/cgroups/squashfs/memfd YA están). **Bloqueo práctico:
     1 GB de RAM** → Android 13 entero + pmOS+Phosh haría thrashing/OOM; A7 lento. Reevaluar si hay
     un móvil con ≥2 GB. El usuario decidió saltarlo.
6. **LED de notificaciones** — ✅✅ RESUELTO Y VALIDADO (0706): el LED **azul respira** en
   `notification-missed-generic` accionado por feedbackd. Hicieron falta **3 piezas** (no solo el
   trigger del kernel): (1) `CONFIG_LEDS_TRIGGER_PATTERN=y` (ya en #241), (2) **color por udev**
   (`73-krillin-leds-color.rules`: `FEEDBACKD_LED_COLOR` por LED — la regla genérica no lo pone), y
   (3) **`sxmo` en el grupo `feedbackd`** (los sysfs son `root:feedbackd` g+w; sin el grupo →
   "Permission denied"; fix persistente + `local.d/led-feedbackd-group.start`). Funciona con el perfil
   `quiet` (el feedback `Led` está a nivel `silent` en el tema). Detalle:
   `mainline/userspace/HANDOFF-LED-NOTIF-0706.md`.
7. **Touch al DT** — retirar `touch-power.start` cuando edt-ft5x06 + regulador mt6323 estén en el
   DT (`vin-supply` + `reset-gpios`); hoy es un power-cycle userspace.
8. **Sesión con elogind/PAM (greetd)** — el fix "de verdad" del aislamiento de la sesión
   (dbus-run-session): daría inhibit/loginctl/polkit por sesión, y phosh podría manejar el botón
   NATIVO (se podría retirar el grab + parte del daemon). También arregla "No Login session,
   screen blank/lock will be unreliable".
9. **WiFi fino** (no bloqueante): TKIP grupo (cerrado como no-necesario: era el repetidor mal
   configurado; WPA1/TKIP obsoleto), tablas TX-power, roaming, redes abiertas/ocultas.

## ⚠️ Lecciones operativas (leer antes de trabajar en el móvil)

- **`apk add` puede corromper libs de core** en este móvil (visto 0705: `apk add wlopm` → musl/kmod-libs
  mal escritas → udevd segfault → NM/GUI degradados). Tras CUALQUIER apk: `udevadm --version` +
  `rc-status | grep crashed`. Reparación: `apk fix <libs>`.
- **Transferencias de ficheros SOLO por base64/tar en foreground** — los heredocs expandibles y los `&`
  en la cadena ssh anidada se comen contenido (dos incidentes ya: aa-sshd-keys no-op, patch vacío).
- **busybox `pgrep -x` compara contra el cmdline completo** → "phosh no está" falso. Buscar por
  `/proc/*/comm`.
- El kernel se construye en la Pi (`~/build-diag.sh` → `boot-diag.img` → dd a `mmcblk0` seek=83968
  con verify md5). Phosh se construye EN el móvil (`/root/build/phosh`, ninja).

## Cómo verificar un boot sano
```
dmesg | grep "random: crng"        # < 10s
rc-status | grep crashed           # vacío
pgrep -x phoc                      # vivo (phosh: buscar por /proc/*/comm)
nmcli -t dev status | grep wlan0   # connected
ip -4 addr show usb0 | grep inet   # 172.16.42.1
```
