// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-btif.c — BTIF (DMA) + CONSYS STP/WMT bring-up + Bluetooth HCI (hci0).
 *
 * Historia: it.2 el CONSYS contesta (BTIF-DMA + FLUSH de cola); it.3 descarga del patch;
 * it.4 las 4 radios encendidas (func_on); it.5 el BT responde HCI; **it.6 registra un hci0 real**
 * para que BlueZ/Phosh lo usen (escanear/vincular/visible).
 *
 * Arquitectura BT: el bring-up (patch + func_on(BT)) se dispara por debugfs (rootfs+firmware listos),
 * que ADEMÁS registra hci0. TX: skb HCI -> [H4 type][datos] -> STP-BT (type=0) -> BTIF-DMA. RX: un
 * kthread lee STP-BT -> hci_recv_frame(hdev). Índices STP: BT=0 FM=1 GPS=2 WIFI=3 WMT=4.
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
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/skbuff.h>
#include <linux/kfifo.h>
#include <linux/crc16.h>	/* CRC-16/ARC (poly 0xA001) para STP FULL mode del ROMv1 */
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/completion.h>
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

#define BTIF_PHYS	0x1100c000
#define BTIF_LSR	0x14
#define BTIF_FAKELCR	0x0c
#define BTIF_DMA_EN	0x4c
#define BTIF_TRI_LVL	0x60
#define BTIF_HANDSHAKE	0x6c
#define DMA_EN_RX	(1u << 0)
#define DMA_EN_TX	(1u << 1)
#define DMA_EN_AUTORST	(1u << 2)
#define HANDSHAKE_EN	(1u << 0)
#define TRI_LVL_VAL	(1u | (1u << 4))

#define APDMA_TX_PHYS	0x11000780
#define APDMA_RX_PHYS	0x11000800
#define APDMA_SIZE	0x80
#define VFF_INT_FLAG	0x00
#define VFF_INT_EN	0x04
#define VFF_EN		0x08
#define VFF_RST		0x0c
#define VFF_STOP	0x10
#define VFF_FLUSH	0x14
#define VFF_ADDR	0x1c
#define VFF_LEN		0x24
#define VFF_THRE	0x28
#define VFF_WPT		0x2c
#define VFF_RPT		0x30
#define VFF_VALID	0x3c
#define VFF_LEFT	0x40
#define DMA_WPT_MASK	0x0000ffff
#define DMA_WPT_WRAP	0x00010000

#define TX_RING		0x2000
#define RX_RING		0x2000

#define PERICFG_PHYS	0x10003000
#define PERI_PDN0_CLR	0x10
#define PERI_CG_BTIF	(1u << 20)

#define STP_TYPE_BT	0
#define STP_TYPE_FM	1
#define STP_TYPE_GPS	2
#define STP_TYPE_WMT	4
#define GPS_FIFO_SZ	16384
#define FM_FIFO_SZ	8192
#define RXBUF_SZ	2048
#define MAX_STP_PAYLOAD	2048	/* limite de length plausible en un header STP */
#define TXPKT_SZ	1100
#define WMTBUF_SZ	1010

#define PATCH_HDR	28
#define FRAG_SIZE	1000
#define FRAG_1ST	0x1
#define FRAG_MID	0x2
#define FRAG_LAST	0x3

extern bool mt6582_consys_ready;
extern int mt6582_consys_hw_rst(void);	/* mt6582-consys.c: MTCMOS+rails off->on */
extern u32 mt6582_consys_cpupcr(void);	/* PC del MCU (diagnostico salto al fw) */
extern int mt6582_consys_bt_vcn33(bool on);	/* VCN33 modo-HW BT (0x416[5], para RF-cal ROMv1) */
extern int mt6582_consys_wifi_vcn33(bool on);	/* VCN33 modo-HW WiFi (0x418[14], para RF-cal ROMv1) */
int mt6582_consys_func_on(u8 type);	/* exportadas para el driver WiFi (mt6582-wifi.c) */
int mt6582_consys_func_off(u8 type);

struct mt6582_btif {
	struct device *dev;
	void __iomem *base, *txdma, *rxdma;
	void *tx_ring, *rx_ring;
	dma_addr_t tx_phys, rx_phys;
	u32 tx_off, tx_wrap, rx_off, rx_wrap;
	u8 rxbuf[RXBUF_SZ];
	u32 rxlen;
	u32 resync_drops;	/* bytes descartados buscando un header STP valido */
	/* STP FULL mode (ROMv1): tras el SET_STP el chip exige framing con seq/ack +
	 * CRC-16/ARC (el mt6572_82 toleraba el modo simple; el ROMv1 NO). stp_full=false
	 * = modo simple (dummy CRC) hasta el SET_STP del bring-up. Estado de secuencia
	 * segun stp_core.c (init/reset): txseq=0, txack=7, exp_rxseq=0. */
	bool stp_full;
	u8 txseq;	/* mi secuencia TX (0..7), INDEX_INC tras cada frame de datos */
	u8 txack;	/* ack que embebo = ultimo rxseq en-orden recibido del chip */
	u8 exp_rxseq;	/* siguiente rxseq esperado del chip (diagnostico) */
	u8 txpkt[TXPKT_SZ];
	u8 wmtbuf[WMTBUF_SZ];
	struct dentry *dbg;
	bool brought_up;
	struct hci_dev *hdev;
	struct task_struct *rx_task;
	struct mutex tx_lock;
	int dbg_n;
	/* GPS: canal STP 2 -> /dev/stpgps (passthrough crudo, igual que el downstream). */
	struct kfifo gps_fifo;
	wait_queue_head_t gps_wq;
	struct mutex gps_rd_lock;
	u32 gps_rx, gps_rd_calls, gps_rd_data, gps_rd_eintr, gps_rd_block;	/* DEBUG gps_read */
	u8 gps_tx[1024];
	/* FM: canal STP 1. El driver stock mtk_fm_drv registra un callback de RX
	 * (mtk_wcn_stp_register_event_cb) que invocamos cuando llega un frame FM.
	 * TX va por stp_send(STP_TYPE_FM). El driver FM gestiona su propio /dev/fm. */
	void (*fm_rx_cb)(const u8 *data, u32 len);
	struct mutex fm_cb_lock;
	u8 fm_tx[1024];
	/* WMT runtime (canal 4): API exportada func_on/off para otras radios (p.ej. WiFi).
	 * El kthread RX rutea el EVT WMT aquí vía completion (no compite por rxbuf). */
	struct completion wmt_done;
	struct mutex wmt_lock;
	struct mutex bringup_lock;	/* serializa el bring-up: BT/GPS/WiFi no compiten -> sin doble */
	u8 wmt_rx[64];
	u32 wmt_rxlen;
};

static struct mt6582_btif *g_btif;

static const u8 GEN_HCR[20] = {
	0x01,0x08,0x10,0x00,0x02,0x01,0x00,0x01,0x08,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00 };
static const u8 PATCH_ADDR_CMD[20] = {
	0x01,0x08,0x10,0x00,0x01,0x01,0x00,0x01,0x3c,0x02,0x09,0x02,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff };
