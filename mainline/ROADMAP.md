# Hoja de ruta — mainline Linux / postmarketOS en BQ Aquaris E4.5 "krillin" (MT6582)

_Actualizado: 2026-06-21_ · Repo backup: **Sidihidi/bq-aquiaris-postmarketos** (`mainline/{drivers,dts,docs,userspace}`)
Teléfono `root@172.16.42.1` (USB) desde la Pi `cpcd@192.168.0.38`. Flash = `dd` a `/dev/mmcblk0` sector **83968**.

---

## ★ ESTRATEGIA (acordada 2026-06-21)
1. **CONECTIVIDAD al 100%** — GPS + BT + WiFi. **No avanzar hasta que los tres funcionen.**
2. **Botones / menús de Phosh** — brillo, encendido/apagado/reinicio del dispositivo, toggles + menús de GPS/WiFi/BT.
3. **Sensores + autorrotación.**

---

## 0. ESTADO — qué FUNCIONA
| Subsistema | Estado | Notas |
|---|---|---|
| Display | ✅ | mtk_drm DRM nativo (panel HX8389 qHD 540×960) + Phosh GPU lima |
| Táctil | ✅ | FT5336 (I2C0, EINT117) end-to-end |
| Almacenamiento | ✅ | eMMC `mmcblk0` + SD `mmcblk1` (rootfs Alpine 3.24) |
| Red / consola | ✅ | USB gadget `cdc_ether` + SSH `172.16.42.1` |
| **Boot** | ✅ | Estable (seedrng/haveged); **sshd con bucle de reintento** (zz-sshd.start) |
| **Backlight HW** | ✅ | `+0xa0` PWM_DUTY; **CLI `bl 0-100`** + daemon. SLIDER de Phosh pendiente (sesión elogind) |
| Pila energía | 🟡 | **polkit + UPower + elogind + login1** instalados (groundwork batería + suspend) |
| **Bluetooth** | ✅ | RF arreglado (VCN33 al boot) + **S24 emparejado**. Audio A2DP aparte |
| **GPS** | 🟡 | **Protocolo `0xAAF0` DECODIFICADO + bridge nativo C escrito** (`mtkgps_aaf0.c`, sin mnld). Falta 1 recaptura (`t=0x05`, trigger GPSLogger) |
| **WiFi** | 🟢🟡 | **Fase 1 + SCAN FUNCIONAN** — **14 beacons reales escaneados** (cmd por TC4/puerto-1, beacons por puerto-0). Falta wiphy/netdev p/`iw`, luego connect |

---

## 1. ★★ CONECTIVIDAD — prioridad nº1 (cerrar los 3)

> **★ NVRAM/eFUSE (06-21):** nvram borrada (golden=solo log, proinfo=ceros) pero la **cal RF vive en eFUSE intacta** — GPS (06-20) y BT (06-19, el S24 lo vio como 'BlueZ 5.86') radiaron *tras* el borrado. ⇒ **restaurar nvram NO arregla el RF**; BT/WiFi RF roto = **driver/init**. BT = **regresión** (06-21 `RX/TX=0`; sospechoso: cambios VCN33/consys del WiFi). Identidad MAC/IMEI sí perdida → regenerar (baja prioridad). Detalle: memoria `reference_mt6582_nvram_rf_cal`.

### 🟦 Bluetooth — ✅ FUNCIONA (RF + emparejamiento, 06-21)
- ✅ **RESUELTO EN HW**: la regresión era el cambio VCN33 del consys (06-20) que dejó de encender el raíl RF de 3.3V al boot → `RX/TX=0`. Fix: encender **VCN33_BT** (`ANALDO_CON16` 0x0416 bit7) + VCN33_WIFI **antes del bringup** en `zz-consys-bt.start` (vía `pwrap_poke`, sin reflashear). Tras reboot el **S24 ve "krillin-bq"**.
- ✅ **Emparejamiento**: agente auto-yes persistente `zzz-bt-agent.start` (bluetoothctl NoInputNoOutput + `echo yes`) → **S24 VINCULADO** (`/var/lib/bluetooth`, Bonded/Paired yes, reconecta solo).
- Pendiente (no bloquea): fix limpio en el driver consys; audio A2DP (sin ALSA, aparte); reverificar scan activo. Memorias `reference_mt6582_bt_rf_fix`, `reference_mt6582_wifi_consys`.

