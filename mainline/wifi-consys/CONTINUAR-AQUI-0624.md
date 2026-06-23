# CONTINUAR AQUÍ — BQ Aquaris E4.5 (krillin) mainline — 2026-06-24

Handoff para la sesión del Mac. (Credenciales Pi/WiFi: las de siempre, en tus notas/memoria — no las pongo aquí por si el repo es público. El PAT de GitHub vive en `push_m2b.py`.)

---

## Estado global del dispositivo
| Subsistema | Estado |
|---|---|
| Display (mtk_drm/HX8389) + Phosh acelerado (lima) | ✅ |
| Touch (ft5336) + teclado | ✅ |
| Backlight (slider Phosh) | ✅ |
| Bluetooth (hci0, S24 vinculado) | ✅ |
| USB gadget (usb0, red + SSH) | ✅ |
| **WiFi ABIERTO** | ✅ **navega** (DHCP+ping, auto-connect al boot + lease renewal) |
| **WiFi WPA2** | 🟡 **data-path implementado, BLOQUEADO por bug del kernel — EL TRAMO FINAL (empezar por aquí)** |
| GPS | 🟡 infra desplegada, el chip NO streamea (gap de bring-up) |
| FM radio | ❌ sin empezar |

---

## Acceso / infraestructura
- **Pi** (fuente de verdad del kernel): `cpcd@192.168.0.38` (oscila .123/.124). Árbol: `~/mainline/linux-7.0.12`. Downstream de referencia: `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan`.
- **Móvil** (desde la Pi, por USB): `root@172.16.42.1`. Antes: `sudo ip addr replace 172.16.42.2/24 dev usb0`. SSH con `-o StrictHostKeyChecking=no -o ConnectTimeout=8`.
- **Build SOLO del driver WiFi**: `~/wifi-iter-w.sh` en la Pi (sube wifi.c+reg.h, compila zImage, dd a **sector 83968**, reboot). Driver local: `Desktop\pmos-krillin\wifi-work\mac\{mt6582-wifi.c, mt6582-wifi-reg.h}`.
- **Reglas de oro:** `fastboot flash boot <img>` NUNCA `fastboot boot`. mtkclient: `wo` NUNCA `wf`. NO restaurar LK Lollipop (bueno = KitKat 1.5.2). Boot a veces se cuelga (intermitente, ~1/4) → **power-cycle manual** (síntoma: "No route to host", el gadget USB no enumera).
- Para comandos al móvil con comillas, usa el wrapper base64 (ver `wifi-work/test_wpa2.py`): `echo <b64> | base64 -d | sh`.

---

## 🔑 WPA2 — EL TRAMO FINAL (empezar por aquí mañana)

**Todo el data-path WPA2 ya está en el driver** (`mt6582-wifi.c`, compila, OPEN intacto): IE RSN en el ASSOC (`sme->ie`), `auth_mode=AUTH_MODE_WPA2_PSK(7)`/`enc_status=ENC_STATUS_CCMP_KEY_ABSENT(7)` en SET_BSS_INFO, flag **1X para EAPOL** en `wifi_tx_data` (`ETH_P_PAE`→`HIF_TX_FLAG_1X_FRAME` bit6), `.add_key` con `net_type=AIS`, `cipher_suites={CCMP,TKIP}` en el wiphy. Verificado en HW: el connect funciona para WPA2 (el ASSOC OPEN a la red WPA2 dio `status=13` = el AP exige RSN → nuestro enfoque es correcto).

### ★★ EL MURO = BUG DEL KERNEL (no del driver)
`include/uapi/linux/nl80211.h` del kernel 7.0.12 tiene **`NL80211_CMD_NEW_BEACON` y `NL80211_CMD_DEL_BEACON` como entradas SEPARADAS** del enum `nl80211_commands`, en vez de **ALIAS** de `START_AP`/`STOP_AP` (¡el kernel-doc del propio fichero, líneas ~424/426, dice "old alias for %NL80211_CMD_START_AP"!). Eso mete **2 slots de más** → desde ahí todo desplazado +2: **`CONNECT=48` (estándar 0x2e=46)**, `AUTHENTICATE=39` (37), `ASSOCIATE=40` (38), `DISCONNECT=50` (48). La **ABI nl80211 queda rota** vs userspace estándar → `wpa_supplicant v2.11` busca `CONNECT=46`, no lo halla → *"nl80211: Driver does not support authentication/association or connect commands"* + ciphers vacíos en `iw list`.

