// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-wifi.c — driver WiFi (cfg80211 full-MAC) del combo CONSYS del MT6582.
 *
 * ============================ LA DECISIÓN ARQUITECTÓNICA ============================
 * El WiFi del MT6582 NO usa el enlace BTIF/STP (el que mt6582-btif.c usa para BT y GPS).
 * Tiene su PROPIO bloque HIF estilo-SDIO mapeado en el bus AHB en 0x180F0000 (registros
 * "MCR": WCIR/WHLPCR/WHISR/WTDR0/WRDR0/...) y un canal PDMA propio en 0x11000180.
 * STP/WMT sólo se usa para (a) encender la radio (func_on(WIFI)) y (b) coordinar reset.
 * El firmware WIFI_RAM_CODE se descarga por el PUERTO DE DATOS del HIF (WTDR0) como
 * comandos INIT_CMD_ID_DOWNLOAD_BUF — NO por STP. Ver WIFI-DESIGN.md §0.
 * ===================================================================================
 *
 * Estado de este fichero: SCAFFOLD de la Fase 0 (ver WIFI-ROADMAP.md):
 *   probe -> EPROBE_DEFER hasta mt6582_consys_ready -> mt6582_consys_func_on(WIFI)
 *   -> ioremap(HIF + PDMA) -> driver-own -> leer WCIR(==0x6582) -> descargar firmware
 *   -> INIT_CMD_ID_WIFI_START -> poll WLAN_READY.
 * Disparado por debugfs (igual que el bring-up del btif). Las Fases 1-3 (cmd/event,
 * scan, connect, netdev) están como TODOs marcados.
 *
 * Estilo calcado de mt6582-btif.c: kernel C, tabs, helpers rd()/wr(), dev_info/dev_err.
 */
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/firmware.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/crc32.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <net/cfg80211.h>

#include "mt6582-wifi-reg.h"

/* ===== dependencias del resto del CONSYS (ya implementadas) ===== */
/* exportado por mt6582-consys.c: el MCU del CONSYS corre y el chip-id leyó 0x6582 */
extern bool mt6582_consys_ready;

/* Provista (EXPORT_SYMBOL_GPL) por mt6582-btif.c — manda el comando WMT func_on y espera el EVT,
 * thread-safe con el RX-thread del BT. type: 0=BT 1=FM 2=GPS 3=WIFI 4=WMT. */
extern int mt6582_consys_func_on(u8 type);

/* Provista por mt6582-consys.c: enciende/apaga VCN33_WIFI (LDO RF). El OEM lo hace en func_on(WIFI),
 * no al boot del consys; sin el flanco off->on el MAC no afirma WLAN_READY. */
extern int mt6582_consys_wifi_vcn33(bool on);

#define WMTDRV_TYPE_WIFI	3	/* == canal STP WIFI; type-id del func_on para la radio WiFi */

/* nombre del firmware del MAC WiFi en /lib/firmware (contenido = el blob de ~160KB).
 * Downstream: CFG_FW_FILENAME="WIFI_RAM_CODE" con sufijo de chip ("_SOC" para 6582). */
#define WIFI_FW_NAME		"mediatek/mt6582/WIFI_RAM_CODE"

#define DRIVER_OWN_POLL		1000	/* iteraciones de poll para driver-own / ready */
#define DLM_BUF_SZ		(sizeof(struct init_hif_tx_header) + \
				 sizeof(struct init_cmd_download_buf) + CMD_PKT_SIZE_FOR_IMAGE)

struct mt6582_wifi {
	struct device		*dev;
	void __iomem		*hif;		/* registros MCR @ 0x180F0000 */
	void __iomem		*mcu;		/* @ 0x18070000 (chip-id/CPUPCR) */
	void __iomem		*pdma;		/* canal PDMA @ 0x11000180 */
	struct mutex		hif_lock;	/* serializa acceso al puerto HIF */
	struct dentry		*dbg;
	bool			started;	/* firmware corriendo + WLAN_READY */
	u8			cmd_seq;	/* secuencia de comandos init/runtime */
	int			dbg_evt;	/* limita el log de depuración de eventos init */
	u8			*dlm;		/* buffer de descarga (DMA-able) */
	dma_addr_t		dlm_phys;
	/* Fase 1: cfg80211 full-MAC */
	struct wiphy		*wiphy;
	struct net_device	*ndev;
	struct wireless_dev	wdev;
	struct cfg80211_scan_request *scan_req;	/* scan en curso (NULL = idle) */
	unsigned long		scan_deadline;	/* jiffies: forzar scan_done si el FW no manda SCAN_DONE */
	struct task_struct	*rx_thread;	/* sondea el RX (beacons MGMT + eventos) */
	struct sk_buff_head	tx_queue;	/* Fase 3: cola TX de datos (ndo_xmit encola; el rx_thread escribe bajo hif_lock) */
	u8			scan_seq;
	u8			mgmt_seq;	/* tx-seq de frames mgmt: !=0 => el FW manda EVENT_ID_TX_DONE */
	bool			cfg_registered;
	/* Fase 2: connect */
	u8			mac[ETH_ALEN];		/* MAC permanente (de phase1) */
	bool			connecting;
	bool			connected;	/* Fase 3: asociado L2 -> permite el TX de datos (connecting solo dura el handshake) */
	u8			connect_bssid[ETH_ALEN];
	u8			connect_ssid[32];	/* para el IE SSID del ASSOC-REQ */
	u8			connect_ssid_len;
	struct cfg80211_bss	*connect_bss;	/* BSS retenido del .connect -> cfg80211_connect_bss (evita WARN sme.c:845) */
	u8			connect_channel;	/* canal guardado para el SET_BSS_INFO del JoinComplete */
	u16			assoc_id;		/* AID del ASSOC-RESP -> CMD_INDICATE_PM_BSS_CONNECTED */
	struct delayed_work	auto_bringup;	/* auto-levanta wlan0 al boot, con reintento (WLAN_READY flaky) */
	u8			bringup_tries;
};

static struct mt6582_wifi *g_wifi;

static inline u32 rd(void __iomem *b, u32 o) { return readl(b + o); }
static inline void wr(void __iomem *b, u32 o, u32 v) { writel(v, b + o); }

/* ======================================================================
 *  Capa HIF: propiedad driver-own y (Fase 0) escritura/lectura PIO del puerto.
 *  En Fase 3 esto pasa a PDMA (0x11000180) + IRQ (WHISR). Ver ahb_pdma.c.
 * ====================================================================== */

/* pedir driver-own: escribir FW_OWN_REQ_CLR y poll de IS_DRIVER_OWN (mtreg.h WHLPCR) */
static int wifi_set_driver_own(struct mt6582_wifi *w)
{
	int t;

	wr(w->hif, MCR_WHLPCR, WHLPCR_FW_OWN_REQ_CLR);
	for (t = 0; t < DRIVER_OWN_POLL; t++) {
		if (rd(w->hif, MCR_WHLPCR) & WHLPCR_IS_DRIVER_OWN)
			return 0;
		udelay(50);
	}
	dev_err(w->dev, "driver-own TIMEOUT (WHLPCR=0x%08x)\n", rd(w->hif, MCR_WHLPCR));
	return -ETIMEDOUT;
}

static void wifi_set_fw_own(struct mt6582_wifi *w)
{
	wr(w->hif, MCR_WHLPCR, WHLPCR_FW_OWN_REQ_SET);
}

/* Configura HSTCR antes de un acceso al puerto: burst 4DW + puerto destino + nº de bytes
 * (HifAhbDmaEnhanceModeConf, ahb.c:2155). Es lo que hace avanzar el FIFO del chip. */
static void wifi_hstcr(struct mt6582_wifi *w, u32 target, u32 size)
{
	u32 cnt = (size & 0x3) ? (size + 4) : size;

	/* Erratum "HIF 92B, 4B problem between 2 block transmissions" (downstream ahb.c:2153):
	 * leer un registro no-func0 (WHIER) ANTES de programar HSTCR, o las transferencias cuya
	 * longitud cae en el caso malo se corrompen. */
	(void)rd(w->hif, MCR_WHIER);

	wr(w->hif, MCR_HSTCR,
	   (HIF_BURST_4DW << HSTCR_BURST_OFFSET) |
	   (target << HSTCR_TARGET_OFFSET) |
	   (cnt & HSTCR_TRANS_CNT));
}

/* escribir 'len' bytes al puerto de datos TX0 en modo PIO (palabras de 32 bits).
 * TODO(Fase 3): sustituir por PDMA (HifPdmaConfig/Start con Dst=WIFI_HIF_PHYS+MCR_WTDR0). */
static void wifi_port_write_pio(struct mt6582_wifi *w, const void *buf, u32 len)
{
	const u32 *p = buf;
	u32 i, words = (len + 3) / 4;

	wifi_hstcr(w, HIF_TARGET_TXD0, len);
	for (i = 0; i < words; i++)
		wr(w->hif, MCR_WTDR0, p[i]);
}

/* leer 'len' bytes del puerto de datos RX0 en modo PIO.
 * TODO(Fase 1): el flujo real es: poll WHISR.RX0_DONE -> leer WRPLR (longitud) -> leer aquí. */
static void wifi_port_read_pio(struct mt6582_wifi *w, void *buf, u32 len)
{
	u32 *p = buf;
	u32 i, words = (len + 3) / 4;

	wifi_hstcr(w, HIF_TARGET_RXD0, len);
	for (i = 0; i < words; i++)
		p[i] = rd(w->hif, MCR_WRDR0);
}

/* esperar un evento init en RX0 y devolver su 'status' (CMD_RESULT). Versión POLLING simple. */
static int wifi_wait_init_event(struct mt6582_wifi *w, u8 expect_eid, u32 ms)
{
	struct init_wifi_event *ev;
	u8 rxbuf[128];
	u32 wrplr, plen, readlen;
	int loops = ms * 20;	/* el downstream sondea cada 50us (nicRxWaitResponse, nic_rx.c:3359) */

	/* La fase de descarga sondea WRPLR (longitud RX0). El HW añade además un DW (4B) de status
	 * TRAS el paquete: hay que leer ALIGN_4(plen)+4 para drenarlo, o el siguiente evento sale
	 * desplazado 4 bytes. (poll WRPLR de nicRxWaitResponse + DW de nic_rx.c:2526.) */
	while (loops-- > 0) {
		wrplr = rd(w->hif, MCR_WRPLR);
		plen = wrplr & 0xffff;			/* longitud del paquete pendiente en RX0 */
		if (plen == 0) {			/* respuesta aún no lista */
			udelay(50);
			continue;
		}
		readlen = ALIGN(plen, 4);
		if (readlen > sizeof(rxbuf))
			return -EMSGSIZE;
		wifi_port_read_pio(w, rxbuf, readlen);	/* HSTCR (en port_read) avanza el FIFO */
		if (w->dbg_evt < 6) { w->dbg_evt++;
			dev_info(w->dev, "evt WRPLR=0x%08x plen=%u rx=%*ph\n",
				 wrplr, plen, min_t(u32, plen, 16), rxbuf); }
		ev = (struct init_wifi_event *)rxbuf;
		if (ev->eid != expect_eid) {
			dev_warn(w->dev, "init evt EID=0x%x (esperaba 0x%x)\n",
				 ev->eid, expect_eid);
			return -EBADMSG;
		}
		/* para CMD_RESULT, el byte tras la cabecera es ucStatus (0=OK) */
		return rxbuf[sizeof(*ev)];
	}
	return -ETIMEDOUT;
}

