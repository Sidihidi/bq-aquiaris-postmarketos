# WiFi Fase 2 — `.connect` (blueprint de implementación, structs extraídos 2026-06-22)

> Estado: **structs + CMD_IDs + evento + coreografía EXTRAÍDOS** del downstream
> (`combo/drv_wlan/mt6628/wlan/include/nic_cmd_event.h` — misma familia que el CONSYS MT6582).
> Falta: codificar en `mt6582-wifi.c` + build/flash/test + iterar la coreografía. La infra del driver
> (`wifi_send_cmd` por TC4/puerto-1) ya está; el `.scan` es la plantilla.
>
> **Por qué esto desbloquea NM/Phosh Y el connect**: wpa_supplicant NO inicializa `wlan0` sin `.connect`
> (`nl80211: Driver does not support authentication/association or connect commands`). Con `.connect`
> registrado → wpa_supplicant arranca → NM/Phosh muestran el scan **y** se puede conectar.

## CMD_IDs (runtime, por TC4/puerto-1) y EVENT_IDs
```
CMD_ID_ADD_REMOVE_KEY    0x08    CMD_ID_BSS_ACTIVATE_CTRL  0x15
CMD_ID_DEFAULT_KEY_ID    0x09    CMD_ID_SET_BSS_INFO       0x16
CMD_ID_INFRASTRUCTURE    0x0a    CMD_ID_UPDATE_STA_RECORD  0x17
EVENT_ID_CONNECTION_STATUS  0x03   EVENT_ID_ASSOC_INFO  0x08
```

## Structs (transcritos a C para el driver, `__packed`)
```c
/* CMD_ID_SET_BSS_INFO (0x16) — el comando central de "conectar a este BSS" */
struct cmd_set_bss_info {
	u8 net_type_idx;	/* NETWORK_TYPE_AIS=0 */
	u8 conn_state;		/* PARAM_MEDIA_STATE_CONNECTED=2 / DISCONNECTED=0 */
	u8 cur_op_mode;		/* OP_MODE_INFRASTRUCTURE=1 */
	u8 ssid_len;
	u8 ssid[32];
	u8 bssid[6];
	u8 is_qbss;
	u8 rsv1;
	u16 op_rate_set;	/* p.ej. 0 = que el FW elija; o el rate set del beacon */
	u16 basic_rate_set;
	u8 sta_rec_idx_of_ap;	/* el índice del STA-record del AP (de UPDATE_STA_RECORD) */
	u8 rsv2, rsv3;
	u8 nonht_basic_phy;
	u8 auth_mode;		/* AUTH_MODE_OPEN=0 / SHARED=1 / WPA2PSK=7 (extraer ENUM_PARAM_AUTH_MODE) */
	u8 enc_status;		/* ENUM_PARAM_ENCRYPTION_STATUS: DISABLED=0 / WEP / TKIP / CCMP-AES */
	u8 phy_type_set;
	u8 own_mac[6];		/* w->mac (la permanente que leemos en Fase1) */
	u8 wapi_mode;
	u8 is_ap_mode;
	u8 rsv[1];
} __packed;	/* 65 B aprox — verificar tamaño vs el FW */

/* CMD_ID_ADD_REMOVE_KEY (0x08) — instalar PTK/GTK (para WPA2) */
struct cmd_802_11_key {
	u8 add_remove;		/* 1=add 0=remove */
	u8 tx_key;
	u8 key_type;		/* 1=pairwise 0=group */
	u8 is_authenticator;
	u8 peer_addr[6];
	u8 net_type;
	u8 algorithm_id;	/* CIPHER_SUITE: NONE/WEP40/WEP104/TKIP/CCMP */
	u8 key_id;
	u8 key_len;
	u8 rsv[2];
	u8 key_material[32];
	u8 key_rsc[16];
} __packed;	/* 64 B */

struct cmd_bss_activate_ctrl {	/* CMD_ID_BSS_ACTIVATE_CTRL (0x15) */
	u8 net_type_idx; u8 active; u8 rsv[2];
} __packed;

/* CMD_SET_BSS_RLM_PARAM (canal/banda) — fija el canal del AP antes de asociar */
struct cmd_set_bss_rlm_param {
	u8 net_type_idx, rf_band, primary_channel, rf_sco;
	u8 erp_prot, ht_prot, gf_mode, tx_rifs;
	u16 ht_op3, ht_op2; u8 ht_op1, short_preamble, short_slot, check_id /*=0x72*/;
} __packed;

/* EVENT_ID_CONNECTION_STATUS (0x03) — llega async al kthread RX */
struct event_connection_status {
	u8 media_status;	/* 2 = CONNECTED, 0 = DISCONNECTED -> cfg80211_connect_result/disconnected */
	u8 reason;
	u8 infra_mode; u8 ssid_len; u8 ssid[32]; u8 bssid[6];
	u8 auth_mode; u8 enc_status;
	u16 beacon_period; u16 aid; u16 atim;
	u8 net_type; u8 rsv[1]; u32 freq_khz;
} __packed;
```
(`CMD_UPDATE_STA_RECORD_T` es grande — el record del AP: idx, type, mac[6], aid, listen_int, phy, rates,
ht_cap… Para OPEN basta lo básico; extraer completo del header para WPA2/HT.)