static const u8 PATCH_ADDR_EVT[8] = { 0x02,0x08,0x04,0x00,0x00,0x00,0x00,0x01 };
static const u8 PATCH_PADDR_CMD[20] = {
	0x01,0x08,0x10,0x00,0x01,0x01,0x00,0x01,0xc4,0x04,0x09,0x02,0x00,0x3f,0x00,0x01,0xff,0xff,0xff,0xff };
static const u8 PATCH_PADDR_EVT[8] = { 0x02,0x08,0x04,0x00,0x00,0x00,0x00,0x01 };
static const u8 PATCH_EVT[5] = { 0x02,0x01,0x01,0x00,0x00 };
static const u8 WMT_RESET_CMD[5] = { 0x01,0x07,0x01,0x00,0x04 };
static const u8 WMT_RESET_EVT[5] = { 0x02,0x07,0x01,0x00,0x00 };

static inline u32 rd(void __iomem *b, u32 o) { return readl(b + o); }
static inline void wr(void __iomem *b, u32 o, u32 v) { writel(v, b + o); }

static void vff_setup(void __iomem *dma, dma_addr_t ring, u32 size, u32 thre)
{
	wr(dma, VFF_RST, 0x1); udelay(10); wr(dma, VFF_RST, 0x0);
	wr(dma, VFF_STOP, 0x0);
	wr(dma, VFF_ADDR, (u32)ring);
	wr(dma, VFF_LEN, size);
	wr(dma, VFF_THRE, thre);
	wr(dma, VFF_WPT, 0); wr(dma, VFF_RPT, 0);
	wr(dma, VFF_INT_FLAG, 0x3); wr(dma, VFF_INT_EN, 0x0);
	wr(dma, VFF_EN, 0x1);
}

static void btif_hw_init(struct mt6582_btif *b)
{
	void __iomem *p = ioremap(PERICFG_PHYS, 0x100);

	if (p) { writel(PERI_CG_BTIF, p + PERI_PDN0_CLR); iounmap(p); }
	wr(b->base, BTIF_FAKELCR, 0);
	wr(b->base, BTIF_HANDSHAKE, rd(b->base, BTIF_HANDSHAKE) | HANDSHAKE_EN);
	wr(b->base, BTIF_TRI_LVL, TRI_LVL_VAL);
	wr(b->base, BTIF_DMA_EN, DMA_EN_TX | DMA_EN_RX | DMA_EN_AUTORST);
	vff_setup(b->txdma, b->tx_phys, TX_RING, TX_RING - 7);
	vff_setup(b->rxdma, b->rx_phys, RX_RING, 1);
	b->tx_off = b->tx_wrap = b->rx_off = b->rx_wrap = 0;
	b->rxlen = 0;
}

static void btif_tx_raw(struct mt6582_btif *b, const u8 *buf, u32 len)
{
	u32 off = b->tx_off, first = min_t(u32, len, TX_RING - off);
	int t;

	memcpy((u8 *)b->tx_ring + off, buf, first);
	if (len > first)
		memcpy(b->tx_ring, buf + first, len - first);
	dma_wmb();
	if (off + len >= TX_RING) { b->tx_off = off + len - TX_RING; b->tx_wrap ^= DMA_WPT_WRAP; }
	else b->tx_off = off + len;
	wr(b->txdma, VFF_EN, 0x1);
	wr(b->txdma, VFF_WPT, b->tx_off | b->tx_wrap);
	for (t = 0; t < 200000; t++) {
		u32 v = rd(b->txdma, VFF_VALID) & 0xffff;

		if (v == 0) break;
		if (v < 8) wr(b->txdma, VFF_FLUSH, 0x1);
		udelay(2);
	}
}

/* Envia un frame STP. En modo simple (stp_full=false, arranque) el header es
 * 0x80 + trailer dummy 00 00. En FULL mode (tras SET_STP) el header lleva seq/ack +
 * checksum de header y el trailer es el CRC-16/ARC real sobre el payload — replica
 * exacta de stp_send_data_no_ps() rama uart_fullset de stp_core.c. */
static void stp_send(struct mt6582_btif *b, u8 type, const u8 *pl, u32 len)
{
	u32 n;

	if (len + 6 > TXPKT_SZ) return;
	if (b->stp_full) {
		u16 crc;

		b->txpkt[0] = 0x80 + (b->txseq << 3) + b->txack;
		b->txpkt[1] = (type << 4) | ((len >> 8) & 0x0f);
		b->txpkt[2] = len & 0xff;
		b->txpkt[3] = (b->txpkt[0] + b->txpkt[1] + b->txpkt[2]) & 0xff;
		memcpy(b->txpkt + 4, pl, len);
		n = 4 + len;
		crc = crc16(0, pl, len);		/* CRC-16/ARC sobre el payload */
		b->txpkt[n++] = crc & 0xff;
		b->txpkt[n++] = (crc >> 8) & 0xff;
		b->txseq = (b->txseq + 1) & 0x7;	/* INDEX_INC(txseq) */
	} else {
		b->txpkt[0] = 0x80;
		b->txpkt[1] = (type << 4) | ((len >> 8) & 0x0f);
		b->txpkt[2] = len & 0xff;
		b->txpkt[3] = 0x00;
		memcpy(b->txpkt + 4, pl, len);
		n = 4 + len;
		b->txpkt[n++] = 0x00; b->txpkt[n++] = 0x00;
	}
	btif_tx_raw(b, b->txpkt, n);
}

/* ACK suelto de FULL mode (stp_core.c stp_send_ack): header de 4 bytes con seq=0,
 * length=0, sin payload ni CRC. Se manda tras recibir cada frame de DATOS del chip
 * para que su tx-timer (180ms) no retransmita. No incrementa txseq. */
static void stp_send_ack(struct mt6582_btif *b)
{
	u8 hdr[4];

	hdr[0] = 0x80 + (0 << 3) + b->txack;
	hdr[1] = 0x00;
	hdr[2] = 0x00;
	hdr[3] = (hdr[0] + hdr[1] + hdr[2]) & 0xff;
	/* En runtime el kthread RX llama a stp_pop_frame->stp_send_ack sin tx_lock, y
	 * podria competir con un stp_send de TX (GPS/FM/BT). Serializo el btif_tx_raw.
	 * En bring-up el lock esta libre (single-thread), sin deadlock (stp_pop_frame
	 * nunca se llama con tx_lock tomado). */
	mutex_lock(&b->tx_lock);
	btif_tx_raw(b, hdr, 4);
	mutex_unlock(&b->tx_lock);
}

static void rx_drain(struct mt6582_btif *b)
{
	u32 wpt = rd(b->rxdma, VFF_WPT), wo = wpt & DMA_WPT_MASK, ww = wpt & DMA_WPT_WRAP;
	u32 valid, i, o;

	valid = (ww == b->rx_wrap) ? (wo - b->rx_off) : (RX_RING - b->rx_off + wo);
	if (!valid || valid > RX_RING) return;
	for (i = 0; i < valid; i++) {
		o = (b->rx_off + i) % RX_RING;
		if (b->rxlen < RXBUF_SZ)
			b->rxbuf[b->rxlen++] = ((u8 *)b->rx_ring)[o];
	}
	b->rx_off = wo; b->rx_wrap = ww;
	wr(b->rxdma, VFF_RPT, wo | ww);
}

