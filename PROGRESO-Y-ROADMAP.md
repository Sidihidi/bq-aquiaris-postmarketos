# Progreso y roadmap — mainline Linux + WiFi/GPS en el BQ Aquaris E4.5 "krillin" (MT6582)

_Última actualización: **2026-06-22** (sesión nocturna 21→22 jun). Repo: **Sidihidi/bq-aquiaris-postmarketos**._
_Teléfono `root@172.16.42.1` (USB) desde la Pi `cpcd@192.168.0.38`. Flash = `dd` a `/dev/mmcblk0` sector **83968**._

---

## TL;DR — desde el viernes hemos avanzado MUCHO

El **viernes (19-jun)** el WiFi solo "respondía": el chip leía su id `0x6582` por el bus HIF y poco más; el firmware del MAC **no arrancaba**.
El **lunes (22-jun)** el **WiFi ESCANEA REDES REALES** y el **GPS tiene su protocolo descifrado** con un bridge nativo en C. Cuatro hitos grandes, **todos probados en hardware**:

| # | Hito | Prueba |
|---|---|---|
| 1 | **El MAC WiFi arranca** (`WLAN_READY=1`) | Faltaba la **calibración RF** (`01 14 01 00 01`) que el OEM corre tras los parches; sin ella el PHY/PLL del MAC arrancaba sin calibrar y el firmware se colgaba en la entrada. **Era el muro de toda la saga.** |
| 2 | **GPS: protocolo `0xAAF0` decodificado + bridge nativo C** | Frame `AA F0│LEN│SEQ│TYPE│FE│payload(XOR-0xCA)│AA 0F`; el **chip calcula la posición**. Bridge `mtkgps_aaf0.c`, sin mnld/bionic. |
| 3 | **WiFi: bucle comando/evento runtime** | El firmware responde `NIC_CAPABILITY` + la MAC, por **TC4/puerto-1**. |
| 4 | **WiFi: SCAN — 14 redes reales** | Escaneó los vecinos de Cartagena (vodafone, MOVISTAR, Telecartagena), canales 1-13, RSSI reales (-63…-95 dBm). |

Y un **quinto LOGRADO (2026-06-22)**: **cfg80211 `wlan0` CERRADO** — `wlan0` registra (wiphy+netdev) y **`iw dev wlan0 scan` lista 16 redes reales desde userspace** (Open-UPCT, eduroam, MikroTik, cpcd…). El scan ya está expuesto a userspace; falta NetworkManager/Phosh + el connect (Fase 2). (El driver real con cfg80211 = 1059 líneas, sincronizado al repo en `mainline/wifi-consys/wifi/mt6582-wifi.c`.)

---

## Línea de tiempo (viernes → lunes)

- **Vie 19** — Tesis HIF confirmada en HW: `func_on(WIFI)` exportado del btif + `WCIR=0x00106582` (el WiFi responde por el bloque HIF @0x180F0000). Descarga de firmware empezada.
- **Sáb 20** — Descarga de firmware **al 100%** (HSTCR, firmware device-correcto, parches 1432), pero `WLAN_READY` **timeout** (el MAC no arranca). Captura del Android stock (KitKat/Lineage) como verdad de referencia.
- **Dom 21** — **BREAKTHROUGH**: un subagente RE-ando el `conn_soc` halló la **calibración RF** que faltaba → **`WLAN_READY=1`** al primer intento. BT emparejado (S24). Protocolo GPS `0xAAF0` capturado.
- **Lun 22** — **cmd/event** (NIC_CAPABILITY + MAC), **SCAN funcionando (14 beacons reales)**, **cfg80211 wiphy/netdev** implementado (compila; registro en debug).

---

## Estado por subsistema

| Subsistema | Estado | Detalle |
|---|---|---|
| Display | ✅ | mtk_drm DRM nativo (panel HX8389 qHD) + Phosh GPU lima |
| Táctil | ✅ | FT5336 (I2C0, EINT117) end-to-end |
| Bluetooth | ✅ | RF arreglado (VCN33) + S24 emparejado |
| **WiFi MAC** | ✅ | Arranca (RF-cal → `WLAN_READY=1`) |
| **WiFi cmd/event** | ✅ | El FW responde NIC_CAPABILITY + MAC (TC4/puerto-1) |
| **WiFi SCAN** | ✅ | 14 beacons reales escaneados (cmd puerto-1, beacons MGMT puerto-0) |
| **WiFi cfg80211 / `wlan0`** | ✅ | **wlan0 registra + `iw dev wlan0 scan` lista 16 redes reales** (Open-UPCT, eduroam, cpcd…) → cfg80211→userspace OK. Falta NM/Phosh + connect |
| **WiFi connect** | ⬜ | Fase 2 (no empezada) — `.connect`/`.add_key` |
| **GPS** | 🟡 | Protocolo `0xAAF0` decodificado + bridge nativo escrito; falta 1 recaptura (comando RUN) |
| Boot | 🟡 | Estable la mayoría de veces, pero **sshd cae intermitentemente** (≈1/4 reboots) → power-cycle |

