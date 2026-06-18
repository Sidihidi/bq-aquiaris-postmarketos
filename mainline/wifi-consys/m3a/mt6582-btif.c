// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-btif.c — transporte BTIF del MT6582 (canal serie AP<->CONSYS), modo PIO.
 *
 * ===== FASE C+D v2: TX + RX POR POLLING (sin IRQ) (2026-06-18) =====
 * v1 hizo TX TIMEOUT: esperaba SOLO LSR.THRE, pero tras el init el HW quedaba en
 * LSR=0x40 (TEMT=1, THRE=0). El downstream (`_btif_is_tx_allow`) permite TX si
 * LSR & (TEMT | THRE) — CUALQUIERA. Además faltaban el modo "new handshake"
 * (HANDSHAKE 0x6C bit0) y el TRI_LVL correcto. v2 replica `hal_btif_hw_init` del
 * downstream (FAKELCR normal + HANDSHAKE on + TRI_LVL TX8/RX1 + DMA off+AUTORST),
 * pero SIN habilitar IER (el RX por IRQ causaba el bootloop; aquí RX por polling).
 *
 * Cadena: BTIF (esto) -> STP (trama+CRC16) -> WMT (handshake+FW) -> radios.
 * Registros BTIF (base 0x1100C000): RBR/THR 0x00, IER 0x04, IIR 0x08(ro)/FIFOCTRL
 * 0x08(wo), FAKELCR 0x0c, LSR 0x14, DMA_EN 0x4c, TRI_LVL 0x60, WAK 0x64, HANDSHAKE 0x6c.
 */
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define BTIF_RBR		0x00	/* RX buffer (ro) */
#define BTIF_THR		0x00	/* TX holding (wo) */
#define BTIF_IER		0x04
#define BTIF_IIR		0x08	/* ro */
#define BTIF_FAKELCR		0x0c
#define BTIF_LSR		0x14
#define BTIF_DMA_EN		0x4c
#define BTIF_TRI_LVL		0x60
#define BTIF_WAK		0x64
#define BTIF_HANDSHAKE		0x6c

#define LSR_DR			(1u << 0)	/* dato recibido listo */
#define LSR_THRE		(1u << 5)	/* TX holding vacío */
#define LSR_TEMT		(1u << 6)	/* TX shift vacío */
#define FAKELCR_NORMAL		0x00
#define HANDSHAKE_EN		(1u << 0)	/* new handshake mode */
#define DMA_EN_RX		(1u << 0)
#define DMA_EN_TX		(1u << 1)
#define DMA_EN_AUTORST		(1u << 2)
/* TRI_LVL: TX threshold [3:0]=8, RX threshold [6:4]=1, loopback [7]=0 */
#define TRI_LVL_VAL		(((8u) & 0xf) | (((1u) & 0x7) << 4))

/* PERICFG: clock-gate del BTIF (MT_CG_PERI_BTIF=20). Ya estaba on; lo dejamos. */
#define PERICFG_PHYS		0x10003000
#define PERICFG_SIZE		0x100
#define PERI_PDN0_CLR		0x10
#define PERI_PDN0_STA		0x18
#define PERI_CG_BTIF		(1u << 20)

#define BTIF_RX_LOG		96

struct mt6582_btif {
	struct device *dev;
	void __iomem *base;
	int irq;
	u8 rxlog[BTIF_RX_LOG];
	unsigned int rxcnt;
};

static inline u32 btif_rd(struct mt6582_btif *b, u32 off) { return readl(b->base + off); }
static inline void btif_wr(struct mt6582_btif *b, u32 off, u32 v) { writel(v, b->base + off); }

static void mt6582_btif_clk_ungate(struct device *dev)
{
	void __iomem *peri = ioremap(PERICFG_PHYS, PERICFG_SIZE);

	if (!peri)
		return;
	writel(PERI_CG_BTIF, peri + PERI_PDN0_CLR);
	dev_info(dev, "clk BTIF: PDN0_STA=0x%08x (bit20 gated=%u)\n",
		 readl(peri + PERI_PDN0_STA), !!(readl(peri + PERI_PDN0_STA) & PERI_CG_BTIF));
	iounmap(peri);
}