/* ======================================================================
 *  Descarga de firmware (Fase 0): WIFI_RAM_CODE por el puerto HIF como
 *  comandos INIT_CMD_ID_DOWNLOAD_BUF. Ver wlan_lib.c:3140 (wlanImageSectionDownload).
 * ====================================================================== */

/* manda UNA sección (troceada en chunks <= CMD_PKT_SIZE_FOR_IMAGE) al destino dado. */
static int wifi_dl_section(struct mt6582_wifi *w, u32 dest, const u8 *data, u32 size)
{
	u32 off;
	int ret;

	for (off = 0; off < size; off += CMD_PKT_SIZE_FOR_IMAGE) {
		u32 chunk = min_t(u32, CMD_PKT_SIZE_FOR_IMAGE, size - off);
		struct init_hif_tx_header *th = (void *)w->dlm;
		struct init_cmd_download_buf *db = (void *)(th + 1);
		u32 total = sizeof(*th) + sizeof(*db) + chunk;

		th->tx_byte_count = cpu_to_le16(total);
		th->ether_type_offset = 0;
		th->cs_flags = 0;
		th->cid = INIT_CMD_ID_DOWNLOAD_BUF;
		th->seq_num = ++w->cmd_seq;
		th->reserved = 0;
		db->address = cpu_to_le32(dest + off);
		db->length = cpu_to_le32(chunk);
		db->crc32 = cpu_to_le32(crc32_le(~0, data + off, chunk) ^ ~0);
		/* ACK + FW CIFRADO: el firmware del MT6582 va cifrado (downstream CFG_ENABLE_FW_ENCRYPTION=1,
		 * wlan_lib.c:3198). Sin este bit el chip rechaza el chunk con status=1 (invalid param). */
		db->data_mode = cpu_to_le32(DOWNLOAD_BUF_ACK_OPTION | DOWNLOAD_BUF_ENCRYPTION_MODE);
		memcpy(db + 1, data + off, chunk);

		mutex_lock(&w->hif_lock);
		wifi_port_write_pio(w, w->dlm, total);
		ret = wifi_wait_init_event(w, INIT_EVENT_ID_CMD_RESULT, 200);
		mutex_unlock(&w->hif_lock);

		if (ret < 0) {
			dev_err(w->dev, "DL chunk @0x%08x: sin ACK (%d)\n", dest + off, ret);
			return ret;
		}
		if (ret != 0) {	/* ucStatus != success */
			dev_err(w->dev, "DL chunk @0x%08x: FW rechazó status=%d\n", dest + off, ret);
			return -EIO;
		}
	}
	return 0;
}

/* DIAG (read-FW): leer una palabra del espacio del chip vía INIT_CMD_ID_ACCESS_REG (query).
 * Para ver si el FW se descifró bien en la RAM del MCU (0x6a000) o es basura (clave mala).
 * INIT_CMD_ACCESS_REG = ucSetQuery(1)+rsv(3)+u4Address(4)+u4Data(4); evento = u4Address(4)+u4Data(4). */
static u32 wifi_access_reg_read(struct mt6582_wifi *w, u32 addr)
{
	struct init_hif_tx_header *th = (void *)w->dlm;
	u8 *ar = (u8 *)(th + 1);
	u32 total = sizeof(*th) + 12;
	struct init_wifi_event *ev;
	u8 rxbuf[64];
	u32 wrplr, plen, readlen, val = 0xdeadbeef;
	int loops = 200;

	th->tx_byte_count = cpu_to_le16(total);
	th->ether_type_offset = 0;
	th->cs_flags = 0;
	th->cid = INIT_CMD_ID_ACCESS_REG;
	th->seq_num = ++w->cmd_seq;
	th->reserved = 0;
	ar[0] = 0;			/* ucSetQuery = 0 (read) */
	ar[1] = ar[2] = ar[3] = 0;
	*(__le32 *)(ar + 4) = cpu_to_le32(addr);
	*(__le32 *)(ar + 8) = 0;

	mutex_lock(&w->hif_lock);
	wifi_port_write_pio(w, w->dlm, total);
	while (loops-- > 0) {
		wrplr = rd(w->hif, MCR_WRPLR);
		plen = wrplr & 0xffff;
		if (plen == 0) { udelay(50); continue; }
		readlen = ALIGN(plen, 4);
		if (readlen > sizeof(rxbuf))
			break;
		wifi_port_read_pio(w, rxbuf, readlen);
		ev = (struct init_wifi_event *)rxbuf;
		if (ev->eid == INIT_EVENT_ID_ACCESS_REG)
			val = le32_to_cpup((__le32 *)(rxbuf + sizeof(*ev) + 4));
		else
			val = 0xbade0000u | ev->eid;	/* respuesta inesperada */
		break;
	}
	mutex_unlock(&w->hif_lock);
	return val;
}

/* ======================================================================
 *  FASE 1 — cmd/event runtime por TC4/puerto-1 (WTDR1/WRDR1).
 *  Los comandos runtime NO van por el puerto 0 de la descarga: van por TC4 ->
 *  puerto 1 (WTDR1=0x2c tx, WRDR1=0x34 rx), longitud en WRPLR mitad-alta. El body
 *  de un evento empieza en offset 8 (struct wifi_event), no 12. (RE downstream.)
 * ====================================================================== */

/* escribir 'len' bytes al puerto de datos TX1 en PIO (clon de wifi_port_write_pio, target TXD1). */
static void wifi_port1_write_pio(struct mt6582_wifi *w, const void *buf, u32 len)
{
	const u32 *p = buf;
	u32 i, words = (len + 3) / 4;

	wifi_hstcr(w, HIF_TARGET_TXD1, len);
	for (i = 0; i < words; i++)
		wr(w->hif, MCR_WTDR1, p[i]);
}

/* leer 'len' bytes del puerto de datos RX1 en PIO (target RXD1). */
static void wifi_port1_read_pio(struct mt6582_wifi *w, void *buf, u32 len)
{
	u32 *p = buf;
	u32 i, words = (len + 3) / 4;

	wifi_hstcr(w, HIF_TARGET_RXD1, len);
	for (i = 0; i < words; i++)
		p[i] = rd(w->hif, MCR_WRDR1);
}

/* enviar un comando runtime (QUERY/SET) por TC4/puerto-1. 'resp_reserve' = tamaño del evento
 * de respuesta (el FW dimensiona TxByteCount para incluirlo). El caller debe tener hif_lock. */
static int wifi_send_cmd(struct mt6582_wifi *w, u8 cid, u8 set_query,
			 const void *body, u16 body_len, u16 resp_reserve)
{
	struct wifi_cmd *c = (void *)w->dlm;
	u16 info_len = sizeof(*c) + (body_len ? body_len : resp_reserve);

	c->tx_byte_count_up = cpu_to_le16(ALIGN(info_len, 4) & 0x0fff);	/* UP=0 */
	c->ether_type_offset = 0;
	c->resource_pkttype_cs = (WIFI_TC4 << HIF_TX_RESOURCE_SHIFT) |
				 (HIF_TX_PKT_TYPE_CMD << HIF_TX_PKT_TYPE_SHIFT);	/* = 0x50 */
	c->cid = cid;
	c->set_query = set_query;	/* 0 = QUERY */
	c->seq_num = ++w->cmd_seq;
	c->reserved2 = 0;
	if (body && body_len)
		memcpy((u8 *)w->dlm + sizeof(*c), body, body_len);
	else
		memset((u8 *)w->dlm + sizeof(*c), 0, resp_reserve);
	wifi_port1_write_pio(w, w->dlm, info_len);
	return w->cmd_seq;
}

/* sondear el puerto-1 RX por un evento con EID dado; copia el body (tras struct wifi_event) a out. */
static int wifi_poll_event(struct mt6582_wifi *w, u8 want_eid, void *out, u32 out_len, u32 ms)
{
	struct wifi_event *ev;
	u8 rx[96];
	u32 wrplr, plen, rl;
	int loops = ms * 20;	/* 50us/iter, como nicRxWaitResponse */

	while (loops-- > 0) {
		wrplr = rd(w->hif, MCR_WRPLR);
		plen = WRPLR_RX1_LEN(wrplr);		/* *** puerto 1 = mitad ALTA *** */
		if (!plen) { udelay(50); continue; }
		rl = ALIGN(plen, 4);
		if (rl > sizeof(rx))
			return -EMSGSIZE;
		wifi_port1_read_pio(w, rx, rl);
		ev = (struct wifi_event *)rx;
		if ((le16_to_cpu(ev->packet_type) & HIF_RX_PKT_TYPE_MASK) != HIF_RX_PKT_TYPE_EVENT) {
			dev_warn(w->dev, "Fase1: pkt no-EVENT WRPLR=0x%08x rx=%*ph\n",
				 wrplr, min_t(u32, plen, 16), rx);
			return -EBADMSG;
		}
		if (ev->eid != want_eid) {
			dev_warn(w->dev, "Fase1: EID=0x%x (esperaba 0x%x)\n", ev->eid, want_eid);
			return -EBADMSG;
		}
		if (plen > sizeof(*ev))
			memcpy(out, rx + sizeof(*ev), min_t(u32, out_len, plen - sizeof(*ev)));
		return 0;
	}
	return -ETIMEDOUT;
}

/* "hello world" del firmware: MAC permanente (BASIC_CONFIG) + capacidades (NIC_CAPABILITY).
 * Un EVENT_ID_NIC_CAPABILITY válido = el bucle cmd/event FUNCIONA (mata ~70% del riesgo Fase 1). */
static void wifi_phase1_hello(struct mt6582_wifi *w)
{
	u8 cap[32], bc[12];
	struct cmd_set_domain_info dom;
	int ret;

	mutex_lock(&w->hif_lock);
	/* 1) MAC permanente: CMD_ID_BASIC_CONFIG (query) -> EVENT_ID_BASIC_CONFIG (MAC @body+0) */
	wifi_send_cmd(w, CMD_ID_BASIC_CONFIG, 0, NULL, 0, sizeof(bc));
	ret = wifi_poll_event(w, EVENT_ID_BASIC_CONFIG, bc, sizeof(bc), 1000);
	if (!ret) {
		memcpy(w->mac, bc, ETH_ALEN);		/* para SET_BSS_INFO.own_mac */
		dev_info(w->dev, "*** Fase1: MAC permanente = %pM ***\n", bc);
	}
	else
		dev_warn(w->dev, "Fase1: BASIC_CONFIG sin respuesta (%d)\n", ret);
	/* 2) capacidades: CMD_ID_GET_NIC_CAPABILITY (query) -> EVENT_ID_NIC_CAPABILITY */
	wifi_send_cmd(w, CMD_ID_GET_NIC_CAPABILITY, 0, NULL, 0, sizeof(cap));
	ret = wifi_poll_event(w, EVENT_ID_NIC_CAPABILITY, cap, sizeof(cap), 1000);
	if (!ret)
		dev_info(w->dev, "*** Fase1: NIC_CAP ProductID=0x%04x FW=0x%04x 5Goff=%d efuse=%d macValid=%d ***\n",
			 le16_to_cpup((__le16 *)cap), le16_to_cpup((__le16 *)(cap + 2)),
			 cap[6], cap[8], cap[9]);
	else
		dev_warn(w->dev, "Fase1: NIC_CAPABILITY sin respuesta (%d)\n", ret);
	/* 3) dominio regulatorio 2.4G ch1-13 (mundo, reg_class 81). IMPRESCINDIBLE antes del scan:
	 *    sin SET_DOMAIN_INFO el FW escanea con un dominio de canales sin inicializar => scan
	 *    flaky (0 ó ~16 beacons según el boot). El OEM lo manda una vez en wlanAdapterStart. */
	memset(&dom, 0, sizeof(dom));
	dom.subband[0].reg_class = 81;
	dom.subband[0].band = 1;		/* BAND_2G4 */
	dom.subband[0].chan_span = 1;		/* CHNL_SPAN_5 */
	dom.subband[0].first_chan = 1;
	dom.subband[0].num_chans = 13;
	wifi_send_cmd(w, CMD_ID_SET_DOMAIN_INFO, 1, &dom, sizeof(dom), 0);
	dev_info(w->dev, "*** Fase1: SET_DOMAIN_INFO 2.4G ch1-13 ***\n");
	mutex_unlock(&w->hif_lock);
}

