// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-btif.c — transporte BTIF del MT6582 en modo DMA/VFF + STP/WMT al CONSYS.
 *
 * ITERACIÓN 3 (2026-06-19): descarga del PATCH (bring-up del chip).
 *  - it.2 LOGRÓ que el CONSYS conteste (BTIF-DMA + FLUSH de la cola). Ver historial abajo.
 *  - it.3 añade: helper TX→RX síncrono (wmt_cmd: envía, espera el EVT STP, verifica) + parse de
 *    tramas STP en RX + request_firmware + descarga del patch en fragmentos (mtk_wcn_soc_patch_dwn).
 *  - Como el probe corre ANTES del rootfs (driver built-in), la descarga se dispara por **debugfs**
 *    (`echo 1 > /sys/kernel/debug/mt6582_btif/bringup`) cuando ya hay rootfs+firmware. Itera sin reflashear.
 *  Datos de la captura del stock: e1_1 addr 00 00 0e f0 (21 frags), e1_0 addr 00 00 06 00 (41 frags),
 *  FRAG_SIZE=1000, header patch 28B. Comandos extraídos de wmt_ic_soc.c.
 *
 *  it.2 fix (el bug que desbloqueó todo): el TX-DMA no expulsa la cola parcial (<8B) sin FLUSH
 *  (TX_DMA_FLUSH @0x14 bit0); WPT wrap en bit16 (DMA_WPT_WRAP); RX valid=WPT-RPT.
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

/* --- BTIF core @0x1100C000 --- */
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

/* --- APDMA VFF (TX @0x11000780, RX @0x11000800) --- */
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

#define TX_RING		0x2000	/* 8KB (frags de 1KB + STP) */
#define RX_RING		0x2000

#define PERICFG_PHYS	0x10003000
#define PERI_PDN0_CLR	0x10
#define PERI_CG_BTIF	(1u << 20)

#define STP_TYPE_WMT	4
#define STP_TYPE_BT	0	/* BT_TASK_INDX */
#define STP_TYPE_FM	1
#define STP_TYPE_GPS	2
#define RXBUF_SZ	1024
#define TXPKT_SZ	1100	/* trama STP máx (frag 1000 + WMT hdr 5 + STP 4 + CRC 2) */
#define WMTBUF_SZ	1010	/* WMT cmd de un fragmento (5 + 1000) */

/* patch protocol (wmt_ic_soc.c) */
#define PATCH_HDR	28
#define FRAG_SIZE	1000
#define FRAG_1ST	0x1
#define FRAG_MID	0x2
#define FRAG_LAST	0x3

extern bool mt6582_consys_ready;

struct mt6582_btif {
	struct device *dev;
	void __iomem *base, *txdma, *rxdma;
	void *tx_ring, *rx_ring;
	dma_addr_t tx_phys, rx_phys;
	u32 tx_off, tx_wrap, rx_off, rx_wrap;
	u8 rxbuf[RXBUF_SZ];
	u32 rxlen;
	u8 txpkt[TXPKT_SZ];
	u8 wmtbuf[WMTBUF_SZ];
	struct dentry *dbg;
};

static struct mt6582_btif *g_btif;

/* comandos WMT (payloads, se envuelven en STP). De wmt_ic_soc.c. */
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
/* func_on(type): {01,06,02,00,type,01}; EVT {02,06,01,00,status}. BT=0 FM=1 GPS=2 WIFI=3 */

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

/* TX por DMA: copiar al ring + WPT|wrap + FLUSH de la cola parcial. */
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

/* Envuelve payload en STP MAND y lo envía. */
static void stp_send(struct mt6582_btif *b, u8 type, const u8 *pl, u32 len)
{
	u32 n;

	if (len + 6 > TXPKT_SZ) return;
	b->txpkt[0] = 0x80;
	b->txpkt[1] = (type << 4) | ((len >> 8) & 0x0f);
	b->txpkt[2] = len & 0xff;
	b->txpkt[3] = 0x00;
	memcpy(b->txpkt + 4, pl, len);
	n = 4 + len;
	b->txpkt[n++] = 0x00; b->txpkt[n++] = 0x00;	/* CRC (MAND: 0) */
	btif_tx_raw(b, b->txpkt, n);
}

/* drena RX-DMA al acumulador rxbuf. */
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

/* espera una trama STP completa; copia el payload a out; devuelve len del payload o <0. */
static int wmt_wait_frame(struct mt6582_btif *b, u8 *out, u32 max, u32 ms)
{
	int loops = ms * 10;

	while (loops-- > 0) {
		rx_drain(b);
		if (b->rxlen >= 4) {
			u32 len = ((b->rxbuf[1] & 0x0f) << 8) | b->rxbuf[2];
			u32 frame = 4 + len + 2;

			if (b->rxlen >= frame) {
				u32 n = min_t(u32, len, max);

				memcpy(out, b->rxbuf + 4, n);
				memmove(b->rxbuf, b->rxbuf + frame, b->rxlen - frame);
				b->rxlen -= frame;
				return len;
			}
		}
		udelay(100);
	}
	return -ETIMEDOUT;
}

