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
#define MCR_WCIR		0x0000	/* chip id + ready */
#define MCR_WHLPCR		0x0004	/* low-power / own */
#define MCR_WHCR		0x000c	/* HIF control */
#define MCR_WHISR		0x0010	/* int status (read) */
#define MCR_WHIER		0x0014	/* int enable */
#define MCR_WASR		0x0018	/* abnormal status */
#define MCR_WSICR		0x001c	/* SW int a device */
#define MCR_WTSR0		0x0020	/* TX status 0 (buffers libres) */
#define MCR_WTSR1		0x0024
#define MCR_WTDR0		0x0028	/* *** puerto de datos TX 0 *** */
#define MCR_WTDR1		0x002c
#define MCR_WRDR0		0x0030	/* *** puerto de datos RX 0 *** */
#define MCR_WRDR1		0x0034
#define MCR_H2DSM0R		0x0038	/* host->device mailbox */
#define MCR_D2HRM0R		0x0040	/* device->host mailbox */
#define MCR_D2HRM1R		0x0044
#define MCR_D2HRM2R		0x0048
#define MCR_WRPLR		0x0050	/* RX packet length */

/* WCIR 0x00 */
#define WCIR_WLAN_READY		BIT(21)
#define WCIR_POR_INDICATOR	BIT(20)
#define WCIR_REVISION_ID	GENMASK(19, 16)
#define WCIR_CHIP_ID		GENMASK(15, 0)
#define WIFI_CHIP_ID_6582	0x6582
#define WIFI_CHIP_ID_6572	0x6572

/* WHLPCR 0x04 — propiedad FW-own / driver-own */
#define WHLPCR_FW_OWN_REQ_SET	BIT(8)	/* escribir: pedir FW-own (dormir) */
#define WHLPCR_IS_DRIVER_OWN	BIT(8)	/* leer: 1 = somos driver-own */
#define WHLPCR_FW_OWN_REQ_CLR	BIT(9)	/* escribir: pedir driver-own (despertar) */
#define WHLPCR_INT_EN_SET	BIT(0)
#define WHLPCR_INT_EN_CLR	BIT(1)

/* WHCR 0x0c */
#define WHCR_RX_ENHANCE_MODE_EN	BIT(16)
#define WHCR_MAX_HIF_RX_LEN_NUM	GENMASK(7, 4)
#define WHCR_W_MAILBOX_RD_CLR	BIT(2)
#define WHCR_W_INT_CLR_CTRL	BIT(1)

/* WHISR 0x10 / WHIER 0x14 */
#define WHISR_D2H_SW_INT	GENMASK(31, 8)
#define WHISR_FW_OWN_BACK_INT	BIT(4)
#define WHISR_ABNORMAL_INT	BIT(3)
#define WHISR_RX1_DONE_INT	BIT(2)
#define WHISR_RX0_DONE_INT	BIT(1)
#define WHISR_TX_DONE_INT	BIT(0)
#define WHIER_DEFAULT		(WHISR_RX0_DONE_INT | WHISR_RX1_DONE_INT | \
				 WHISR_TX_DONE_INT  | WHISR_ABNORMAL_INT  | \
				 WHISR_D2H_SW_INT)

/* WRPLR 0x50 */
#define WRPLR_RX0_LEN(v)	((v) & 0xffff)
#define WRPLR_RX1_LEN(v)	(((v) >> 16) & 0xffff)

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
#define INIT_CMD_ID_DOWNLOAD_BUF	1
#define INIT_CMD_ID_WIFI_START		2
#define INIT_CMD_ID_ACCESS_REG		3
#define INIT_EVENT_ID_CMD_RESULT	1
#define INIT_EVENT_ID_ACCESS_REG	2

#define DOWNLOAD_BUF_ENCRYPTION_MODE	BIT(0)
#define DOWNLOAD_BUF_NO_CRC_CHECKING	BIT(30)
#define DOWNLOAD_BUF_ACK_OPTION		BIT(31)

#define CMD_PKT_SIZE_FOR_IMAGE		2048	/* máx payload por chunk (hif_tx.h:102) */

/* firma de la cabecera del firmware seccionado: BUILD_SIGN('M','T','K','W') */
#define MTK_WIFI_SIGNATURE		0x57544b4d	/* 'M' 'T' 'K' 'W' en LE */

/* cabecera HIF de un comando init (4B) + cabecera de comando init (4B) — packed */
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
#define HIF_TX_PKT_TYPE_DATA	0
#define HIF_TX_PKT_TYPE_CMD	1
#define HIF_TX_PKT_TYPE_MGMT	3
#define HIF_TX_PKT_TYPE_SHIFT	6	/* en el byte 3 (ucResource_PktType_CSflags), bits6-7 */

#define HIF_RX_PKT_TYPE_DATA	0
#define HIF_RX_PKT_TYPE_EVENT	1
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

/* Opcodes runtime que nos importan (CONFIRMAR valores numéricos en nic_cmd_event.h,
 * el enum tiene miembros condicionales que pueden desplazarlos). */
/* CMD_ID_GET_NIC_CAPABILITY  -> EVENT_ID_NIC_CAPABILITY (lee MAC permanente) */
/* CMD_ID_BASIC_CONFIG        -> fija MAC */
/* CMD_ID_SCAN_REQ_V2         -> beacons como MGMT + EVENT_ID_SCAN_DONE */
/* CMD_ID_SET_BSS_INFO        -> conectar */
/* CMD_ID_ADD_REMOVE_KEY      -> claves (CMD_802_11_KEY) */

#endif /* _MT6582_WIFI_REG_H */