/* loguear un beacon/probe-resp (sin cfg80211 todavía): SSID + canal + RSSI. (Valida el path firmware.) */
static void wifi_rx_mgmt_log(struct mt6582_wifi *w, u8 *rx, u32 plen, int *beacons)
{
	struct hif_rx_header *h = (void *)rx;
	u32 off = 12 + (h->header_len_offset & HIF_RX_HDR_OFFSET_MASK);
	u8 *frame = rx + off;
	u32 flen = (plen > off) ? plen - off : 0;
	u8 subtype = flen ? (frame[0] & 0xf0) >> 4 : 0xff;
	s32 dbm = (min_t(u32, h->rcpi, 220) >> 1) - 110;
	char ssid[33] = "";
	u8 *ie;
	u32 ielen;

	if (subtype != 8 && subtype != 5)	/* beacon=8, probe-resp=5 */
		return;
	(*beacons)++;
	if (flen >= 38) {			/* 24 mac-hdr + 12 fixed (+2 IE mín) */
		ie = frame + 36; ielen = flen - 36;
		while (ielen >= 2) {
			u8 id = ie[0], len = ie[1];

			if (2u + len > ielen)
				break;
			if (id == 0) {		/* SSID IE */
				u8 n = min_t(u8, len, 32);

				memcpy(ssid, ie + 2, n); ssid[n] = 0;
				break;
			}
			ie += 2 + len; ielen -= 2 + len;
		}
	}
	if (w->dbg_evt < 40) { w->dbg_evt++;
		dev_info(w->dev, "Fase1 BEACON: SSID='%s' ch=%u %ddBm\n",
			 ssid[0] ? ssid : "(oculto)", h->hw_channel_num, dbm); }
}

/* drenar AMBOS puertos RX una vez: port0 (DATA/MGMT) + port1 (EVENT). Devuelve 1 si SCAN_DONE. */
static int wifi_rx_drain_log(struct mt6582_wifi *w, int *beacons)
{
	u32 wrplr = rd(w->hif, MCR_WRPLR);
	u32 l0 = WRPLR_RX0_LEN(wrplr), l1 = WRPLR_RX1_LEN(wrplr);
	u8 rx[1600];

	if (l0 && ALIGN(l0, 4) <= sizeof(rx)) {
		struct hif_rx_header *h = (void *)rx;

		wifi_port_read_pio(w, rx, ALIGN(l0, 4));		/* port 0 */
		if ((le16_to_cpu(h->packet_type) & HIF_RX_PKT_TYPE_MASK) == HIF_RX_PKT_TYPE_MGMT)
			wifi_rx_mgmt_log(w, rx, l0, beacons);
	}
	if (l1 && ALIGN(l1, 4) <= sizeof(rx)) {
		struct wifi_event *ev = (void *)rx;

		wifi_port1_read_pio(w, rx, ALIGN(l1, 4));		/* port 1 */
		if ((le16_to_cpu(ev->packet_type) & HIF_RX_PKT_TYPE_MASK) == HIF_RX_PKT_TYPE_EVENT &&
		    ev->eid == EVENT_ID_SCAN_DONE)
			return 1;
	}
	return 0;
}

/* scan pasivo 2.4G (prereq SET_DOMAIN_INFO) + sondeo de beacons ~6s. Síncrono (bring-up). */
static void __maybe_unused wifi_scan_test(struct mt6582_wifi *w)
{
	struct cmd_set_domain_info dom;
	struct cmd_scan_req_v2 sc;
	int i, beacons = 0, done = 0;

	mutex_lock(&w->hif_lock);
	/* tabla regulatoria mundo: 2.4G ch1-13 */
	memset(&dom, 0, sizeof(dom));
	dom.subband[0].reg_class = 81;
	dom.subband[0].band = 1;	/* BAND_2G4 */
	dom.subband[0].chan_span = 1;	/* CHNL_SPAN_5 */
	dom.subband[0].first_chan = 1;
	dom.subband[0].num_chans = 13;
	wifi_send_cmd(w, CMD_ID_SET_DOMAIN_INFO, 1, &dom, sizeof(dom), 0);
	/* scan pasivo, el FW expande los canales 2.4G */
	memset(&sc, 0, sizeof(sc));
	sc.seq_num = 0x55;
	sc.network_type = NETWORK_TYPE_AIS;
	sc.scan_type = SCAN_TYPE_PASSIVE;
	sc.channel_type = SCAN_CHANNEL_2G4;
	wifi_send_cmd(w, CMD_ID_SCAN_REQ_V2, 1, &sc,
		      offsetof(struct cmd_scan_req_v2, ie), 0);
	dev_info(w->dev, "*** Fase1: scan pasivo 2.4G enviado, sondeando ~6s ***\n");
	for (i = 0; i < 600 && !done; i++) {
		done = wifi_rx_drain_log(w, &beacons);
		msleep(10);
	}
	dev_info(w->dev, "*** Fase1: scan %s — %d beacons recibidos ***\n",
		 done ? "DONE" : "timeout", beacons);
	mutex_unlock(&w->hif_lock);
}

/* ======================================================================
 *  FASE 1 (cfg80211): wiphy + netdev wlan0 + scan pasivo expuesto a userspace.
 *  RX por kthread polling (sin IRQ aún): beacons MGMT (puerto 0) ->
 *  cfg80211_inform_bss_frame; EVENT_ID_SCAN_DONE (puerto 1) -> cfg80211_scan_done.
 *  El scan lo dispara `iw wlan0 scan` vía .scan. (wifi_*_log de arriba = el PoC
 *  que validó 14 beacons; esta es la versión cfg80211 real.)
 * ====================================================================== */
#define CHAN2G(_ch, _freq) { .band = NL80211_BAND_2GHZ, .center_freq = (_freq), \
			     .hw_value = (_ch), .max_power = 20 }
static struct ieee80211_channel wifi_2ghz_channels[] = {
	CHAN2G(1, 2412), CHAN2G(2, 2417), CHAN2G(3, 2422), CHAN2G(4, 2427),
	CHAN2G(5, 2432), CHAN2G(6, 2437), CHAN2G(7, 2442), CHAN2G(8, 2447),
	CHAN2G(9, 2452), CHAN2G(10, 2457), CHAN2G(11, 2462), CHAN2G(12, 2467),
	CHAN2G(13, 2472),
};
static struct ieee80211_rate wifi_rates[] = {
	{ .bitrate = 10 }, { .bitrate = 20 }, { .bitrate = 55 }, { .bitrate = 110 },
	{ .bitrate = 60 }, { .bitrate = 90 }, { .bitrate = 120 }, { .bitrate = 180 },
	{ .bitrate = 240 }, { .bitrate = 360 }, { .bitrate = 480 }, { .bitrate = 540 },
};
static struct ieee80211_supported_band wifi_band_2ghz = {
	.band = NL80211_BAND_2GHZ,
	.channels = wifi_2ghz_channels,
	.n_channels = ARRAY_SIZE(wifi_2ghz_channels),
	.bitrates = wifi_rates,
	.n_bitrates = ARRAY_SIZE(wifi_rates),
};
/* dominio regulatorio custom (mundo, 2.4G ch1-13, solo pasivo) — evita depender de regulatory.db */
static const struct ieee80211_regdomain wifi_regd = {
	.n_reg_rules = 1,
	.alpha2 = "99",
	.reg_rules = {
		REG_RULE(2412 - 10, 2472 + 10, 40, 0, 20, NL80211_RRF_NO_IR),
	},
};

/* indicar un beacon/probe-resp a cfg80211 (recibido por el puerto 0 = MGMT). */
static void wifi_send_assoc(struct mt6582_wifi *w, const u8 *bssid);
static void wifi_send_join(struct mt6582_wifi *w);

static void wifi_rx_mgmt(struct mt6582_wifi *w, u8 *rx, u32 plen)
{
	struct hif_rx_header *h = (void *)rx;
	u32 off = 12 + (h->header_len_offset & HIF_RX_HDR_OFFSET_MASK);
	u8 *frame = rx + off;
	u32 flen = (plen > off) ? plen - off : 0;
	u8 subtype = flen ? (frame[0] & 0xf0) >> 4 : 0xff;
	s32 dbm = (min_t(u32, h->rcpi, 220) >> 1) - 110;
	struct ieee80211_channel *ch;
	struct cfg80211_bss *bss;

	/* AUTH(11)/ASSOC-RESP(1)/DEAUTH(12)/DISASSOC(10) = respuesta del AP a nuestro mgmt-TX (SAA) */
	if (subtype == 11 || subtype == 1 || subtype == 12 || subtype == 10)
		dev_info(w->dev, "*** mgmt-RX subtype=%u len=%u %*ph ***\n",
			 subtype, flen, min_t(int, flen, 30), frame);
	/* SAA paso 2/3: el AP responde a nuestro AUTH/ASSOC. Estamos bajo hif_lock => podemos TX. */
	if (w->connecting && subtype == 11 && flen >= 30) {		/* AUTH seq=2 */
		u16 aseq = le16_to_cpu(*(__le16 *)(frame + 26));
		u16 st = le16_to_cpu(*(__le16 *)(frame + 28));

		if (aseq == 2 && st == 0) {
			dev_info(w->dev, "*** AUTH-2 OK (status=0) -> ASSOC-REQ ***\n");
			wifi_send_assoc(w, w->connect_bssid);
		} else {
			cfg80211_connect_bss(w->ndev, w->connect_bssid, w->connect_bss, NULL, 0, NULL, 0,
					     WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_ATOMIC,
					     NL80211_TIMEOUT_UNSPECIFIED);
			w->connect_bss = NULL;
			w->connecting = false;
		}
		return;
	}
	if (w->connecting && subtype == 1 && flen >= 28) {		/* ASSOC-RESP */
		u16 st = le16_to_cpu(*(__le16 *)(frame + 26));

		if (st == 0) {
			w->assoc_id = le16_to_cpu(*(__le16 *)(frame + 28)) & 0x3FFF;	/* AID (cap@24,status@26,AID@28; sin los 2 bits altos) */
			wifi_send_join(w);	/* JoinComplete: SET_BSS_INFO(CONNECTED) -> el FW enruta datos */
		}
		cfg80211_connect_bss(w->ndev, w->connect_bssid, w->connect_bss, NULL, 0, NULL, 0,
				     st == 0 ? WLAN_STATUS_SUCCESS : WLAN_STATUS_UNSPECIFIED_FAILURE,
				     GFP_ATOMIC, NL80211_TIMEOUT_UNSPECIFIED);
		w->connect_bss = NULL;
		w->connecting = false;
		w->connected = (st == 0);	/* habilita el TX de datos (ndo_xmit) */
		dev_info(w->dev, "*** ASSOC-RESP status=%u (%s) ***\n",
			 st, st == 0 ? "CONNECTED" : "rechazado");
		return;
	}
	if ((subtype != 8 && subtype != 5) || flen < 36 || !w->wiphy)
		return;
	ch = ieee80211_get_channel(w->wiphy,
				   ieee80211_channel_to_frequency(h->hw_channel_num,
								  NL80211_BAND_2GHZ));
	if (!ch)
		return;
	bss = cfg80211_inform_bss_frame(w->wiphy, ch,
					(struct ieee80211_mgmt *)frame, flen,
					dbm * 100, GFP_ATOMIC);
	if (bss)
		cfg80211_put_bss(w->wiphy, bss);
}