/* envía un comando WMT y verifica que el EVT coincide. */
static int wmt_cmd(struct mt6582_btif *b, const u8 *cmd, u32 clen,
		   const u8 *evt, u32 elen, const char *name)
{
	u8 rx[64];
	int plen;

	b->rxlen = 0;
	stp_send(b, STP_TYPE_WMT, cmd, clen);
	plen = wmt_wait_frame(b, rx, sizeof(rx), 400);
	if (plen < 0) {
		dev_err(b->dev, "wmt_cmd[%s]: TIMEOUT (sin EVT)\n", name);
		return plen;
	}
	if ((u32)plen < elen || memcmp(rx, evt, elen) != 0) {
		dev_err(b->dev, "wmt_cmd[%s]: EVT no coincide (plen=%d): %*ph\n",
			name, plen, min_t(int, plen, 16), rx);
		return -EBADMSG;
	}
	return 0;
}

/* descarga un patch: ADDRESS -> P_ADDRESS(addr) -> fragmentos. */
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
	dev_info(b->dev, "patch %s: %u bytes, %u frags, addr %4ph\n", fwname, blen, fragnum, addr);

	ret = wmt_cmd(b, PATCH_ADDR_CMD, 20, PATCH_ADDR_EVT, 8, "ADDR");
	if (ret) goto out;
	memcpy(paddr, PATCH_PADDR_CMD, 20);
	memcpy(paddr + 12, addr, 4);
	ret = wmt_cmd(b, paddr, 20, PATCH_PADDR_EVT, 8, "PADDR");
	if (ret) goto out;

	for (seq = 0, off = 0; seq < fragnum; seq++, off += FRAG_SIZE) {
		u32 fsz = min_t(u32, FRAG_SIZE, blen - off);
		u16 wlen = 1 + fsz;	/* FRAG byte + datos */

		b->wmtbuf[0] = 0x01; b->wmtbuf[1] = 0x01;
		b->wmtbuf[2] = wlen & 0xff; b->wmtbuf[3] = (wlen >> 8) & 0xff;
		b->wmtbuf[4] = (seq == fragnum - 1) ? FRAG_LAST : (seq == 0 ? FRAG_1ST : FRAG_MID);
		memcpy(b->wmtbuf + 5, body + off, fsz);
		ret = wmt_cmd(b, b->wmtbuf, 5 + fsz, PATCH_EVT, 5, "FRAG");
		if (ret) { dev_err(b->dev, "patch frag %u/%u fallo\n", seq, fragnum); goto out; }
	}
	dev_info(b->dev, "*** patch %s DESCARGADO OK (%u frags) ***\n", fwname, fragnum);
out:
	release_firmware(fw);
	return ret;
}

/* enciende una función/radio: func_on(type). BT=0 FM=1 GPS=2 WIFI=3. */
static int func_on(struct mt6582_btif *b, u8 type, const char *name)
{
	u8 cmd[6] = { 0x01, 0x06, 0x02, 0x00, type, 0x01 };
	u8 rx[16];
	int plen;

	b->rxlen = 0;
	stp_send(b, STP_TYPE_WMT, cmd, 6);
	plen = wmt_wait_frame(b, rx, sizeof(rx), 2000);	/* func_on puede tardar */
	if (plen < 5) {
		dev_err(b->dev, "func_on[%s]: TIMEOUT/corto (plen=%d)\n", name, plen);
		return -EIO;
	}
	if (rx[0] == 0x02 && rx[1] == 0x06 && rx[4] == 0) {
		dev_info(b->dev, "func_on[%s]: *** RADIO %s ENCENDIDO *** EVT=%*ph\n",
			 name, name, min_t(int, plen, 8), rx);
		return 0;
	}
	dev_warn(b->dev, "func_on[%s]: status=%d EVT=%*ph (no encendió)\n",
		 name, rx[4], min_t(int, plen, 8), rx);
	return -EIO;
}

