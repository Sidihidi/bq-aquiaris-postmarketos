// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-btif.c — transporte BTIF del MT6582 (canal serie AP<->CONSYS), modo PIO.
 *
 * ===== FASE A: DIAGNÓSTICO PASIVO (2026-06-18) =====
 * La v1 hacía BOOTLOOP (boot-m3a.img: cuelga en el logo, antes de consola). Dos
 * sospechas (HITO-WIFI-M3A.md): (1) tormenta de IRQ en SPI50 (request_irq+IER con
 * la ISR sin limpiar la fuente real), (2) BTIF SIN CLOCK -> el primer readl/writel
 * da *external abort* y cuelga. Confirmado en el downstream: el clock del BTIF es
 * MT_CG_PERI_BTIF (=bit 20 de PERI_PDN0), y el M1 encendió el CONSYS pero NADIE
 * ungateó el clock del BTIF (periférico del lado AP) -> casi seguro la causa (2).
 *
 * Esta Fase A ataca AMBAS y es 100% pasiva:
 *   - UNGATEA el clock del BTIF (PERI_PDN0_CLR bit20 @ 0x10003010) ANTES de tocar
 *     el BTIF  ->  mata la causa (2).
 *   - NO request_irq, NO escribe IER, NO hace TX  ->  mata la causa (1).
 *   - Solo LEE LSR/IIR y los loguea. Si el boot llega a Phosh/SSH y LSR es un valor
 *     sano (no 0xffffffff), el bus BTIF vive con el clock -> seguimos con TX/RX
 *     (Fase C/D) y luego IRQ (Fase E).
 *
 * Registros del core BTIF (base física 0x1100C000): RBR/THR 0x00, IER 0x04,
 * IIR 0x08 (ro) / FIFOCTRL 0x08 (wo), FAKELCR 0x0c, LSR 0x14.  IRQ = GIC_SPI 50.
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
#define BTIF_FIFOCTRL		0x08	/* wo */
#define BTIF_FAKELCR		0x0c
#define BTIF_LSR		0x14
#define LSR_DR			(1u << 0)	/* dato recibido listo */
#define LSR_THRE		(1u << 5)	/* TX holding vacío */
#define LSR_TEMT		(1u << 6)	/* TX shift vacío */

/* PERICFG: clock-gate del BTIF. MT_CG_PERI_BTIF = 20 (downstream mt_clkmgr.h).
 * Ungate (power up) = escribir el bit a PERI_PDN0_CLR; estado en PERI_PDN0_STA
 * (bit=1 => clock GATED). Base física 0x10003000 (el 0xF0003000 era el virtual). */
#define PERICFG_PHYS		0x10003000
#define PERICFG_SIZE		0x100
#define PERI_PDN0_SET		0x08
#define PERI_PDN0_CLR		0x10
#define PERI_PDN0_STA		0x18
#define PERI_CG_BTIF		(1u << 20)

struct mt6582_btif {
	struct device *dev;
	void __iomem *base;
	int irq;
};

/* Ungatea el clock del BTIF en PERICFG. Devuelve PERI_PDN0_STA tras el ungate
 * (bit20 debería quedar a 0). Pasivo y seguro: PERICFG siempre está accesible. */
static u32 mt6582_btif_clk_ungate(struct device *dev)
{
	void __iomem *peri;
	u32 before = 0, after = 0;

	peri = ioremap(PERICFG_PHYS, PERICFG_SIZE);
	if (!peri) {
		dev_warn(dev, "faseA: no pude mapear PERICFG @0x%08x\n", PERICFG_PHYS);
		return 0xdeadbeef;
	}
	before = readl(peri + PERI_PDN0_STA);
	writel(PERI_CG_BTIF, peri + PERI_PDN0_CLR);	/* ungate */
	after = readl(peri + PERI_PDN0_STA);
	dev_info(dev, "faseA clock BTIF: PDN0_STA 0x%08x -> 0x%08x (bit20 gated: %u -> %u)\n",
		 before, after,
		 !!(before & PERI_CG_BTIF), !!(after & PERI_CG_BTIF));
	iounmap(peri);
	return after;
}

static int mt6582_btif_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_btif *b;
	u32 lsr, iir;

	b = devm_kzalloc(dev, sizeof(*b), GFP_KERNEL);
	if (!b)
		return -ENOMEM;
	b->dev = dev;

	b->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(b->base))
		return PTR_ERR(b->base);

	/* el IRQ se resuelve pero NO se solicita (Fase A: sin ISR, sin tormenta) */
	b->irq = platform_get_irq(pdev, 0);

	/* PASO 1: ungatear el clock ANTES de tocar el BTIF (causa más probable) */
	mt6582_btif_clk_ungate(dev);

	/* PASO 2: lectura PASIVA del BTIF — sin escribir nada, sin IRQ, sin TX.
	 * Si esto cuelga el boot -> el clock no bastó (revisar MTCMOS/reset).
	 * Si arranca y LSR != 0xffffffff -> bus vivo, seguimos con Fase C (TX). */
	lsr = readl(b->base + BTIF_LSR);
	iir = readl(b->base + BTIF_IIR);
	dev_info(dev, "faseA BTIF pasivo: irq=%d LSR=0x%08x IIR=0x%08x %s\n",
		 b->irq, lsr, iir,
		 (lsr == 0xffffffff) ? "(0xffffffff => bus MUERTO, clock/base?)"
				     : "(bus vivo)");

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

MODULE_DESCRIPTION("MediaTek MT6582 BTIF transport (PIO) for CONSYS STP/WMT — Fase A");
MODULE_LICENSE("GPL");
