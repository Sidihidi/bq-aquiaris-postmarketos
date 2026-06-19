// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-btif.c — transporte BTIF del MT6582 en modo DMA/VFF (TX+RX), como el stock.
 *
 * ===== ITERACIÓN 2 (2026-06-19): arreglo del TX-DMA =====
 * La it.1 daba RX=0 + PC del MCU congelado (no recibía). El bug (hallado en hal_dma_send_data):
 *  - El DMA NO expulsa la cola parcial de un paquete (<8 bytes) sin un FLUSH explícito
 *    (TX_DMA_FLUSH @0x14 bit0). El stock lo hace en su IRQ handler; nosotros (polled) a mano.
 *  - El wrap del WPT va en el bit16 (DMA_WPT_WRAP=0x10000), la offset en [15:0] con tope vff_size.
 *  - TX threshold = vff_size-7 (DMA_TX_THRE). STOP @0x10 debe estar a 0.
 * Secuencia de envío (replica de hal_dma_send_data): memcpy al ring -> dsb -> EN -> escribir
 * WPT|wrap -> flush mientras 0<VALID<8 hasta VALID==0. RX: valid = WPT-RPT (wrap-aware).
 */
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>

/* --- BTIF core @0x1100C000 --- */
#define BTIF_PHYS	0x1100c000
#define BTIF_THR	0x00
#define BTIF_LSR	0x14
#define BTIF_FAKELCR	0x0c
#define BTIF_DMA_EN	0x4c
#define BTIF_TRI_LVL	0x60
#define BTIF_HANDSHAKE	0x6c
#define LSR_THRE	(1u << 5)
#define LSR_TEMT	(1u << 6)
#define DMA_EN_RX	(1u << 0)
#define DMA_EN_TX	(1u << 1)
#define DMA_EN_AUTORST	(1u << 2)
#define HANDSHAKE_EN	(1u << 0)
#define TRI_LVL_VAL	(1u | (1u << 4))	/* tx_thre=1 rx_thre=1, loop bit7=0 */

/* --- APDMA VFF (TX @0x11000780, RX @0x11000800); mismo layout --- */
#define APDMA_TX_PHYS	0x11000780
#define APDMA_RX_PHYS	0x11000800
#define APDMA_SIZE	0x80
#define VFF_INT_FLAG	0x00	/* W1C */
#define VFF_INT_EN	0x04
#define VFF_EN		0x08	/* bit0 = arrancar motor */
#define VFF_RST		0x0c	/* bit0 warm reset */
#define VFF_STOP	0x10	/* bit0 stop (debe ser 0 para enviar) */
#define VFF_FLUSH	0x14	/* bit0 flush (expulsa cola parcial) */
#define VFF_ADDR	0x1c
#define VFF_LEN		0x24
#define VFF_THRE	0x28
#define VFF_WPT		0x2c
#define VFF_RPT		0x30
#define VFF_VALID	0x3c
#define VFF_LEFT	0x40
#define DMA_WPT_MASK	0x0000ffff
#define DMA_WPT_WRAP	0x00010000

#define TX_RING		0x1000	/* 4KB */
#define RX_RING		0x2000	/* 8KB */

#define PERICFG_PHYS	0x10003000
#define PERI_PDN0_CLR	0x10
#define PERI_CG_BTIF	(1u << 20)

#define STP_TYPE_WMT	4	/* WMT_TASK_INDX */
#define BTIF_RX_LOG	128

/* lo pone a true mt6582-consys.c cuando el CONSYS ya está encendido + activado */
extern bool mt6582_consys_ready;

struct mt6582_btif {
	struct device *dev;
	void __iomem *base;	/* BTIF core */
	void __iomem *txdma;	/* APDMA TX */
	void __iomem *rxdma;	/* APDMA RX */
	void *tx_ring;
	dma_addr_t tx_phys;
	void *rx_ring;
	dma_addr_t rx_phys;
	u32 tx_off;		/* offset de escritura TX (0..TX_RING-1) */
	u32 tx_wrap;		/* bit de wrap TX (0 o DMA_WPT_WRAP) */
	u32 rx_off;		/* offset de lectura RX */
	u32 rx_wrap;
	u8 rxlog[BTIF_RX_LOG];
	unsigned int rxcnt;
};

static inline u32 rd(void __iomem *b, u32 o) { return readl(b + o); }
static inline void wr(void __iomem *b, u32 o, u32 v) { writel(v, b + o); }

static void btif_clk_ungate(void)
{
	void __iomem *p = ioremap(PERICFG_PHYS, 0x100);

	if (p) {
		writel(PERI_CG_BTIF, p + PERI_PDN0_CLR);
		iounmap(p);
	}
}

