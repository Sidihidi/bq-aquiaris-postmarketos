// SPDX-License-Identifier: GPL-2.0
/* H2: leer (READ-ONLY) el estado actual del remap BANK4 del MD + CCIF. */
#include <linux/module.h>
#include <linux/io.h>

static int __init b4_init(void)
{
	void __iomem *mcu   = ioremap(0x10200000, 0x1000);   /* MCUSYS_CFGREG */
	void __iomem *infra = ioremap(0x10001000, 0x1000);   /* INFRACFG_AO */
	void __iomem *ccif  = ioremap(0x1020A000, 0x100);    /* AP_CCIF */
	if (!mcu || !infra || !ccif) { pr_err("bank4read: ioremap fail\n"); goto out; }

	pr_info("bank4read: AP_BANK4_MAP0(0x10200200)=0x%08x  MAP1(204)=0x%08x\n",
		readl(mcu + 0x200), readl(mcu + 0x204));
	pr_info("bank4read: MD1_BANK0_MAP0(0x10001300)=0x%08x  MAP1(304)=0x%08x\n",
		readl(infra + 0x300), readl(infra + 0x304));
	pr_info("bank4read: MD1_BANK4_MAP0(0x10001308)=0x%08x  MAP1(30C)=0x%08x\n",
		readl(infra + 0x308), readl(infra + 0x30c));
	pr_info("bank4read: TOPAXI_PROT_EN(0x10001220)=0x%08x  STA1(228)=0x%08x\n",
		readl(infra + 0x220), readl(infra + 0x228));
	pr_info("bank4read: AP_CCIF CON(0x1020A000)=0x%08x  BUSY(+4)=0x%08x  START(+8)=0x%08x\n",
		readl(ccif + 0x00), readl(ccif + 0x04), readl(ccif + 0x08));
out:
	if (mcu)   iounmap(mcu);
	if (infra) iounmap(infra);
	if (ccif)  iounmap(ccif);
	return -EINVAL;   /* devuelve error para no dejarse cargado (one-shot: solo el dump) */
}
module_init(b4_init);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("H2 read-only dump BANK4 remap + CCIF");