### FIX (en el kernel)
En `~/mainline/linux-7.0.12/include/uapi/linux/nl80211.h`, dentro de `enum nl80211_commands`:
```c
NL80211_CMD_NEW_BEACON = NL80211_CMD_START_AP,   /* restaurar el alias (quitar la entrada suelta) */
...
NL80211_CMD_DEL_BEACON = NL80211_CMD_STOP_AP,    /* restaurar el alias */
```
(Es decir: poner el `= NL80211_CMD_START_AP/STOP_AP` que les falta.) Restaura los valores estándar (CONNECT=46…).

### Pasos
1. Editar el `nl80211.h` (las 2 líneas).
2. **REBUILD COMPLETO del kernel** (NO vale `wifi-iter-w.sh`; `nl80211.h` se incluye por todas partes → compila largo, ~20-40 min): `cd ~/mainline/linux-7.0.12 && make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage` + empaquetar + flashear (ver el final de `wifi-iter-w.sh` para el empaquetado/dd).
3. Test (red: SSID **"hola"**, WPA2, pass **"kakatua1"**). En el móvil: parar el auto-connect OPEN (`mv /etc/local.d/wifi-hola.start /etc/local.d/wifi-hola.start.off; pkill -f wifi-hola; pkill wpa_supplicant`), `ip link set wlan0 up`, escribir `/tmp/wpa.conf`:
   ```
   ctrl_interface=/var/run/wpa_supplicant
   network={ ssid="hola" psk="kakatua1" key_mgmt=WPA-PSK proto=RSN pairwise=CCMP group=CCMP scan_ssid=1 }
   ```
   luego `wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -dd -t -f /tmp/wpa.log -B` + vigilar `wpa_cli -i wlan0 status` (debe llegar a `COMPLETED`) → `udhcpc -i wlan0`. Si conecta → 4-way handshake → `.add_key` → DHCP cifrado. (Harness completo: `test_wpa2.py` en la máquina Windows — tiene la pass de la Pi, no está en GitHub.)
4. **CAVEAT:** el `iw` del móvil quizá está compilado contra los headers ROTOS (el `iw connect` del auto-connect OPEN funcionaba = usa 48). Tras el fix, ese `iw connect` puede romperse. Plan: migrar el auto-connect OPEN a wpa_supplicant (`key_mgmt=NONE`) o recompilar iw. El auto-connect OPEN está en `/etc/local.d/wifi-hola.start`.
5. Posible siguiente escollo (ya fichado): tras instalar la clave, puede que el FW necesite `enc_status`→`ENC_STATUS_CCMP_ENABLED(6)` (ahora va KEY_ABSENT) vía un SET_BSS_INFO de refresco.

(Detalle completo: memoria `reference_mt6582_wifi_wpa2`.)

---

## GPS (siguiente tras WPA2)
Infra desplegada: btif expone `/dev/stpgps` (STP canal 2, TX `gps_write`→`stp_send`, RX kfifo), bridge `mtkgps_aaf0` (en este repo: `gps/mtkgps_aaf0.c`; protocolo 0xAAF0 decodificado, scan-decoder auto-detecta la posición), `gpsd` + servicio `zzz-gps.start`. **El chip NO streamea** pese al START_BURST: `func_on(GPS)` enciende el subsistema pero **no arranca el firmware GPS**. Lineage lo arranca vía `/sys/class/gpsdrv/gps/` (driver `gpsdrv` del downstream, AUSENTE en mainline). Pendiente: replicar ese arranque en el btif + cielo despejado para el fix. Trazas de referencia: `~/mnld-init.strace`, `~/mnld-live.strace` en la Pi. (Memoria `reference_mt6582_gps`.)

## FM radio (después)
De cero: el btif tiene el concepto del canal STP FM=1 pero no hace `func_on[FM]` ni expone `/dev/fm`. Subsistema nuevo.

---

## GitHub (`Sidihidi/bq-aquiaris-postmarketos`, `mainline/wifi-consys/`)
- `wifi/mt6582-wifi.c` + `mt6582-wifi-reg.h` — driver con WPA2 data-path (commit c197f833+).
- `wifi-hola.start` — servicio auto-connect OPEN (OpenRC local.d).
- `gps/mtkgps_aaf0.c` — bridge GPS.
- `tools/test_wpa2.py`, `tools/apply_cleanup.sh` — harness de test + limpieza .remove de drivers.

## Recordatorios
- El WiFi abierto navega: si quieres WiFi YA (antes del fix WPA2), pon "hola" en abierta (el auto-connect conecta solo al boot).
- Co-autor de commits: `Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>`.
- Cada vez más cerca del 100%. 🚀
