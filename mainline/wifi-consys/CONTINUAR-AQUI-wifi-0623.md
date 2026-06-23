# 🎯 WiFi connect — BREAKTHROUGH 2026-06-23 (sesión Opus): el `MPDU_ERROR` era el CANAL

> Handoff para la sesión del Mac. Repo: **Sidihidi/bq-aquiaris-postmarketos**.
> Driver autoritativo: `mainline/wifi-consys/wifi/mt6582-wifi.c` (commit **1a5fb6d2**) + `mt6582-wifi-reg.h` (**f4b6349f**).
> Pi `cpcd@192.168.0.38`, móvil mainline `root@172.16.42.1` (USB). Flash mainline = `fastboot flash boot ~/mainline/pkg/boot-btifDMA-sd.img`.

---

## ⭐ EL HALLAZGO QUE RESUELVE TODA LA SAGA DEL MPDU_ERROR

El `TX_RESULT_MPDU_ERROR` (status=3) en el AUTH del connect L2 **NO era el firmware** — era el **CANAL EQUIVOCADO** en el comando de prueba.

- El AP "hola" (BSSID `fa:6a:24:00:df:aa`) **AUTO-CAMBIA de canal** (lo vimos en ch11/2462 y luego saltó a otro).
- El comando de prueba usaba `iw dev wlan0 connect hola 2412 …` (canal 1) — un valor **ASUMIDO** del `FASE2-CONNECT.md` que nadie verificó contra el scan.
- El `CH_PRIVILEGE` concedía ch1, el MAC se paraba en ch1, el AUTH salía por ch1 → **no hay AP ahí → sin ACK → `MPDU_ERROR`** (MPDU_ERROR = típicamente "sin ACK del AP", confirmado por RE del downstream).

**Con el canal REAL del scan (2462 en ese momento):**
```
*** TX-DONE seq=1 status=0 (SUCCESS transmitido+ACK) ***
*** mgmt-RX subtype=11 len=30 b0 00 3a 01 <DA> fa6a2400dfaa fa6a2400dfaa <seq> 00 00 02 00 00 00 ***
                                                                              ^^^^^ AUTH seq=2, status=0
```
**EL AUTH FUNCIONA.** Toda la coreografía (`BSS_ACTIVATE → CH_PRIVILEGE → grant 0x18 → UPDATE_STA_RECORD(STATE_1, idx 0) → wifi_send_auth`) estaba **correcta desde el principio**. Las 3 pasadas de RE del FW (índice 1→0, tasas 0x3fff/0x0003, delay cmd-vs-TX) **verificaron** que era correcta pero NINGUNA era el bug.

### ⚠️ LECCIÓN AL PROBAR — NUNCA hardcodear el canal
El AP auto-cambia → un canal fijo falla intermitentemente. **SIEMPRE escanear y usar el canal del beacon**: `iw connect <ssid>` sin canal usa el de la caché del scan (robusto), o parsear `iw dev wlan0 scan` y pasar la freq real:
```python
sc = phone('iw dev wlan0 scan')
for blk in sc.split('BSS '):
    if 'SSID: hola' in blk:
        freq = re.search(r'freq:\s*([0-9]+)', blk).group(1)
phone(f'iw dev wlan0 connect hola {freq} fa:6a:24:00:df:aa')
```

---

## ✅ ASSOC implementado (commit 1a5fb6d2) — paso 2/3 del SAA

Tras el AUTH, el SAA sigue: AUTH-2 → ASSOC-REQ → ASSOC-RESP → conectado. Implementado en `mt6582-wifi.c`:

