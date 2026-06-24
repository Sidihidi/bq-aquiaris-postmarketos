# Handoff — sesión 0624 (tarde, Opus): scan-crash RESUELTO + WPA2/CCMP causa-raíz + brillo

Continúa de `handoff-wifi-0624/`. Driver autoritativo: `mac/mt6582-wifi.c` + `mac/mt6582-wifi-reg.h` (= lo desplegado en la Pi).

## WiFi

### ✅ SCAN-CRASH RESUELTO
Guard WCIR **por-palabra** en `wifi_port_read_pio` Y `wifi_port1_read_pio`: re-sondea `MCR_WCIR` (registro estático, no toca el FIFO) ANTES de cada `rd(MCR_WRDR0)`; si `!= WIFI_CHIP_ID_6582` → `w->started=false`, `memset(resto,0)`, return. **Probado en HW: el móvil sobrevive `iw scan` estando conectado** (antes = hard-lockup/WDT + power-cycle). El guard `wifi_hif_alive` 1-shot de la sesión Mac NO bastaba.

### 🟡 WPA2/DHCP — CCMP roto a NIVEL FW (documentado, NO resuelto)
**El host hace TODO bien** (verificado byte-a-byte vs downstream): claves CCMP instaladas (`cipher=0xfac04`, PTK `tx_key=1`, GTK `peer=ff:ff:ff`), struct `cmd_802_11_key`, `CIPHER_SUITE_CCMP=4`, `CMD_ID_ADD_REMOVE_KEY=0x08`/`DEFAULT_KEY_ID=0x09`, `tx_key`/`key_type` (de BIT31/BIT30 de u4KeyIndex), StaRec a STATE_3 + `EVENT_ACTIVATE_STA_REC`(0x13), `wpa_cli` dice `pairwise/group=CCMP COMPLETED`. **PERO el FW no cifra/descifra DATOS:** `tx=55 rx=2`, 0 respuestas uni/broadcast (ARP estática→BSSID tampoco), mientras la red **ABIERTA navega**.
- **enc_status: PROBADO `ENABLED(6)`** (gl_cfg80211.c:726 mapea CCMP→ENABLED, nic.c:2034 lo manda) en HW con subnet/gateway REALES → **NO arregla el DHCP + causa `eid=0x1b`** (beacon timeout). REVERTIDO a `KEY_ABSENT(7)`.
- **OJO falso positivo:** un `ping 8.8.8.8` exitoso salía por `usb0` (NAT de la Pi, default route), NO por wlan0. Forzado `-I wlan0` falla. Verificar SIEMPRE con la IP/contador de wlan0.
- **Red de test:** SSID `hola`, WPA2, pass `kakatua1`, subnet `10.181.211.x`, gateway `10.181.211.204`, BSSID `e6:92:82:f4:ce:44`.
- **Leads restantes** (inciertos): (1) instrumentar si el FW ACKea/aplica `CMD_802_11_KEY` (¿programa la HW CCMP?); (2) RE del handler downstream de `EVENT_ACTIVATE_STA_REC`(0x13) — ¿el host debe responder algo para abrir el data-path cifrado?; (3) comparar la secuencia post-assoc COMPLETA (¿falta `UPDATE_WMM`(0x21) si el AP es QoS?).

Driver limpio (quitado el `saved_bi`/refresh muerto del experimento enc_status). Auto-connect WPA2 en `wifi-hola-wpa2.start` (la PSK aquí va saneada; pon la real al desplegar).

### ⚠️ NO TOCAR
El enum `nl80211` (`CONNECT=48`, desplazado) es **self-consistent** kernel↔userspace de Alpine → cambiarlo ROMPE el WPA2. El "muro = bug enum" de una sesión previa era falso.

## Brillo (aparte)
`bl 0-100` (comando) funciona (daemon→PWM `+0xa0`). El slider de Phosh: el fix de la **carrera** (name-wait en `phosh-session.sh` para que el shim D-Bus registre el nombre ANTES de `exec phosh`) hizo que Phosh **detecte** el backend (`ON_SET Brightness=35` al arrancar, antes 0), pero el slider AL MOVERLO sigue inerte → falta `gsd-power` completo (sesión elogind de sxmo) o recompilar Phosh.

— Co-Authored-By: Claude Opus 4.8 (1M context)