/* extrae una trama STP completa de rxbuf (no bloqueante). Devuelve len payload o 0; *type = canal. */
static int stp_pop_frame(struct mt6582_btif *b, u8 *out, u32 max, u8 *type)
{
	u32 len, frame, n;

	/* RESYNC-RX: descartar bytes hasta un header STP plausible. Sin esto un
	 * byte espurio desincroniza el parser PARA SIEMPRE (leia el header a
	 * ciegas). (1) 0x7f = byte de resync del FW (stp_core.c:2137, manda
	 * 4x0x7f seguidos para forzarlo); (2) byte0 lleva sync-bit 0x80 y
	 * byte1[6:4] = canal valido 0..4. En el caso feliz (trafico normal) el
	 * primer byte ya es plausible y esto no descarta nada. */
	for (;;) {
		while (b->rxlen >= 4) {
			u8 h0 = b->rxbuf[0];
			u8 chan = (b->rxbuf[1] >> 4) & 0x07;
			u32 l = ((b->rxbuf[1] & 0x0f) << 8) | b->rxbuf[2];

			/* En FULL mode el byte0 = 0x80+(seq<<3)+ack (sigue con sync-bit
			 * 0x80) y el byte3 = checksum de header (h0+h1+h2)&0xff -> lo
			 * verificamos para un resync mucho mas robusto. */
			if (h0 != 0x7f && (h0 & 0x80) && chan <= 4 && l <= MAX_STP_PAYLOAD &&
			    (!b->stp_full ||
			     b->rxbuf[3] == ((b->rxbuf[0] + b->rxbuf[1] + b->rxbuf[2]) & 0xff)))
				break;			/* header plausible */
			b->resync_drops++;
			memmove(b->rxbuf, b->rxbuf + 1, --b->rxlen);
		}
		if (b->rxlen < 4) return 0;
		len = ((b->rxbuf[1] & 0x0f) << 8) | b->rxbuf[2];

		/* FULL mode: un frame con length=0 es un ACK suelto del chip (4 bytes,
		 * sin payload ni CRC). Se consume y se reintenta con el siguiente frame. */
		if (b->stp_full && len == 0) {
			memmove(b->rxbuf, b->rxbuf + 4, b->rxlen - 4);
			b->rxlen -= 4;
			continue;
		}

		frame = 4 + len + 2;			/* header + payload + CRC */
		if (b->rxlen < frame) return 0;
		*type = (b->rxbuf[1] >> 4) & 0x0f;

		/* FULL mode: seguir la secuencia del chip -> ack de este seq (embebido en
		 * mi proximo frame de datos) + ACK suelto ya para que no retransmita. */
		if (b->stp_full) {
			u8 seq = (b->rxbuf[0] & 0x38) >> 3;

			b->txack = seq;
			b->exp_rxseq = (seq + 1) & 0x7;
		}
		n = min_t(u32, len, max);
		memcpy(out, b->rxbuf + 4, n);
		memmove(b->rxbuf, b->rxbuf + frame, b->rxlen - frame);
		b->rxlen -= frame;
		if (b->stp_full)
			stp_send_ack(b);
		return n;
	}
}

/* bloqueante: espera una trama y compara con el EVT esperado (para el bring-up síncrono). */
static int wmt_wait_frame(struct mt6582_btif *b, u8 *out, u32 max, u32 ms)
{
	int loops = ms * 10, plen;
	u8 type;

	while (loops-- > 0) {
		rx_drain(b);
		plen = stp_pop_frame(b, out, max, &type);
		if (plen > 0) return plen;
		udelay(100);
	}
	return -ETIMEDOUT;
}

static int wmt_cmd_to(struct mt6582_btif *b, const u8 *cmd, u32 clen,
		      const u8 *evt, u32 elen, const char *name, u32 ms)
{
	u8 rx[64];
	int plen;

	b->rxlen = 0;
	stp_send(b, STP_TYPE_WMT, cmd, clen);
	plen = wmt_wait_frame(b, rx, sizeof(rx), ms);
	if (plen < 0) {
		/* diagnostico: que hay en el rxbuf sin parsear (¿llego ALGO del chip?) */
		dev_err(b->dev, "wmt_cmd[%s]: TIMEOUT (rxlen=%u drops=%u raw=%*ph)\n",
			name, b->rxlen, b->resync_drops,
			min_t(int, b->rxlen, 16), b->rxbuf);
		return plen;
	}
	if ((u32)plen < elen || memcmp(rx, evt, elen) != 0) {
		dev_err(b->dev, "wmt_cmd[%s]: EVT no coincide (plen=%d): %*ph\n",
			name, plen, min_t(int, plen, 16), rx);
		return -EBADMSG;
	}
	return 0;
}

/* El ROM pausa los ACKs ~430-500ms durante la descarga del patch (flow-control interno,
 * medido en la captura canonica de LineageOS 0716) -> el timeout debe superarlas. */
static int wmt_cmd(struct mt6582_btif *b, const u8 *cmd, u32 clen,
		   const u8 *evt, u32 elen, const char *name)
{
	return wmt_cmd_to(b, cmd, clen, evt, elen, name, 2000);
}

static int patch_dwn(struct mt6582_btif *b, const char *fwname, const u8 addr[4])
{
	const struct firmware *fw;
	u8 paddr[20];
	const u8 *body;
	u32 blen, fragnum, seq, off;
	int ret;

	ret = request_firmware(&fw, fwname, b->dev);
	if (ret) { dev_err(b->dev, "request_firmware(%s)=%d\n", fwname, ret); return ret; }
	if (fw->size <= PATCH_HDR) { release_firmware(fw); return -EINVAL; }
	body = fw->data + PATCH_HDR;
	blen = fw->size - PATCH_HDR;
	fragnum = (blen + FRAG_SIZE - 1) / FRAG_SIZE;

	ret = wmt_cmd(b, PATCH_ADDR_CMD, 20, PATCH_ADDR_EVT, 8, "ADDR");
	if (ret) goto out;
	memcpy(paddr, PATCH_PADDR_CMD, 20);
	memcpy(paddr + 12, addr, 4);
	ret = wmt_cmd(b, paddr, 20, PATCH_PADDR_EVT, 8, "PADDR");
	if (ret) goto out;
	for (seq = 0, off = 0; seq < fragnum; seq++, off += FRAG_SIZE) {
		u32 fsz = min_t(u32, FRAG_SIZE, blen - off);
		u16 wlen = 1 + fsz;

		if (seq)	/* pacing: el stock efectivo va a ~43ms/frag; no saturar el ROM */
			usleep_range(3000, 5000);

		b->wmtbuf[0] = 0x01; b->wmtbuf[1] = 0x01;
		b->wmtbuf[2] = wlen & 0xff; b->wmtbuf[3] = (wlen >> 8) & 0xff;
		b->wmtbuf[4] = (seq == fragnum - 1) ? FRAG_LAST : (seq == 0 ? FRAG_1ST : FRAG_MID);
		memcpy(b->wmtbuf + 5, body + off, fsz);
		/* el ULTIMO frag dispara la validacion del patch entero en el ROM: con
		 * nuestra descarga a toda velocidad (sin las pausas del stock) el ROM
		 * hace todo el trabajo al final y tarda >2s en ACKear */
		ret = wmt_cmd_to(b, b->wmtbuf, 5 + fsz, PATCH_EVT, 5, "FRAG",
				 (seq == fragnum - 1) ? 12000 : 2000);
		if (ret) {
			dev_err(b->dev, "patch %s: fallo en frag %u/%u\n",
				fwname, seq + 1, fragnum);
			goto out;
		}
	}
	dev_info(b->dev, "*** patch %s DESCARGADO OK (%u frags) ***\n", fwname, fragnum);
out:
	release_firmware(fw);
	return ret;
}