/* Configura un canal APDMA VFF (TX o RX). thre = umbral en bytes. */
static void vff_setup(void __iomem *dma, dma_addr_t ring_phys, u32 size, u32 thre)
{
	wr(dma, VFF_RST, 0x1);
	udelay(10);
	wr(dma, VFF_RST, 0x0);
	wr(dma, VFF_STOP, 0x0);		/* asegurar NO stop */
	wr(dma, VFF_ADDR, (u32)ring_phys);
	wr(dma, VFF_LEN, size);
	wr(dma, VFF_THRE, thre);
	wr(dma, VFF_WPT, 0);
	wr(dma, VFF_RPT, 0);
	wr(dma, VFF_INT_FLAG, 0x3);	/* W1C pendientes */
	wr(dma, VFF_INT_EN, 0x0);	/* polled, sin IRQ */
	wr(dma, VFF_EN, 0x1);		/* arrancar motor */
}

static void mt6582_btif_hw_init(struct mt6582_btif *b)
{
	wr(b->base, BTIF_FAKELCR, 0);				/* modo normal */
	wr(b->base, BTIF_HANDSHAKE, rd(b->base, BTIF_HANDSHAKE) | HANDSHAKE_EN);
	wr(b->base, BTIF_TRI_LVL, TRI_LVL_VAL);			/* tx/rx thre + loopback off */
	/* modo DMA en TX y RX */
	wr(b->base, BTIF_DMA_EN, DMA_EN_TX | DMA_EN_RX | DMA_EN_AUTORST);

	vff_setup(b->txdma, b->tx_phys, TX_RING, TX_RING - 7);	/* DMA_TX_THRE = n-7 */
	vff_setup(b->rxdma, b->rx_phys, RX_RING, 1);		/* RX: flag con cualquier byte */
	b->tx_off = 0; b->tx_wrap = 0;
	b->rx_off = 0; b->rx_wrap = 0;
}

/* TX por DMA (replica de hal_dma_send_data): copiar al ring + WPT|wrap + FLUSH de la cola. */
static int mt6582_btif_tx(struct mt6582_btif *b, const u8 *buf, size_t len)
{
	u32 off = b->tx_off, first;
	int t;

	if (len > TX_RING)
		return -EINVAL;
	first = min_t(u32, len, TX_RING - off);
	memcpy((u8 *)b->tx_ring + off, buf, first);
	if (len > first)
		memcpy(b->tx_ring, buf + first, len - first);
	dma_wmb();
	/* nuevo offset (tope vff_size) + toggle wrap si cruza */
	if (off + len >= TX_RING) {
		b->tx_off = off + len - TX_RING;
		b->tx_wrap ^= DMA_WPT_WRAP;
	} else {
		b->tx_off = off + len;
	}
	wr(b->txdma, VFF_EN, 0x1);			/* asegurar DMA habilitado */
	wr(b->txdma, VFF_WPT, b->tx_off | b->tx_wrap);	/* dispara el envío */
	/* drenar: el DMA envía el grueso; la cola (<8) necesita FLUSH manual */
	for (t = 0; t < 50000; t++) {
		u32 valid = rd(b->txdma, VFF_VALID) & 0xffff;

		if (valid == 0)
			break;
		if (valid < 8)
			wr(b->txdma, VFF_FLUSH, 0x1);	/* expulsar cola parcial */
		udelay(2);
	}
	return 0;
}

/* RX por DMA: valid = WPT-RPT (wrap-aware); leer del ring, avanzar RPT. */
static void mt6582_btif_rx_drain(struct mt6582_btif *b)
{
	u32 wpt = rd(b->rxdma, VFF_WPT);
	u32 wpt_off = wpt & DMA_WPT_MASK, wpt_wrap = wpt & DMA_WPT_WRAP;
	u32 valid, i, o;

	if (wpt_wrap == b->rx_wrap)
		valid = wpt_off - b->rx_off;
	else
		valid = RX_RING - b->rx_off + wpt_off;
	if (!valid || valid > RX_RING)
		return;
	for (i = 0; i < valid; i++) {
		o = (b->rx_off + i) % RX_RING;
		if (b->rxcnt < BTIF_RX_LOG)
			b->rxlog[b->rxcnt] = ((u8 *)b->rx_ring)[o];
		b->rxcnt++;
	}
	b->rx_off = wpt_off;
	b->rx_wrap = wpt_wrap;
	wr(b->rxdma, VFF_RPT, wpt_off | wpt_wrap);	/* devolver crédito */
}

static void mt6582_btif_rx_poll(struct mt6582_btif *b, unsigned int ms)
{
	unsigned int loops = ms * 10;

	while (loops--) {
		mt6582_btif_rx_drain(b);
		udelay(100);
	}
}

