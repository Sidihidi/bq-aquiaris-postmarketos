# CONTINUAR AQUÍ — BQ Aquaris E4.5 (krillin) mainline — 2026-06-24

Handoff para la sesión de casa. (Credenciales Pi/WiFi: las de siempre, en tus notas/memoria — no las pongo aquí por si el repo es público. El PAT de GitHub vive en `push_m2b.py`.)

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
| **WiFi WPA2** | 🟡 **autentica + fix del data-path aplicado (enc_status=6 en el join) — PROBAR esto primero** |
| GPS | 🟡 infra desplegada, el chip NO streamea (gap de bring-up) |
| FM radio | ❌ sin empezar |

---

## Acceso / infraestructura
- **Pi de casa**: `cpcd@192.168.0.38`. (La Pi de build de la sesión Mac es la `.123`, otra distinta — no las confundas.) Árbol del kernel: `~/mainline/linux-7.0.12`. Downstream de referencia: `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan`.
- **Móvil** (desde la Pi, por USB): `root@172.16.42.1`. Antes: `sudo ip addr replace 172.16.42.2/24 dev usb0`. SSH con `-o StrictHostKeyChecking=no -o ConnectTimeout=8`.
- **Build SOLO del driver WiFi**: `~/wifi-iter-w.sh` en la Pi (sube wifi.c+reg.h, compila zImage, dd a **sector 83968**, reboot).
- **Reglas de oro:** `fastboot flash boot <img>` NUNCA `fastboot boot`. mtkclient: `wo` NUNCA `wf`. NO restaurar LK Lollipop (bueno = KitKat 1.5.2). Boot a veces se cuelga (intermitente, ~1/4) → **power-cycle manual** (síntoma: "No route to host", el gadget USB no enumera).
- Para comandos al móvil con comillas, usa el wrapper base64.

---

## 🔑 WPA2 — fix del data-path APLICADO (probar esto primero)

**El data-path WPA2 está completo + autentica** (4-way `COMPLETED`, `[PTK=CCMP GTK=CCMP]`, `iw link Connected`). Esta sesión (Mac, con 2 audits del usuario) encontró y arregló el bloqueo del **data-path cifrado** (el DHCP no sacaba IP). **Falta probarlo en HW** (el móvil quedó colgado tras los tests → power-cycle).

### El bug (arreglado, commit 7a3ddf5): `enc_status` en el join
El driver mandaba `ENC_STATUS_CCMP_KEY_ABSENT(7)` en el `SET_BSS_INFO` del join; debe ser **`ENC_STATUS_CCMP_ENABLED(6)`**. Verificado contra el downstream mt6628: `eEncStatus` es la **política de cifrado** (CCMP=6), guardada al connect (`wlanoidSetEncryptionStatus`, wlan_oid.c:3395) y enviada en el join (nic.c:2034, rama AIS). El `KEY_ABSENT(7)` es solo el valor que el **QUERY** le devuelve a userspace según `fgTransmitKeyExist` (wlan_oid.c:3261) — NO el valor del comando al FW. Con 7 el FW no transiciona a cifrar → datos sin cifrar → DHCP sin OFFER. Con 6 activa CCMP y espera la clave por `CMD_802_11_KEY`; **transiciona solo al instalarla** — sin 2º SET_BSS_INFO (el "refresh" que se probó CRASHEA y es innecesario).

### Fixes complementarios (commit d0ab69c)
- `tx_key = pairwise?1:0`: la **GTK NO debe marcarse TX-key** (IS_TRANSMIT_KEY=BIT31 en el downstream); si no, el FW puede cifrar el TX unicast con la GTK → el AP no descifra.
- `.del_key` implementado (antes stub, CMD add_remove=0); `.set_default_key` se deja stub (el downstream también es no-op, "work around aosp", WEP-legacy).
- `btif.c` `.remove()` sincronizado repo←Pi; `*.bak*` al `.gitignore`.

