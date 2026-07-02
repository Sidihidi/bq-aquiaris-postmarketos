/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mt6582-wifi-reg.h — registros del HIF WiFi (AHB slave) del CONSYS MT6582.
 *
 * El WiFi NO va por BTIF/STP: tiene su propio bloque de registros estilo-SDIO ("MCR")
 * mapeado en el bus AHB en 0x180F0000, y un canal PDMA propio en 0x11000180.
 * Valores extraídos del downstream:
 *   - include/nic/mtreg.h                         (registros MCR del HIF)
 *   - os/linux/hif/ahb/include/hif.h              (HIF_DRV_BASE, CONN_MCU_DRV_BASE)
 *   - os/linux/hif/ahb/include/hif_pdma.h         (AP_DMA_HIF_BASE y su mapa)
 *   - include/nic_init_cmd_event.h / wlan_lib.h   (structs de descarga de firmware)
 */
#ifndef _MT6582_WIFI_REG_H
#define _MT6582_WIFI_REG_H

#include <linux/types.h>

/* ===== bloque HIF del WiFi (AHB slave) ===== */
#define WIFI_HIF_PHYS		0x180F0000	/* HIF_DRV_BASE */
#define WIFI_HIF_LEN		0x005c		/* HIF_DRV_LENGTH */
#define WIFI_MCU_PHYS		0x18070000	/* CONN_MCU_DRV_BASE (chip-id, CPUPCR) */
#define WIFI_MCU_LEN		0x0200

/* Registros MCR (offsets de 32 bits desde WIFI_HIF_PHYS) — mtreg.h:84-146 */

/* WCIR 0x00 */
#define WIFI_CHIP_ID_6582	0x6582
#define WIFI_CHIP_ID_6572	0x6572

/* WHLPCR 0x04 — propiedad FW-own / driver-own */

/* WHCR 0x0c */

/* WHISR 0x10 / WHIER 0x14 */

/* HSTCR (0x58) = HSIF transaction count. Hay que escribirlo ANTES de CADA acceso al puerto de
 * datos (HifAhbDmaEnhanceModeConf, ahb.c:2130): le dice al chip burst + puerto + nº de bytes.
 * SIN él, el FIFO/WRPLR NO avanza (WRPLR se queda clavado). */
#define HSTCR_BURST_OFFSET	24	/* HSTCR_AFF_BURST_LEN = BITS(24,25) */
#define HSTCR_TARGET_OFFSET	20	/* HSTCR_TRANS_TARGET   = BITS(20,22) */
#define HSTCR_TRANS_CNT		GENMASK(19, 2)	/* HSTCR_HSIF_TRANS_CNT */
#define HIF_BURST_4DW		1
#define HIF_TARGET_TXD0		0
#define HIF_TARGET_RXD0		2
#define HIF_TARGET_TXD1		1	/* comandos runtime (TC4 -> WTDR1) */
#define HIF_TARGET_RXD1		3	/* eventos runtime (WRDR1) */
#define HIF_TARGET_WHISR	4	/* TRANS_TARGET_WHISR: leer el bloque enhance (handshake RX) */
/* RX0|RX1|TX|ABNORMAL|D2H_SW (=0xffffff0f, downstream mtreg.h:231); el FW lo necesita p/señalar el boot */

/* WRPLR 0x50 */
#define WRPLR_RX0_LEN(v)	((v) & 0xffff)
#define WRPLR_RX1_LEN(v)	(((v) >> 16) & 0xffff)

/* Bloque "enhance" de interrupción (CFG_SDIO_INTR_ENHANCE=1, ON por defecto en el downstream):
 * el FW lo entrega por el puerto WHISR (HSTCR target #4) y trae el nº de tramas RX + la longitud de
 * cada una. LEER este bloque es el read-clear que re-arma RX0_DONE en el FW = el HANDSHAKE que antes
 * NO hacíamos (sondeábamos WRPLR crudo + W1C). Layout EXACTO = ENHANCE_MODE_DATA_STRUCT_T
 * (downstream mtreg.h:149-174), verificado: 88 bytes. whisr=dword[0], num_valid=dword[3],
 * rx0_len=dword[4..11], rx1_len=dword[12..19]. */
