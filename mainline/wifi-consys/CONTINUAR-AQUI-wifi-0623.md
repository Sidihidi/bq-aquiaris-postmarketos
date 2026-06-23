# 🎯 WiFi — CONTINUAR AQUÍ (handoff 2026-06-23 NOCHE, sesión Opus)

> Repo: **Sidihidi/bq-aquiaris-postmarketos**. Driver: `mainline/wifi-consys/wifi/mt6582-wifi.c` + `mt6582-wifi-reg.h`.
> Pi = puente USB al móvil. **IP DINÁMICA** (fue `.38` por la mañana, `.123` por la tarde — busca la actual: router / `arp -a` / `nmap`). Móvil mainline `root@172.16.42.1` por USB (a través de la Pi).
> **Build+flash WiFi**: editar el `.c` en el repo → `scp` al kernel-tree del Pi (`~/mainline/linux-7.0.12/drivers/soc/mediatek/`) → `bash ~/wifi-iter-w.sh` (build zImage + `dd` a sector 83968 + reboot). Para cambios de **DTS**: `make ... dtbs` antes y re-empaquetar.

---

## 🟢 3 AVANCES GRANDES ESTA SESIÓN (todos en el commit de hoy)

### 1. FLAKINESS `WLAN_READY` CURADA — `regulator-always-on` en VCN33  ✅ CONFIRMADO EN HW
- **Causa raíz**: la cal RF (`01 14 01 00 01`) la hace `bring_up_chip` (path de BT, `mt6582-btif.c`) UNA vez al boot con VCN33 on. Pero **`vcn33_wifi` se auto-apagaba a los ~31s** (regulador sin holder → limpieza del core: `vcn33_wifi: disabling`) → se perdía la cal RF → scan a **0 beacons** (~3/4 boots). La cal **NO es re-emitible en runtime** (lo probé: da timeout vía WMT).
- **Fix**: `regulator-always-on;` en el nodo `ldo_vcn33_wifi` (`dts/mt6582-bq-krillin.dts`). VCN33 nunca se apaga → la cal del boot se preserva.
- **CLAVE / no repetir mi error**: **NO** conmutar VCN33 off→on en `wifi_bringup` — ese flanco TIRA la cal. El `mt6582_consys_wifi_vcn33(true)` del bring-up es un no-op (refcount++), sin flanco, y WLAN_READY afirma igual.
- **Confirmado**: tras los 31s (donde antes fallaba), bring-up → `WLAN_READY=1` + scan **5 redes reales** (eduroam, Open-UPCT...). Antes 0. `vcn33 disabling=0`.

### 2. CONNECT SE REGISTRA — `cfg80211_connect_bss` con BSS retenido  ✅ CONFIRMADO EN HW
- **Causa**: el driver reportaba con `cfg80211_connect_result(dev, bssid, ...)` (solo BSSID) → cfg80211 buscaba el BSS en su caché por BSSID y NO lo encontraba → `WARN_ON(bss_not_found)` en `net/wireless/sme.c:845` → **abortaba el registro** (`iw link = Not connected`) AUNQUE el AUTH+ASSOC iban perfectos (status=0).
- **Fix**: retener el BSS — `cfg80211_get_bss(...)` en `.connect` → `w->connect_bss` (campo nuevo); reportar con `cfg80211_connect_bss(..., w->connect_bss, ..., NL80211_TIMEOUT_UNSPECIFIED)`. Liberarlo en `.disconnect`. (El kernel lo documenta en `cfg80211.h`: *"hold a reference ... to avoid a warning if the bss is expired during the connection"*.)
- **Confirmado**: `iw link = Connected to fa:6a:24:00:df:aa`, `WARN sme.c = 0`. **L2 cerrada y registrada.**

