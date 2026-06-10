// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek MT6582 MUSB glue layer (mainline bring-up for BQ Aquaris E4.5)
 *
 * Derived from drivers/usb/musb/mediatek.c (Min Guo, MediaTek). The MT6582
 * embeds a Mentor (MUSB) controller wrapped with MediaTek's L1-interrupt
 * aggregation and TX/RX toggle registers -- identical wrapper to the newer
 * SoCs that mediatek.c targets. What differs on this 2014 SoC:
 *   - clocks: no mainline CCF driver -> we poke the gates directly
 *             (UNIVPLL_CON0 bit26 = USB 48M PHY clk; PERI_PDN0 bit10 = USB MAC)
 *   - PHY:    old MTK USB2 PHY, no mainline driver -> the power-on register
 *             sequence (ported from downstream usb20_phy.c) is inlined here.
 * Peripheral (gadget) only -- the goal is USB networking/SSH to a host.
 * PIO only (no Inventra DMA) for first bring-up.
 *
 * Hardware (verified from the downstream kernel):
 *   MAC  base 0x11200000  (reg index 0)
 *   SIF  base 0x11210000  (reg index 1), PHY regs at SIF + 0x800 + off
 *   IRQ  GIC_SPI 32 level
 *   APMIXEDSYS 0x10209000  UNIVPLL_CON0 = +0x220, USB 48M = bit26
 *   PERICFG    0x10003000  PERI_PDN0_CLR = +0x10 (USB MAC = bit10),
 *              reset-all-USB = bit29 of PERICFG+0x00
 */

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/usb/usb_phy_generic.h>
#include "musb_core.h"
#include "musb_dma.h"

/* MediaTek wrapper registers (same as mediatek.c) */
#define USB_L1INTS		0x00a0
#define USB_L1INTM		0x00a4
#define MTK_MUSB_TXFUNCADDR	0x0480
#define MUSB_RXTOG		0x80
#define MUSB_RXTOGEN		0x82
#define MUSB_TXTOG		0x84
#define MUSB_TXTOGEN		0x86
#define MTK_TOGGLE_EN		GENMASK(15, 0)
#define TX_INT_STATUS		BIT(0)
#define RX_INT_STATUS		BIT(1)
#define USBCOM_INT_STATUS	BIT(2)
#define DMA_INT_STATUS		BIT(3)

/* clock / reset register offsets within the ioremapped windows */
#define UNIVPLL_CON0_OFF	0x0220
#define UNIVPLL_USB48M		BIT(26)
#define PERI_PDN0_CLR_OFF	0x0010
#define PERI_PDN0_USB		BIT(10)
#define PERICFG_USB_RST		BIT(29)		/* bit29 of PERICFG + 0x00 */

struct mt6582_glue {
	struct device *dev;
	struct musb *musb;
	struct platform_device *musb_pdev;
	struct platform_device *usb_phy;
	struct usb_phy *xceiv;
	void __iomem *sif;		/* USB PHY (SIF) window */
	void __iomem *apmixed;		/* APMIXEDSYS (PLL) window */
	void __iomem *pericfg;		/* PERICFG window */
};

/* --- old MTK USB2 PHY access: 8-bit regs at SIF + 0x800 + offset --- */
static inline u8 phy_r8(struct mt6582_glue *g, u32 off)
{
	return readb(g->sif + 0x800 + off);
}
static inline void phy_w8(struct mt6582_glue *g, u32 off, u8 val)
{
	writeb(val, g->sif + 0x800 + off);
}
static inline void phy_set8(struct mt6582_glue *g, u32 off, u8 m)
{
	phy_w8(g, off, phy_r8(g, off) | m);
}
static inline void phy_clr8(struct mt6582_glue *g, u32 off, u8 m)
{
	phy_w8(g, off, phy_r8(g, off) & ~m);
}

