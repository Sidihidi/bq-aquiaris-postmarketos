// SPDX-License-Identifier: GPL-2.0
/*
 * md1-boot — H3 SCAFFOLD del bring-up del MD (CCCI boot-only) del MT6582.
 * Port del camino de arranque del downstream dual_ccci (ver H3-BRINGUP-CCCI-SPEC-0713.md).
 *
 * ⚠️⚠️  NO ESTA COMPLETO NI VALIDADO. Escribe AP_BANK4_MAP (puede corromper la DRAM del AP) y
 * suelta el MD CPU (boot-slave keys). REQUIERE: (1) nodo reserved-memory con el carveout de 24MB,
 * (2) firmware modem_1_wg_n.img en /lib/firmware/, (3) kernel RECUPERABLE. Por defecto DRY-RUN
 * (solo calcula+loguea, no escribe nada peligroso). Solo con arm=1 escribe de verdad.
 * FALTA: set_md_runtime (HS2), parse GFH robusto, EMI-MPU. Alcanza como mucho a intentar HS1.
 */
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/firmware.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>

/* --- físicos (H1/H2/H3 RE) --- */
#define MCUSYS_PHYS	0x10200000
#define INFRA_PHYS	0x10001000
#define CCIF_PHYS	0x1020A000
#define MDRGU_PHYS	0x20050000
#define BS_VECTOR_PHYS	0x20190000
#define BS_KEY_PHYS	0x2019379C
#define BS_EN_PHYS	0x20195488

#define AP_BANK4_MAP0	0x200	/* +MCUSYS */
#define AP_BANK4_MAP1	0x204
#define MD1_BANK4_MAP0	0x308	/* +INFRA */
#define MD1_BANK4_MAP1	0x30c
#define WDT_MD_MODE	0x00	/* +MDRGU */
#define WDT_MD_MODE_KEY	(0x22 << 8)

#define CCIF_CON	0x00
#define CCIF_BUSY	0x04
#define CCIF_ACK	0x14
#define CCIF_RCHNUM	0x10
#define CCIF_CON_ARB	0x01

#define KERN_EMI_BASE	0x80000000UL
#define INVALID_OFFSET	0x02000000UL
#define INVALID_ADDR	0x3E000000UL
#define BS_KEY_VAL	0x3567C766
#define BS_EN_VAL	0xA3B66175
#define FW_NAME		"modem_1_wg_n.img"

/* MD1_MEM_SIZE=22MB img, SMEM 2MB al final del carveout de 24MB */
#define MD_IMG_SIZE	(22u << 20)
#define MD_SMEM_SIZE	(2u  << 20)

static unsigned long carveout_base;	/* base física del reserved-memory (0 = refuse) */
module_param(carveout_base, ulong, 0);
static int arm;				/* 0 = dry-run (no escribe regs peligrosos ni kick) */
module_param(arm, int, 0);

static void __iomem *mcu, *infra, *ccif, *mdrgu, *bs_vec, *bs_key, *bs_en;
static int ccif_irq;
static struct dentry *dbg;

/* remap: empaquetado del downstream (set_md_smem_remap MD_SYS1). des = SMEM base. */
static u32 md_remap1(u32 des_off)
{
	return (((des_off >> 24) | 0x1) & 0xFF)
	     + ((((INVALID_ADDR + INVALID_OFFSET * 0) >> 16) | 1 << 8)  & 0xFF00)
	     + ((((INVALID_ADDR + INVALID_OFFSET * 1) >> 8)  | 1 << 16) & 0xFF0000)
	     + ((((INVALID_ADDR + INVALID_OFFSET * 2) >> 0)  | 1 << 24) & 0xFF000000);
}
static u32 md_remap2(void)
{
	return ((((INVALID_ADDR + INVALID_OFFSET * 3) >> 24) | 0x1) & 0xFF)
	     + ((((INVALID_ADDR + INVALID_OFFSET * 4) >> 16) | 1 << 8)  & 0xFF00)
	     + ((((INVALID_ADDR + INVALID_OFFSET * 5) >> 8)  | 1 << 16) & 0xFF0000)
	     + ((((INVALID_ADDR + INVALID_OFFSET * 6) >> 0)  | 1 << 24) & 0xFF000000);
}

static irqreturn_t md1_ccif_isr(int irq, void *d)
{
	u32 rch = readl(ccif + CCIF_RCHNUM);
	pr_info("md1-boot: *** CCIF IRQ *** RCHNUM=0x%08x CON=0x%08x BUSY=0x%08x  <-- el MD hablo\n",
		rch, readl(ccif + CCIF_CON), readl(ccif + CCIF_BUSY));
	writel(0xFF, ccif + CCIF_ACK);	/* ack para no re-disparar */
	return IRQ_HANDLED;
}

