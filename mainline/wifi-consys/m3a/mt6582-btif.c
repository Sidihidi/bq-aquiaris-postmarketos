// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-btif.c — transporte BTIF del MT6582 (canal serie AP<->CONSYS), modo PIO.
 *
 * ⛔ ESTA v1 HACE BOOTLOOP (boot-m3a.img, 2026-06-18): cuelga en el logo, muy temprano.
 *    Sospecha: tormenta de IRQ SPI50 (request_irq+IER_RXFEN con la ISR sin limpiar la
 *    fuente real) o BTIF sin clock (readl -> external abort). Ver HITO-WIFI-M3A.md:
 *    reescribir POR FASES (A pasivo solo-ioremap+log, B clock+reset MCU, C TX polled,
 *    D RX polled, E IRQ al final). NO flashear tal cual sin el arreglo por fases.
 *
 * M3a "el CONSYS habla": el BTIF es el bus serie interno por el que corre STP/WMT
 * para hablar con el CONSYS (WiFi/BT/GPS/FM). Este driver levanta el BTIF en modo
 * PIO (sin DMA VFF, mucho más simple) y expone TX/RX por bytes + logging de RX.
 *
 * Encima de esto va la capa STP (trama 4B + CRC16 + resync) y WMT (handshake +
 * descarga de firmware) — ver HITO-WIFI-M3A.md. Este fichero es SOLO el transporte.
 *
 * Registros del core BTIF (de downstream hal_btif.h, base física 0x1100C000):
 *   RBR/THR 0x00 (RX ro / TX wo), IER 0x04, IIR 0x08 (ro) / FIFOCTRL 0x08 (wo),
 *   FAKELCR 0x0C, LSR 0x14, SLEEP_EN 0x48, DMA_EN 0x4C, RTOCNT 0x54,
 *   TRI_LVL 0x60, WAK 0x64, HANDSHAKE 0x6C.   IRQ = GIC_SPI 50.
 *
 * Es un 16550 extendido -> en PIO: TX = poll LSR.THRE + write THR; RX = IRQ + read RBR.
 */
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#define BTIF_RBR		0x00	/* RX buffer (ro) */
#define BTIF_THR		0x00	/* TX holding (wo) */
#define BTIF_IER		0x04
#define BTIF_IIR		0x08	/* ro */
#define BTIF_FIFOCTRL		0x08	/* wo */
#define BTIF_FAKELCR		0x0c
#define BTIF_LSR		0x14
#define BTIF_SLEEP_EN		0x48
#define BTIF_DMA_EN		0x4c
#define BTIF_RTOCNT		0x54
#define BTIF_TRI_LVL		0x60
#define BTIF_WAK		0x64

#define IER_RXFEN		(1u << 0)	/* RX buffer tiene datos */
#define IER_TXEEN		(1u << 1)	/* TX holding vacío */
#define IIR_RX			(1u << 2)
#define FIFOCTRL_CLR_RX		(1u << 1)
#define FIFOCTRL_CLR_TX		(1u << 2)
#define FAKELCR_NORMAL		0x00
#define LSR_DR			(1u << 0)	/* dato recibido listo */
#define LSR_THRE		(1u << 5)	/* TX holding vacío */
#define LSR_TEMT		(1u << 6)	/* TX shift vacío */
#define DMA_EN_OFF		0x00		/* PIO: ni RX ni TX DMA */
#define WAK_BIT			(1u << 0)

#define BTIF_RX_LOG		64

struct mt6582_btif {
	struct device *dev;
	void __iomem *base;
	int irq;
	spinlock_t lock;
	/* log de los primeros bytes RX para ver si el CONSYS contesta */
	u8 rxlog[BTIF_RX_LOG];
	unsigned int rxcnt;
};

static inline u32 btif_rd(struct mt6582_btif *b, u32 off) { return readl(b->base + off); }
static inline void btif_wr(struct mt6582_btif *b, u32 off, u32 v) { writel(v, b->base + off); }