struct enhance_mode_data {
	__le32	whisr;			/* dword[0]  RX0_DONE=BIT1, RX1_DONE=BIT2, TX_DONE=BIT0... */
	u8	tq_cnt[6];		/* dword[1..2] créditos TX por TC (rTxInfo.u.ucTQ0..5Cnt) */
	__le16	tx_rsrv;
	__le16	num_valid_rx0;		/* dword[3].lo  nº de tramas válidas en RX0 (WRDR0) */
	__le16	num_valid_rx1;		/* dword[3].hi  nº de tramas válidas en RX1 (WRDR1) */
	__le16	rx0_len[16];		/* dword[4..11]  longitud de cada trama RX0 */
	__le16	rx1_len[16];		/* dword[12..19] longitud de cada trama RX1 */
	__le32	rcv_mailbox0;		/* dword[20]  D2HRM0 */
	__le32	rcv_mailbox1;		/* dword[21]  D2HRM1 */
} __packed;	/* 4+8+68+8 = 88 = sizeof(ENHANCE_MODE_DATA_STRUCT_T) */

/* ===== canal PDMA dedicado al HIF del WiFi ===== hif_pdma.h:62-113
 * (es OTRO canal del mismo AP-DMA; el BTIF usa 0x11000780/0x800, NO colisiona) */
#define WIFI_PDMA_PHYS		0x11000180	/* AP_DMA_HIF_BASE (no-OF) */
#define WIFI_PDMA_LEN		0x0054
#define PDMA_INT_FLAG		0x0000		/* bit0 FLAG_0 */
#define PDMA_INT_EN		0x0004
#define PDMA_EN			0x0008		/* bit0 EN */
#define PDMA_RST		0x000c		/* bit0 warm, bit1 hard */
#define PDMA_STOP		0x0010
#define PDMA_FLUSH		0x0014
#define PDMA_CON		0x0018		/* bits16-17 burst, bit1 fix, bit0 dir(0=TX,1=RX) */
#define PDMA_SRC_ADDR		0x001c
#define PDMA_DST_ADDR		0x0020
#define PDMA_LEN		0x0024		/* bits0-19 */
#define PDMA_CR_FLAG_0		BIT(0)
#define PDMA_CR_EN		BIT(0)
#define PDMA_CON_BURST_4_4	(3u << 16)	/* HIF_PDMA_BURST_4_4 */
#define PDMA_CON_DIR_RX		BIT(0)

/* ===== descarga de firmware (comandos INIT por el puerto HIF) =====
 * nic_init_cmd_event.h:71-152, wlan_lib.h:568-583 */

/* cabecera de un comando init (4B) + cabecera de comando init (4B) — packed */
struct init_hif_tx_header {
	__le16	tx_byte_count;	/* tamaño total del paquete */
	u8	ether_type_offset;	/* 0 */
	u8	cs_flags;		/* 0 */
	/* INIT_WIFI_CMD_T: */
	u8	cid;			/* INIT_CMD_ID_* */
	u8	seq_num;
	__le16	reserved;
	/* sigue el payload (p.ej. struct init_cmd_download_buf) */
} __packed;

/* payload de INIT_CMD_ID_DOWNLOAD_BUF */
struct init_cmd_download_buf {
	__le32	address;	/* destino en RAM del chip = section.dest_addr (+offset chunk) */
	__le32	length;		/* tamaño del chunk */
	__le32	crc32;		/* CRC32 del chunk */
	__le32	data_mode;	/* DOWNLOAD_BUF_* */
	/* siguen los bytes del chunk */
} __packed;

/* payload de INIT_CMD_ID_WIFI_START */
struct init_cmd_wifi_start {
	__le32	override;	/* 1 = usar 'address' como punto de arranque; normalmente 0 */
	__le32	address;
} __packed;

/* cabecera de un evento init (RX, leído de WRDR0) */
struct init_wifi_event {
	__le16	rx_byte_count;
	u8	eid;			/* INIT_EVENT_ID_* */
	u8	seq_num;
	/* sigue el payload (p.ej. status u8 para CMD_RESULT) */
} __packed;

/* cabecera del fichero de firmware "dividido en secciones" — wlan_lib.h:568-583 */
struct fwdl_section_info {
	__le32	offset;		/* offset del dato dentro del fichero */
	__le32	reserved;
	__le32	length;		/* bytes de la sección */
	__le32	dest_addr;	/* dirección destino en la RAM del chip */
} __packed;

