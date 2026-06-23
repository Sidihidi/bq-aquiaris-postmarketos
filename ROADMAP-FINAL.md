# 🗺️ ROADMAP FINAL + TODO — pmOS en BQ Aquaris E4.5 (krillin / MT6582)

> **Documento maestro y único** (2026-06-23). Unifica y supersede los roadmaps dispersos del repo.
> **Objetivo**: Phosh al **100% + todos los drivers mainline**, y desde ahí el salto a **Android 12**.
> **HW**: MT6582 (Cortex-A7 ×4 armv7), Mali-400 MP2, 1 GB RAM, 540×960 (2014). **SW**: mainline Linux 7.0.12 + Alpine (postmarketOS).
> **Setup**: Pi build `cpcd@192.168.0.123` · teléfono `root@172.16.42.1` (USB) · pmOS en SD (`mmcblk1p1`) · boot en sector **83968**.

## 🧭 Cómo trabajamos
**Una fase a la vez, en orden.** Cada punto: implementar → flashear → **verificar en HW** → marcar ✓. No se avanza al siguiente hasta que el actual funcione al 100%.

---

## ✅ Lo que YA funciona
- Mainline **7.0.12 + Alpine**, arranca desde SD.
- **Display**: `mediatek-drm` + **lima** (Mali-400) + **Phosh** (Wayland).
- **Táctil** FT5336 · **PMIC** MT6323 en DT · **Batería %** (AUXADC/VBAT) · **Carga**.
- **Bluetooth** (hci0): emparejado + toggle en Phosh. ✓
- **WiFi**: escanea redes reales; `.connect` softMAC implementado (asociación sin confirmar).
- **GPS**: protocolo `0xAAF0` decodificado; cadena gpsd→geoclue→Phosh validada (falta arrancar el motor).

---

## 🚧 FASE 0 — BOOT ESTABLE  *(prioridad máxima · EN CURSO)*
**Problema**: arranques no deterministas (a veces sin GUI, a veces sin SSH, a veces cuelgue). **Causa real identificada**: **phoc (compositor) crashea ~3 min tras arrancar** (assert wlroots `wlr_backend_finish` → "Broken pipe"), y dejaba la GUI muerta sin recuperación. *(El parche de supervisores en background se revirtió: peleaba con el apagado de OpenRC y colgaba los reinicios en "Stopping System Message Bus".)*
- [ ] **0.1** Recuperar el móvil del cuelgue de apagado (`reboot -f`) — *en curso*
- [ ] **0.2** Confirmar la causa del crash de phoc a los ~3 min (sospecha: blanqueo por inactividad / **DPMS** del panel)
- [ ] **0.3** Arreglar la causa: desactivar idle/DPMS de phoc (o el `off` del panel en `mediatek-drm`)
- [ ] **0.4** sshd robusto vía **OpenRC `supervise-daemon`** (reinicia limpio, NO pelea con el apagado)
- [ ] **0.5** Verificar: **5 reboots** → GUI + SSH + hci0 SIEMPRE, sin cuelgue de apagado

## 📶 FASE 1 — WiFi 100%
**Estado**: scan OK (dominio regulatorio custom), `.connect` softMAC (handshake AUTH/ASSOC conducido por el host). **Falta el data-path** (lo más gordo) + WPA2.
- [ ] **1.1** Confirmar **asociación** con red ABIERTA (AUTH-2 → ASSOC-RESP → `Connected`)
- [ ] **1.2** **Data-path RX**: recibir paquetes (PDMA + IRQ del HIFSYS → `netif_rx`)
- [ ] **1.3** **Data-path TX**: `ndo_start_xmit` (enviar por el HIF/PDMA)
- [ ] **1.4** **WPA2**: EAPOL passthrough + `.add_key` (claves PTK/GTK al FW) + 4-way handshake
- [ ] **1.5** Verificar: conectar a tu red **WPA2 real** → DHCP → **navegar** en Phosh

## 🛰️ FASE 2 — GPS 100%
**Estado**: `0xAAF0` decodificado, cadena a Phosh validada. Falta arrancar el motor.
- [ ] **2.1** Capturar el **START_SEQ de mnld** (Lineage + GPSLogger + `strace -p $(pidof mnld)`) → modo RUN (t=0x05)
- [ ] **2.2** Implementar la secuencia en el bridge/driver
- [ ] **2.3** Verificar: **fix real** (lat/lon) → geoclue → Phosh/Mapas