/* HS slew-rate calibration (ported from downstream hs_slew_rate_cal) */
#define FRA 48
#define PARA 28
static void mt6582_phy_slew_cal(struct mt6582_glue *g)
{
	unsigned long data, x;
	u8 value;
	u32 tmp;
	int ret;

	phy_w8(g, 0x15, 0x80);			/* enable ring oscillator */
	udelay(1);
	phy_w8(g, 0x711, 0x01);			/* enable free run clock */
	phy_w8(g, 0x701, 0x04);			/* cyclecnt */
	phy_set8(g, 0x703, 0x01);		/* enable freq meter */

	ret = readb_poll_timeout(g->sif + 0x800 + 0x710, tmp, (tmp & 0x1),
				 100, 3 * 1000 * 1000);
	if (ret) {
		dev_warn(g->dev, "PHY slew cal timeout, using default\n");
		value = 0x4;
	} else {
		data = readl(g->sif + 0x800 + 0x70c);
		x = data ? (1024 * FRA * PARA) / data : 0;
		value = (u8)(x / 1000);
		if ((x - value * 1000) / 100 >= 5)
			value++;
	}
	phy_clr8(g, 0x703, 0x01);
	phy_clr8(g, 0x711, 0x01);
	phy_w8(g, 0x15, value << 4);
	phy_clr8(g, 0x15, 0x80);
}

/* Power on the PHY in DEVICE mode (ported from usb_phy_poweron) */
static void mt6582_phy_poweron(struct mt6582_glue *g)
{
	dev_info(g->dev, "mt6582-musb: PHY poweron START\n");
	/* USB 48M PHY clock (UNIVPLL_CON0 bit26) */
	writel(readl(g->apmixed + UNIVPLL_CON0_OFF) | UNIVPLL_USB48M,
	       g->apmixed + UNIVPLL_CON0_OFF);
	/* USB MAC peripheral clock: clear power-down bit */
	writel(PERI_PDN0_USB, g->pericfg + PERI_PDN0_CLR_OFF);

	/* reset the whole USB IP (PHY+MAC): PERICFG[0] bit29 */
	writel(readl(g->pericfg) | PERICFG_USB_RST, g->pericfg);
	mdelay(10);
	writel(readl(g->pericfg) & ~PERICFG_USB_RST, g->pericfg);

	udelay(50);
	phy_clr8(g, 0x6b, 0x04);	/* force_uart_en = 0 -> USB function */
	phy_clr8(g, 0x6e, 0x01);	/* RG_UART_EN = 0 */
	phy_clr8(g, 0x1a, 0x80);	/* RG_USB20_BC11_SW_EN = 0 */
	phy_clr8(g, 0x22, 0x03);	/* DP/DM 100K pull-down off */
	phy_clr8(g, 0x6a, 0x04);	/* release force suspendm */
	udelay(800);
	phy_clr8(g, 0x6c, 0x10);	/* force enter DEVICE mode */
	phy_set8(g, 0x6c, 0x2e);
	phy_set8(g, 0x6d, 0x3e);

	mt6582_phy_slew_cal(g);
	dev_info(g->dev, "MT6582 USB PHY powered on (device mode)\n");
}

/* ---- musb core glue (copied from mediatek.c, PIO, peripheral) ---- */
static irqreturn_t generic_interrupt(int irq, void *__hci)
{
	unsigned long flags;
	irqreturn_t retval = IRQ_NONE;
	struct musb *musb = __hci;

	spin_lock_irqsave(&musb->lock, flags);
	musb->int_usb = musb_clearb(musb->mregs, MUSB_INTRUSB);
	musb->int_rx = musb_clearw(musb->mregs, MUSB_INTRRX);
	musb->int_tx = musb_clearw(musb->mregs, MUSB_INTRTX);

	if ((musb->int_usb & MUSB_INTR_RESET) && !is_host_active(musb)) {
		musb_ep_select(musb->mregs, 0);
		musb_writeb(musb->mregs, MUSB_FADDR, 0);
	}
	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval = musb_interrupt(musb);

	spin_unlock_irqrestore(&musb->lock, flags);
	return retval;
}

