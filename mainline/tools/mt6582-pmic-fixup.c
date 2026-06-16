// SPDX-License-Identifier: GPL-2.0
/*
 * MT6582 (BQ Aquaris E4.5 "krillin") PMIC fixup.
 *
 * Until the full pwrap + mt6323 regulator stack is wired into the DT, enable
 * the touchscreen supply (MT6323 VGP1) here from a late_initcall, talking to
 * the PMIC over the pwrap WACS2 interface (the LK already initialised pwrap).
 * VGP1 = MT6323 reg 0x050A bit15 (from mt6323-regulator.c: DIGLDO_CON7).
 *
 * Validated by hand first (tools/pwrap_poke.c): CID 0x2023, VGP1 0x0000->0x8000
 * powers the FT5336 (responds 0x5a). See mainline/HITO-I2C-TOUCH.md.
 */
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/printk.h>

#define PWRAP_BASE	0x1000D000
#define WACS2_CMD	0x9C
#define WACS2_RDATA	0xA0
#define WACS2_VLDCLR	0xA4
#define MT6323_VGP1_REG	0x050A
#define VGP1_EN		0x8000

static int pwrap_wait(void __iomem *pw, u32 fsm)
{
	int i;
	for (i = 0; i < 1000000; i++)
		if (((readl(pw + WACS2_RDATA) >> 16) & 0x7) == fsm)
			return 0;
	return -1;
}

static int __init krillin_pmic_init(void)
{
	void __iomem *pw;
	u32 v;

	if (!of_machine_is_compatible("bq,krillin"))
		return 0;

	pw = ioremap(PWRAP_BASE, 0x1000);
	if (!pw) {
		pr_warn("krillin-pmic: ioremap pwrap failed\n");
		return 0;
	}

	/* pwrap read VGP1 */
	if (pwrap_wait(pw, 0)) { pr_warn("krillin-pmic: pwrap busy\n"); goto out; }
	writel((MT6323_VGP1_REG >> 1) << 16, pw + WACS2_CMD);
	if (pwrap_wait(pw, 6)) { pr_warn("krillin-pmic: pwrap read timeout\n"); goto out; }
	v = readl(pw + WACS2_RDATA) & 0xffff;
	writel(1, pw + WACS2_VLDCLR);

	/* pwrap write VGP1 |= EN */
	v |= VGP1_EN;
	if (pwrap_wait(pw, 0)) { pr_warn("krillin-pmic: pwrap busy(w)\n"); goto out; }
	writel((1u << 31) | ((MT6323_VGP1_REG >> 1) << 16) | (v & 0xffff), pw + WACS2_CMD);

	pr_info("krillin-pmic: VGP1 (touch supply) enabled = 0x%04x\n", v);
out:
	iounmap(pw);
	return 0;
}
late_initcall(krillin_pmic_init);