/* TX por polling. FIX v2: permitir si LSR & (THRE | TEMT) — igual que el downstream. */
static int mt6582_btif_tx(struct mt6582_btif *b, const u8 *buf, size_t len)
{
	size_t i;
	int t;

	for (i = 0; i < len; i++) {
		for (t = 0; t < 100000; t++) {
			if (btif_rd(b, BTIF_LSR) & (LSR_THRE | LSR_TEMT))
				break;
			udelay(1);
		}
		if (t == 100000)
			return -ETIMEDOUT;
		btif_wr(b, BTIF_THR, buf[i]);
	}
	for (t = 0; t < 100000; t++) {
		if (btif_rd(b, BTIF_LSR) & LSR_TEMT)
			break;
		udelay(1);
	}
	return 0;
}

/* RX por POLLING durante ms milisegundos (acotado). Acumula bytes en rxlog. */
static void mt6582_btif_rx_poll(struct mt6582_btif *b, unsigned int ms)
{
	unsigned int loops = ms * 10;	/* 100 us por vuelta */
	u8 c;

	while (loops--) {
		while (btif_rd(b, BTIF_LSR) & LSR_DR) {
			c = btif_rd(b, BTIF_RBR) & 0xff;
			if (b->rxcnt < BTIF_RX_LOG)
				b->rxlog[b->rxcnt] = c;
			b->rxcnt++;
		}
		udelay(100);
	}
}

/* init PIO fiel a hal_btif_hw_init() del downstream, pero SIN habilitar IER. */
static void mt6582_btif_hw_init(struct mt6582_btif *b)
{
	u32 v;

	btif_wr(b, BTIF_FAKELCR, FAKELCR_NORMAL);		/* modo normal */
	v = btif_rd(b, BTIF_HANDSHAKE);
	btif_wr(b, BTIF_HANDSHAKE, v | HANDSHAKE_EN);		/* new handshake mode */
	btif_wr(b, BTIF_TRI_LVL, TRI_LVL_VAL);			/* TX8/RX1, loopback off */
	v = btif_rd(b, BTIF_DMA_EN);
	v &= ~(DMA_EN_TX | DMA_EN_RX);				/* PIO: DMA off */
	v |= DMA_EN_AUTORST;
	btif_wr(b, BTIF_DMA_EN, v);
	btif_wr(b, BTIF_IER, 0);				/* SIN IRQ (ni TX ni RX) */
}

static int mt6582_btif_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_btif *b;
	static const u8 stp_resync[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
	u32 lsr0, lsr1;
	int ret;

	b = devm_kzalloc(dev, sizeof(*b), GFP_KERNEL);
	if (!b)
		return -ENOMEM;
	b->dev = dev;

	b->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(b->base))
		return PTR_ERR(b->base);
	b->irq = platform_get_irq(pdev, 0);	/* resuelto pero NO solicitado */

	mt6582_btif_clk_ungate(dev);

	lsr0 = btif_rd(b, BTIF_LSR);
	mt6582_btif_hw_init(b);
	lsr1 = btif_rd(b, BTIF_LSR);

	ret = mt6582_btif_tx(b, stp_resync, sizeof(stp_resync));
	if (ret) {
		dev_err(dev, "faseCD2: TX resync TIMEOUT (LSR pre=0x%x post-init=0x%x now=0x%x)\n",
			lsr0, lsr1, btif_rd(b, BTIF_LSR));
		return 0;
	}

	mt6582_btif_rx_poll(b, 250);

	dev_info(dev, "faseCD2: resync TX OK (LSR pre=0x%x post-init=0x%x). RX=%u bytes: %*ph\n",
		 lsr0, lsr1, b->rxcnt,
		 min_t(unsigned int, b->rxcnt, BTIF_RX_LOG), b->rxlog);
	if (b->rxcnt)
		dev_info(dev, "faseCD2: *** EL CONSYS RESPONDE (%u bytes) — canal AP<->CONSYS VIVO ***\n",
			 b->rxcnt);
	else
		dev_info(dev, "faseCD2: RX=0 (el resync solo quizá no basta; siguiente = STP+WMT QUERY_BAUD, o WAK)\n");

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

MODULE_DESCRIPTION("MediaTek MT6582 BTIF transport (PIO) for CONSYS STP/WMT — Fase C+D v2");
MODULE_LICENSE("GPL");