struct firmware_divided_download {
	__le32	signature;	/* == MTK_WIFI_SIGNATURE si es formato seccionado */
	__le32	crc;		/* CRC32 del fichero desde offset 8 */
	__le32	num_entries;
	__le32	reserved;
	struct fwdl_section_info section[];	/* num_entries entradas */
} __packed;

/* ===== runtime cmd/event (Fase 1+) — sólo cabeceras, los structs por comando van aparte =====
 * hif_tx.h:158-171, hif_rx.h:131-141, nic_cmd_event.h */
#define HIF_TX_PKT_TYPE_MGMT	3
#define HIF_TX_PKT_TYPE_SHIFT	6	/* en el byte 3 (ucResource_PktType_CSflags), bits6-7 */

#define HIF_RX_PKT_TYPE_MGMT	3
#define HIF_RX_PKT_TYPE_MASK	0x3	/* bits0-1 de u2PacketType */

/* cabecera de comando runtime (8B; los 4 primeros coinciden con la cabecera HIF) */
struct wifi_cmd {
	__le16	tx_byte_count_up;
	u8	ether_type_offset;
	u8	resource_pkttype_cs;	/* PKT_TYPE en bits6-7 */
	u8	cid;			/* ENUM_CMD_ID (confirmar valor en nic_cmd_event.h) */
	u8	set_query;		/* 1=SET, 0=QUERY */
	u8	seq_num;
	u8	reserved2;
	/* sigue el payload del comando */
} __packed;

/* HIF_TX_HEADER completo (16B) para frames de GESTIÓN 802.11 (AUTH/ASSOC); el cmd usa solo 8B.
 * hif_tx.h:158-171. Van por TC4 -> PUERTO 1 igual que el cmd, pero con PKT_TYPE=MGMT(3). */
#define HIF_TX_FLAG_802_11	0x80	/* ucPktFormtId_Flags bit7: frame 802.11 crudo */
#define HIF_TX_NEED_ACK		0x01	/* ucAck_BIP_BasicRate bit0: pedir ACK al peer */
#define HIF_TX_BASIC_RATE	0x04	/* ucAck_BIP_BasicRate bit2: TX a basic-rate (AUTH/ASSOC lo EXIGEN, si no MPDU_ERROR) */
/* flags del HIF_TX_HEADER para DATOS 802.3 (de hif_tx.h downstream, nic_tx.c:1638-1648) */
#define HIF_TX_FLAG_1X_FRAME	0x40	/* ucPktFormtId_Flags bit6: frame de datos (1X) — el FW lo EXIGE para datos */
#define HIF_TX_BURST_END	0x20	/* ucForwardingType_SessionID bit5: fin de burst — sin esto el FW espera más paquetes */
struct hif_tx_header {
	__le16	tx_byte_count_up;	/* bits0-11 = byte count (hdr+frame), UP en 12-15 */
	u8	ether_type_offset;
	u8	resource_pkttype_cs;	/* (TC<<2) | (PKT_TYPE<<6) */
	u8	wlan_header_len;	/* bits0-5: longitud cabecera 802.11 (24 mgmt) */
	u8	pktfmt_flags;		/* bit7 = 802.11 crudo, bits4-5 = net_type, bit6 = 1x */
	__le16	llh;			/* BOW (0) */
	__le16	seq_no;			/* BOW (0) */
	u8	sta_rec_idx;		/* índice del STA-record del AP */
	u8	fwd_sess;
	u8	pkt_seq;
	u8	ack_bip_rate;		/* bit0 = need_ack */
	u8	reserved[2];
} __packed;	/* 16B */

/* cabecera HIF de RX (12B) + cabecera de evento (cuando pkt_type==EVENT) */
struct hif_rx_header {
	__le16	packet_len;
	__le16	packet_type;		/* bits0-1 = PKT_TYPE */
	u8	header_len_offset;
	u8	flags_80211_reorder;
	__le16	seqno_tid;
	u8	sta_rec_idx;
	u8	rcpi;			/* RSSI */
	u8	hw_channel_num;		/* <=14 => 2.4G */
	u8	reserved;
} __packed;