---

## Mini-roadmap — cómo continuar

### 🔴 Inmediato (1-2 sesiones)

1. **Cerrar cfg80211 / `wlan0`** (el WiFi ya escanea a nivel driver; falta exponerlo a userspace).
   - Power-cycle → re-disparar el bring-up (`echo 1 > /sys/kernel/debug/mt6582_wifi/bringup`) → leer el `dmesg`: aparecerá `cfg80211: wiphy_register=<err>` / `register_netdev=<err>` / `alloc_netdev fallo` → arreglar ese paso.
   - Sospechas: el orden/uso de `wiphy_apply_custom_regulatory`, o el `wireless_dev` embebido (`w->wdev`).
   - Instalar `iw` en el teléfono (`apk add iw` vía NAT, o binario static) → **`iw wlan0 scan`** debe listar las redes → NetworkManager.
   - Nota regdb: cfg80211 built-in pide `regulatory.db` a los 0.96 s (antes de montar el rootfs SD) → `-2`; el `REGULATORY_CUSTOM_REG` (regd propio) debería hacerlo irrelevante; si no, meter el regdb en el initramfs.

2. **GPS al 100%** (protocolo ya decodificado; falta el comando RUN exacto).
   - Flashear Lineage (sector 83968, `lineage13-boot.img`) → **trigger = GPSLogger** (`am start -n com.mendhak.gpslogger/.shortcuts.ShortcutStart`, NO la UI de ajustes) → `strace -p $(pidof mnld) -s 1024` → el `t=0x05` exacto + un frame `0x30` completo (con cielo) → pegar en `mtkgps_aaf0.c` → NMEA real → gpsd → Phosh.
   - Restaurar mainline = `adb reboot bootloader` + `sudo fastboot flash boot ~/mainline/pkg/boot-btifDMA-sd.img` (sin sleep).

### 🟦 Corto plazo

3. **WiFi Fase 2 — conectar a una red WPA2** (tras cfg80211).
   - `.connect`: `CMD_ID_INFRASTRUCTURE` + `CMD_ID_SET_BSS_INFO` + STA-record; `.add_key`: `CMD_ID_ADD_REMOVE_KEY`. Coreografía en `mgmt/ais_fsm.c` del downstream. Objetivo: `wpa_cli status = COMPLETED`.

4. **Estabilizar el boot** — el `sshd`-down recurrente (lleva varios power-cycles) merece un fix: endurecer `zz-sshd.start` (bucle de reintento más robusto) o investigar el cuelgue temprano de runlevel boot.

### 🟪 Medio plazo

5. **WiFi Fase 3 — data-path** (navegar): `ndo_start_xmit` + RX data + PDMA + IRQ del HIFSYS. Objetivo: DHCP + ping.
6. **Botones de Phosh** (brillo/power/toggles), **sensores + autorrotación**.

---

## Dónde está cada cosa (GitHub `Sidihidi/bq-aquiaris-postmarketos`)

| Fichero | Qué es | Commit |
|---|---|---|
| `mainline/drivers/mt6582-btif.c` | btif con la **calibración RF** (arranca BT/GPS/MAC) | `daa77829` |
| `mainline/drivers/mt6582-wifi.c` | driver WiFi: cmd/event + scan + cfg80211 | `1b47c5b0` |
| `mainline/drivers/mt6582-wifi-reg.h` | regs HIF + CID/EID runtime + structs scan | `488fd6c1` |
| `mainline/userspace/mtkgps_aaf0.c` | bridge GPS nativo `0xAAF0`→NMEA | `3b305604` |

**Memorias detalladas** (en el `.claude` local, con todas las recetas y offsets): `reference_mt6582_wifi_hif`, `reference_mt6582_gps`, `reference_mt6582_bt_rf_fix`, `reference_mt6582_boot_stability`, etc.

---

## Notas operativas (recetas turnkey)

- **Build + flash WiFi**: `~/wifi-iter.sh` en la Pi (build zImage + ensambla boot.img + `dd` sector 83968 + reboot). Para que el check apunte al driver WiFi: `sed 's#mt6582-btif.c#mt6582-wifi.c#' ~/wifi-iter.sh`.
- **CONFIG_CFG80211=y** (built-in) ya está en `build-krillin/.config` (un driver built-in no enlaza un módulo).
- **Disparar el WiFi**: `echo 1 > /sys/kernel/debug/mt6582_wifi/bringup` (corre func_on→descarga FW→WLAN_READY→cmd/event→scan→cfg80211).
- **Recaptura GPS**: ver paso 2 del roadmap (Lineage + GPSLogger + strace -s 1024).
- **Si sshd no arranca tras un reboot** (ping OK pero sin SSH): es el cuelgue intermitente → **power-cycle físico**.
- **mtkclient**: `wo <offset> <len> <img>` NUNCA `wf`. **fastboot**: `flash boot <img>` NUNCA `boot`. NUNCA restaurar el LK Lollipop (bootloop); el bueno es KitKat 1.5.2.
