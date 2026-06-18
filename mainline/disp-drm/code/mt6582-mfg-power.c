// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-mfg-power.c — enciende el power-domain del GPU (MFG) por SPM MTCMOS
 * ANTES de que lima probe (subsys_initcall < device_initcall de lima).
 * Secuencia de downstream mt_spm_mtcmos.c:spm_mtcmos_ctrl_mfg(STA_POWER_ON).
 * Sin scpsys mainline para MT6582 -> poke directo. Idempotente (si ya on, no hace nada).
 */
#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>

#define SPM_PHYS		0x10006000
#define SPM_POWERON_CFG		0x000
#define SPM_MFG_PWR_CON		0x214
#define SPM_PWR_STATUS		0x60c
#define SPM_PWR_STATUS_S	0x610
#define SPM_KEY			0x0b160001	/* (PROJECT_CODE 0xb16 << 16) | 1 */
#define MFG_PWR_STA		(1U << 4)

#define PWR_RST_B	(1U << 0)
#define PWR_ISO		(1U << 1)
#define PWR_ON		(1U << 2)
#define PWR_ON_S	(1U << 3)
#define PWR_CLK_DIS	(1U << 4)
#define SRAM_PDN	(0xfU << 8)
#define MFG_SRAM_ACK	(1U << 12)

static int __init mt6582_mfg_power_init(void)
{
	void __iomem *spm;
	u32 v;
	int t;

	spm = ioremap(SPM_PHYS, 0x1000);
	if (!spm)
		return -ENOMEM;

	if (readl(spm + SPM_PWR_STATUS) & MFG_PWR_STA) {
		pr_info("mt6582-mfg: ya encendido\n");
		iounmap(spm);
		return 0;
	}

	writel(SPM_KEY, spm + SPM_POWERON_CFG);		/* desbloquear escrituras SPM */

	writel(readl(spm + SPM_MFG_PWR_CON) | PWR_ON, spm + SPM_MFG_PWR_CON);
	writel(readl(spm + SPM_MFG_PWR_CON) | PWR_ON_S, spm + SPM_MFG_PWR_CON);
	for (t = 0; t < 2000; t++) {
		if ((readl(spm + SPM_PWR_STATUS) & MFG_PWR_STA) &&
		    (readl(spm + SPM_PWR_STATUS_S) & MFG_PWR_STA))
			break;
		udelay(10);
	}

	v = readl(spm + SPM_MFG_PWR_CON);
	v &= ~PWR_CLK_DIS;
	v &= ~PWR_ISO;
	v |= PWR_RST_B;
	writel(v, spm + SPM_MFG_PWR_CON);

	writel(readl(spm + SPM_MFG_PWR_CON) & ~SRAM_PDN, spm + SPM_MFG_PWR_CON);
	for (t = 0; t < 2000; t++) {
		if (!(readl(spm + SPM_MFG_PWR_CON) & MFG_SRAM_ACK))
			break;
		udelay(10);
	}

	pr_info("mt6582-mfg: MFG power on (PWR_STATUS=0x%x, MFG_PWR_CON=0x%x)\n",
		readl(spm + SPM_PWR_STATUS), readl(spm + SPM_MFG_PWR_CON));
	iounmap(spm);
	return 0;
}
subsys_initcall(mt6582_mfg_power_init);