/* --- API de transporte (la usará la capa STP) --- */
int mt6582_btif_tx(struct mt6582_btif *b, const u8 *buf, size_t len)
{
	size_t i;
	int t;

	for (i = 0; i < len; i++) {
		for (t = 0; t < 100000; t++) {
			if (btif_rd(b, BTIF_LSR) & LSR_THRE)
				break;
			udelay(1);
		}
		if (t == 100000)
			return -ETIMEDOUT;
		btif_wr(b, BTIF_THR, buf[i]);
	}
	/* esperar a que vacíe el shift register */
	for (t = 0; t < 100000; t++) {
		if (btif_rd(b, BTIF_LSR) & LSR_TEMT)
			break;
		udelay(1);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(mt6582_btif_tx);

static irqreturn_t mt6582_btif_isr(int irq, void *data)
{
	struct mt6582_btif *b = data;
	unsigned long flags;
	u8 c;

	spin_lock_irqsave(&b->lock, flags);
	while (btif_rd(b, BTIF_LSR) & LSR_DR) {
		c = btif_rd(b, BTIF_RBR) & 0xff;
		if (b->rxcnt < BTIF_RX_LOG)
			b->rxlog[b->rxcnt] = c;
		b->rxcnt++;
	}
	spin_unlock_irqrestore(&b->lock, flags);
	return IRQ_HANDLED;
}

static void mt6582_btif_hw_init(struct mt6582_btif *b)
{
	btif_wr(b, BTIF_FAKELCR, FAKELCR_NORMAL);		/* modo normal */
	btif_wr(b, BTIF_DMA_EN, DMA_EN_OFF);			/* PIO, sin DMA */
	btif_wr(b, BTIF_FIFOCTRL, FIFOCTRL_CLR_RX | FIFOCTRL_CLR_TX);
	btif_wr(b, BTIF_TRI_LVL, 0);				/* trigger mínimo */
	btif_wr(b, BTIF_WAK, WAK_BIT);				/* despertar módulo */
	btif_wr(b, BTIF_IER, IER_RXFEN);			/* IRQ al recibir */
}

static int mt6582_btif_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_btif *b;
	int ret;
	/* resync STP = 4x 0x7F: sincroniza/despierta la máquina STP del CONSYS */
	static const u8 stp_resync[4] = { 0x7f, 0x7f, 0x7f, 0x7f };

	b = devm_kzalloc(dev, sizeof(*b), GFP_KERNEL);
	if (!b)
		return -ENOMEM;
	b->dev = dev;
	spin_lock_init(&b->lock);

	b->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(b->base))
		return PTR_ERR(b->base);

	b->irq = platform_get_irq(pdev, 0);
	if (b->irq < 0)
		return b->irq;

	ret = devm_request_irq(dev, b->irq, mt6582_btif_isr, 0, "mt6582-btif", b);
	if (ret)
		return ret;

	mt6582_btif_hw_init(b);
	platform_set_drvdata(pdev, b);

	/* M3a v1: el CONSYS ya está encendido (mt6582-consys/M1) y su MCU debería
	 * estar fuera de reset (ver HITO-WIFI-M3A.md: TOPRGU 0x10007000+0x18 bit12
	 * key 0x88<<24). Mandamos un resync STP y miramos si el CONSYS responde algo. */
	if (mt6582_btif_tx(b, stp_resync, sizeof(stp_resync)) == 0) {
		msleep(50);
		dev_info(dev, "BTIF init OK, resync enviado; RX=%u bytes: %*ph\n",
			 b->rxcnt, min_t(unsigned int, b->rxcnt, BTIF_RX_LOG), b->rxlog);
	} else {
		dev_err(dev, "BTIF TX timeout (LSR=0x%x)\n", btif_rd(b, BTIF_LSR));
	}
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

MODULE_DESCRIPTION("MediaTek MT6582 BTIF transport (PIO) for CONSYS STP/WMT");
MODULE_LICENSE("GPL");
