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
| **GPS** | 🟡 | Protocolo `0xAAF0` capturado + bridge desplegado. Falta el MOTOR |
| **WiFi** | 🔴 | Bring-up a WIFI_START + FW OK; `WLAN_READY`=0 (MAC no arranca) |

---

## 1. ★★ CONECTIVIDAD — prioridad nº1 (cerrar los 3)

> **★ NVRAM/eFUSE (06-21):** nvram borrada (golden=solo log, proinfo=ceros) pero la **cal RF vive en eFUSE intacta** — GPS (06-20) y BT (06-19, el S24 lo vio como 'BlueZ 5.86') radiaron *tras* el borrado. ⇒ **restaurar nvram NO arregla el RF**; BT/WiFi RF roto = **driver/init**. BT = **regresión** (06-21 `RX/TX=0`; sospechoso: cambios VCN33/consys del WiFi). Identidad MAC/IMEI sí perdida → regenerar (baja prioridad). Detalle: memoria `reference_mt6582_nvram_rf_cal`.

### 🟦 Bluetooth — ✅ FUNCIONA (RF + emparejamiento, 06-21)
- ✅ **RESUELTO EN HW**: la regresión era el cambio VCN33 del consys (06-20) que dejó de encender el raíl RF de 3.3V al boot → `RX/TX=0`. Fix: encender **VCN33_BT** (`ANALDO_CON16` 0x0416 bit7) + VCN33_WIFI **antes del bringup** en `zz-consys-bt.start` (vía `pwrap_poke`, sin reflashear). Tras reboot el **S24 ve "krillin-bq"**.
- ✅ **Emparejamiento**: agente auto-yes persistente `zzz-bt-agent.start` (bluetoothctl NoInputNoOutput + `echo yes`) → **S24 VINCULADO** (`/var/lib/bluetooth`, Bonded/Paired yes, reconecta solo).
- Pendiente (no bloquea): fix limpio en el driver consys; audio A2DP (sin ALSA, aparte); reverificar scan activo. Memorias `reference_mt6582_bt_rf_fix`, `reference_mt6582_wifi_consys`.

### 🛰️ GPS (~70% — falta el motor)
- ✅ Protocolo **capturado 100%** en Lineage: `mnld`↔`/dev/stpgps` = binario **`0xAAF0`** + NMEA out + PMTK/AGPS (posición real de Murcia). Bridge `mtk-gps-bridge`→`gpsd`→geoclue desplegado en mainline.
- ❌ **FALTA:** el **MOTOR** que hable `0xAAF0`. Vías: (a) correr el `mnld` de Android (falló por `__system_properties_init`), (b) motor nativo mínimo desde `wifi-work/mnld-*.strace`, (c) modo NMEA-directo del chip.
- Memoria `reference_mt6582_gps`.

### 📶 WiFi (~60% — el más duro)
- ✅ `func_on(WIFI)`→`WCIR=0x6582`→descarga FW (2 secc + ACKs)→`WIFI_START`. **VCN33 descartado** (enable + modo-HW `ANALDO_CON17=0xd000` verificado).
- ❌ **FALTA:** `WLAN_READY` nunca se afirma (mbox `D2HRM0R`=0, el MAC no arranca). **Sospechoso nº1:** los init que el scaffold SALTA vs OEM `wlanAdapterStart` — `nicInitSystemService`→`nicTxInitialize`→`nicRxInitialize`→`nicTxInitResetResource`.
- Driver `mt6582-wifi.c` (HIF `0x180F0000`). Memoria `reference_mt6582_wifi_hif`.

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
