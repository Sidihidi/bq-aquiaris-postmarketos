// SPDX-License-Identifier: GPL-2.0
/*
 * md1-mtcmos — H1 del plan módem M1: encender el dominio MTCMOS del MD (SYS_MD1)
 * por el SPM y medir SPM_PWR_STATUS bit0. Clon EXACTO de la secuencia CONNSYS
 * (mt6582-consys.c consys_spm_power_on, PROBADA: WiFi/BT) con los regs del MD1.
 * Módulo out-of-tree cargable en caliente: NO arranca la CPU del MD (eso necesita
 * los boot-slave keys, H3) — solo alimenta el dominio. Medible sin CCCI.
 */
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

#define SPM_PHYS		0x10006000
#define INFRACFG_AO_PHYS	0x10001000

#define SPM_POWERON_CFG		0x000
#define SPM_KEY			0x0b160001
#define SPM_MD_PWR_CON		0x284		/* MD1: CONN(0x280) + 4 */
#define SPM_PWR_STATUS		0x60c
#define SPM_PWR_STATUS_S	0x610
#define MD1_PWR_STA_MASK	(1U << 0)	/* MD1 = bit0 (CONN = bit1) */

/* bits de *_PWR_CON (mismo layout que CONN/MFG) */
#define PWR_RST_B		(1U << 0)
#define PWR_ISO			(1U << 1)
#define PWR_ON			(1U << 2)
#define PWR_ON_S		(1U << 3)
#define PWR_CLK_DIS		(1U << 4)
#define MD_SRAM_PDN		(1U << 8)

#define TOPAXI_PROT_EN		0x220
#define TOPAXI_PROT_STA1	0x228
#define MD1_PROT_MASK		0x00B8

static void __iomem *spm;
static void __iomem *infra;
static struct dentry *dbg;
static int autorun;
module_param(autorun, int, 0);
MODULE_PARM_DESC(autorun, "1 = encender MD1 al cargar el modulo");

static void md1_dump(const char *tag)
{
	pr_info("md1-mtcmos: %s CON=0x%08x STA=0x%08x(md1=%d) STA_S=0x%08x(md1=%d) PROT_STA1=0x%08x(md1=%d)\n",
		tag,
		readl(spm + SPM_MD_PWR_CON),
		readl(spm + SPM_PWR_STATUS),   !!(readl(spm + SPM_PWR_STATUS)   & MD1_PWR_STA_MASK),
		readl(spm + SPM_PWR_STATUS_S), !!(readl(spm + SPM_PWR_STATUS_S) & MD1_PWR_STA_MASK),
		readl(infra + TOPAXI_PROT_STA1), !!(readl(infra + TOPAXI_PROT_STA1) & MD1_PROT_MASK));
}

static int md1_power_on(void)
{
	int t;

	writel(SPM_KEY, spm + SPM_POWERON_CFG);			/* desbloquear SPM (idempotente) */

	writel(readl(spm + SPM_MD_PWR_CON) | PWR_ON,   spm + SPM_MD_PWR_CON);
	writel(readl(spm + SPM_MD_PWR_CON) | PWR_ON_S, spm + SPM_MD_PWR_CON);

	for (t = 0; t < 2000; t++) {
		if ((readl(spm + SPM_PWR_STATUS)   & MD1_PWR_STA_MASK) &&
		    (readl(spm + SPM_PWR_STATUS_S) & MD1_PWR_STA_MASK))
			break;
		udelay(10);
	}
	if (t == 2000) {
		pr_err("md1-mtcmos: TIMEOUT MD1_PWR_STA (CON=0x%x STA=0x%x)\n",
		       readl(spm + SPM_MD_PWR_CON), readl(spm + SPM_PWR_STATUS));
		return -ETIMEDOUT;
	}

	writel(readl(spm + SPM_MD_PWR_CON) & ~PWR_CLK_DIS, spm + SPM_MD_PWR_CON);
	writel(readl(spm + SPM_MD_PWR_CON) & ~PWR_ISO,     spm + SPM_MD_PWR_CON);
	writel(readl(spm + SPM_MD_PWR_CON) | PWR_RST_B,    spm + SPM_MD_PWR_CON);
	writel(readl(spm + SPM_MD_PWR_CON) & ~MD_SRAM_PDN, spm + SPM_MD_PWR_CON);

	/* liberar proteccion de bus TOPAXI del MD1 */
	writel(readl(infra + TOPAXI_PROT_EN) & ~MD1_PROT_MASK, infra + TOPAXI_PROT_EN);
	for (t = 0; t < 2000; t++) {
		if (!(readl(infra + TOPAXI_PROT_STA1) & MD1_PROT_MASK))
			break;
		udelay(10);
	}
	udelay(10);
	pr_info("md1-mtcmos: power-on OK (prot wait=%dus)\n", t * 10);
	return 0;
}

static ssize_t md1_read(struct file *f, char __user *ub, size_t n, loff_t *p)
{
	char buf[256];
	int len = scnprintf(buf, sizeof(buf),
		"MD_PWR_CON = 0x%08x\nPWR_STATUS   md1(bit0) = %d\nPWR_STATUS_S md1(bit0) = %d\nTOPAXI_PROT_STA1 md1 = %d (0=liberado)\n",
		readl(spm + SPM_MD_PWR_CON),
		!!(readl(spm + SPM_PWR_STATUS)   & MD1_PWR_STA_MASK),
		!!(readl(spm + SPM_PWR_STATUS_S) & MD1_PWR_STA_MASK),
		!!(readl(infra + TOPAXI_PROT_STA1) & MD1_PROT_MASK));
	return simple_read_from_buffer(ub, n, p, buf, len);
}

static ssize_t md1_write(struct file *f, const char __user *ub, size_t n, loff_t *p)
{
	char c = 0;

	if (n && get_user(c, ub))
		return -EFAULT;
	if (c == '1') {
		md1_dump("pre");
		md1_power_on();
		md1_dump("post");
	}
	return n;
}

static const struct file_operations md1_fops = {
	.owner = THIS_MODULE,
	.read  = md1_read,
	.write = md1_write,
};

static int __init md1_init(void)
{
	spm   = ioremap(SPM_PHYS, 0x1000);
	infra = ioremap(INFRACFG_AO_PHYS, 0x1000);
	if (!spm || !infra) {
		if (spm)   iounmap(spm);
		if (infra) iounmap(infra);
		return -ENOMEM;
	}
	md1_dump("baseline");
	dbg = debugfs_create_dir("md1_mtcmos", NULL);
	debugfs_create_file("power", 0644, dbg, NULL, &md1_fops);
	if (autorun)
		md1_power_on();
	pr_info("md1-mtcmos: cargado. echo 1 > /sys/kernel/debug/md1_mtcmos/power\n");
	return 0;
}

static void __exit md1_exit(void)
{
	debugfs_remove_recursive(dbg);
	iounmap(spm);
	iounmap(infra);
}

module_init(md1_init);
module_exit(md1_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MT6582 MD1 MTCMOS power-on (H1 modem bring-up)");