### 🛰️ GPS (~90% — protocolo decodificado + bridge nativo escrito)
- ✅ **Protocolo `0xAAF0` DECODIFICADO 100%**: frame `AA F0|LEN|SEQ|TYPE|FE|payload(XOR-0xCA en lecturas)|AA 0F`; START=206B (7 config + `0x05` RUN); posición=frame `0x30` (**la calcula el chip** — libmnl no tiene solver). Bridge `mtk-gps-bridge`→`gpsd`→geoclue ya en mainline.
- ✅ **MOTOR = bridge nativo en C** (`wifi-work/mtkgps_aaf0.c`, sin mnld/bionic) → respaldado en GitHub `mainline/userspace/`.
- 🟡 **FALTA 1 recaptura** (procedimiento RESUELTO): el `t=0x05` RUN se truncó + el offset del `0x30` sin confirmar. Receta: Lineage(sec 83968) + **trigger GPSLogger `am start -n com.mendhak.gpslogger/.shortcuts.ShortcutStart`** + `strace -p $(pidof mnld) -s 1024` → pegar el RUN + offset → bridge emite NMEA real. Memoria `reference_mt6582_gps`.

### 📶 WiFi — ✅ MAC VIVO (06-21: la CALIBRACIÓN RF rompió EL MURO → `WLAN_READY=1`)
- 🎯 **RESUELTO el arranque del MAC**: faltaba la **calibración RF** del CONSYS (`01 14 01 00 01` → EVT `02 14 02 00 00 01`), que el OEM (`mtk_wcn_soc_sw_init`, TRAS los parches) corre y nuestro `bring_up_chip` omitía. Sin ella el PHY/PLL/AFE del MAC arrancaba sin calibrar → su FW saltaba (`ACCESS_REG(0x6a000)=0xdeadbeef`) y se colgaba en la entrada (BT/GPS la toleran). Añadida tras `RESET-2` en `mt6582-btif.c` → al boot `*** RF-CAL OK ***` → trigger WiFi → **`*** firmware ARRANCADO — WLAN_READY=1 ***`** al primer intento. (Hallado por subagente RE-ando `conn_soc`.) Hipótesis previas (VCN33 timing/HW-mode, EMI, nic-init host-memory, command-path) confirmadas NO-causa.
- ➡️ **SIGUIENTE FASE = cfg80211** (ya SIN muro HW): el MAC es full-MAC (el FW hace 802.11); toca registrar wiphy + path comando/evento por los mailboxes HIF (`H2DSM0R=0x38`/`D2HRM0R=0x40`) + scan/assoc + netdev+PDMA+IRQ (roadmap Fase 1-3 en `wifi-work/WIFI-ROADMAP.md`). **PENDIENTE: respaldar el btif (con la RF-cal) a GitHub `mainline/drivers`.**
- Infra nueva 06-21: Lineage-13 arranca en eMMC + **adb** (Pi `/usr/bin/adb`) + **fastboot** (`adb reboot bootloader`+`sudo fastboot flash boot` SIN sleep). GPS grabado (`mnld`+`libmnl.so`+`gps.default.so`+`gps.conf`) en Pi `~/gps-grab/`. Driver `mt6582-wifi.c` con ACCESS_REG read-FW + test post-START + dump seguro. Memoria `reference_mt6582_wifi_hif`.

---

## 2. Botones / menús de Phosh (tras conectividad)
- **Brillo (slider):** HW listo (`bl`). El slider necesita sesión elogind→gsd-power (o entender por qué Phosh ignoró el shim D-Bus). Memoria `reference_mt6582_backlight`.
- **Power button (EINT):** bloquear/apagar pantalla; va ANTES que suspend.
- **Apagado / reinicio** del dispositivo desde el menú de Phosh.
- **Toggles + menús de GPS / WiFi / BT** (rfkill, conexiones).

## 3. Sensores + autorrotación
- Accel ST **LSM330** (i2c0@0x1D, polling, driver mainline) → `iio-sensor-proxy` → Phosh. Memoria `reference_mt6582_sensors_rotation`.

---

## Backup / continuar
- GitHub `Sidihidi/bq-aquiaris-postmarketos`: `mainline/drivers` (kernel), `mainline/dts`, `mainline/docs`, `mainline/userspace` (configs del tel con rutas).
- Kernel tree en la Pi: `~/mainline/linux-7.0.12`. Flash desde pmOS viva: `reference_mt6582_flash_frompmos`.
