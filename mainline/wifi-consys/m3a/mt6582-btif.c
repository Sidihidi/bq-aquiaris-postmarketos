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
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
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
#define STP_TYPE_GPS	2
#define STP_TYPE_WMT	4
#define GPS_FIFO_SZ	16384
#define RXBUF_SZ	2048
#define TXPKT_SZ	1100
#define WMTBUF_SZ	1010

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
	bool brought_up;
	struct hci_dev *hdev;
	struct task_struct *rx_task;
	struct mutex tx_lock;
	int dbg_n;
	/* GPS: canal STP 2 -> /dev/stpgps (passthrough crudo, igual que el downstream). */
	struct kfifo gps_fifo;
	wait_queue_head_t gps_wq;
	struct mutex gps_rd_lock;
	u8 gps_tx[1024];
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
	b->txpkt[n++] = 0x00; b->txpkt[n++] = 0x00;
	btif_tx_raw(b, b->txpkt, n);
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

	if (b->rxlen < 4) return 0;
	*type = (b->rxbuf[1] >> 4) & 0x0f;
	len = ((b->rxbuf[1] & 0x0f) << 8) | b->rxbuf[2];
	frame = 4 + len + 2;
	if (b->rxlen < frame) return 0;
	n = min_t(u32, len, max);
	memcpy(out, b->rxbuf + 4, n);
	memmove(b->rxbuf, b->rxbuf + frame, b->rxlen - frame);
	b->rxlen -= frame;
	return n;
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

static int wmt_cmd(struct mt6582_btif *b, const u8 *cmd, u32 clen,
		   const u8 *evt, u32 elen, const char *name)
{
	u8 rx[64];
	int plen;

	b->rxlen = 0;
	stp_send(b, STP_TYPE_WMT, cmd, clen);
	plen = wmt_wait_frame(b, rx, sizeof(rx), 400);
	if (plen < 0) { dev_err(b->dev, "wmt_cmd[%s]: TIMEOUT\n", name); return plen; }
	if ((u32)plen < elen || memcmp(rx, evt, elen) != 0) {
		dev_err(b->dev, "wmt_cmd[%s]: EVT no coincide (plen=%d): %*ph\n",
			name, plen, min_t(int, plen, 16), rx);
		return -EBADMSG;
	}
	return 0;
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

		b->wmtbuf[0] = 0x01; b->wmtbuf[1] = 0x01;
		b->wmtbuf[2] = wlen & 0xff; b->wmtbuf[3] = (wlen >> 8) & 0xff;
		b->wmtbuf[4] = (seq == fragnum - 1) ? FRAG_LAST : (seq == 0 ? FRAG_1ST : FRAG_MID);
		memcpy(b->wmtbuf + 5, body + off, fsz);
		ret = wmt_cmd(b, b->wmtbuf, 5 + fsz, PATCH_EVT, 5, "FRAG");
		if (ret) goto out;
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
	static const u8 a_e1_1[4] = { 0x00, 0x00, 0x0e, 0xf0 };
	static const u8 a_e1_0[4] = { 0x00, 0x00, 0x06, 0x00 };
	int ret;

	dev_info(b->dev, "=== BRING-UP del CONSYS (patch + func_on BT) ===\n");
	ret = wmt_cmd(b, GEN_HCR, 20, (const u8[]){0x02,0x08}, 2, "GEN_HCR");
	if (ret) return ret;
	ret = patch_dwn(b, "mt6572_82_patch_e1_1_hdr.bin", a_e1_1);
	if (ret) return ret;
	wmt_cmd(b, WMT_RESET_CMD, 5, WMT_RESET_EVT, 5, "RESET-1");
	ret = patch_dwn(b, "mt6572_82_patch_e1_0_hdr.bin", a_e1_0);
	if (ret) return ret;
	wmt_cmd(b, WMT_RESET_CMD, 5, WMT_RESET_EVT, 5, "RESET-2");
	ret = func_on(b, 0, "BT");
	if (ret) return ret;
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

			if (type == STP_TYPE_GPS) {	/* NMEA/MNL -> /dev/stpgps */
				kfifo_in(&b->gps_fifo, buf, plen);
				wake_up_interruptible(&b->gps_wq);
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

/* dispara (debugfs): bring-up del chip + registra hci0 + arranca el hilo RX. */
static int bringup(struct mt6582_btif *b)
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

	if (mutex_lock_interruptible(&b->gps_rd_lock))
		return -ERESTARTSYS;
	while (kfifo_is_empty(&b->gps_fifo)) {
		if (f->f_flags & O_NONBLOCK) { ret = -EAGAIN; goto out; }
		mutex_unlock(&b->gps_rd_lock);
		if (wait_event_interruptible(b->gps_wq, !kfifo_is_empty(&b->gps_fifo)))
			return -ERESTARTSYS;
		if (mutex_lock_interruptible(&b->gps_rd_lock))
			return -ERESTARTSYS;
	}
	ret = kfifo_to_user(&b->gps_fifo, u, n, &copied);
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
	misc_register(&gps_miscdev);
	dev_info(dev, "BT: echo 1 > /sys/kernel/debug/mt6582_btif/bringup  (levanta hci0)\n");
	dev_info(dev, "GPS: /dev/stpgps (abrir dispara bring-up; userspace mnld-equiv -> gpsd)\n");
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

MODULE_DESCRIPTION("MediaTek MT6582 BTIF + CONSYS bring-up + Bluetooth HCI");
MODULE_LICENSE("GPL");