### Probar (red SSID **"hola"** WPA2 / o **"cpcd"**; claves en tu memoria)
1. Flashear el zImage nuevo (ya compilado en la Pi `.123`; en casa: `wifi-iter-w.sh` o `dd` a sector 83968).
2. En el móvil: parar el auto-connect OPEN (`mv /etc/local.d/wifi-hola.start{,.off}; pkill -f wifi-hola; pkill wpa_supplicant`), `ip link set wlan0 up`, escribir `/tmp/wpa.conf`:
   ```
   ctrl_interface=/var/run/wpa_supplicant
   network={ ssid="hola" psk="..." key_mgmt=WPA-PSK proto=RSN pairwise=CCMP group=CCMP scan_ssid=1 }
   ```
   `wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -dd -t > /tmp/wpa.log 2>&1 &` (¡sin `-f`, este build no lo tiene!), esperar `wpa_cli -i wlan0 status` = `COMPLETED`, luego `udhcpc -i wlan0`. **Si saca IP → WPA2 navega.** 🎉
3. Si AÚN no saca IP: plan B = el path de RX de datos cifrados (`wifi_rx_data` con CCMP) / el filtro RX. NO re-mandar SET_BSS_INFO.

> ⚠️ El "fix nl80211.h" de versiones previas de este handoff **NO aplica** — ese bug NO existe (alias `NEW_BEACON=START_AP` presentes, CONNECT=46, `iw list` muestra CCMP/TKIP). Verificado 2×. Descartado.

---

## GPS (siguiente tras WPA2)
Infra desplegada: btif expone `/dev/stpgps` (STP canal 2), bridge `mtkgps_aaf0` (protocolo 0xAAF0 decodificado), `gpsd` + servicio `zzz-gps.start`. **El chip NO streamea** pese al START_BURST: `func_on(GPS)` enciende el subsistema pero no arranca el firmware GPS. Lineage lo arranca vía `/sys/class/gpsdrv/gps/` (driver `gpsdrv` del downstream, AUSENTE en mainline). Pendiente: replicar ese arranque en el btif + cielo despejado.

## FM radio (después)
De cero: el btif tiene el concepto del canal STP FM=1 pero no hace `func_on[FM]` ni expone `/dev/fm`.

---

## Estabilidad del boot (del audit 0624 — pendiente, no urge)
- **pstore/ramoops**: el dts tiene nodo ramoops; verificar `reserved-memory` (`no-map`) + `CONFIG_PSTORE_RAM=y` para leer el dmesg tras un cuelgue duro (`/sys/fs/pstore/`). Lo que más ayudaría a entender los cuelgues (ahora un cuelgue duro es ciego).
- **WDT hardware** (mtk_wdt, RGU @0x10007000) + `wd_keepalive`: auto-recuperación sin power-cycle si el kernel se cuelga.
- **Phosh + BT/GPS/WiFi → OpenRC `supervise-daemon`** (no `local.d`+nohup): la GUI se respawnea si cae (~1/3 de los boots no sube la GUI).
- **Watchdog del gadget USB**: re-pokear soft-connect si usb0 pierde carrier → no perder la consola.

---

## GitHub (`Sidihidi/bq-aquiaris-postmarketos`, `mainline/wifi-consys/`)
- `wifi/mt6582-wifi.c` + `mt6582-wifi-reg.h` — driver WiFi (WPA2 data-path, commits d0ab69c + 7a3ddf5).
- `m3a/mt6582-btif.c` — CONSYS/BT/GPS (con `.remove()` sincronizado).
- `wifi-hola.start` — auto-connect OPEN. `gps/mtkgps_aaf0.c` — bridge GPS.

## Recordatorios
- El WiFi abierto navega: si quieres WiFi YA, pon "hola" en abierta (auto-connect al boot).
- Co-autor de commits: `Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>`.
- Cada vez más cerca del 100%. 🚀