static int func_on(struct mt6582_btif *b, u8 type, const char *name)
{
	u8 cmd[6] = { 0x01, 0x06, 0x02, 0x00, type, 0x01 };
	u8 rx[16];
	int plen;

	b->rxlen = 0;
	stp_send(b, STP_TYPE_WMT, cmd, 6);
	plen = wmt_wait_frame(b, rx, sizeof(rx), 2000);
	if (plen >= 5 && rx[0] == 0x02 && rx[1] == 0x06 && rx[4] == 0) {
		dev_info(b->dev, "func_on[%s]: *** RADIO ENCENDIDO ***\n", name);
		return 0;
	}
	dev_warn(b->dev, "func_on[%s]: no encendió (plen=%d)\n", name, plen);
	return -EIO;
}

/* bring-up del chip: patch + reset + func_on(BT). Síncrono (sin kthread aún). */
static int bring_up_chip(struct mt6582_btif *b)
{
	/* Stream canónico CAPTURADO en LineageOS (GPS 13 sats, mismo HW, 0716) — RE-LEÍDO
	 * byte a byte (0716 casa): es **MULTI-PATCH**, no single. Address command 0e f0,
	 * luego dos patches: **patch_1_1 (21460B/22 frags) @ 0e f0 PRIMERO**, luego
	 * **patch_1_0 (80784B/81 frags) @ 06 00**, con RESET (init_table_3) tras cada uno.
	 * El #301 single-patch (solo patch_1_0 @ 0e f0) moría en frag 81/81 porque al ROM
	 * le FALTABA patch_1_1 (y el address de patch_1_0 estaba mal: 0e f0 en vez de 06 00).
	 * 0e f0 es de patch_1_1; el Mac lo confundió. Es la MISMA estructura que el mt6572_82
	 * (pequeño@0ef0 → RESET → grande@0600 → RESET), solo cambian los ficheros a ROMv1. */
	static const u8 a_p11[4] = { 0x00, 0x00, 0x0e, 0xf0 };	/* patch_1_1 -> 0e f0 */
	static const u8 a_p10[4] = { 0x00, 0x00, 0x06, 0x00 };	/* patch_1_0 -> 06 00 */
	int ret;

	dev_info(b->dev, "=== BRING-UP del CONSYS (HW-RST + ROMv1 MULTI-patch: 1_1@0ef0 + 1_0@0600) ===\n");
	/* HW-RST del CONSYS justo antes de descargar (réplica del stock: el patch es el
	 * primer tráfico WMT que ve el ROM tras el reset). */
	ret = mt6582_consys_hw_rst();
	if (ret) return ret;
	btif_hw_init(b);
	msleep(20);

	/* estado STP al reset: arranca en modo SIMPLE; el SET_STP de abajo conmuta a FULL. */
	b->stp_full = false;
	b->txseq = 0; b->txack = 7; b->exp_rxseq = 0;

	/* BTIF sw_init del stock (wmt_ic_soc.c: rama WMT_HIF_BTIF, que el mt6582-btif
	 * simplificado SE SALTABA): query STP default (init_table_1_2) + SET_STP a
	 * DF 0E 68 01 (init_table_4) + query STP (init_table_5). El stock del krillin usa
	 * UART (no lo hace), pero por BTIF el sw_init SÍ lo hace, y el ROMv1 (a diferencia
	 * del mt6572_82) parece necesitar este SET_STP del chip para no quedar mudo tras
	 * el reset. (init_table_1_2/5 son query TEST-ONLY; init_table_4 es el que importa.) */
	wmt_cmd(b, (const u8[]){0x01,0x04,0x01,0x00,0x04}, 5,
		(const u8[]){0x02,0x04}, 2, "QUERY-STP");
	/* SET_STP (init_table_4): se ENVIA en modo simple; al recibirlo el chip conmuta a
	 * STP FULL. El stock NO espera respuesta -> manda set_stp, reconfigura el STP host a
	 * full (WMT_CTRL_STP_CONF/MTKSTP_BTIF_FULL_MODE) y duerme 10ms. Replico eso:
	 * stp_send simple, flip a full, reset de seq, sleep y descarto la respuesta. */
	stp_send(b, STP_TYPE_WMT,
		 (const u8[]){0x01,0x04,0x05,0x00,0x03,0xDF,0x0E,0x68,0x01}, 9);
	b->stp_full = true;
	b->txseq = 0; b->txack = 7; b->exp_rxseq = 0;
	msleep(10);
	b->rxlen = 0;		/* flush: la respuesta al set_stp no se usa (como el stock) */
	dev_info(b->dev, "*** STP FULL MODE activado (seq/ack + CRC-16/ARC) ***\n");
	/* QUERY_STP (init_table_5): 1er request/response en FULL -> sincroniza la secuencia
	 * y confirma que el chip acepta el framing full. */
	if (wmt_cmd(b, (const u8[]){0x01,0x04,0x01,0x00,0x04}, 5,
		    (const u8[]){0x02,0x04}, 2, "QUERY-STP2"))
		dev_warn(b->dev, "QUERY-STP2 (full) fallo — el chip no acepta full mode?\n");
	else
		dev_info(b->dev, "*** QUERY-STP2 OK en FULL MODE (chip habla full) ***\n");

	/* ⚡ POWER-ON de las DLM (la RAM del MCU) — LA PIEZA QUE FALTABA (0717).
	 * 3 SET_REG al reg chip-side 0x80100060, valor 0, mascaras [11:8],[7:4],[3]:
	 * limpian los bits de power-down por bancos. El kernel 3.10 de LineageOS los
	 * manda tras el query STP y ANTES del patch ("power on dlm cmd1/2/3", bytes
	 * extraidos de su vmlinux); el 3.4 de bq-src del que portamos NO los tenia.
	 * Sin esto la RAM 0x60000+ esta APAGADA: el ROM ACKea los frags pero las
	 * escrituras se pierden, y el fw salta a memoria vacia (PC sweep lineal desde
	 * ~0x60000+, visto con CPUPCR). Verificado en Lineage vivo via wmt_dbg:
	 * 0x80100060=0x00000000 y patch_1_0 presente en 0x60000 sin hueco. */
	{
		static const u8 dlm1[20] = { 0x01,0x08,0x10,0x00,0x01,0x01,0x00,0x01,
			0x60,0x00,0x10,0x80,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00 };
		static const u8 dlm2[20] = { 0x01,0x08,0x10,0x00,0x01,0x01,0x00,0x01,
			0x60,0x00,0x10,0x80,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x00 };
		static const u8 dlm3[20] = { 0x01,0x08,0x10,0x00,0x01,0x01,0x00,0x01,
			0x60,0x00,0x10,0x80,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00 };
		static const u8 dlm_evt[8] = { 0x02,0x08,0x04,0x00,0x00,0x00,0x00,0x01 };

		if (wmt_cmd(b, dlm1, 20, dlm_evt, 8, "DLM-ON-1") ||
		    wmt_cmd(b, dlm2, 20, dlm_evt, 8, "DLM-ON-2") ||
		    wmt_cmd(b, dlm3, 20, dlm_evt, 8, "DLM-ON-3"))
			dev_warn(b->dev, "power-on DLM fallo (sigo)\n");
		else
			dev_info(b->dev, "*** DLM (RAM del MCU) ENCENDIDA (3x SET_REG 0x80100060) ***\n");
	}

	/* Multi-patch (source wmt_ic_soc.c sw_init loop 6.3): patch_dwn + init_table_3
	 * (WMT_RESET) TRAS CADA patch.
	 *
	 * ⚠️ CLAVE (stream ground-truth 0716 + bisecciones 0717): el chip es ESTRICTO con
	 * la secuencia — un frame fuera de orden se DESCARTA en silencio (probado: RESET
	 * con seq=0 forzado = rxlen=0, ni EVT ni reboot). Los WMT_RESET van EN SECUENCIA.
	 * RESET-1 (patch a medias) NO rebota el chip: la numeración CONTINÚA (probado:
	 * patch_1_0 descarga con seq continuo tras él). RESET-2 (todos los patches) SÍ
	 * rebota al fw parcheado, que arranca con contadores FRESCOS (captura: EVT del
	 * reset seq=0/ack=0 `0x80`, luego RF-cal ack 0x81, coex EVT 0x92, FM 0x9b) → hay
	 * que resincronizar el host a 0 DESPUÉS del EVT del RESET-2, no antes de enviarlo
	 * (si no, el fw espera seq=0, recibe seq altos y DESCARTA TODO → RF-CAL -110). */
	ret = patch_dwn(b, "ROMv1_patch_1_1_hdr.bin", a_p11);	/* patch_1_1 PRIMERO @ 0e f0 */
	if (ret) return ret;
	wmt_cmd(b, WMT_RESET_CMD, 5, WMT_RESET_EVT, 5, "RESET-1");
	ret = patch_dwn(b, "ROMv1_patch_1_0_hdr.bin", a_p10);	/* patch_1_0 SEGUNDO @ 06 00 */
	if (ret) return ret;
	if (!wmt_cmd(b, WMT_RESET_CMD, 5, WMT_RESET_EVT, 5, "RESET-2")) {
		/* El fw manda el INBAND RESET `80 00 00 80` (stp_core.c:2234, "Resync STP
		 * with firmware!") y luego su EVT con seq=0 — nuestro parser ya salta el
		 * inband (len=0) y el pop del EVT dejó txack=0. Solo falta resetear el
		 * txseq (stock stp_rest_ctx_state + EVT => txseq=0, txack=0): el proximo
		 * comando (RF-CAL) sale con header 0x80 como en la captura. */
		b->txseq = 0; b->exp_rxseq = 1;
		/* DIAGNOSTICO: trayectoria del PC del MCU justo tras el reset (¿donde
		 * arranca el fw y donde descarrila?). 3 rafagas: ya / +1ms / +50ms. */
		{
			u32 pc[24];
			int k;

			for (k = 0; k < 8; k++) { pc[k] = mt6582_consys_cpupcr(); udelay(20); }
			usleep_range(1000, 1200);
			for (; k < 16; k++) { pc[k] = mt6582_consys_cpupcr(); udelay(20); }
			msleep(50);
			for (; k < 24; k++) { pc[k] = mt6582_consys_cpupcr(); udelay(20); }
			dev_info(b->dev, "PC+0us:  %08x %08x %08x %08x %08x %08x %08x %08x\n",
				 pc[0], pc[1], pc[2], pc[3], pc[4], pc[5], pc[6], pc[7]);
			dev_info(b->dev, "PC+1ms:  %08x %08x %08x %08x %08x %08x %08x %08x\n",
				 pc[8], pc[9], pc[10], pc[11], pc[12], pc[13], pc[14], pc[15]);
			dev_info(b->dev, "PC+50ms: %08x %08x %08x %08x %08x %08x %08x %08x\n",
				 pc[16], pc[17], pc[18], pc[19], pc[20], pc[21], pc[22], pc[23]);
		}
		dev_info(b->dev, "*** RESET-2 OK -> STP resincronizado (fw parcheado, seq 0) ***\n");
	}

	/* DIAGNOSTICO decisivo: ¿está el fw VIVO tras el reset? QUERY_STP es inocuo
	 * (no toca RF/PALDO). Si responde -> fw arriba y el bloqueo esta en RF-CAL
	 * (alimentacion A-die/PALDO). Si calla -> el fw no arranco. */
	if (!wmt_cmd(b, (const u8[]){0x01,0x04,0x01,0x00,0x04}, 5,
		     (const u8[]){0x02,0x04}, 2, "QUERY-STP3-postreset"))
		dev_info(b->dev, "*** FW VIVO tras reset (QUERY-STP3 OK) ***\n");

	/* CALIBRACIÓN RF del CONSYS (OEM mtk_wcn_soc_sw_init, wmt_ic_soc.c:967, TRAS los parches
	 * y ANTES de func_on). Sin esto el PHY/PLL/AFE del MAC WiFi NO se calibra → su FW arranca
	 * pero WLAN_READY nunca se afirma (BT/GPS la toleran, por eso enlazan igual). VCN33 ya está
	 * on (boot script zz-consys-bt). La cal RF tarda más que un cmd normal → timeout amplio. */
	{
		static const u8 rfcal[5] = { 0x01, 0x14, 0x01, 0x00, 0x01 };
		u8 rx[16];
		int plen;

		/* PALDO on: el stock (captura sw_init) enciende VCN33 en modo-HW por el path
		 * de BT (0x416[5]) y de WiFi (0x418[14]) JUSTO antes de la RF-cal, y los apaga
		 * justo después. El ROMv1 lo NECESITA: sin el PALDO su RF-cal cuelga el chip
		 * (RF-CAL -110 y todo mudo). VCN33 ya está on; esto solo conmuta a modo-HW. */
		{
			int rb = mt6582_consys_bt_vcn33(true);
			int rw = mt6582_consys_wifi_vcn33(true);

			dev_info(b->dev, "PALDO on: bt=%d wifi=%d\n", rb, rw);
		}
		b->rxlen = 0;
		stp_send(b, STP_TYPE_WMT, rfcal, 5);
		plen = wmt_wait_frame(b, rx, sizeof(rx), 5000);
		mt6582_consys_wifi_vcn33(false);	/* PALDO off (orden inverso, como el stock) */
		mt6582_consys_bt_vcn33(false);
		if (plen >= 5 && rx[0] == 0x02 && rx[1] == 0x14 && rx[4] == 0x00)
			dev_info(b->dev, "*** RF-CAL OK ***\n");
		else
			dev_warn(b->dev, "RF-CAL fallo/timeout (plen=%d): %*ph\n",
				 plen, plen > 0 ? min_t(int, plen, 8) : 0, rx);
	}

	/* COEX antenna config (stock wmt_stp_init_coex -> WMT_COEX_SETTING_CONFIG con
	 * coex_wmt_ant_mode=1 del WMT_SOC.cfg del krillin). Configura el enrutado de antena
	 * del combo; el stock lo hace en sw_init. Sin esto el path RF puede quedar mal ruteado. */
	{
		static const u8 coex[6] = { 0x01, 0x10, 0x02, 0x00, 0x01, 0x01 };
		dev_info(b->dev, "coex: enviando COEX ant_mode=1...\n");
		if (wmt_cmd(b, coex, 6, (const u8[]){0x02,0x10}, 2, "COEX"))
			dev_warn(b->dev, "COEX fallo (sigo)\n");
		else
			dev_info(b->dev, "*** COEX OK ***\n");
	}

	/* STRAP_CONF FM-comm (stock init_table_5_1, último paso del sw_init capturado
	 * en LineageOS tras el coex). */
	{
		static const u8 fmcomm[6] = { 0x01, 0x05, 0x02, 0x00, 0x02, 0x02 };
		if (wmt_cmd(b, fmcomm, 6, (const u8[]){0x02,0x05}, 2, "FM-COMM"))
			dev_warn(b->dev, "FM-COMM fallo (sigo)\n");
		else
			dev_info(b->dev, "*** FM-COMM OK ***\n");
	}

	ret = func_on(b, 0, "BT");
	if (ret) return ret;
	/* Init RF del GPS (stock wmt_func_gps_pre_ctrl, ANTES del func_on GPS):
	 * GSYNC del chip a modo MUX via WMT SET_REG -> reg 0x80050078 bits[30:28]=0x1
	 * (mask 0x7<<28) (mtk_wcn_soc_gps_sync_ctrl, wmt_ic_soc.c:1113 del stock GPL).
	 * El LNA de la antena (GPIO 47 host, OUT_H) lo enciende userspace (zzy-gps-lna).
	 * Sin esta init el frontend RF del GPS no engancha satelites ($GPGSV,1,1,0). */
	{
		static const u8 gsync[20] = {
			0x01, 0x08, 0x10, 0x00,	/* WMT SET_REG, len=16 */
			0x01, 0x01, 0x00, 0x00,	/* op=write, type=reg */
			0x78, 0x00, 0x05, 0x80,	/* addr 0x80050078 LE */
			0x00, 0x00, 0x00, 0x10,	/* val  0x1<<28  LE */
			0x00, 0x00, 0x00, 0x70,	/* mask 0x7<<28  LE */
		};
		dev_info(b->dev, "GPS: enviando GSYNC (reg 0x80050078 bits[30:28]=1)...\n");
		if (wmt_cmd(b, gsync, 20, (const u8[]){0x02,0x08}, 2, "GSYNC-GPS"))
			dev_warn(b->dev, "*** GSYNC GPS FALLO ***\n");
		else
			dev_info(b->dev, "*** GSYNC GPS OK ***\n");
	}

	/* GPS encendido aquí (síncrono, antes del kthread RX) para no competir por rxbuf.
	 * El streaming NMEA lo dispara el daemon de userspace (mnld-equiv) vía /dev/stpgps. */
	if (func_on(b, STP_TYPE_GPS, "GPS"))
		dev_warn(b->dev, "GPS no encendió (sigo: BT ok)\n");
	dev_info(b->dev, "*** CHIP LISTO + BT/GPS ON — registrando hci0 + /dev/stpgps ***\n");
	return 0;
}

