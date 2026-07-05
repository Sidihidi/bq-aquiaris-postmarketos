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

## 🟡 PARCIAL / POR VALIDAR

| Área | Qué falta | Siguiente paso |
|---|---|---|
| **Batería %** | El % que ve Phosh viene de `battery-upower`→`test_power`; la lectura VBAT por AUXADC (`/usr/local/bin/battery`) fallaba (0%) | Depurar la lectura AUXADC del MT6323 por pwrap |
| **GPS** | Cadena completa implementada (bridge stpgps→gpsd→geoclue) | Validar un fix real en exterior |
| **Botón: fluidez** | El lock/wake tarda ~1-2s (cada acción = `su`+gdbus nuevos) | Daemon-hijo persistente EN la sesión (socket) para latencia ~0; debounce fino |
| **Bloqueo por inactividad** | Con el parche no-DPMS, el idle-blank de phosh ya no oscurece (solo marca estado); idle-delay dejado a 0 | Mini-listener de `ActiveChanged`→flag del backlight → idle-off completo |
| **Vibración** | `regulator-haptic` existe (event2) | Cablear con feedbackd/phosh y validar |
| **Vídeo/YouTube** | ✅ FUNCIONA (0705): `mpv <url>` o **Livi** (GUI móvil) reproducen YouTube 360p h264 fluido (yt-dlp + nodejs + gst completo instalados; captura de prueba verificada). SIN SONIDO hasta que el driver de audio esté. El player web de YouTube en Epiphany sigue siendo demasiado pesado para esta CPU — usar Livi/mpv | Al llegar el audio: quitar `--no-audio` y validar A/V sync |

## ❌ PENDIENTE (por portear/implementar)

En orden de ataque sugerido (impacto/dificultad):

1. **Audio (ALSA para MT6582-AFE)** — EN CURSO (0705): validación HW completada — INFRA_AUDIO ya
   encendido por LK, AFE @0x11220000 lee/escribe sin colgar, PDN_AFE/I2S=0 de fábrica, IRQ=GIC_SPI
   104 confirmado. Esqueleto Fase A en `sound/soc/mediatek/mt6582/` (árbol Pi). Siguiente: PCM DL1
   + IRQ + nodo DT (Fase A.2), luego codec ANA por pwrap (Fase B) y machine driver (C).
   Plan/estado: `mainline/audio/HANDOFF-AUDIO-PORT-0702.md`.
2. **Suspend/resume** — bloquea la autonomía real. Hoy `enable-suspend false`. Requiere validar
   suspend de: WiFi (port), consys/BT, panel, pwrap. Tocará el driver WiFi (suspend hooks).
3. **Módem 2G/3G (llamadas/SMS/datos)** — el hueso más gordo (CCCI/EEMCS del downstream +
   integración ModemManager/ofono). Evaluar viabilidad tras audio.
4. **Sensores** (acelerómetro/luz/proximidad, I2C) — para rotación y apagar pantalla en llamada.
   `net.hadess.SensorProxy` hoy ausente (warning de phosh-ambient). Drivers IIO pequeños.
5. **Cámaras** — muy abajo en prioridad (ISP MTK complejo).
6. **LED de notificaciones** (mt6323-led: "Failed to locate of_node" → añadir nodo al DT).
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