static int md1_do_boot(void)
{
	const struct firmware *fw;
	void __iomem *img;
	u32 smem_off = (carveout_base + MD_IMG_SIZE) - KERN_EMI_BASE;  /* SMEM offset en DRAM */
	u32 r1 = md_remap1(smem_off), r2 = md_remap2();
	int ret;

	pr_info("md1-boot: carveout=0x%lx SMEM=0x%lx  remap MD1_BANK4 = 0x%08x : 0x%08x  (arm=%d)\n",
		carveout_base, carveout_base + MD_IMG_SIZE, r1, r2, arm);

	ret = request_firmware(&fw, FW_NAME, NULL);
	if (ret) { pr_err("md1-boot: falta %s en /lib/firmware (%d)\n", FW_NAME, ret); return ret; }
	pr_info("md1-boot: firmware %s = %zu B (esperado 5100832)\n", FW_NAME, fw->size);
	/* GFH check-header al FINAL del img (H0): magic "CHECK_HEADER" */
	if (fw->size > 12)
		pr_info("md1-boot: tail16 = %*ph\n", 16, fw->data + fw->size - 16);

	if (!arm) {
		pr_info("md1-boot: DRY-RUN (arm=0): NO copio firmware, NO escribo remap/keys, NO kick.\n");
		release_firmware(fw);
		return 0;
	}

	/* ---- A PARTIR DE AQUI escribe HW real (arm=1) ---- */
	img = ioremap(carveout_base, fw->size);
	if (!img) { release_firmware(fw); return -ENOMEM; }
	memcpy_toio(img, fw->data, fw->size);
	iounmap(img);
	release_firmware(fw);

	/* remap (H2): AP + MD bank4 */
	writel(md_remap1((carveout_base + MD_IMG_SIZE)), mcu + AP_BANK4_MAP0); /* TODO: fórmula AP (INVALID*14..) */
	writel(r1, infra + MD1_BANK4_MAP0);
	writel(r2, infra + MD1_BANK4_MAP1);

	/* CCIF init: reset + ARB */
	writel(1, ccif + CCIF_CON);
	writel(0xFF, ccif + CCIF_ACK);
	writel(CCIF_CON_ARB, ccif + CCIF_CON);

	/* kick (ungate_md1): WDT off + boot-slave keys (En arranca el MD) */
	writel(WDT_MD_MODE_KEY, mdrgu + WDT_MD_MODE);
	writel(BS_KEY_VAL, bs_key);
	writel(0x0,        bs_vec);
	writel(BS_EN_VAL,  bs_en);
	pr_info("md1-boot: MD kicked. Esperando CCIF IRQ (HS1 = MD_INIT_START_BOOT)...\n");
	return 0;
}

static ssize_t boot_write(struct file *f, const char __user *ub, size_t n, loff_t *p)
{
	char c = 0;
	if (n && get_user(c, ub)) return -EFAULT;
	if (c == '1') {
		if (!carveout_base) { pr_err("md1-boot: falta carveout_base= (refuse)\n"); return -EINVAL; }
		md1_do_boot();
	}
	return n;
}
static const struct file_operations boot_fops = { .owner = THIS_MODULE, .write = boot_write };

static int __init md1b_init(void)
{
	struct device_node *np;

	mcu    = ioremap(MCUSYS_PHYS, 0x1000);
	infra  = ioremap(INFRA_PHYS, 0x1000);
	ccif   = ioremap(CCIF_PHYS, 0x100);
	mdrgu  = ioremap(MDRGU_PHYS, 0x40);
	bs_vec = ioremap(BS_VECTOR_PHYS, 0x4);
	bs_key = ioremap(BS_KEY_PHYS, 0x4);
	bs_en  = ioremap(BS_EN_PHYS, 0x4);
	if (!mcu || !infra || !ccif || !mdrgu || !bs_vec || !bs_key || !bs_en) {
		pr_err("md1-boot: ioremap fail\n"); return -ENOMEM;
	}
	/* IRQ CCIF: idealmente por DT; aquí lo dejamos para request cuando exista el nodo.
	 * request_irq del GIC_SPI 100 requiere el mapping DT (irq_of_parse_and_map). */
	np = of_find_compatible_node(NULL, NULL, "mediatek,mt6582-ccif");
	if (np) {
		ccif_irq = irq_of_parse_and_map(np, 0);
		if (ccif_irq && !request_irq(ccif_irq, md1_ccif_isr, 0, "md1-ccif", NULL))
			pr_info("md1-boot: CCIF IRQ %d registrada\n", ccif_irq);
	} else {
		pr_warn("md1-boot: sin nodo DT mediatek,mt6582-ccif -> sin IRQ (HS1 no capturable)\n");
	}

	dbg = debugfs_create_dir("md1_boot", NULL);
	debugfs_create_file("boot", 0644, dbg, NULL, &boot_fops);
	pr_info("md1-boot: cargado. carveout_base=0x%lx arm=%d. echo 1 > /sys/kernel/debug/md1_boot/boot\n",
		carveout_base, arm);
	return 0;
}
static void __exit md1b_exit(void)
{
	if (ccif_irq) free_irq(ccif_irq, NULL);
	debugfs_remove_recursive(dbg);
	iounmap(mcu); iounmap(infra); iounmap(ccif); iounmap(mdrgu);
	iounmap(bs_vec); iounmap(bs_key); iounmap(bs_en);
}
module_init(md1b_init);
module_exit(md1b_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MT6582 MD1 bring-up scaffold (H3, INCOMPLETO)");
