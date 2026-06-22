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
	u8			scan_seq;
	bool			cfg_registered;
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
	int ret;

	mutex_lock(&w->hif_lock);
	/* 1) MAC permanente: CMD_ID_BASIC_CONFIG (query) -> EVENT_ID_BASIC_CONFIG (MAC @body+0) */
	wifi_send_cmd(w, CMD_ID_BASIC_CONFIG, 0, NULL, 0, sizeof(bc));
	ret = wifi_poll_event(w, EVENT_ID_BASIC_CONFIG, bc, sizeof(bc), 1000);
	if (!ret)
		dev_info(w->dev, "*** Fase1: MAC permanente = %pM ***\n", bc);
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

/* drenar AMBOS puertos RX una vez: port0 (MGMT->inform_bss) + port1 (EVENT->scan_done). hif_lock. */
static void wifi_rx_drain(struct mt6582_wifi *w)
{
	u32 wrplr = rd(w->hif, MCR_WRPLR);
	u32 l0 = WRPLR_RX0_LEN(wrplr), l1 = WRPLR_RX1_LEN(wrplr);
	u8 rx[1600];

	if (l0 && ALIGN(l0, 4) <= sizeof(rx)) {
		struct hif_rx_header *h = (void *)rx;

		wifi_port_read_pio(w, rx, ALIGN(l0, 4));
		if ((le16_to_cpu(h->packet_type) & HIF_RX_PKT_TYPE_MASK) == HIF_RX_PKT_TYPE_MGMT)
			wifi_rx_mgmt(w, rx, l0);
	}
	if (l1 && ALIGN(l1, 4) <= sizeof(rx)) {
		struct wifi_event *ev = (void *)rx;

		wifi_port1_read_pio(w, rx, ALIGN(l1, 4));
		if ((le16_to_cpu(ev->packet_type) & HIF_RX_PKT_TYPE_MASK) == HIF_RX_PKT_TYPE_EVENT &&
		    ev->eid == EVENT_ID_SCAN_DONE && w->scan_req) {
			struct cfg80211_scan_info info = { .aborted = false };

			cfg80211_scan_done(w->scan_req, &info);
			w->scan_req = NULL;
		}
	}
}

/* kthread RX: sondea ~20ms; cierra el scan por timeout si el FW no manda SCAN_DONE. */
static int wifi_rx_thread(void *data)
{
	struct mt6582_wifi *w = data;

	while (!kthread_should_stop()) {
		mutex_lock(&w->hif_lock);
		wifi_rx_drain(w);
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

static struct cfg80211_ops wifi_cfg_ops = {
	.scan = wifi_cfg_scan,
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
	dev_kfree_skb(skb);		/* TX = Fase 3 */
	ndev->stats.tx_dropped++;
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
	 * 1b. encender VCN33_WIFI AHORA (flanco fresco off->on), imitando HifAhbProbe del OEM.
	 * El consys ya NO lo enciende al boot; el MAC WiFi necesita este flanco coincidente con el
	 * arranque del firmware o nunca afirma WLAN_READY (D2HRM0R=0). Es el FIX del bring-up.
	 */
	ret = mt6582_consys_wifi_vcn33(true);
	if (ret)
		dev_warn(w->dev, "VCN33_WIFI enable=%d (sigo de todas formas)\n", ret);
	else
		dev_info(w->dev, "VCN33_WIFI ON (flanco off->on para reset del MAC)\n");
	usleep_range(2000, 3000);	/* asentar el rail RF antes de tocar el HIF */

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

	/*
	 * ================= TODO Fase 1: cmd/event + wiphy + netdev =================
	 * - wifi_send_cmd(CID, set/query, payload): arma struct wifi_cmd (PKT_TYPE=CMD en
	 *   resource_pkttype_cs bits6-7) + payload, escribe WTDR0; ver nic_tx.c:nicTxCmd.
	 * - RX dispatcher (kthread o IRQ): poll WHISR.RX0_DONE -> WRPLR -> leer WRDR0 ->
	 *   struct hif_rx_header; según (packet_type & HIF_RX_PKT_TYPE_MASK):
	 *     EVENT -> despachar por struct wifi_event.eid
	 *     MGMT  -> cfg80211_inform_bss_frame (beacons del scan)
	 *     DATA  -> netif_rx (Fase 3)
	 * - CMD_ID_GET_NIC_CAPABILITY (query) -> EVENT_ID_NIC_CAPABILITY (lee MAC permanente).
	 * - CMD_ID_BASIC_CONFIG (fija MAC), CMD_ID_SET_DOMAIN_INFO (canales 1-13).
	 * - wiphy_new(&mt6582_wifi_cfg80211_ops, ...); banda 2.4G (14 canales); cipher_suites
	 *   {WEP40,WEP104,TKIP,CCMP,AES_CMAC}; iftype STATION; wiphy_register().
	 * - alloc_netdev/register_netdev "wlan0" + wireless_dev.
	 * Ficheros: gl_cfg80211.c, gl_init.c:737-897/2259-2300, nic_cmd_event.h.
	 *
	 * ================= TODO Fase 2: cfg80211 ops connect/key =================
	 * - .scan    -> CMD_ID_SCAN_REQ_V2 (struct CMD_SCAN_REQ_V2) -> SCAN_DONE.
	 * - .connect -> CMD_ID_INFRASTRUCTURE + CMD_ID_SET_BSS_INFO + activar STA record;
	 *               coreografía: estudiar mgmt/ais_fsm.c:aisFsmRunEventJoinRequest.
	 * - .add_key/.del_key/.set_default_key -> CMD_ID_ADD_REMOVE_KEY (CMD_802_11_KEY).
	 * - eventos CONNECTION_STATUS/ASSOC_INFO -> cfg80211_connect_result/roamed/disconnected.
	 *
	 * ================= TODO Fase 3: data path netdev =================
	 * - ndo_start_xmit: prepend HIF_TX_HEADER_T (PKT_TYPE=DATA) -> WTDR0 (PDMA).
	 * - PDMA real (0x11000180) para TX y RX; IRQ WF_HIF_IRQ (WHISR) + tasklet/NAPI.
	 * - control de flujo TX por TC (WTSR0 / EVENT_ID_TX_DONE). Ver nic_tx.c/ahb_pdma.c.
	 * ===========================================================================
	 */

	dev_info(w->dev, "*** Fase 0 COMPLETA. cfg80211/netdev = TODO (Fases 1-3) ***\n");
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

	dev_info(dev, "mapeado HIF@0x%x PDMA@0x%x — echo 1 > /sys/kernel/debug/mt6582_wifi/bringup\n",
		 WIFI_HIF_PHYS, WIFI_PDMA_PHYS);
	dev_info(dev, "(Fase 0: func_on(WIFI) + descarga de WIFI_RAM_CODE. cfg80211 = TODO.)\n");
	return 0;
}

static void mt6582_wifi_remove(struct platform_device *pdev)	/* kernel 7.0.12: remove devuelve void */
{
	struct mt6582_wifi *w = platform_get_drvdata(pdev);

	/* TODO(Fase 1+): unregister_netdev / wiphy_unregister / parar RX-thread o IRQ. */
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