/* despachar un EVENT del FW por su eid. *** EN ESTE CHIP LOS EVENTOS ASYNC LLEGAN POR EL PUERTO 0 ***
 * (packet_type=EVENT=1); el puerto 1 solo trae las respuestas SÍNCRONAS de cmd (NIC_CAP, MAC). */
static void wifi_handle_event(struct mt6582_wifi *w, u8 *rx, u32 len)
{
	struct wifi_event *ev = (void *)rx;

	if (ev->eid == EVENT_ID_SCAN_DONE && w->scan_req) {
		struct cfg80211_scan_info info = { .aborted = false };

		cfg80211_scan_done(w->scan_req, &info);
		w->scan_req = NULL;
		dev_info(w->dev, "*** SCAN_DONE ***\n");
	} else if (ev->eid == EVENT_ID_TX_DONE) {
		u8 seq = rx[8], st = rx[9];	/* EVENT_TX_DONE_T: body[0]=seq, body[1]=status TX_RESULT */

		dev_info(w->dev, "*** TX-DONE seq=%u status=%u (%s) ***\n", seq, st,
			 st == 0 ? "SUCCESS transmitido+ACK" :
			 st == 1 ? "LIFE_TIMEOUT (sin ACK del AP)" :
			 st == 3 ? "MPDU_ERROR" : "fallo-TX");
	} else if (ev->eid == EVENT_ID_CONNECTION_STATUS && w->connecting) {
		struct event_connection_status *cs = (void *)(rx + sizeof(*ev));
		bool ok = (cs->media_status == MEDIA_STATE_CONNECTED);

		cfg80211_connect_result(w->ndev,
			ok ? cs->bssid : w->connect_bssid, NULL, 0, NULL, 0,
			ok ? WLAN_STATUS_SUCCESS : WLAN_STATUS_UNSPECIFIED_FAILURE,
			GFP_ATOMIC);
		w->connecting = false;
		dev_info(w->dev, "*** EVENT_CONNECTION_STATUS media=%u (%s) %pM ***\n",
			 cs->media_status, ok ? "CONNECTED" : "fail", cs->bssid);
	} else if (ev->eid != 0x0e) {	/* 0x0e = heartbeat periódico del FW (~30ms); ignorar sin spamear */
		dev_info(w->dev, "evt eid=0x%02x len=%u %*ph\n",
			 ev->eid, len, min_t(int, len, 20), rx);
	}
}

/* Fase 3 TX: enviar un paquete de DATOS (skb Ethernet) por WTDR0 (puerto 0, TC1, PKT_TYPE=DATA).
 * El FW full-MAC encapsula 802.3 -> 802.11. Caller con hif_lock (lo llama el rx_thread). */
static void wifi_tx_data(struct mt6582_wifi *w, struct sk_buff *skb)
{
	struct hif_tx_header *h = (void *)w->dlm;
	u16 frame_len = skb->len;
	u16 total = sizeof(*h) + frame_len;
	bool bmc = is_multicast_ether_addr(skb->data);	/* DA = skb->data[0..5]; bit0 del byte0 = mcast/bcast */

	if (frame_len > 1600 || !w->ndev)
		return;
	memset(h, 0, sizeof(*h));
	h->tx_byte_count_up = cpu_to_le16(total & 0x0fff);	/* incluye los 16B de cabecera HIF */
	h->ether_type_offset = 14;				/* ((ETH_HLEN-2)+16)>>1 palabras, incl. HIF */
	/* broadcast/multicast (¡el DHCP DISCOVER!): TC5 + staRecIdx=0xFF; unicast: TC1 + staRecIdx=0 (el AP).
	 * El FW pone el addressing 802.11 (A1=BSSID, A3=DA). que_mgt.c: BMCAST -> TC5_INDEX, idx=0xFF. */
	h->resource_pkttype_cs = ((bmc ? WIFI_TC_BMCAST : WIFI_TC_DATA) << HIF_TX_RESOURCE_SHIFT) |
				 (HIF_TX_PKT_TYPE_DATA << HIF_TX_PKT_TYPE_SHIFT);
	h->wlan_header_len = ETH_HLEN;		/* =14: el FW localiza el ethertype para 802.3->802.11 (NO 0) */
	h->sta_rec_idx = bmc ? 0xFF : 0;	/* STA_REC_INDEX_BMCAST / el AP */
	/* HIF_TX_HEADER para datos 802.3 NORMALES (nicTxMsduQueue nic_tx.c:1408-1426, verificado byte a byte):
	 *   fwd_sess = BURST_END (BIT5) — un pkt suelto = fin de burst (fgIsBurstEnd=TRUE en que_mgt.c); sin esto el FW ENCOLA esperando más
	 *   pktfmt   = 0 — ucFormatID=0 | net_type(AIS)=0 | fgIs1x=0 (NO EAPOL) | fgIs802_11=0. (FLAG_1X solo en la ruta EAPOL 1635-1644)
	 *   wlan_hdr = ETH_HLEN(14) ya puesto arriba (ucMacHeaderLen=ETH_HLEN para datos OS, nic_tx.c:2021)
	 *   pkt_seq  = 0 — datos NO piden TX-DONE (solo mgmt); ack_bip = 0 — el FW elige rate/ACK del STA-record */
	h->fwd_sess = HIF_TX_BURST_END;
	h->pktfmt_flags = 0;
	h->pkt_seq = 0;
	h->ack_bip_rate = 0;
	memcpy((u8 *)w->dlm + sizeof(*h), skb->data, frame_len);
	/* dword-cero terminador de TX-aggregation = comportamiento CORRECTO del downstream (HAL_WRITE_TX_PORT, hal.h:300;
	 * lo añade a CADA TX para cerrar el frame). SOLO datos — el init/descarga del FW NO lo lleva (corrompería la
	 * imagen). NOTA: el fix del DHCP fue la MAC del netdev, no esto; se mantiene por fidelidad al downstream. */
	*(__le32 *)((u8 *)w->dlm + ALIGN(total, 4)) = 0;
	wifi_port_write_pio(w, w->dlm, ALIGN(total, 4) + 4);	/* puerto 0 = WTDR0 (+4 = dword-cero terminador) */
	w->ndev->stats.tx_packets++;
	w->ndev->stats.tx_bytes += frame_len;
}

/* Fase 3 RX: paquete de DATOS -> quitar la cabecera HIF (12B + offset) -> Ethernet directo -> netif_rx.
 * El FW ya convirtió 802.11 -> 802.3 (SIN LLC/SNAP). Caller con hif_lock. */
static void wifi_rx_data(struct mt6582_wifi *w, u8 *rx, u32 plen)
{
	struct hif_rx_header *h = (void *)rx;
	u32 off = sizeof(*h) + (h->header_len_offset & HIF_RX_HDR_OFFSET_MASK);	/* 12 + padding(0-3) */
	u32 eth_len;
	struct sk_buff *skb;

	if (!w->ndev || plen <= off)
		return;
	eth_len = plen - off;
	if (eth_len > 1600)
		return;
	skb = netdev_alloc_skb(w->ndev, eth_len + 2);
	if (!skb)
		return;
	skb_reserve(skb, 2);					/* alinea la IP a 4 */
	memcpy(skb_put(skb, eth_len), rx + off, eth_len);
	skb->protocol = eth_type_trans(skb, w->ndev);		/* la trama YA es Ethernet II */
	w->ndev->stats.rx_packets++;
	w->ndev->stats.rx_bytes += eth_len;
	netif_rx(skb);
}

/* drenar AMBOS puertos RX una vez: port0 (MGMT->inform_bss + EVENT->handle) + port1 (EVENT). hif_lock. */
static void wifi_rx_drain(struct mt6582_wifi *w)
{
	u32 wrplr = rd(w->hif, MCR_WRPLR);
	u32 l0 = WRPLR_RX0_LEN(wrplr), l1 = WRPLR_RX1_LEN(wrplr);
	u8 rx[1600];

	if (l0 && ALIGN(l0, 4) <= sizeof(rx)) {
		struct hif_rx_header *h = (void *)rx;
		u16 pt;

		wifi_port_read_pio(w, rx, ALIGN(l0, 4));
		pt = le16_to_cpu(h->packet_type) & HIF_RX_PKT_TYPE_MASK;
		if (pt == HIF_RX_PKT_TYPE_MGMT)
			wifi_rx_mgmt(w, rx, l0);
		else if (pt == HIF_RX_PKT_TYPE_EVENT)	/* *** los EVENTOS del FW llegan por aquí *** */
			wifi_handle_event(w, rx, l0);
		else if (pt == HIF_RX_PKT_TYPE_DATA)	/* *** Fase 3: paquetes de DATOS -> netif_rx *** */
			wifi_rx_data(w, rx, l0);
	}
	if (l1 && ALIGN(l1, 4) <= sizeof(rx)) {
		struct wifi_event *ev = (void *)rx;

		wifi_port1_read_pio(w, rx, ALIGN(l1, 4));
		if ((le16_to_cpu(ev->packet_type) & HIF_RX_PKT_TYPE_MASK) == HIF_RX_PKT_TYPE_EVENT)
			wifi_handle_event(w, rx, l1);	/* por si algún evento llegara también por aquí */
	}
}

/* kthread RX: sondea ~20ms; cierra el scan por timeout si el FW no manda SCAN_DONE. */
static int wifi_rx_thread(void *data)
{
	struct mt6582_wifi *w = data;
	struct sk_buff *skb;

	while (!kthread_should_stop()) {
		mutex_lock(&w->hif_lock);
		wifi_rx_drain(w);
		while ((skb = skb_dequeue(&w->tx_queue)) != NULL) {	/* Fase 3: TX de datos encolados */
			wifi_tx_data(w, skb);
			dev_kfree_skb(skb);
		}
		if (w->scan_req && time_after(jiffies, w->scan_deadline)) {
			struct cfg80211_scan_info info = { .aborted = false };

			cfg80211_scan_done(w->scan_req, &info);
			w->scan_req = NULL;
		}
		mutex_unlock(&w->hif_lock);
		msleep(20);
	}
	return 0;
}