static irqreturn_t mt6582_musb_interrupt(int irq, void *dev_id)
{
	irqreturn_t retval = IRQ_NONE;
	struct musb *musb = (struct musb *)dev_id;
	u32 l1_ints;

	l1_ints = musb_readl(musb->mregs, USB_L1INTS) &
		  musb_readl(musb->mregs, USB_L1INTM);

	if (l1_ints & (TX_INT_STATUS | RX_INT_STATUS | USBCOM_INT_STATUS))
		retval = generic_interrupt(irq, musb);

	return retval;
}

static u32 mt6582_busctl_offset(u8 epnum, u16 offset)
{
	return MTK_MUSB_TXFUNCADDR + offset + 8 * epnum;
}

static u8 mt6582_clearb(void __iomem *addr, unsigned int offset)
{
	u8 data = musb_readb(addr, offset);

	musb_writeb(addr, offset, data);	/* W1C */
	return data;
}

static u16 mt6582_clearw(void __iomem *addr, unsigned int offset)
{
	u16 data = musb_readw(addr, offset);

	musb_writew(addr, offset, data);	/* W1C */
	return data;
}

static u16 mt6582_get_toggle(struct musb_qh *qh, int is_out)
{
	struct musb *musb = qh->hw_ep->musb;
	u8 epnum = qh->hw_ep->epnum;

	return musb_readw(musb->mregs, is_out ? MUSB_TXTOG : MUSB_RXTOG) &
	       (1 << epnum);
}

static u16 mt6582_set_toggle(struct musb_qh *qh, int is_out, struct urb *urb)
{
	struct musb *musb = qh->hw_ep->musb;
	u8 epnum = qh->hw_ep->epnum;
	u16 value, toggle = usb_gettoggle(urb->dev, qh->epnum, is_out);

	value = musb_readw(musb->mregs, is_out ? MUSB_TXTOG : MUSB_RXTOG);
	value |= toggle << epnum;
	musb_writew(musb->mregs, is_out ? MUSB_TXTOG : MUSB_RXTOG, value);
	return 0;
}

static int mt6582_musb_init(struct musb *musb)
{
	struct device *dev = musb->controller;
	struct mt6582_glue *glue = dev_get_drvdata(dev->parent);

	glue->musb = musb;
	musb->xceiv = glue->xceiv;
	musb->is_host = false;
	musb->isr = mt6582_musb_interrupt;

	/* TX/RX toggle enable (MTK wrapper) */
	musb_writew(musb->mregs, MUSB_TXTOGEN, MTK_TOGGLE_EN);
	musb_writew(musb->mregs, MUSB_RXTOGEN, MTK_TOGGLE_EN);

	mt6582_phy_poweron(glue);

	/* unmask the L1 sources we use (TX/RX/USBCOM) */
	musb_writel(musb->mregs, USB_L1INTM,
		    TX_INT_STATUS | RX_INT_STATUS | USBCOM_INT_STATUS);
	return 0;
}

static int mt6582_musb_exit(struct musb *musb)
{
	return 0;
}

static const struct musb_platform_ops mt6582_musb_ops = {
	.init		= mt6582_musb_init,
	.exit		= mt6582_musb_exit,
	.clearb		= mt6582_clearb,
	.clearw		= mt6582_clearw,
	.busctl_offset	= mt6582_busctl_offset,
	.get_toggle	= mt6582_get_toggle,
	.set_toggle	= mt6582_set_toggle,
};

#define MT6582_MUSB_MAX_EP_NUM	8
#define MT6582_MUSB_RAM_BITS	11

static const struct musb_fifo_cfg mt6582_musb_fifo_cfg[] = {
	{ .hw_ep_num = 1, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 1, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 2, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 2, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 3, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 3, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 4, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 4, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 5, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 5, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 6, .style = FIFO_TX, .maxpacket = 1024, },
	{ .hw_ep_num = 6, .style = FIFO_RX, .maxpacket = 1024, },
	{ .hw_ep_num = 7, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 7, .style = FIFO_RX, .maxpacket = 64, },
};