/* Envía un paquete STP MAND: header(4) + payload + CRC(2)=0000. type=WMT. */
static int mt6582_btif_stp_send(struct mt6582_btif *b, u8 type, const u8 *payload, u32 len)
{
	u8 pkt[4 + 64 + 2];
	u32 n = 0;

	if (len > 64)
		return -EINVAL;
	pkt[0] = 0x80;
	pkt[1] = (type << 4) | ((len >> 8) & 0x0f);
	pkt[2] = len & 0xff;
	pkt[3] = 0x00;
	n = 4;
	memcpy(pkt + n, payload, len);
	n += len;
	pkt[n++] = 0x00;
	pkt[n++] = 0x00;
	return mt6582_btif_tx(b, pkt, n);
}

static int mt6582_btif_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_btif *b;
	void __iomem *mcu;
	int ret, k;
	u32 pc0;
	/* GEN_HCR register-read (wmt_core_hw_check, el PRIMER comando del stock) */
	static const u8 gen_hcr[20] = {
		0x01, 0x08, 0x10, 0x00,  0x02, 0x01, 0x00, 0x01,
		0x08, 0x00, 0x00, 0x80,  0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0x00, 0x00
	};

	if (!mt6582_consys_ready)
		return -EPROBE_DEFER;

	b = devm_kzalloc(dev, sizeof(*b), GFP_KERNEL);
	if (!b)
		return -ENOMEM;
	b->dev = dev;
	if (dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32)))
		dev_warn(dev, "dma mask 32 fallo\n");

	b->base = ioremap(BTIF_PHYS, 0x100);
	b->txdma = ioremap(APDMA_TX_PHYS, APDMA_SIZE);
	b->rxdma = ioremap(APDMA_RX_PHYS, APDMA_SIZE);
	b->tx_ring = dma_alloc_coherent(dev, TX_RING, &b->tx_phys, GFP_KERNEL);
	b->rx_ring = dma_alloc_coherent(dev, RX_RING, &b->rx_phys, GFP_KERNEL);
	if (!b->base || !b->txdma || !b->rxdma || !b->tx_ring || !b->rx_ring)
		return -ENOMEM;

	btif_clk_ungate();
	mt6582_btif_hw_init(b);
	dev_info(dev, "BTIF-DMA init: DMA_EN=0x%x HS=0x%x TRI=0x%x tx=%pad rx=%pad\n",
		 rd(b->base, BTIF_DMA_EN), rd(b->base, BTIF_HANDSHAKE),
		 rd(b->base, BTIF_TRI_LVL), &b->tx_phys, &b->rx_phys);

	mcu = ioremap(0x18070000, 0x200);
	pc0 = mcu ? rd(mcu, 0x160) : 0;

	ret = mt6582_btif_stp_send(b, STP_TYPE_WMT, gen_hcr, sizeof(gen_hcr));
	/* estado del TX tras enviar: ¿drenó el VFF? ¿transmitió el BTIF? */
	dev_info(dev, "STP TX GEN_HCR rc=%d | TX_VFF valid=0x%x left=0x%x rpt=0x%x wpt=0x%x | BTIF_LSR=0x%x | PC=0x%x\n",
		 ret, rd(b->txdma, VFF_VALID), rd(b->txdma, VFF_LEFT),
		 rd(b->txdma, VFF_RPT), rd(b->txdma, VFF_WPT),
		 rd(b->base, BTIF_LSR), pc0);

	for (k = 0; k < 20; k++) {
		mt6582_btif_rx_poll(b, 100);
		dev_info(dev, "DIAG k=%2d PC=0x%-7x RX_wpt=0x%x rpt=0x%x RX=%u: %*ph\n",
			 k, mcu ? rd(mcu, 0x160) : 0, rd(b->rxdma, VFF_WPT),
			 rd(b->rxdma, VFF_RPT), b->rxcnt,
			 min_t(unsigned int, b->rxcnt, 16), b->rxlog);
		if (b->rxcnt)
			break;
	}
	if (mcu)
		iounmap(mcu);

	if (b->rxcnt)
		dev_info(dev, "*** EL CONSYS CONTESTA (%u bytes) — M3a CONSEGUIDO, WiFi en marcha ***\n",
			 b->rxcnt);
	else
		dev_info(dev, "RX=0 (si TX_VFF valid->0 = TX salió; si PC sigue igual = MCU no reacciona al comando)\n");

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
	.driver = {
		.name = "mt6582-btif",
		.of_match_table = mt6582_btif_of_ids,
	},
};
module_platform_driver(mt6582_btif_driver);

MODULE_DESCRIPTION("MediaTek MT6582 BTIF (DMA mode) for CONSYS STP/WMT");
MODULE_LICENSE("GPL");