/* .scan: manda CMD_SCAN_REQ_V2 pasivo 2.4G; los beacons llegan async por el kthread. */
static int wifi_cfg_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
{
	struct mt6582_wifi *w = g_wifi;
	struct cmd_scan_req_v2 sc;

	if (!w || !w->started)
		return -ENODEV;
	mutex_lock(&w->hif_lock);
	if (w->scan_req) {
		mutex_unlock(&w->hif_lock);
		return -EBUSY;
	}
	memset(&sc, 0, sizeof(sc));
	sc.seq_num = ++w->scan_seq;
	sc.network_type = NETWORK_TYPE_AIS;
	sc.scan_type = SCAN_TYPE_PASSIVE;
	sc.channel_type = SCAN_CHANNEL_2G4;
	wifi_send_cmd(w, CMD_ID_SCAN_REQ_V2, 1, &sc,
		      offsetof(struct cmd_scan_req_v2, ie), 0);
	w->scan_req = request;
	w->scan_deadline = jiffies + msecs_to_jiffies(8000);
	mutex_unlock(&w->hif_lock);
	dev_info(w->dev, "scan: pasivo 2.4G lanzado\n");
	return 0;
}

/* enviar un frame de GESTIÓN 802.11 (AUTH/ASSOC) por TC4/puerto-1 con HIF_TX_HEADER de 16B
 * (PKT_TYPE=MGMT). El caller debe tener hif_lock. *** El MT6582 es softMAC: el HOST manda los
 * frames de auth/assoc (no el FW) — módulo SAA del downstream. *** */
static void wifi_send_mgmt(struct mt6582_wifi *w, const void *frame, u16 frame_len, u8 sta_idx)
{
	struct hif_tx_header *h = (void *)w->dlm;
	u16 total = sizeof(*h) + frame_len;

	memset(h, 0, sizeof(*h));
	h->tx_byte_count_up = cpu_to_le16(total & 0x0fff);
	h->ether_type_offset = (sizeof(*h) + 24) >> 1;	/* mgmt: cabecera 802.11 = 24B, sin LLC */
	h->resource_pkttype_cs = (WIFI_TC4 << HIF_TX_RESOURCE_SHIFT) |
				 (HIF_TX_PKT_TYPE_MGMT << HIF_TX_PKT_TYPE_SHIFT);	/* 0xD0 */
	h->wlan_header_len = 24;
	h->pktfmt_flags = HIF_TX_FLAG_802_11;		/* frame 802.11 crudo (net_type AIS=0) */
	h->sta_rec_idx = sta_idx;
	h->ack_bip_rate = HIF_TX_NEED_ACK | HIF_TX_BASIC_RATE;	/* AUTH/ASSOC a basic-rate, o el FW da MPDU_ERROR */
	if (++w->mgmt_seq == 0)		/* pkt_seq != 0 => el FW reporta EVENT_ID_TX_DONE con el status */
		w->mgmt_seq = 1;
	h->pkt_seq = w->mgmt_seq;
	memcpy((u8 *)w->dlm + sizeof(*h), frame, frame_len);
	wifi_port1_write_pio(w, w->dlm, total);
}

/* construir y enviar un AUTH Open-System (seq=N) al AP (paso 1 del SAA). */
static void wifi_send_auth(struct mt6582_wifi *w, const u8 *bssid, u16 seq)
{
	struct {
		__le16 fc, dur;
		u8 da[6], sa[6], bssid[6];
		__le16 seq_ctrl, alg, auth_seq, status;
	} __packed f;

	memset(&f, 0, sizeof(f));
	f.fc = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_AUTH);	/* 0x00b0 */
	memcpy(f.da, bssid, ETH_ALEN);
	memcpy(f.sa, w->mac, ETH_ALEN);
	memcpy(f.bssid, bssid, ETH_ALEN);
	f.alg = cpu_to_le16(0);			/* Open System */
	f.auth_seq = cpu_to_le16(seq);
	wifi_send_mgmt(w, &f, sizeof(f), 0);	/* sta_rec_idx = el MISMO que el UPDATE_STA_RECORD (=0) */
	dev_info(w->dev, "*** mgmt-TX: AUTH open seq=%u -> %pM ***\n", seq, bssid);
}

/* construir y enviar el ASSOC-REQ (paso 2 del SAA, tras recibir el AUTH seq=2). IEs: SSID + tasas. */
static void wifi_send_assoc(struct mt6582_wifi *w, const u8 *bssid)
{
	u8 buf[128];
	struct {
		__le16	fc, dur;
		u8	da[6], sa[6], bssid[6];
		__le16	seq_ctrl, cap, listen_int;
	} __packed *h = (void *)buf;
	u8 *ie = buf + sizeof(*h);

	memset(buf, 0, sizeof(buf));
	h->fc = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ASSOC_REQ);	/* 0x0000 */
	memcpy(h->da, bssid, ETH_ALEN);
	memcpy(h->sa, w->mac, ETH_ALEN);
	memcpy(h->bssid, bssid, ETH_ALEN);
	h->cap = cpu_to_le16(WLAN_CAPABILITY_ESS);	/* red abierta: solo ESS */
	h->listen_int = cpu_to_le16(1);
	*ie++ = WLAN_EID_SSID;       *ie++ = w->connect_ssid_len;
	memcpy(ie, w->connect_ssid, w->connect_ssid_len); ie += w->connect_ssid_len;
	*ie++ = WLAN_EID_SUPP_RATES; *ie++ = 8;
	memcpy(ie, "\x82\x84\x8b\x96\x0c\x12\x18\x24", 8); ie += 8;	/* 1,2,5.5,11(b),6,9,12,18 */
	*ie++ = WLAN_EID_EXT_SUPP_RATES; *ie++ = 4;
	memcpy(ie, "\x30\x48\x60\x6c", 4); ie += 4;			/* 24,36,48,54 */
	wifi_send_mgmt(w, buf, ie - buf, 0);
	dev_info(w->dev, "*** mgmt-TX: ASSOC-REQ -> %pM ***\n", bssid);
}

/* JoinComplete (tras ASSOC-RESP status=0): SET_BSS_INFO con conn_state=CONNECTED -> el FW entra en
 * estado de DATOS y enruta TX/RX. Sin esto queda asociado pero NO reenvía datos (DHCP falla, RX=0).
 * Paso 5 de aisFsmRunEventJoinRequest. Caller bajo hif_lock (lo llama wifi_rx_mgmt). */
static void wifi_send_join(struct mt6582_wifi *w)
{
	struct cmd_set_bss_info bi = {};
	struct cmd_update_sta_record sta = {};
	struct cmd_ps_profile ps = {};
	struct cmd_ch_privilege chp = {};
	struct cmd_pm_bss_connected pm = {};
	__le32 rxf;

	bi.net_type_idx = NETWORK_TYPE_AIS;
	bi.conn_state = MEDIA_STATE_CONNECTED;	/* =0. ENUM_PARAM_MEDIA_STATE_T (downstream wlan_oid.h:372) = {CONNECTED=0, DISCONNECTED=1}: el 2 estaba FUERA DE RANGO -> el FW ignoraba el SET_BSS_INFO -> nunca entraba en estado de datos (RX=0). Confirmado: reg.h, el EVENT_CONNECTION_STATUS y el enum del downstream coinciden en 0. */
	bi.op_mode = OP_MODE_INFRASTRUCTURE;
	bi.ssid_len = w->connect_ssid_len;
	memcpy(bi.ssid, w->connect_ssid, w->connect_ssid_len);
	memcpy(bi.bssid, w->connect_bssid, ETH_ALEN);
	bi.op_rate_set = cpu_to_le16(RATE_SET_ERP);
	bi.basic_rate_set = cpu_to_le16(BASIC_RATE_SET_ERP);
	bi.sta_rec_idx_of_ap = 0;	/* STA-record del AP (idx 0, creado en .connect) */
	bi.auth_mode = AUTH_MODE_OPEN;
	bi.enc_status = ENC_STATUS_DISABLED;
	bi.phy_type_set = PHY_TYPE_SET_802_11BG;
	memcpy(bi.own_mac, w->mac, ETH_ALEN);
	bi.rlm.net_type_idx = NETWORK_TYPE_AIS;
	bi.rlm.rf_band = 1;		/* BAND_2G4 */
	bi.rlm.primary_channel = w->connect_channel;
	bi.rlm.check_id = 0x72;
	wifi_send_cmd(w, CMD_ID_SET_BSS_INFO, 1, &bi, sizeof(bi), 0);

	/* promover el STA a STATE_3 (asociado/data-ready). SIN esto el FW descarta TODO dato del STA:
	 * secCheckClassError() (privacy.c) tira cada RX de datos como Class-3-error Y manda DEAUTH al AP,
	 * y rechaza el TX. Es el cnmStaRecChangeState(STATE_3) del downstream -> abre la cola TX del STA. */
	sta.index = 0;				/* el mismo staRec del AP (creado en .connect) */
	sta.sta_type = STA_TYPE_LEGACY_AP;
	memcpy(sta.mac_addr, w->connect_bssid, ETH_ALEN);
	sta.net_type_index = NETWORK_TYPE_AIS;
	sta.desired_phy_type_set = PHY_TYPE_SET_802_11BG;
	sta.desired_nonht_rate_set = cpu_to_le16(RATE_SET_ERP);
	sta.bss_basic_rate_set = cpu_to_le16(BASIC_RATE_SET_ERP);
	sta.sta_state = STA_STATE_3;		/* =2: asociado, Class 1,2&3 -> el FW acepta DATOS */
	sta.need_resp = 1;			/* el FW responde EVENT_ACTIVATE_STA_REC -> activa la cola TX */
	wifi_send_cmd(w, CMD_ID_UPDATE_STA_RECORD, 1, &sta, sizeof(sta), 0);

	/* CAM (siempre despierto): sin esto el FW entra en power-save, DUERME, y pierde tanto los beacons del
	 * AP (EVENT_ID_BSS_BEACON_TIMEOUT 0x1b a los 30s -> desconexión) como la OFFER del DHCP (el AP la
	 * manda mientras el FW duerme). = nicConfigPowerSaveProfile(AIS, Param_PowerModeCAM) del downstream. */
	ps.net_type_index = NETWORK_TYPE_AIS;
	ps.ps_profile = 0;			/* Param_PowerModeCAM = 0 */
	wifi_send_cmd(w, CMD_ID_POWER_SAVE_MODE, 1, &ps, sizeof(ps), 0);

	/* FILTRO RX (clave para la OFFER): el FW por DEFECTO NO entrega broadcast al host -> la OFFER del
	 * DHCP se descarta en la ENTRADA. CMD_ID_SET_RX_FILTER con DIRECTED|MULTICAST|BROADCAST (0x0B) abre
	 * la entrega de la OFFER (broadcast) + unicast a mi MAC. = u4OsPacketFilter=PARAM_PACKET_FILTER_SUPPORTED
	 * del downstream (wlan_lib.c:1248); un driver mínimo que solo replica el JOIN se lo salta. */
	rxf = cpu_to_le32(0x0000000B);
	wifi_send_cmd(w, CMD_ID_SET_RX_FILTER, 1, &rxf, sizeof(rxf), 0);

	/* INDICAR BSS CONECTADO al power-management del FW (beacon-interval/DTIM/AID) = comportamiento CORRECTO del
	 * downstream (nicPmIndicateBssConnected, nic.c:2167; el stock lo manda tras el assoc). NOTA: NO fue el fix del
	 * DHCP ni evita el 0x1b (benigno); se mantiene por fidelidad. beacon=100/DTIM=1 por defecto (CAM=siempre despierto). */
	pm.net_type_idx = NETWORK_TYPE_AIS;
	pm.dtim_period = 1;
	pm.assoc_id = cpu_to_le16(w->assoc_id);
	pm.beacon_interval = cpu_to_le16(100);
	wifi_send_cmd(w, CMD_ID_INDICATE_PM_BSS_CONNECTED, 1, &pm, sizeof(pm), 0);

	/* Soltar el privilege del canal tras el join = comportamiento CORRECTO del downstream (aisFsmReleaseCh al
	 * entrar en AIS_STATE_NORMAL_TR, ais_fsm.c:2632): el CH_PRIVILEGE(REQ) del .connect es TEMPORAL (max_interval
	 * 5s); al soltarlo el FW cae al CANAL-HOME del BSS (fijado por el SET_BSS_INFO de arriba). action=ABORT, mismo
	 * token (0). NOTA: NO fue el fix del DHCP (lo fue la MAC del netdev = MAC del FW, ver wifi_register_cfg80211);
	 * se mantiene por fidelidad al downstream. El EVENT 0x1b a +30s es BENIGNO (la conexión sobrevive; lo ignoramos). */
	chp.net_type_idx = NETWORK_TYPE_AIS;
	chp.token_id = 0;			/* = el token del REQ en .connect (zero-init) */
	chp.action = CMD_CH_ACTION_ABORT;
	chp.primary_channel = w->connect_channel;
	chp.rf_band = 1;			/* BAND_2G4 */
	memcpy(chp.bssid, w->connect_bssid, ETH_ALEN);
	wifi_send_cmd(w, CMD_ID_CH_PRIVILEGE, 1, &chp, sizeof(chp), 0);

	dev_info(w->dev, "*** JoinComplete: SET_BSS_INFO + STA->STATE_3 + PS=CAM + RX_FILTER + CH_ABORT(home ch=%u) -> data-path ON ***\n",
		 w->connect_channel);
}