/* hilo RX: STP-BT -> BlueZ. */
static int btif_rx_thread(void *data)
{
	struct mt6582_btif *b = data;
	u8 buf[512];
	u8 type;
	int plen;

	while (!kthread_should_stop()) {
		rx_drain(b);
		while ((plen = stp_pop_frame(b, buf, sizeof(buf), &type)) > 0) {
			struct sk_buff *skb;

			if (type == STP_TYPE_WMT) {	/* EVT de func_on/off -> wmt_func() */
				b->wmt_rxlen = min_t(u32, plen, sizeof(b->wmt_rx));
				memcpy(b->wmt_rx, buf, b->wmt_rxlen);
				complete(&b->wmt_done);
				continue;
			}
			if (type == STP_TYPE_GPS) {	/* NMEA/MNL -> /dev/stpgps */
				kfifo_in(&b->gps_fifo, buf, plen);
				b->gps_rx++;	/* DEBUG: frame GPS entregado al fifo */
				wake_up_interruptible(&b->gps_wq);
				continue;
			}
			if (type == STP_TYPE_FM) {	/* RDS/eventos FM -> driver mtk_fm_drv */
				mutex_lock(&b->fm_cb_lock);
				if (b->fm_rx_cb)
					b->fm_rx_cb(buf, plen);
				mutex_unlock(&b->fm_cb_lock);
				continue;
			}
			if (type != STP_TYPE_BT || !b->hdev || plen < 2)
				continue;
			skb = bt_skb_alloc(plen - 1, GFP_KERNEL);
			if (!skb)
				continue;
			hci_skb_pkt_type(skb) = buf[0];	/* H4 type */
			skb_put_data(skb, buf + 1, plen - 1);
			if (b->dbg_n < 40) { b->dbg_n++;
				dev_info(b->dev, "RX<- HCI t=0x%x len=%d: %*ph\n", buf[0], plen, min_t(int, plen, 12), buf); }
			if (hci_recv_frame(b->hdev, skb) < 0)
				b->hdev->stat.err_rx++;
		}
		usleep_range(700, 1500);
	}
	return 0;
}