/* prueba que el BT habla HCI: HCI RESET (0x0C03) por STP-BT -> Command Complete. */
static void bt_hci_reset(struct mt6582_btif *b)
{
	static const u8 hci_reset[4] = { 0x01, 0x03, 0x0c, 0x00 };	/* H4 cmd + opcode 0x0C03 + len 0 */
	u8 rx[32];
	int plen, k;

	b->rxlen = 0;
	stp_send(b, STP_TYPE_BT, hci_reset, 4);
	for (k = 0; k < 3; k++) {	/* puede llegar un evento de init antes */
		plen = wmt_wait_frame(b, rx, sizeof(rx), 700);
		if (plen < 0) { dev_err(b->dev, "BT HCI RESET: sin respuesta\n"); return; }
		if (plen >= 2 && rx[0] == 0x04) {	/* HCI event packet */
			if (plen >= 6 && rx[1] == 0x0e && rx[4] == 0x03 && rx[5] == 0x0c)
				dev_info(b->dev, "*** BLUETOOTH VIVO: HCI Command Complete (RESET) %*ph ***\n",
					 min_t(int, plen, 8), rx);
			else
				dev_info(b->dev, "*** BT responde HCI event %*ph (el chip BT habla) ***\n",
					 min_t(int, plen, 12), rx);
			return;
		}
		dev_info(b->dev, "BT frame %d (no-HCI) plen=%d: %*ph\n", k, plen, min_t(int, plen, 12), rx);
	}
}

static int bringup(struct mt6582_btif *b)
{
	static const u8 a_e1_1[4] = { 0x00, 0x00, 0x0e, 0xf0 };
	static const u8 a_e1_0[4] = { 0x00, 0x00, 0x06, 0x00 };
	int ret;

	dev_info(b->dev, "=== BRING-UP: patch + reset + func_on ===\n");
	ret = wmt_cmd(b, GEN_HCR, 20, (const u8[]){0x02,0x08}, 2, "GEN_HCR");
	if (ret) { dev_err(b->dev, "canal KO (GEN_HCR)\n"); return ret; }
	dev_info(b->dev, "canal OK (GEN_HCR responde)\n");

	/* descarga del patch + WMT_RESET tras cada uno (como el stock, init_table_3) */
	ret = patch_dwn(b, "mt6572_82_patch_e1_1_hdr.bin", a_e1_1);
	if (ret) return ret;
	wmt_cmd(b, WMT_RESET_CMD, 5, WMT_RESET_EVT, 5, "RESET-1");
	ret = patch_dwn(b, "mt6572_82_patch_e1_0_hdr.bin", a_e1_0);
	if (ret) return ret;
	wmt_cmd(b, WMT_RESET_CMD, 5, WMT_RESET_EVT, 5, "RESET-2");
	dev_info(b->dev, "*** PATCH + RESET COMPLETO — encendiendo radios ***\n");

	/* encender radios (BT es el más fácil: no necesita WIFI_RAM_CODE) */
	if (func_on(b, 0, "BT") == 0)
		bt_hci_reset(b);	/* probar que el BT habla HCI de verdad */
	func_on(b, 1, "FM");
	func_on(b, 2, "GPS");
	func_on(b, 3, "WIFI");
	return 0;
}

static ssize_t bringup_write(struct file *f, const char __user *u, size_t n, loff_t *o)
{
	if (g_btif) bringup(g_btif);
	return n;
}
static const struct file_operations bringup_fops = { .write = bringup_write, .owner = THIS_MODULE };

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
	dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));
	b->base = ioremap(BTIF_PHYS, 0x100);
	b->txdma = ioremap(APDMA_TX_PHYS, APDMA_SIZE);
	b->rxdma = ioremap(APDMA_RX_PHYS, APDMA_SIZE);
	b->tx_ring = dma_alloc_coherent(dev, TX_RING, &b->tx_phys, GFP_KERNEL);
	b->rx_ring = dma_alloc_coherent(dev, RX_RING, &b->rx_phys, GFP_KERNEL);
	if (!b->base || !b->txdma || !b->rxdma || !b->tx_ring || !b->rx_ring)
		return -ENOMEM;

	btif_hw_init(b);
	dev_info(dev, "BTIF-DMA init: DMA_EN=0x%x\n", rd(b->base, BTIF_DMA_EN));

	/* prueba el canal en boot (GEN_HCR) */
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
	dev_info(dev, "trigger bring-up: echo 1 > /sys/kernel/debug/mt6582_btif/bringup\n");
	platform_set_drvdata(pdev, b);
	return 0;
}

static const struct of_device_id mt6582_btif_of_ids[] = {
	{ .compatible = "mediatek,mt6582-btif" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_btif_of_ids);

static struct platform_driver mt6582_btif_driver = {
	.probe = mt6582_btif_probe,
	.driver = { .name = "mt6582-btif", .of_match_table = mt6582_btif_of_ids },
};
module_platform_driver(mt6582_btif_driver);

MODULE_DESCRIPTION("MediaTek MT6582 BTIF (DMA) + CONSYS STP/WMT patch download");
MODULE_LICENSE("GPL");