/* esperar el grant del canal (EVENT_ID_CH_PRIVILEGE, 0x18) por el puerto 0; ignora beacons/heartbeats.
 * El downstream BLOQUEA aquí antes del JOIN — sin el grant el BSS queda off-channel (BSS-ABSENCE 0x19)
 * y el AUTH se descarta (TX FLUSHED). Caller con hif_lock (el kthread no compite). */
static bool wifi_wait_grant(struct mt6582_wifi *w, u32 ms)
{
	u8 rx[1600];
	u32 t;

	for (t = 0; t < ms; t += 5) {
		u32 wrplr = rd(w->hif, MCR_WRPLR);
		u32 l0 = WRPLR_RX0_LEN(wrplr);

		if (l0 && ALIGN(l0, 4) <= sizeof(rx)) {
			struct wifi_event *ev = (void *)rx;

			wifi_port_read_pio(w, rx, ALIGN(l0, 4));
			if ((le16_to_cpu(ev->packet_type) & HIF_RX_PKT_TYPE_MASK) == HIF_RX_PKT_TYPE_EVENT &&
			    ev->eid == EVENT_ID_CH_PRIVILEGE) {
				dev_info(w->dev, "*** CH_PRIVILEGE GRANT recibido ***\n");
				return true;
			}
		} else {
			msleep(5);
		}
	}
	return false;
}

/* .connect (softMAC, coreografía verificada del downstream): BSS_ACTIVATE -> CH_PRIVILEGE(REQ) ->
 * ESPERAR el grant (0x18) -> UPDATE_STA_RECORD(STATE_1) -> AUTH. SIN SET_BSS_INFO (solo tras el assoc).
 * El connect lo conduce el HOST (SAA): AUTH/ASSOC por mgmt-TX, respuestas por mgmt-RX (puerto 0). */
static int wifi_cfg_connect(struct wiphy *wiphy, struct net_device *ndev,
			    struct cfg80211_connect_params *sme)
{
	struct mt6582_wifi *w = g_wifi;
	struct cmd_bss_activate act = { .net_type_idx = NETWORK_TYPE_AIS, .active = 1 };
	struct cmd_update_sta_record sta = {};
	struct cmd_ch_privilege chp = {};
	struct cfg80211_bss *cbss = NULL;
	u8 bssid[ETH_ALEN];
	u32 ch;

	if (!w || !w->started)
		return -ENODEV;
	if (sme->ssid_len > 32)
		return -EINVAL;

	/* Necesitamos el BSSID + canal del AP: de sme, o de la caché del scan (cfg80211). */
	if (sme->bssid && sme->channel) {
		memcpy(bssid, sme->bssid, ETH_ALEN);
		ch = sme->channel->hw_value;
	} else {
		cbss = cfg80211_get_bss(wiphy, sme->channel, sme->bssid,
					sme->ssid, sme->ssid_len,
					IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);
		if (!cbss) {
			dev_warn(w->dev, ".connect: '%.*s' no está en la caché — escanea antes\n",
				 sme->ssid_len, sme->ssid);
			return -ENOENT;
		}
		memcpy(bssid, cbss->bssid, ETH_ALEN);
		ch = cbss->channel->hw_value;
		cfg80211_put_bss(wiphy, cbss);
	}

	/* Retener el BSS para devolverlo en cfg80211_connect_bss al reportar el resultado: si NO lo
	 * pasamos, cfg80211 hace un lookup por BSSID que FALLA si el BSS expiró de su caché durante el
	 * handshake -> WARN_ON(bss_not_found) en sme.c:845 + connect abortado ("Not connected").
	 * (Doc del kernel en cfg80211.h: "hold a reference ... to avoid a warning if the bss is expired".) */
	if (w->connect_bss)
		cfg80211_put_bss(wiphy, w->connect_bss);
	w->connect_bss = cfg80211_get_bss(wiphy, sme->channel, bssid, sme->ssid, sme->ssid_len,
					  IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);

	mutex_lock(&w->hif_lock);
	memcpy(w->connect_bssid, bssid, ETH_ALEN);
	w->connect_channel = ch;
	w->connect_ssid_len = min_t(u8, sme->ssid_len, sizeof(w->connect_ssid));
	memcpy(w->connect_ssid, sme->ssid, w->connect_ssid_len);

	/* 1) activar la red/BSS AIS (active=1) — ANTES del CH_PRIVILEGE (el downstream lo hace en SEARCH) */
	wifi_send_cmd(w, CMD_ID_BSS_ACTIVATE_CTRL, 1, &act, sizeof(act), 0);

	/* 2) CH_PRIVILEGE(REQ) y BLOQUEAR hasta el grant (0x18). Saltarse esto = BSS off-channel
	 *    (EVENT 0x19 BSS-ABSENCE) y el AUTH descartado (TX FLUSHED). El msleep NO equivale al grant. */
	chp.net_type_idx = NETWORK_TYPE_AIS;
	chp.action = CMD_CH_ACTION_REQ;
	chp.primary_channel = ch;
	chp.rf_band = 1;			/* BAND_2G4 */
	chp.max_interval = cpu_to_le32(5000);
	memcpy(chp.bssid, bssid, ETH_ALEN);
	wifi_send_cmd(w, CMD_ID_CH_PRIVILEGE, 1, &chp, sizeof(chp), 0);
	if (!wifi_wait_grant(w, 1000))
		dev_warn(w->dev, ".connect: sin grant CH_PRIVILEGE en 1s\n");

	/* 2) STA-record del AP (idx 1) — IMPRESCINDIBLE: sin él el FW no sabe a quién asociar */
	sta.index = 0;	/* cnmStaRecAlloc da el PRIMER slot libre = 0 para el 1er STA del AIS (NO 1) */
	sta.sta_type = STA_TYPE_LEGACY_AP;
	memcpy(sta.mac_addr, bssid, ETH_ALEN);
	sta.net_type_index = NETWORK_TYPE_AIS;
	sta.desired_phy_type_set = PHY_TYPE_SET_802_11BG;
	sta.desired_nonht_rate_set = cpu_to_le16(RATE_SET_ERP);
	sta.bss_basic_rate_set = cpu_to_le16(BASIC_RATE_SET_ERP);
	sta.sta_state = STA_STATE_1;	/* auth pendiente: el host hará el handshake AUTH/ASSOC (SAA) */
	wifi_send_cmd(w, CMD_ID_UPDATE_STA_RECORD, 1, &sta, sizeof(sta), 0);

	/* CARRERA cmd-vs-TX: el FW procesa los CMD (cola de comandos) y los frames MGMT (cola TX) en
	 * hilos DISTINTOS. El AUTH (TX) puede salir ANTES de que el UPDATE_STA_RECORD se aplique =>
	 * WTBL del STA vacío en ese instante => MPDU_ERROR. Damos tiempo a que el FW asiente el STA. */
	msleep(50);

	/* 4) el host arranca el handshake: AUTH Open seq=1. *** SIN SET_BSS_INFO aquí ***: el downstream
	 *    solo manda SET_BSS_INFO en JoinComplete (tras el assoc-rsp); mandarlo antes confunde al FW
	 *    (MPDU_ERROR). La respuesta (AUTH seq=2) llega como mgmt-RX por el puerto 0; luego el ASSOC-REQ. */
	wifi_send_auth(w, bssid, 1);

	w->connecting = true;
	mutex_unlock(&w->hif_lock);
	dev_info(w->dev, "*** .connect: SSID='%.*s' ch=%u BSSID=%pM (OPEN+CHPRIV+STA-rec) enviado ***\n",
		 sme->ssid_len, sme->ssid, ch, bssid);
	return 0;
}

static int wifi_cfg_disconnect(struct wiphy *wiphy, struct net_device *ndev, u16 reason)
{
	struct mt6582_wifi *w = g_wifi;
	struct cmd_set_bss_info bss = {};

	if (!w || !w->started)
		return -ENODEV;
	mutex_lock(&w->hif_lock);
	bss.net_type_idx = NETWORK_TYPE_AIS;
	bss.conn_state = MEDIA_STATE_DISCONNECTED;
	bss.op_mode = OP_MODE_INFRASTRUCTURE;
	memcpy(bss.own_mac, w->mac, ETH_ALEN);
	wifi_send_cmd(w, CMD_ID_SET_BSS_INFO, 1, &bss, sizeof(bss), 0);
	w->connecting = false;
	w->connected = false;
	if (w->connect_bss) {
		cfg80211_put_bss(wiphy, w->connect_bss);
		w->connect_bss = NULL;
	}
	mutex_unlock(&w->hif_lock);
	cfg80211_disconnected(ndev, reason, NULL, 0, true, GFP_KERNEL);
	dev_info(w->dev, "*** .disconnect enviado ***\n");
	return 0;
}

/* .add_key (WPA2): PTK/GTK vía CMD_802_11_KEY. */
static int wifi_cfg_add_key(struct wiphy *wiphy, struct net_device *ndev, int link_id,
			    u8 key_idx, bool pairwise, const u8 *mac_addr,
			    struct key_params *params)
{
	struct mt6582_wifi *w = g_wifi;
	struct cmd_802_11_key k = {};