static int btif_hci_open(struct hci_dev *hdev) { return 0; }	/* chip ya levantado */
static int btif_hci_close(struct hci_dev *hdev) { return 0; }
static int btif_hci_flush(struct hci_dev *hdev) { return 0; }

static int btif_hci_send(struct hci_dev *hdev, struct sk_buff *skb)
{
	struct mt6582_btif *b = hci_get_drvdata(hdev);
	u32 n = skb->len;

	if (n + 1 > WMTBUF_SZ) { kfree_skb(skb); return -EINVAL; }
	mutex_lock(&b->tx_lock);
	b->wmtbuf[0] = hci_skb_pkt_type(skb);	/* H4 type */
	memcpy(b->wmtbuf + 1, skb->data, n);
	if (b->dbg_n < 40) { b->dbg_n++;
		dev_info(b->dev, "TX-> HCI t=0x%x len=%u: %*ph\n", b->wmtbuf[0], n + 1,
			 min_t(int, n + 1, 8), b->wmtbuf); }
	stp_send(b, STP_TYPE_BT, b->wmtbuf, n + 1);
	mutex_unlock(&b->tx_lock);

	switch (hci_skb_pkt_type(skb)) {
	case HCI_COMMAND_PKT: hdev->stat.cmd_tx++; break;
	case HCI_ACLDATA_PKT: hdev->stat.acl_tx++; break;
	case HCI_SCODATA_PKT: hdev->stat.sco_tx++; break;
	}
	kfree_skb(skb);
	return 0;
}