struct wifi_event {
	__le16	packet_len;
	__le16	packet_type;
	u8	eid;			/* ENUM_EVENT_ID */
	u8	seq_num;
	u8	reserved2[2];
	/* sigue el payload del evento */
} __packed;

/* Opcodes runtime (verificados en nic_cmd_event.h + structs compilados con -DMT6628).
 * OJO: los comandos runtime van por TC4 -> PUERTO 1 (WTDR1/WRDR1), no el puerto 0 de la descarga.
 * La MAC permanente NO está en NIC_CAPABILITY: está en BASIC_CONFIG (body+0). El body del evento
 * empieza en offset 8 (struct wifi_event), no 12. */
#define WIFI_TC4			4	/* TC de comandos: byte3 = (TC4<<2)|(CMD<<6) = 0x50 */
#define WIFI_TC_DATA			1	/* TC1 = datos best-effort/no-QoS (puerto 0): byte3 = (TC1<<2)|(DATA<<6) = 0x04 */
#define WIFI_TC_BMCAST			5	/* TC5 = broadcast/multicast: el FW EXIGE TC5 + staRecIdx=0xFF (que_mgt.c) */
#define HIF_TX_RESOURCE_SHIFT		2	/* TC en bits2-5 del byte3 */
/* SCAN (Fase 1 cont., verificado downstream). OJO: el SET_DOMAIN_INFO/SCAN son SET (ucSetQuery=1).
 * Beacons llegan por PUERTO 0 (MGMT); SCAN_DONE por puerto 1 (EVENT). */
#define SCAN_TYPE_PASSIVE		0
#define NETWORK_TYPE_AIS		0
#define HIF_RX_HDR_OFFSET_MASK		0x3	/* bits0-1 de hif_rx_header.header_len_offset */

struct param_ssid { __le32 len; u8 ssid[32]; } __packed;	/* 36B */
struct chan_info  { u8 band, chan; } __packed;			/* 2B */
struct cmd_scan_req_v2 {
	u8	seq_num, network_type, scan_type, ssid_type;	/* +0 */
	struct param_ssid ssid[4];				/* +4  (144) */
	__le16	probe_delay, dwell_time;			/* +148 */
	u8	channel_type, channel_list_num;			/* +152 (list_num=0: el FW expande 2.4G) */
	struct chan_info chan_list[32];				/* +154 (64) */
	__le16	ie_len;						/* +218 */
	u8	ie[600];					/* +220 (sizeof total = 820) */
} __packed;
struct cmd_subband { u8 reg_class, band, chan_span, first_chan, num_chans, rsv[3]; } __packed; /* 8B */
struct cmd_set_domain_info {
	__le16	country_code, rsv;
	struct cmd_subband subband[6];
	u8	bw_2g4, bw_5g, rsv2[2];				/* sizeof = 56 */
} __packed;

/* ---- Fase 2: CONNECT (structs/CMD_IDs de combo/mt6628, ver FASE2-CONNECT.md) ---- */
#define ENC_STATUS_DISABLED		0
#define ENC_STATUS_CCMP_ENABLED		6	/* ENUM_ENCRYPTION3_ENABLED: CCMP con clave */
#define ENC_STATUS_CCMP_KEY_ABSENT	7	/* ENUM_ENCRYPTION3_KEY_ABSENT: CCMP, clave aún no instalada (SET_BSS_INFO pre-4way-handshake) */
#define CIPHER_NONE			0
#define CIPHER_TKIP			2	/* CIPHER_SUITE_TKIP (privacy.h:121); el 3 es TKIP_WO_MIC, NO usar */
#define CIPHER_CCMP			4
/* valores para el STA-record del AP (de wlan_def.h): LEGACY_AP=BIT(0)|BIT(6); STATE_3=2;
 * BG=HR_DSSS(0)|ERP(1); RATE_SET_ERP=1..54M; BASIC_RATE_SET_ERP=1/2/5.5/11M */
#define MEDIA_STATE_DISCONNECTED	1

struct cmd_bss_activate { u8 net_type_idx, active, rsv[2]; } __packed;	/* 4 */

/* CMD_ID_DEFAULT_KEY_ID (0x09) — cual clave (idx) es la TX-default. {net_type, key_id, rsv[2]}. */
struct cmd_default_key_id { u8 net_type_idx, key_id, rsv[2]; } __packed;	/* 4 */