	if (!w || !w->started || !params || params->key_len > 32)
		return -EINVAL;
	mutex_lock(&w->hif_lock);
	k.add_remove = 1;
	k.tx_key = 1;
	k.key_type = pairwise ? 1 : 0;
	if (mac_addr)
		memcpy(k.peer_addr, mac_addr, ETH_ALEN);
	k.algorithm_id = (params->cipher == WLAN_CIPHER_SUITE_CCMP) ? CIPHER_CCMP : CIPHER_NONE;
	k.key_id = key_idx;
	k.key_len = params->key_len;
	memcpy(k.key_material, params->key, params->key_len);
	wifi_send_cmd(w, CMD_ID_ADD_REMOVE_KEY, 1, &k, sizeof(k), 0);
	mutex_unlock(&w->hif_lock);
	return 0;
}

static int wifi_cfg_del_key(struct wiphy *wiphy, struct net_device *ndev, int link_id,
			    u8 key_idx, bool pairwise, const u8 *mac_addr)
{
	return 0;	/* TODO: CMD_802_11_KEY add_remove=0 */
}
static int wifi_cfg_set_default_key(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				    u8 key_idx, bool unicast, bool multicast)
{
	return 0;	/* TODO: CMD_ID_DEFAULT_KEY_ID */
}

static struct cfg80211_ops wifi_cfg_ops = {
	.scan = wifi_cfg_scan,
	.connect = wifi_cfg_connect,
	.disconnect = wifi_cfg_disconnect,
	.add_key = wifi_cfg_add_key,
	.del_key = wifi_cfg_del_key,
	.set_default_key = wifi_cfg_set_default_key,
};

static int wifi_ndo_open(struct net_device *ndev)
{
	netif_start_queue(ndev);
	return 0;
}
static int wifi_ndo_stop(struct net_device *ndev)
{
	netif_stop_queue(ndev);
	return 0;
}
static netdev_tx_t wifi_ndo_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	struct mt6582_wifi *w = g_wifi;

	/* ndo_start_xmit corre en softirq (no puede dormir): ENCOLAR; el rx_thread lo escribe
	 * por WTDR0 bajo hif_lock. Solo TX cuando hay asociación (connecting). Flow-control simple. */
	if (!w || !w->connected || skb->len > 1600 || skb_queue_len(&w->tx_queue) > 128) {
		dev_kfree_skb(skb);
		ndev->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}
	skb_queue_tail(&w->tx_queue, skb);
	return NETDEV_TX_OK;
}
static const struct net_device_ops wifi_netdev_ops = {
	.ndo_open = wifi_ndo_open,
	.ndo_stop = wifi_ndo_stop,
	.ndo_start_xmit = wifi_ndo_xmit,
};

/* registrar wiphy + wlan0 + arrancar el kthread RX. Llamado tras WLAN_READY + phase1. */
static int wifi_register_cfg80211(struct mt6582_wifi *w)
{
	struct wiphy *wiphy;
	struct net_device *ndev;
	int ret;

	if (w->cfg_registered)
		return 0;
	wiphy = wiphy_new(&wifi_cfg_ops, 0);
	if (!wiphy)
		return -ENOMEM;
	set_wiphy_dev(wiphy, w->dev);
	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
	wiphy->bands[NL80211_BAND_2GHZ] = &wifi_band_2ghz;
	wiphy->max_scan_ssids = 1;
	wiphy->max_scan_ie_len = 512;
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wiphy->regulatory_flags = REGULATORY_CUSTOM_REG;
	ret = wiphy_register(wiphy);
	if (ret) {
		dev_err(w->dev, "cfg80211: wiphy_register=%d\n", ret);
		wiphy_free(wiphy);
		return ret;
	}
	w->wiphy = wiphy;
	wiphy_apply_custom_regulatory(wiphy, &wifi_regd);
	dev_info(w->dev, "cfg80211: wiphy registrado (phy)\n");

	ndev = alloc_netdev(0, "wlan0", NET_NAME_UNKNOWN, ether_setup);
	if (!ndev) {
		dev_err(w->dev, "cfg80211: alloc_netdev fallo\n");
		wiphy_unregister(wiphy); wiphy_free(wiphy); w->wiphy = NULL;
		return -ENOMEM;
	}
	w->ndev = ndev;
	ndev->netdev_ops = &wifi_netdev_ops;
	/* MAC del netdev = MAC del FW (BASIC_CONFIG), NO una random: el AUTH/ASSOC y el SET_BSS_INFO.own_mac
	 * usan w->mac, así que el AP asocia ESA MAC. Si el netdev (origen de los frames de DATOS) usa otra,
	 * el DISCOVER del DHCP sale con source-MAC NO asociada -> el FW/AP lo descarta -> 0 OFFER, 0 DATA RX.
	 * (causa raíz que halló el auditor; encaja con el síntoma exacto). Guarda: si BASIC_CONFIG falló, random. */
	if (is_valid_ether_addr(w->mac))
		eth_hw_addr_set(ndev, w->mac);
	else
		eth_hw_addr_random(ndev);
	w->wdev.wiphy = wiphy;
	w->wdev.iftype = NL80211_IFTYPE_STATION;
	w->wdev.netdev = ndev;
	ndev->ieee80211_ptr = &w->wdev;
	SET_NETDEV_DEV(ndev, w->dev);
	ret = register_netdev(ndev);
	if (ret) {
		dev_err(w->dev, "cfg80211: register_netdev=%d\n", ret);
		free_netdev(ndev); w->ndev = NULL;
		wiphy_unregister(wiphy); wiphy_free(wiphy); w->wiphy = NULL;
		return ret;
	}

	skb_queue_head_init(&w->tx_queue);
	w->rx_thread = kthread_run(wifi_rx_thread, w, "mt6582-wifi-rx");
	w->cfg_registered = true;
	dev_info(w->dev, "*** cfg80211: wiphy + wlan0 registrados, RX-thread vivo ***\n");
	return 0;
}

/* arranca el firmware ya descargado: INIT_CMD_ID_WIFI_START + poll WLAN_READY. */
static int wifi_fw_start(struct mt6582_wifi *w)
{
	struct init_hif_tx_header *th = (void *)w->dlm;
	struct init_cmd_wifi_start *st = (void *)(th + 1);
	u32 total = sizeof(*th) + sizeof(*st);
	u32 pc0;
	int t;

	th->tx_byte_count = cpu_to_le16(total);
	th->ether_type_offset = 0;
	th->cs_flags = 0;
	th->cid = INIT_CMD_ID_WIFI_START;
	th->seq_num = ++w->cmd_seq;
	th->reserved = 0;
	st->override = 0;	/* usar la dirección de arranque por defecto del FW */
	st->address = 0;

	mutex_lock(&w->hif_lock);
	wifi_port_write_pio(w, w->dlm, total);
	mutex_unlock(&w->hif_lock);
	/* WIFI_START no lleva ACK (wlanConfigWifiFunc sólo lo envía); se sondea WLAN_READY. */

	pc0 = rd(w->mcu, CONN_MCU_CPUPCR);	/* PC del MCU antes de esperar el ready */
	for (t = 0; t < 500; t++) {	/* ~5 s (downstream: CFG_RESPONSE_POLLING_TIMEOUT x msleep 10) */
		if (rd(w->hif, MCR_WCIR) & WCIR_WLAN_READY) {
			wifi_phase1_hello(w);	/* Fase 1: hello-world del FW (NIC_CAPABILITY + MAC) */
			wifi_register_cfg80211(w);	/* Fase 1: wiphy + wlan0 + kthread RX -> iw wlan0 scan */
			return 0;
		}
		msleep(10);
	}
	dev_err(w->dev,
		"WLAN_READY TIMEOUT WCIR=0x%08x WASR=0x%08x WHISR=0x%08x mbox=%08x/%08x/%08x\n",
		rd(w->hif, MCR_WCIR), rd(w->hif, MCR_WASR), rd(w->hif, MCR_WHISR),
		rd(w->hif, MCR_D2HRM0R), rd(w->hif, MCR_D2HRM1R), rd(w->hif, MCR_D2HRM2R));
	{
		u32 a, b, cc;

		a = rd(w->mcu, CONN_MCU_CPUPCR); udelay(60);
		b = rd(w->mcu, CONN_MCU_CPUPCR); udelay(60);
		cc = rd(w->mcu, CONN_MCU_CPUPCR);
		dev_err(w->dev, "CPUPCR pre=0x%08x post=0x%08x/0x%08x/0x%08x (si cambia => MCU ejecutando)\n",
			pc0, a, b, cc);
	}
	dev_err(w->dev,
		"DIAG post-START: ACCESS_REG(0x6a000)=0x%08x [responde=ROM al mando, MAC NO salto; 0xdeadbeef=salto al FW (corre/crashea)]\n",
		wifi_access_reg_read(w, 0x6a000));
	/* DIAG: volcar el espacio de registros HIF (0x180F0000) y MCU (0x18070000) AP-side
	 * (funcionan aunque el command-channel esté en FW-own). Buscar el estado/poll del MAC. */
	{
		/* SOLO registros documentados y seguros (saltar FIFOs de datos 0x28-0x3c y offsets
		 * no mapeados, que cuelgan el bus al leerse). */
		static const u16 safe[] = {
			0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
			0x20, 0x24, 0x40, 0x44, 0x48, 0x50, 0x58,
		};
		int k;

		for (k = 0; k < ARRAY_SIZE(safe); k++)
			dev_err(w->dev, "HIFreg[%03x] = %08x\n", safe[k], rd(w->hif, safe[k]));
		dev_err(w->dev, "MCUreg chipid[008]=%08x cpupcr[160]=%08x\n",
			rd(w->mcu, 0x08), rd(w->mcu, 0x160));
	}
	return -ETIMEDOUT;
}

/* descarga el fichero completo: parsea cabecera 'MTKW' (seccionado) o lo trata como plano. */
static int wifi_download_firmware(struct mt6582_wifi *w)
{
	const struct firmware *fw;
	const struct firmware_divided_download *hdr;
	int ret, i;
	u32 sec0_dest = 0;

	ret = request_firmware(&fw, WIFI_FW_NAME, w->dev);
	if (ret) {
		dev_err(w->dev, "request_firmware(%s)=%d\n", WIFI_FW_NAME, ret);
		return ret;
	}
	dev_info(w->dev, "firmware %s: %zu bytes, primeros 4: %4ph\n",
		 WIFI_FW_NAME, fw->size, fw->data);

	hdr = (const void *)fw->data;
	if (fw->size > sizeof(*hdr) &&
	    le32_to_cpu(hdr->signature) == MTK_WIFI_SIGNATURE) {
		u32 n = le32_to_cpu(hdr->num_entries);

		dev_info(w->dev, "FW seccionado: %u secciones\n", n);
		for (i = 0; i < n; i++) {
			u32 foff = le32_to_cpu(hdr->section[i].offset);
			u32 flen = le32_to_cpu(hdr->section[i].length);
			u32 dest = le32_to_cpu(hdr->section[i].dest_addr);

			if (i == 0)
				sec0_dest = dest;

			if (foff + flen > fw->size) {
				dev_err(w->dev, "sección %d fuera de rango\n", i);
				ret = -EINVAL;
				goto out;
			}
			dev_info(w->dev, "  sec %d: %u bytes -> 0x%08x\n", i, flen, dest);
			ret = wifi_dl_section(w, dest, fw->data + foff, flen);
			if (ret)
				goto out;
		}
	} else {
		/*
		 * TODO(Fase 0): blob PLANO (sin cabecera 'MTKW'). Hay que averiguar la
		 * dirección destino por defecto del MT6582 (constante en wlan_lib.c, suele
		 * ser el inicio de la RAM de instrucciones del MCU WiFi). Volcar los primeros
		 * bytes del blob real para decidir. De momento abortamos con aviso.
		 */
		dev_err(w->dev, "FW sin firma 'MTKW' (blob plano): falta dirección destino; ver TODO\n");
		ret = -EINVAL;
		goto out;
	}

	/* DIAG (read-FW): antes de arrancar, leer la RAM REAL donde aterrizó sec0 (el código del
	 * MAC, dest_addr del header) vía ACCESS_REG. Si sale código estructurado → FW bien
	 * descifrado (muro = HW del MAC); si sale 0/basura/0xdeadbeef → la ROM no lee RAM o clave mala.
	 * Leo también 0x6a000 (el PC interno del MAC) por si el ACCESS_REG usa ESE espacio. */
	{
		u32 i;

		for (i = 0; i < 64; i++)
			dev_info(w->dev, "FWdump[%02x] %08x = %08x\n",
				 i, sec0_dest + i * 4, wifi_access_reg_read(w, sec0_dest + i * 4));
	}

	ret = wifi_fw_start(w);
	if (!ret)
		dev_info(w->dev, "*** firmware ARRANCADO — WLAN_READY=1 ***\n");
out:
	release_firmware(fw);
	return ret;
}