/* lógica del bring-up (interna, SIN lock). Llamar SIEMPRE vía la envoltura bringup() que serializa. */
static int __bringup(struct mt6582_btif *b)
{
	struct hci_dev *hdev;
	int ret;

	if (b->brought_up) { dev_info(b->dev, "ya levantado (hci0 existe)\n"); return 0; }
	ret = bring_up_chip(b);
	if (ret) { dev_err(b->dev, "bring-up fallo (%d)\n", ret); return ret; }
	b->brought_up = true;

	/* descartar cualquier evento espurio del bring-up: RX limpio antes del kthread/HCI init */
	msleep(60);
	b->rxlen = 0;
	rx_drain(b);
	b->rxlen = 0;

	hdev = hci_alloc_dev();
	if (!hdev) return -ENOMEM;
	b->hdev = hdev;
	hdev->bus = HCI_UART;
	hci_set_drvdata(hdev, b);
	SET_HCIDEV_DEV(hdev, b->dev);
	hdev->open = btif_hci_open;
	hdev->close = btif_hci_close;
	hdev->flush = btif_hci_flush;
	hdev->send = btif_hci_send;
	/* el CONSYS no soporta Read Local Ext Features page 2 (daba Opcode 0x1004 -38) */
	hci_set_quirk(hdev, HCI_QUIRK_BROKEN_LOCAL_EXT_FEATURES_PAGE_2);

	b->rx_task = kthread_run(btif_rx_thread, b, "mt6582-btif-rx");
	ret = hci_register_dev(hdev);
	if (ret) {
		dev_err(b->dev, "hci_register_dev fallo (%d)\n", ret);
		if (b->rx_task) { kthread_stop(b->rx_task); b->rx_task = NULL; }
		hci_free_dev(hdev); b->hdev = NULL;
		return ret;
	}
	dev_info(b->dev, "*** hci0 REGISTRADO — enciende BT: bluetoothctl power on / Phosh ***\n");
	return 0;
}

/* envoltura que SERIALIZA el bring-up: BT (debugfs), GPS (/dev/stpgps) y WiFi pueden dispararlo
 * a la vez -> sin lock daba DOBLE bring-up y el CONSYS fallaba. El mutex lo hace atomico (1 sola vez). */
static int bringup(struct mt6582_btif *b)
{
	int ret;

	mutex_lock(&b->bringup_lock);
	ret = __bringup(b);
	mutex_unlock(&b->bringup_lock);
	return ret;
}

/* ===== API exportada: encender/apagar una función del CONSYS en runtime =====
 * type: 0=BT 1=FM 2=GPS 3=WIFI. Thread-safe: levanta el chip si hace falta (patch + kthread RX)
 * y manda el comando WMT esperando el EVT por la completion que llena el kthread (sin competir
 * por rxbuf). La usa el driver WiFi (mt6582-wifi.c) para func_on(WIFI). */
static int wmt_func(struct mt6582_btif *b, u8 type, u8 on)
{
	u8 cmd[6] = { 0x01, 0x06, 0x02, 0x00, type, on };
	int ret = -EIO;

	if (!b->brought_up && bringup(b))
		return -EIO;			/* asegura patch + kthread RX vivo */
	mutex_lock(&b->wmt_lock);
	reinit_completion(&b->wmt_done);
	mutex_lock(&b->tx_lock);
	stp_send(b, STP_TYPE_WMT, cmd, 6);
	mutex_unlock(&b->tx_lock);
	if (wait_for_completion_timeout(&b->wmt_done, msecs_to_jiffies(2000)) &&
	    b->wmt_rxlen >= 5 && b->wmt_rx[0] == 0x02 && b->wmt_rx[1] == 0x06 && b->wmt_rx[4] == 0)
		ret = 0;
	mutex_unlock(&b->wmt_lock);
	if (ret)
		dev_warn(b->dev, "wmt_func(type=%u on=%u): sin EVT OK (rxlen=%u)\n", type, on, b->wmt_rxlen);
	return ret;
}

int mt6582_consys_func_on(u8 type)
{
	return g_btif ? wmt_func(g_btif, type, 0x01) : -ENODEV;
}
EXPORT_SYMBOL_GPL(mt6582_consys_func_on);

int mt6582_consys_func_off(u8 type)
{
	return g_btif ? wmt_func(g_btif, type, 0x00) : -ENODEV;
}
EXPORT_SYMBOL_GPL(mt6582_consys_func_off);

/* ===== FM (canal STP 1): API para el driver stock mtk_fm_drv =====
 * El driver FM llama a estas funciones vía nuestro shim stp_exp.h/wmt_exp.h.
 * TX: stp_send(STP_TYPE_FM). RX: callback registrado por el driver FM. */
int mt6582_stp_fm_send(const u8 *data, u32 len)
{
	struct mt6582_btif *b = g_btif;

	if (!b)
		return -ENODEV;
	mutex_lock(&b->tx_lock);
	stp_send(b, STP_TYPE_FM, data, len);
	mutex_unlock(&b->tx_lock);
	return len;
}
EXPORT_SYMBOL_GPL(mt6582_stp_fm_send);

int mt6582_stp_fm_register_rx(void (*cb)(const u8 *data, u32 len))
{
	struct mt6582_btif *b = g_btif;

	if (!b)
		return -ENODEV;
	mutex_lock(&b->fm_cb_lock);
	b->fm_rx_cb = cb;
	mutex_unlock(&b->fm_cb_lock);
	return 0;
}
EXPORT_SYMBOL_GPL(mt6582_stp_fm_register_rx);

static ssize_t bringup_write(struct file *f, const char __user *u, size_t n, loff_t *o)
{
	if (g_btif) bringup(g_btif);
	return n;
}
static const struct file_operations bringup_fops = { .write = bringup_write, .owner = THIS_MODULE };

/* ===== GPS: /dev/stpgps — passthrough crudo del canal STP 2 (NMEA/MNL) =====
 * Mismo contrato que el stp_chrdev_gps del downstream: read/write entregan el
 * payload STP ya desensamblado. Un daemon de userspace (mnld-equiv) arranca el
 * GPS y vuelca NMEA, que se enchufa a gpsd -> geoclue -> Phosh. */
static int gps_open(struct inode *ino, struct file *f)
{
	struct mt6582_btif *b = g_btif;

	if (!b)
		return -ENODEV;
	if (!b->brought_up && bringup(b))	/* abre el chip si el BT no lo hizo ya */
		return -EIO;
	kfifo_reset(&b->gps_fifo);
	f->private_data = b;
	return 0;
}

