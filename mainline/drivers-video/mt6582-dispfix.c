// SPDX-License-Identifier: GPL-2.0
/*
 * MT6582 (BQ Aquaris E4.5 "krillin") display fixup.
 *
 * The LK bootloader leaves the DISP OVL configured for the 32bpp (argb8888)
 * framebuffer it used for the boot logo. Our mainline simple-framebuffer is
 * declared r5g6b5 (16bpp) to match what the panel pipeline expects, so the OVL
 * must be reprogrammed to RGB565 or the blue channel is dropped and everything
 * shows with a yellow tint.
 *
 * Until there is a real DRM/fbdev display driver for this SoC, reprogram the
 * OVL here from a late_initcall, cloning the known-good register state read
 * live from the downstream 3.10 kernel (which displays correctly). One-shot.
 *
 * See mainline/HITO-M3-DISPLAY-RESUELTO.md for the full analysis.
 */
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/printk.h>

/* DISP block physical bases */
#define MT6582_OVL	0x14007000
#define MT6582_RDMA	0x14008000
#define MT6582_BLS	0x1400A000
#define MT6582_COLOR	0x1400B000
#define MT6582_MUTEX	0x1400E000

static int __init krillin_dispfix_init(void)
{
	void __iomem *ovl, *rdma, *bls, *col, *mtx;

	if (!of_machine_is_compatible("bq,krillin"))
		return 0;

	ovl  = ioremap(MT6582_OVL,   0x1000);
	rdma = ioremap(MT6582_RDMA,  0x1000);
	bls  = ioremap(MT6582_BLS,   0x1000);
	col  = ioremap(MT6582_COLOR, 0x1000);
	mtx  = ioremap(MT6582_MUTEX, 0x1000);
	if (!ovl) {
		pr_warn("krillin-dispfix: ioremap OVL failed\n");
		return 0;
	}

	/* L3 = framebuffer, RGB565 (CLRFMT=1), 960x540, pitch 1088 @ 0xBF400000 */
	writel(0x400010ff, ovl + 0x90);	/* L3 CON   */
	writel(0xff000000, ovl + 0x94);	/* L3 KEY   */
	writel(0x03c0021c, ovl + 0x98);	/* L3 SRC_SIZE (960<<16 | 540) */
	writel(0x00000000, ovl + 0x9c);	/* L3 OFFSET */
	writel(0xbf400000, ovl + 0xa0);	/* L3 ADDR  */
	writel(0x00000440, ovl + 0xa4);	/* L3 PITCH (544*2) */
	writel(0x00000000, ovl + 0x28);	/* ROI_BGCLR */
	writel(0x03ff0000, ovl + 0xc0);	/* RDMA0_CON fifo */
	writel(0x00000008, ovl + 0x2c);	/* SRC_CON: only L3 */

	if (rdma) {
		writel(0x0000021c, rdma + 0x14);	/* SIZE0 = 540 */
		writel(0x000003c0, rdma + 0x18);	/* SIZE1 = 960 */
		writel(0x0000003f, rdma + 0x00);	/* CON */
	}
	if (col)
		writel(0x20000000, col + 0x400);	/* COLOR CFG_MAIN */
	if (bls) {
		writel(0x00010001, bls + 0x00);		/* BLS_EN */
		writel(0x00100007, bls + 0x10);		/* BLS_SET */
	}
	if (mtx) {					/* commit: MOD/SOF, then toggle EN */
		writel(0x00000680, mtx + 0x2c);
		writel(0x00000001, mtx + 0x30);
		writel(0x00000000, mtx + 0x00);
		writel(0x00000303, mtx + 0x00);
	}

	pr_info("krillin-dispfix: OVL reprogrammed to RGB565 (display tint fix)\n");

	if (rdma) iounmap(rdma);
	if (bls)  iounmap(bls);
	if (col)  iounmap(col);
	if (mtx)  iounmap(mtx);
	iounmap(ovl);
	return 0;
}
late_initcall(krillin_dispfix_init);