struct cmd_set_bss_rlm_param {		/* fija el canal del BSS */
	u8	net_type_idx, rf_band, primary_channel, rf_sco;
	u8	erp_prot, ht_prot, gf_mode, tx_rifs;
	__le16	ht_op3, ht_op2;
	u8	ht_op1, short_preamble, short_slot, check_id;	/* check_id = 0x72 */
} __packed;	/* 16 */

struct cmd_set_bss_info {		/* "conectar a este BSS" */
	u8	net_type_idx, conn_state, op_mode, ssid_len;
	u8	ssid[32];
	u8	bssid[6];
	u8	is_qbss, rsv1;
	__le16	op_rate_set, basic_rate_set;
	u8	sta_rec_idx_of_ap, rsv2, rsv3, nonht_basic_phy;
	u8	auth_mode, enc_status, phy_type_set;
	u8	own_mac[6];
	u8	wapi_mode, is_ap_mode, rsv[1];
	struct cmd_set_bss_rlm_param rlm;	/* rBssRlmParam EMBEBIDO (canal) — el real lo lleva dentro */
} __packed;	/* 80 */

struct cmd_pm_bss_connected {		/* CMD_ID_INDICATE_PM_BSS_CONNECTED (0x1a): arma el monitor de conexion del FW */
	u8	net_type_idx, dtim_period;
	__le16	assoc_id, beacon_interval, atim_window;
	u8	is_uapsd, bmp_delivery_ac, bmp_trigger_ac, rsv;
} __packed;	/* 12 (= CMD_INDICATE_PM_BSS_CONNECTED_T del downstream, byte a byte) */

struct cmd_ch_privilege {		/* CMD_ID_CH_PRIVILEGE (0x20) — pide el canal antes del TX auth/assoc */
	u8	net_type_idx, token_id, action, primary_channel;
	u8	rf_sco, rf_band, req_type, rsv;
	__le32	max_interval;		/* ms */
	u8	bssid[6];
	u8	rsv2[2];
} __packed;	/* 20 */

struct cmd_update_sta_record {		/* CMD_ID_UPDATE_STA_RECORD (0x17) — el registro del AP peer */
	u8	index;			/* sta_rec_idx -> SET_BSS_INFO.sta_rec_idx_of_ap */
	u8	sta_type;		/* STA_TYPE_LEGACY_AP */
	u8	mac_addr[6];		/* BSSID del AP */
	__le16	aid, listen_interval;
	u8	net_type_index;		/* NETWORK_TYPE_AIS */
	u8	desired_phy_type_set;	/* PHY_TYPE_SET_802_11BG */
	__le16	desired_nonht_rate_set;	/* RATE_SET_ERP */
	__le16	bss_basic_rate_set;	/* BASIC_RATE_SET_ERP */
	u8	is_qos, is_uapsd_sup, sta_state, mcs_set;
	u8	sup_mcs32, ampdu_param;
	__le16	ht_cap_info, ht_ext_cap;
	__le32	tx_beamforming_cap;
	u8	asel_cap, rcpi, need_resp, uapsd_ac, uapsd_sp;
	u8	rsv[3];
} __packed;	/* 40 */

struct cmd_ps_profile {			/* CMD_ID_POWER_SAVE_MODE (0x06) — CMD_PS_PROFILE_T downstream */
	u8	net_type_index;		/* NETWORK_TYPE_AIS = 0 */
	u8	ps_profile;		/* Param_PowerModeCAM = 0 (siempre despierto) */
	u8	rsv[2];
} __packed;	/* 4 */

struct cmd_802_11_key {			/* instalar PTK/GTK (WPA2) */
	u8	add_remove, tx_key, key_type, is_auth;
	u8	peer_addr[6];
	u8	net_type, algorithm_id, key_id, key_len, rsv[2];
	u8	key_material[32];
	u8	key_rsc[16];
} __packed;	/* 64 */

struct event_connection_status {	/* EVENT_ID_CONNECTION_STATUS (async, puerto 1) */
	u8	media_status, reason, infra_mode, ssid_len;
	u8	ssid[32];
	u8	bssid[6];
	u8	auth_mode, enc_status;
	__le16	beacon_period, aid, atim;
	u8	net_type, rsv[1];
	__le32	freq_khz;
} __packed;

#endif /* _MT6582_WIFI_REG_H */