static const struct musb_hdrc_config mt6582_musb_hdrc_config = {
	.fifo_cfg	= mt6582_musb_fifo_cfg,
	.fifo_cfg_size	= ARRAY_SIZE(mt6582_musb_fifo_cfg),
	.multipoint	= true,
	.dyn_fifo	= true,
	.num_eps	= MT6582_MUSB_MAX_EP_NUM,
	.ram_bits	= MT6582_MUSB_RAM_BITS,
};

static const struct platform_device_info mt6582_dev_info = {
	.name		= "musb-hdrc",
	.id		= PLATFORM_DEVID_AUTO,
	.dma_mask	= DMA_BIT_MASK(32),
};

static int mt6582_musb_probe(struct platform_device *pdev)
{
	struct musb_hdrc_platform_data *pdata;
	struct mt6582_glue *glue;
	struct platform_device_info pinfo;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;

	dev_info(dev, "mt6582-musb: PROBE ENTER\n");

	glue = devm_kzalloc(dev, sizeof(*glue), GFP_KERNEL);
	if (!glue)
		return -ENOMEM;
	glue->dev = dev;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	/* reg[1] = SIF (PHY); plus the two clock/reset control windows */
	glue->sif = devm_platform_ioremap_resource(pdev, 1);
	if (IS_ERR(glue->sif))
		return dev_err_probe(dev, PTR_ERR(glue->sif), "no SIF reg\n");

	glue->apmixed = ioremap(0x10209000, 0x1000);
	glue->pericfg = ioremap(0x10003000, 0x1000);
	if (!glue->apmixed || !glue->pericfg)
		return -ENOMEM;

	pdata->config = &mt6582_musb_hdrc_config;
	pdata->platform_ops = &mt6582_musb_ops;
	pdata->mode = USB_DR_MODE_PERIPHERAL;

	glue->usb_phy = usb_phy_generic_register();
	if (IS_ERR(glue->usb_phy))
		return dev_err_probe(dev, PTR_ERR(glue->usb_phy),
				     "fail to register usb-phy\n");

	glue->xceiv = devm_usb_get_phy(dev, USB_PHY_TYPE_USB2);
	if (IS_ERR(glue->xceiv)) {
		ret = PTR_ERR(glue->xceiv);
		goto err_unregister_usb_phy;
	}

	platform_set_drvdata(pdev, glue);

	pinfo = mt6582_dev_info;
	pinfo.parent = dev;
	pinfo.res = pdev->resource;	/* res[0] = MAC -> mregs */
	pinfo.num_res = pdev->num_resources;
	pinfo.data = pdata;
	pinfo.size_data = sizeof(*pdata);
	pinfo.fwnode = of_fwnode_handle(np);
	pinfo.of_node_reused = true;

	glue->musb_pdev = platform_device_register_full(&pinfo);
	if (IS_ERR(glue->musb_pdev)) {
		ret = PTR_ERR(glue->musb_pdev);
		dev_err(dev, "failed to register musb device: %d\n", ret);
		goto err_unregister_usb_phy;
	}
	dev_info(dev, "mt6582-musb: PROBE OK, musb-hdrc child registered\n");
	return 0;

err_unregister_usb_phy:
	usb_phy_generic_unregister(glue->usb_phy);
	return ret;
}

static void mt6582_musb_remove(struct platform_device *pdev)
{
	struct mt6582_glue *glue = platform_get_drvdata(pdev);

	platform_device_unregister(glue->musb_pdev);
	usb_phy_generic_unregister(glue->usb_phy);
}

static const struct of_device_id mt6582_musb_match[] = {
	{ .compatible = "mediatek,mt6582-musb", },
	{},
};
MODULE_DEVICE_TABLE(of, mt6582_musb_match);

static struct platform_driver mt6582_musb_driver = {
	.probe = mt6582_musb_probe,
	.remove = mt6582_musb_remove,
	.driver = {
		.name = "musb-mt6582",
		.of_match_table = mt6582_musb_match,
	},
};
module_platform_driver(mt6582_musb_driver);

MODULE_DESCRIPTION("MediaTek MT6582 MUSB Glue Layer");
MODULE_LICENSE("GPL v2");