- **`wifi_rx_mgmt`**: al recibir **subtype 11 (AUTH-2)** con `aseq==2 && status==0` y `w->connecting` → llama `wifi_send_assoc()`. (El RX corre bajo `hif_lock` vía `wifi_rx_thread`, así que puede TX directamente.) Si el status≠0 → `cfg80211_connect_result(FAIL)`.
- **`wifi_send_assoc()`** (nueva): construye el ASSOC-REQ (subtype 0): `fc=0x0000`, da/sa/bssid, `cap=WLAN_CAPABILITY_ESS`, `listen_int=1`, IEs **SSID** + **supported-rates** (1,2,5.5,11,6,9,12,18) + **ext-rates** (24,36,48,54). TX por `wifi_send_mgmt` (TC4/sta_idx 0).
- **`wifi_rx_mgmt`**: al recibir **subtype 1 (ASSOC-RESP)** → `cfg80211_connect_result(SUCCESS/FAIL según status@offset26)`, `connecting=false`.
- **Struct**: añadido `connect_ssid[32]` + `connect_ssid_len`, rellenados en `.connect`.

**Flasheado pero SIN probar end-to-end** (bloqueado por la flakiness de boot, abajo). Falta confirmar que el AP responde el assoc-resp y que `iw link = Connected`. Si el assoc-resp llega con status≠0, ajustar `cap`/rates del ASSOC-REQ para matchear el beacon de "hola".

---

## 🔴 EL CUELLO DE BOTELLA AHORA: flakiness de boot (lo SIGUIENTE a atacar)

Dos cosas hacen MUY lento iterar (cada prueba = cazar un boot bueno):
1. **`WLAN_READY` flaky**: el bring-up del MAC WiFi falla en **~3 de cada 4 boots** (el scan devuelve 0 beacons). Hay que rebootear hasta pillar uno bueno (`echo 1 > /sys/kernel/debug/mt6582_wifi/bringup` + `iw scan`, repetir).
2. **Cuelgue de boot**: a veces el móvil **no vuelve** tras un reboot (gadget enumera pero no completa) → power-cycle físico.

**Esto es el freno nº1 para todo** (connect, BT, lo que sea). Pistas a investigar: la **cal RF por boot** (¿no determinista?), el timing de `func_on`/descarga-FW, el poll de `WLAN_READY`. Ver `reference_mt6582_boot_stability` (el sshd-down ya está resuelto; esto es OTRA cosa, del bring-up del FW WiFi).

---

## ▶️ PRÓXIMOS PASOS
1. **Probar el ASSOC end-to-end**: cazar boot bueno → escanear "hola" → **freq real del scan** → connect → buscar en `dmesg`: `AUTH-2 OK -> ASSOC-REQ`, `mgmt-TX: ASSOC-REQ`, `mgmt-RX subtype=1`, `ASSOC-RESP status=0 (CONNECTED)`, `iw link = Connected`. → **asociación L2 cerrada**.
2. **Atacar la flakiness `WLAN_READY`** (el freno real).
3. **Data-path** (Fase 3 datos): `ndo_start_xmit` + RX data + PDMA → DHCP/ping; luego **WPA2** (4-way handshake + add_key).

---

## 📋 Método de captura Lineage (verdad de referencia, por si hace falta)
- Flashear `~/lineage13-boot.img` por fastboot → krillin Android 6.0.1, adbd root.
- `echo D3 > /dev/wmtWifi` = log verboso wlan a **dmesg** (no logcat): SAA STATE + TX-DONE + cmds.
- FSM SAA del stock: `SEND_AUTH1(1)→WAIT_AUTH2(2)→SEND_ASSOC1(5)→WAIT_ASSOC2(6)→IDLE(0)` (`aa_fsm.h`).
- **kprobe a `kalDevPortWrite`** (sdio.c:1101, el cuello de TODO el TX) **NO disponible** (kernel Lineage sin `CONFIG_KPROBE_EVENTS`/ftrace).
- El framework Android **NO auto-conecta a "hola"** (sin internet) → la captura en vivo pelea con el framework; el `kmsg.log` viejo en `~/android-cap/lineage/` ya tiene el FSM.
- Restaurar mainline = `fastboot flash boot ~/mainline/pkg/boot-btifDMA-sd.img`.