/* ======================================================================
 *  Bring-up de Fase 0 (disparado por debugfs).
 * ====================================================================== */
static int wifi_bringup(struct mt6582_wifi *w)
{
	u32 wcir;
	int ret;

	if (w->started) {
		dev_info(w->dev, "WiFi ya levantado\n");
		return 0;
	}

	dev_info(w->dev, "=== BRING-UP WiFi (func_on + HIF + firmware) ===\n");

	/* 1. encender la radio WiFi por WMT (vía btif). Deja el HIF @0x180F0000 accesible. */
	ret = mt6582_consys_func_on(WMTDRV_TYPE_WIFI);
	if (ret) {
		dev_err(w->dev, "func_on(WIFI) falló (%d) — ¿btif sin exportar la API?\n", ret);
		return ret;
	}
	dev_info(w->dev, "func_on(WIFI) OK\n");

	/*
	 * 1b. VCN33_WIFI: lo dejamos SIEMPRE ON (sin auto-apagado a los ~31s) para PRESERVAR la cal RF
	 * que hace bring_up_chip al boot con VCN33 on. NO conmutar off->on aquí: ese flanco TIRA la cal
	 * (probado: la cal RF NO es re-emitible en runtime -> da timeout). Con VCN33 ya on este enable es
	 * no-op (refcount++) y WLAN_READY afirma igual; el rail se mantiene on vía regulator-always-on.
	 */
	ret = mt6582_consys_wifi_vcn33(true);
	if (ret)
		dev_warn(w->dev, "VCN33_WIFI enable=%d (sigo de todas formas)\n", ret);
	usleep_range(2000, 3000);	/* asentar antes de tocar el HIF */

	/* 2. ganar driver-own para poder tocar los puertos de datos */
	ret = wifi_set_driver_own(w);
	if (ret)
		return ret;

	/* 3. leer chip-id del HIF — comprobante de que el HIF responde (Riesgo B) */
	wcir = rd(w->hif, MCR_WCIR);
	dev_info(w->dev, "WCIR=0x%08x (chip-id=0x%04x rev=%lu ready=%d)\n",
		 wcir, (u32)(wcir & WCIR_CHIP_ID),
		 (wcir & WCIR_REVISION_ID) >> 16, !!(wcir & WCIR_WLAN_READY));
	if ((wcir & WCIR_CHIP_ID) != WIFI_CHIP_ID_6582 &&
	    (wcir & WCIR_CHIP_ID) != WIFI_CHIP_ID_6572) {
		dev_err(w->dev, "HIF no responde el chip-id esperado — parar y revisar HIFSYS clocks\n");
		return -ENODEV;
	}

	/*
	 * nicInitializeAdapter equiv: el FW espera que el host habilite las interrupciones (WHIER)
	 * antes de arrancar, aunque nosotros sondeemos (downstream nic.c:1328 escribe WHIER_DEFAULT
	 * ANTES de la descarga). Sin esto el handshake de boot del FW no completa → WLAN_READY nunca.
	 */
	/* WHCR (nicSDIOInit, downstream nic.c:1013): MAX_HIF_RX_LEN_NUM=0 + RX_ENHANCE off.
	 * El FW espera este estado del motor de status/RX antes de arrancar su MAC. */
	{
		u32 whcr = rd(w->hif, MCR_WHCR);

		whcr &= ~WHCR_MAX_HIF_RX_LEN_NUM;	/* bits 4-7 = SDIO_MAXIMUM_RX_LEN_NUM (0) */
		whcr &= ~WHCR_RX_ENHANCE_MODE_EN;	/* bit 16 off (CFG_SDIO_RX_ENHANCE=0) */
		wr(w->hif, MCR_WHCR, whcr);
	}
	wr(w->hif, MCR_WHISR, rd(w->hif, MCR_WHISR));	/* limpiar status pendiente (W1C) */
	wr(w->hif, MCR_WHIER, WHIER_DEFAULT);

	/* nicDisableInterrupt (downstream nic.c:936, llamado en wlan_lib.c:1309 ANTES de descargar):
	 * el OEM deshabilita el INT global (modo polling) antes de la descarga; nosotros lo saltábamos. */
	wr(w->hif, MCR_WHLPCR, WHLPCR_INT_EN_CLR);

	/* 4. descargar el firmware del MAC WiFi y arrancarlo */
	ret = wifi_download_firmware(w);
	if (ret)
		return ret;

	w->started = true;

	dev_info(w->dev, "*** mt6582-wifi listo: cfg80211 (scan/connect/key) + data-path (Fases 0-3) ***\n");
	return 0;
}

static ssize_t bringup_write(struct file *f, const char __user *u, size_t n, loff_t *o)
{
	if (g_wifi)
		wifi_bringup(g_wifi);
	return n;
}
static const struct file_operations bringup_fops = {
	.write = bringup_write,
	.owner = THIS_MODULE,
};

/* auto-bring-up con REINTENTO: levanta wlan0 solo al boot. El WLAN_READY del FW es flaky cerca del
 * boot (afirma sólo cuando el sistema lleva un rato arriba), así que si falla se reintenta cada 20s
 * hasta 15 veces (~5min). Elimina los reboots y el echo manual. wifi_bringup deja started=false en
 * fallo, por lo que cada reintento re-dispara func_on + descarga del FW limpiamente. */
static void wifi_auto_bringup_work(struct work_struct *work)
{
	struct mt6582_wifi *w = container_of(work, struct mt6582_wifi, auto_bringup.work);

	if (w->started)
		return;
	w->bringup_tries++;
	wifi_bringup(w);
	if (w->started)
		dev_info(w->dev, "auto-bringup: wlan0 ARRIBA al intento %u\n", w->bringup_tries);
	else if (w->bringup_tries < 15) {
		dev_info(w->dev, "auto-bringup: WLAN_READY aún no (intento %u/15), reintento en 20s\n",
			 w->bringup_tries);
		schedule_delayed_work(&w->auto_bringup, msecs_to_jiffies(20000));
	} else {
		dev_err(w->dev, "auto-bringup: agotados 15 intentos — echo 1 > debugfs/bringup a mano\n");
	}
}

/* ======================================================================
 *  probe / remove del platform_driver.
 * ====================================================================== */
static int mt6582_wifi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_wifi *w;

	/* esperar a que el CONSYS esté vivo (igual que el btif) */
	if (!mt6582_consys_ready)
		return -EPROBE_DEFER;

	w = devm_kzalloc(dev, sizeof(*w), GFP_KERNEL);
	if (!w)
		return -ENOMEM;
	w->dev = dev;
	mutex_init(&w->hif_lock);
	dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));

	w->hif = ioremap(WIFI_HIF_PHYS, WIFI_HIF_LEN);
	w->mcu = ioremap(WIFI_MCU_PHYS, WIFI_MCU_LEN);
	w->pdma = ioremap(WIFI_PDMA_PHYS, WIFI_PDMA_LEN);
	if (!w->hif || !w->mcu || !w->pdma)
		return -ENOMEM;

	/* buffer de descarga: DMA-able (en Fase 3 el PDMA escribe desde aquí) */
	w->dlm = dma_alloc_coherent(dev, DLM_BUF_SZ, &w->dlm_phys, GFP_KERNEL);
	if (!w->dlm)
		return -ENOMEM;

	g_wifi = w;
	w->dbg = debugfs_create_dir("mt6582_wifi", NULL);
	debugfs_create_file("bringup", 0200, w->dbg, w, &bringup_fops);
	platform_set_drvdata(pdev, w);

	dev_info(dev, "mapeado HIF@0x%x PDMA@0x%x — auto-bringup en 30s (o echo 1 > debugfs/bringup)\n",
		 WIFI_HIF_PHYS, WIFI_PDMA_PHYS);

	/* auto-levantar wlan0 al boot, con reintento contra la flakiness del WLAN_READY */
	INIT_DELAYED_WORK(&w->auto_bringup, wifi_auto_bringup_work);
	schedule_delayed_work(&w->auto_bringup, msecs_to_jiffies(30000));
	return 0;
}

static void mt6582_wifi_remove(struct platform_device *pdev)	/* kernel 7.0.12: remove devuelve void */
{
	struct mt6582_wifi *w = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&w->auto_bringup);

	/* cleanup cfg80211/netdev/RX-thread (parche 07 del auditor). Solo se ejecuta en rmmod;
	 * con el driver built-in NO se llama, pero deja el .remove() correcto si se compila como modulo. */
	if (w->cfg_registered) {
		if (w->rx_thread) { kthread_stop(w->rx_thread); w->rx_thread = NULL; }
		unregister_netdev(w->ndev);
		wiphy_unregister(w->wiphy);
		wiphy_free(w->wiphy);
		w->cfg_registered = false;
	}

	if (w->started) {
		wifi_set_fw_own(w);			/* devolver el chip a FW-own */
		mt6582_consys_wifi_vcn33(false);	/* apagar el rail RF del WiFi */
	}
	debugfs_remove_recursive(w->dbg);
	if (w->dlm)
		dma_free_coherent(w->dev, DLM_BUF_SZ, w->dlm, w->dlm_phys);
	if (w->hif)
		iounmap(w->hif);
	if (w->mcu)
		iounmap(w->mcu);
	if (w->pdma)
		iounmap(w->pdma);
	g_wifi = NULL;
}

static const struct of_device_id mt6582_wifi_of_ids[] = {
	{ .compatible = "mediatek,mt6582-wifi" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_wifi_of_ids);

static struct platform_driver mt6582_wifi_driver = {
	.probe = mt6582_wifi_probe,
	.remove = mt6582_wifi_remove,
	.driver = {
		.name = "mt6582-wifi",
		.of_match_table = mt6582_wifi_of_ids,
	},
};
module_platform_driver(mt6582_wifi_driver);

MODULE_DESCRIPTION("MediaTek MT6582 CONSYS WiFi (cfg80211 full-MAC) — scaffold Fase 0");
MODULE_LICENSE("GPL");
MODULE_FIRMWARE(WIFI_FW_NAME);