static ssize_t gps_read(struct file *f, char __user *u, size_t n, loff_t *o)
{
	struct mt6582_btif *b = f->private_data;
	unsigned int copied = 0;
	int ret;

	b->gps_rd_calls++;	/* DEBUG */
	if (mutex_lock_interruptible(&b->gps_rd_lock))
		return -ERESTARTSYS;
	while (kfifo_is_empty(&b->gps_fifo)) {
		if (f->f_flags & O_NONBLOCK) { ret = -EAGAIN; goto out; }
		b->gps_rd_block++;	/* DEBUG: entra a esperar (fifo vacio) */
		mutex_unlock(&b->gps_rd_lock);
		if (wait_event_interruptible(b->gps_wq, !kfifo_is_empty(&b->gps_fifo))) {
			b->gps_rd_eintr++;	/* DEBUG: interrumpido por signal */
			return -ERESTARTSYS;
		}
		if (mutex_lock_interruptible(&b->gps_rd_lock))
			return -ERESTARTSYS;
	}
	ret = kfifo_to_user(&b->gps_fifo, u, n, &copied);
	if (!ret && copied)
		b->gps_rd_data++;	/* DEBUG: devolvio datos */
out:
	mutex_unlock(&b->gps_rd_lock);
	return ret ? ret : (ssize_t)copied;
}

static ssize_t gps_write(struct file *f, const char __user *u, size_t n, loff_t *o)
{
	struct mt6582_btif *b = f->private_data;
	u32 len = min_t(size_t, n, sizeof(b->gps_tx));

	mutex_lock(&b->tx_lock);
	if (copy_from_user(b->gps_tx, u, len)) { mutex_unlock(&b->tx_lock); return -EFAULT; }
	stp_send(b, STP_TYPE_GPS, b->gps_tx, len);
	mutex_unlock(&b->tx_lock);
	return len;
}

static __poll_t gps_poll(struct file *f, struct poll_table_struct *wait)
{
	struct mt6582_btif *b = f->private_data;

	poll_wait(f, &b->gps_wq, wait);
	return kfifo_is_empty(&b->gps_fifo) ? 0 : (EPOLLIN | EPOLLRDNORM);
}

static const struct file_operations gps_fops = {
	.owner = THIS_MODULE,
	.open = gps_open,
	.read = gps_read,
	.write = gps_write,
	.poll = gps_poll,
};

static struct miscdevice gps_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "stpgps",
	.fops = &gps_fops,
};

static int mt6582_btif_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_btif *b;
	u8 rx[64];
	int plen;

	if (!mt6582_consys_ready)
		return -EPROBE_DEFER;
	b = devm_kzalloc(dev, sizeof(*b), GFP_KERNEL);
	if (!b) return -ENOMEM;
	b->dev = dev;
	mutex_init(&b->tx_lock);
	mutex_init(&b->gps_rd_lock);
	mutex_init(&b->wmt_lock);
	mutex_init(&b->bringup_lock);
	mutex_init(&b->fm_cb_lock);
	init_completion(&b->wmt_done);
	init_waitqueue_head(&b->gps_wq);
	if (kfifo_alloc(&b->gps_fifo, GPS_FIFO_SZ, GFP_KERNEL))
		return -ENOMEM;
	dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));
	b->base = ioremap(BTIF_PHYS, 0x100);
	b->txdma = ioremap(APDMA_TX_PHYS, APDMA_SIZE);
	b->rxdma = ioremap(APDMA_RX_PHYS, APDMA_SIZE);
	b->tx_ring = dma_alloc_coherent(dev, TX_RING, &b->tx_phys, GFP_KERNEL);
	b->rx_ring = dma_alloc_coherent(dev, RX_RING, &b->rx_phys, GFP_KERNEL);
	if (!b->base || !b->txdma || !b->rxdma || !b->tx_ring || !b->rx_ring)
		return -ENOMEM;

	btif_hw_init(b);
	b->rxlen = 0;
	stp_send(b, STP_TYPE_WMT, GEN_HCR, 20);
	plen = wmt_wait_frame(b, rx, sizeof(rx), 300);
	if (plen >= 2 && rx[0] == 0x02 && rx[1] == 0x08)
		dev_info(dev, "*** CANAL OK: CONSYS responde GEN_HCR (%d bytes) ***\n", plen);
	else
		dev_info(dev, "canal: respuesta inesperada plen=%d\n", plen);

	g_btif = b;
	b->dbg = debugfs_create_dir("mt6582_btif", NULL);
	debugfs_create_file("bringup", 0200, b->dbg, b, &bringup_fops);
	debugfs_create_u32("resync_drops", 0444, b->dbg, &b->resync_drops);
	debugfs_create_u32("gps_rx", 0444, b->dbg, &b->gps_rx);			/* DEBUG */
	debugfs_create_u32("gps_rd_calls", 0444, b->dbg, &b->gps_rd_calls);	/* DEBUG */
	debugfs_create_u32("gps_rd_data", 0444, b->dbg, &b->gps_rd_data);	/* DEBUG */
	debugfs_create_u32("gps_rd_eintr", 0444, b->dbg, &b->gps_rd_eintr);	/* DEBUG */
	debugfs_create_u32("gps_rd_block", 0444, b->dbg, &b->gps_rd_block);	/* DEBUG */
	misc_register(&gps_miscdev);
	dev_info(dev, "BT: echo 1 > /sys/kernel/debug/mt6582_btif/bringup  (levanta hci0)\n");
	dev_info(dev, "GPS: /dev/stpgps (abrir dispara bring-up; userspace mnld-equiv -> gpsd)\n");
	platform_set_drvdata(pdev, b);
	return 0;
}

static void mt6582_btif_remove(struct platform_device *pdev)
{
	struct mt6582_btif *b = platform_get_drvdata(pdev);

	if (!b)
		return;
	/* parar kthread RX */
	if (b->rx_task) {
		kthread_stop(b->rx_task);
		b->rx_task = NULL;
	}
	/* desregistrar HCI */
	if (b->hdev) {
		hci_unregister_dev(b->hdev);
		hci_free_dev(b->hdev);
		b->hdev = NULL;
	}
	/* desregistrar /dev/stpgps */
	misc_deregister(&gps_miscdev);
	debugfs_remove_recursive(b->dbg);
	kfifo_free(&b->gps_fifo);
	if (b->tx_ring) dma_free_coherent(b->dev, TX_RING, b->tx_ring, b->tx_phys);
	if (b->rx_ring) dma_free_coherent(b->dev, RX_RING, b->rx_ring, b->rx_phys);
	if (b->base)   iounmap(b->base);
	if (b->txdma)  iounmap(b->txdma);
	if (b->rxdma)  iounmap(b->rxdma);
	g_btif = NULL;
}

static const struct of_device_id mt6582_btif_of_ids[] = {
	{ .compatible = "mediatek,mt6582-btif" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_btif_of_ids);

static struct platform_driver mt6582_btif_driver = {
	.probe = mt6582_btif_probe,
	.remove = mt6582_btif_remove,
	.driver = { .name = "mt6582-btif", .of_match_table = mt6582_btif_of_ids },
};
module_platform_driver(mt6582_btif_driver);

MODULE_DESCRIPTION("MediaTek MT6582 BTIF + CONSYS bring-up + Bluetooth HCI");
MODULE_LICENSE("GPL");