## Coreografía del `.connect` (de `mgmt/ais_fsm.c:aisFsmRunEventJoinRequest`)
```
1. CMD_ID_INFRASTRUCTURE          -> op-mode STA
2. CMD_ID_BSS_ACTIVATE_CTRL {AIS, active=1}
3. CMD_SET_BSS_RLM_PARAM {primary_channel = sme->channel}   (fija el canal)
4. CMD_ID_UPDATE_STA_RECORD {el AP: bssid, rates, ...}      -> sta_rec_idx
5. CMD_ID_SET_BSS_INFO {conn_state=CONNECTED, ssid, bssid, auth_mode, enc_status, own_mac, sta_rec_idx}
6. [WPA2] wpa_supplicant hace el 4-way handshake por EAPOL (data-path, Fase 3 mínimo) -> .add_key:
   CMD_ID_ADD_REMOVE_KEY {add, pairwise CCMP, key_id, key_material}  (+ group key)
7. el FW asocia -> EVENT_ID_CONNECTION_STATUS (media_status=2) -> cfg80211_connect_result(SUCCESS)
```
**Riesgo (del diseño):** algunos FW MTK de esta época exigen orden exacto (STA_REC antes de ADD_KEY,
CH_PRIVILEGE antes de TX). Se afina leyendo `ais_fsm.c` + probando. **Por eso OPEN primero** (sin paso 6).

## cfg80211_ops a añadir
`.connect` `.disconnect` (`wlanoidSetDisassociate`→deauth en FW) `.add_key` `.del_key`
`.set_default_key` (`CMD_ID_DEFAULT_KEY_ID`). El RX-kthread debe rutear EVENT_ID_CONNECTION_STATUS.

## Plan de prueba
1. **OPEN primero** (sin claves, sin EAPOL): conectar a un AP abierto (p.ej. `Open-UPCT`) →
   `wpa_supplicant`/`iw dev wlan0 connect Open-UPCT` → `EVENT_CONNECTION_STATUS media=2` →
   `cfg80211_connect_result` → `iw dev wlan0 link` = Connected. (DHCP/ping = Fase 3.)
2. **WPA2** después (+ `.add_key` + el data-path mínimo de EAPOL).
3. Aún falta extraer los enums exactos: `ENUM_PARAM_AUTH_MODE`, `ENUM_PARAM_ENCRYPTION_STATUS`,
   `CIPHER_SUITE`, y el `CMD_UPDATE_STA_RECORD_T` completo (`nic_cmd_event.h` del mt6628).