### 3. JoinComplete — `SET_BSS_INFO(CONNECTED)` tras ASSOC-RESP  ⚠️ IMPLEMENTADO, **SIN PROBAR**
- **Por qué**: tras asociar, el DHCP no saca lease. Diagnóstico: **TX sí sale** (`tx_packets` sube, +4 en un DHCP) pero **RX=0** (ni el OFFER ni broadcasts). El driver NO mandaba `SET_BSS_INFO(CONNECTED)` (el JoinComplete, paso 5 del SAA) → el FW queda *asociado* pero NO en estado de DATOS → descarta el TX y no entrega RX.
- **Implementado**: `wifi_send_join()` (nueva) = `SET_BSS_INFO` con `conn_state=CONNECTED, ssid, bssid, op_rate/basic_rate=ERP, sta_rec_idx_of_ap=0, auth=OPEN, enc=OFF, phy=11BG, own_mac, rlm.primary_channel=connect_channel, rlm.check_id=0x72`. Se llama en `wifi_rx_mgmt` al recibir ASSOC-RESP status=0, **antes** de `cfg80211_connect_bss`. Compila limpio y está flasheado.
- **SIN CONFIRMAR**: no llegó a probarse (el móvil no reconectó tras el último flash —flakiness de boot/sshd, no el código— y se guardó el HW).

---

## ▶️ LO SIGUIENTE (probar en casa, con 'hola' levantado)
1. Cazar boot bueno (ya fiable con always-on) → bring-up → scan → **connect a 'hola' con la freq REAL del scan** → `udhcpc -i wlan0`.
2. En `dmesg` buscar `JoinComplete: SET_BSS_INFO(CONNECTED)`. Si **saca IP + `ping 8.8.8.8`** → 🎉 **data-path cerrado, la WiFi navega.**
3. **SI EL DHCP SIGUE FALLANDO (RX=0)** — orden de sospechas:
   - **`conn_state`**: usé **2** (de `FASE2-CONNECT.md`); el `reg.h` dice `MEDIA_STATE_CONNECTED=0`. **Probar `bi.conn_state = 0`** en `wifi_send_join` (es la incógnita nº1).
   - Si no, ¿el FW necesita `UPDATE_STA_RECORD` con `sta_state=STA_STATE_3` antes del SET_BSS_INFO? (la transición a "asociado").
   - Si no, revisar el **RX path**: `wifi_rx_drain` (puerto 0) → `wifi_rx_data` (¿se llama? ¿el `header_len_offset`/strip correcto?). Mirar `rx_packets`.
4. Luego **WPA2**: `.add_key` (CCMP) + dejar a wpa_supplicant hacer el 4-way EAPOL por el data-path.

## 🔑 Secuencia de prueba
```sh
echo 1 > /sys/kernel/debug/mt6582_wifi/bringup ; sleep 8          # bring-up (fiable con always-on)
iw dev wlan0 scan | grep -B40 'SSID: hola' | grep -E 'freq|^BSS'  # freq + BSSID REALES (no hardcodear!)
iw dev wlan0 connect hola <FREQ> <BSSID>
iw dev wlan0 link                                                 # -> Connected
udhcpc -i wlan0 -n -t 6 -q                                        # -> IP?  (el test del data-path)
ping -c3 8.8.8.8
dmesg | grep -iE 'JoinComplete|ASSOC-RESP|RF-CAL'
cat /sys/class/net/wlan0/statistics/{tx_packets,rx_packets}       # RX>0 = el FW entrega datos
```

## 📌 Background preservado
- **El `MPDU_ERROR` del AUTH era el CANAL equivocado**, no el FW. El AP 'hola' (`fa:6a:24:00:df:aa`) auto-cambia de canal → SIEMPRE la freq del scan.
- Coreografía `.connect` (verificada): `BSS_ACTIVATE → CH_PRIVILEGE → grant 0x18 → UPDATE_STA_RECORD(STATE_1, idx 0) → AUTH(basic-rate) → [RX AUTH-2] → ASSOC-REQ → [RX ASSOC-RESP status=0] → JoinComplete (SET_BSS_INFO, NUEVO)`.
- Captura de referencia Lineage: `fastboot flash boot ~/lineage13-boot.img`; `echo D3 > /dev/wmtWifi` = log wlan a dmesg. Restaurar mainline = `fastboot flash boot ~/mainline/pkg/boot-btifDMA-sd.img`.
- Boot/sshd flaky: el sshd-down se resolvió con `sshd_disable_keygen`; el WLAN_READY ya está curado (always-on). Aún hay algún cuelgue de boot ocasional → power-cycle.