## 🖥️ FASE 3 — Phosh 100%
- [ ] **3.1** **Brillo**: exponer `/sys/class/backlight/.../brightness` + permisos → **slider** funcional
- [ ] **3.2** **Botones** power/volumen: DT `mt6323-keys` + `gpio-keys` + mapeo
- [ ] **3.3** **Sensores + autorrotación**: LSM330 (accel) → iio → Phosh rota
- [ ] **3.4** **Audio**: codec MT6582 (altavoz/auricular/micro) → ALSA
- [ ] **3.5** **Suspensión/wake** (power → suspend-to-RAM) + **toggles** WiFi/BT/GPS en la UI

## 🔌 FASE 4 — Periféricos restantes
- [ ] **4.1** **Radio FM** (función del CONSYS, como WiFi/BT) → driver V4L2
- [ ] **4.2** **Vibrador** + LED de notificación
- [ ] **4.3** 🔴 **Cámara** (sensor + ISP MT6582, propietario, sin mainline) — muy difícil
- [ ] **4.4** 🔴 **Módem 3G** (módem MTK propietario, sin driver mainline) — muy difícil

## 🤖 FASE 5 — (FUTURO) Android 12
> El muro histórico de la comunidad fue **no tener drivers para kernels modernos** (MediaTek nunca los liberó); el techo quedó en LineageOS 14.1 (Android 7.1). **Al portear los drivers a mainline, ese muro cae** → un Android moderno pasa a ser viable. Proyecto para **cuando Phosh esté al 100% y todos los drivers funcionen**.
- [ ] **5.1** Estudio de viabilidad formal (requisitos A12, reutilización de drivers, Mali userspace, Treble/GKI, esfuerzo)
- [ ] **5.2** (según el estudio) plan de port

---

## 🔍 Estrategias de extracción de info (capturar del Android stock)
Mucho de lo que bloquea (WiFi connect, GPS, audio) vive en firmware/daemons propietarios de Android. **Estrategia: capturarlo del Android stock (dual-boot) y replicarlo en mainline.**

| Qué falta | Dónde está en Android | Cómo capturarlo |
|---|---|---|
| **GPS START_SEQ** (modo RUN, t=0x05) | daemon `mnld` | Lineage (sector 83968) → `adb root` → `strace -p $(pidof mnld) -s 1024 -e write` → la secuencia que escribe a `/dev/stpgps` al iniciar el fix |
| **WiFi CH_PRIVILEGE + coreografía connect** | driver `wlan` downstream (mt6628) | subir `dbg_level` del driver + `dmesg` al conectar; o leer `mgmt/ais_fsm.c` del downstream (ya en la Pi: `~/mainline/downstream/`) |
| **WiFi data-path (TX/RX, PDMA, IRQ)** | driver `wlan` + HIF | trazar registros HIF (WTDR0/WRDR0) con tráfico real; o `nic_tx.c`/`hif_*.c` del downstream |
| **Calibración RF (WiFi/BT)** | NVRAM (`/data/nvram`) | volcar la NVRAM del Android (el MAC ya arranca con RF-cal parcial) |
| **Audio (routing del codec)** | HAL audio + mixer paths | `tinymix`/`alsa_amixer` dumps en Android → mixer controls del codec MT6582 |

**Método general** (ver `mainline/wifi-consys/m3a/PLAN-B-CAPTURA-ANDROID.md`): arrancar Lineage desde el sector 83968 (`boot-stockadb`), `adb root`, busybox + strace/dmesg/cat → volcar a la Pi por `adb pull` → analizar → replicar en el driver mainline. *(Pendiente: un `krillin-capture.sh` que lo automatice.)*

## 🔧 Recetas clave
- **Reconectar**: `sudo ip addr replace 172.16.42.2/24 dev usb0; ssh root@172.16.42.1`
- **Build+flash kernel** (en la Pi): editar `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-*.c` → `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage` → empaquetar (`cat zImage dtb > zimage-dtb; python3 mtk_hdr.py KERNEL ...; abootimg --create ...`) → `dd` a sector **83968** desde pmOS viva.
- **Recuperar cuelgue de apagado**: `ssh root@172.16.42.1 'sync; reboot -f'` (o power-cycle).
- ⚠️ Repo (Mac) y fuente de build (Pi) **DIVERGEN**: sincronizar con `scp` + comparar md5 antes de editar.

*Drivers en `mainline/wifi-consys/{m3a,wifi,gps}`. Estado por driver en los `.md` de cada carpeta.*
