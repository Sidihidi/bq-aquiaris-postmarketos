// SPDX-License-Identifier: GPL-2.0
/*
 * SPM (Sleep Protocol Manager) del MT6582 — suspend profundo estilo Android.
 * Port del stock (arch/arm/mach-mt6582/mt_spm_sleep.c, PCM v35rc1 2014-03-17):
 * el microcodigo PCM (arrays C, sin blobs) corre en el uP del SPM y gestiona
 * el apagado de dominios y el self-refresh de la DDR por hardware.
 *
 * HITO 1 (este codigo): cpu_pdn=0, infra_pdn=0 — cargar el PCM, configurar
 * wakeup (EINT del PMIC = boton/RTC, y PCM_TIMER de seguridad) y WFI plano.
 * Valida firmware+kick+wake sin apagar nada critico. Registra suspend_ops:
 * "echo mem > /sys/power/state" (s2idle/freeze sigue intacto).
 *
 * Los cores 1-3 los offlinea el framework de suspend (cpu_die en platsmp =
 * aparcado en WFI; el MTCMOS off llega en el hito 2). El SPM exige TODOS los
 * cores en WFI (COREx_WFI_SEL) para ejecutar el vector de sleep.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/suspend.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/cpu_pm.h>
#include <asm/suspend.h>
#include <asm/cacheflush.h>
#include <asm/cp15.h>

/* offsets sobre 0x10006000 (mt_spm.h del downstream) */
#define SPM_POWERON_CONFIG_SET		0x0000
#define SPM_POWER_ON_VAL0		0x0010
#define SPM_POWER_ON_VAL1		0x0014
#define SPM_CLK_SETTLE			0x0100
#define SPM_PCM_CON0			0x0310
#define SPM_PCM_CON1			0x0314
#define SPM_PCM_IM_PTR			0x0318
#define SPM_PCM_IM_LEN			0x031c
#define SPM_PCM_REG_DATA_INI		0x0320
#define SPM_PCM_EVENT_VECTOR0		0x0340
#define SPM_PCM_EVENT_VECTOR1		0x0344
#define SPM_PCM_EVENT_VECTOR2		0x0348
#define SPM_PCM_EVENT_VECTOR3		0x034c
#define SPM_PCM_MAS_PAUSE_MASK		0x0354
#define SPM_PCM_PWR_IO_EN		0x0358
#define SPM_PCM_TIMER_VAL		0x035c
#define SPM_PCM_TIMER_OUT		0x0360
#define SPM_PCM_REG7_DATA		0x039c
#define SPM_PCM_REG9_DATA		0x03a4
#define SPM_PCM_REG12_DATA		0x03b0
#define SPM_PCM_REG13_DATA		0x03b4
#define SPM_PCM_EVENT_REG_STA		0x03c0
#define SPM_PCM_EVENT_VECTOR4		0x03d0
#define SPM_PCM_EVENT_VECTOR5		0x03d4
#define SPM_PCM_EVENT_VECTOR6		0x03d8
#define SPM_PCM_EVENT_VECTOR7		0x03dc
#define SPM_PCM_SW_INT_CLEAR		0x03e4
#define SPM_CLK_CON			0x0400
#define SPM_APMCU_PWRCTL		0x0600
#define SPM_AP_STANBY_CON		0x0608
#define SPM_SLEEP_WAKEUP_EVENT_MASK	0x0810
#define SPM_SLEEP_CPU_WAKEUP_EVENT	0x0814
#define SPM_PCM_WDT_TIMER_VAL		0x0824
#define SPM_SLEEP_ISR_MASK		0x0900
#define SPM_SLEEP_ISR_STATUS		0x0904
#define SPM_SLEEP_ISR_RAW_STA		0x0910
#define SPM_PCM_SRC_REQ			0x0b04
#define SPM_CORE0_WFI_SEL		0x0f00
#define SPM_CORE1_WFI_SEL		0x0f04
#define SPM_CORE2_WFI_SEL		0x0f08
#define SPM_CORE3_WFI_SEL		0x0f0c

#define SPM_PROJECT_CODE		0xb16
#define CON0_PCM_KICK			BIT(0)
#define CON0_IM_KICK			BIT(1)
#define CON0_IM_SLEEP_DVS		BIT(3)
#define CON0_PCM_SW_RESET		BIT(15)
#define CON0_CFG_KEY			(SPM_PROJECT_CODE << 16)
#define CON1_IM_SLAVE			BIT(0)
#define CON1_MIF_APBEN			BIT(3)
#define CON1_PCM_TIMER_EN		BIT(5)
#define CON1_IM_NONRP_EN		BIT(6)
#define CON1_PCM_WDT_EN			BIT(8)
#define CON1_PCM_WDT_WAKE_MODE		BIT(9)
#define CON1_SPM_SRAM_SLP_B		BIT(10)
#define CON1_SPM_SRAM_ISO_B		BIT(11)
#define CON1_CFG_KEY			(SPM_PROJECT_CODE << 16)
#define PCM_PWRIO_EN_R0			BIT(0)
#define PCM_PWRIO_EN_R7			BIT(7)
#define PCM_RF_SYNC_R0			BIT(16)
#define PCM_RF_SYNC_R7			BIT(23)
#define R7_UART_CLK_OFF_REQ		BIT(0)
#define R13_UART_CLK_OFF_ACK		BIT(20)
#define CC_SYSCLK0_EN_0			BIT(0)
#define CC_SYSSETTLE_SEL		BIT(4)
#define CC_LOCK_INFRA_DCM		BIT(5)
#define CC_CXO32K_RM_EN_MD		BIT(9)
#define CC_DISABLE_DORM_PWR		BIT(14)
#define CC_DISABLE_INFRA_PWR		BIT(15)
#define PCM_SW_INT_ALL			0xf
#define ISR_TWAM			BIT(2)
#define ISR_PCM_RETURN			BIT(3)
#define ISR_PCM_IRQ0			BIT(8)
#define ISR_PCM_IRQ1			BIT(9)
#define ISR_PCM_IRQ2			BIT(10)
#define ISR_PCM_IRQ3			BIT(11)
#define ISRM_PCM_IRQ_AUX		(ISR_PCM_IRQ3 | ISR_PCM_IRQ2 | ISR_PCM_IRQ1)
#define ISRM_ALL_EXC_TWAM		(ISR_PCM_IRQ3 | ISR_PCM_IRQ2 | ISR_PCM_IRQ1 | \
					 ISR_PCM_IRQ0 | ISR_PCM_RETURN)
#define ISRM_ALL			(ISRM_ALL_EXC_TWAM | ISR_TWAM)
#define ISRC_ALL_EXC_TWAM		ISR_PCM_RETURN
#define ISRC_ALL			(ISR_PCM_RETURN | ISR_TWAM)

#define WAKE_SRC_KP			BIT(2)
#define WAKE_SRC_EINT			BIT(5)
#define WAKE_SRC_CONN_WDT		BIT(6)
#define WAKE_SRC_CONN			BIT(10)
#define WAKE_SRC_THERM			BIT(21)
#define WAKE_SRC_SYSPWREQ		BIT(24)

#define EVENT_VEC(event, resume, imme, pc) \
	(((event) << 0) | ((resume) << 5) | ((imme) << 6) | ((pc) << 16))

#define SPM_SYSCLK_SETTLE		99		/* ~3 ms */
#define PCM_WDT_TIMEOUT			(30 * 32768)	/* 30 s */
#define PCM_TIMER_MAX_FOR_WDT		(0xffffffff - PCM_WDT_TIMEOUT)

/* wakeup del hito 1: boton/RTC (EINT PMIC), teclado, CONNSYS, termico */
#define WAKE_SRC_FOR_SUSPEND	(WAKE_SRC_KP | WAKE_SRC_EINT | WAKE_SRC_CONN_WDT | \
				 WAKE_SRC_CONN | WAKE_SRC_THERM | WAKE_SRC_SYSPWREQ)

static u32 spm_wake_sec = 600;	/* wake periodico de seguridad (0 = off) */
module_param(spm_wake_sec, uint, 0644);

/* M3: apagar tambien CPU0 (dormant por MTCMOS; resume via BootROM ->
 * vector 0x10001800 -> cpu_resume). 0 = M1 (WFI plano, CPU0 encendido). */
static u32 spm_cpu_pdn;		/* 0 por defecto: M3 en debug (crasheo 0709) */
module_param(spm_cpu_pdn, uint, 0644);

/* microcodigo PCM de suspend, v35rc1 @ 2014-03-17 (verbatim del stock;
 * el tail incluye el programa "normal" post-wake) */
static const u32 pcm_suspend_fw[] = {
	0x19c0001f, 0x001c4bd7, 0x1800001f, 0x17cf0f3f, 0x1b80001f, 0x20000000,
	0x1800001f, 0x17cf0f16, 0x19c0001f, 0x001c4be7, 0xd80002c6, 0x17c07c1f,
	0x18c0001f, 0x10006234, 0xc0c012e0, 0x1200041f, 0x18c0001f, 0x10006240,
	0xe0e00f16, 0xe0e00f1e, 0xe0e00f0e, 0xe0e00f0f, 0xc0c01c00, 0x10c0041f,
	0x1b00001f, 0x7fffd7ff, 0xf0000000, 0x17c07c1f, 0x1b00001f, 0x3fffc7ff,
	0x1b80001f, 0x20000004, 0xd800072c, 0x17c07c1f, 0xc0c01c00, 0x10c07c1f,
	0xd80005e6, 0x17c07c1f, 0x18c0001f, 0x10006240, 0xe0e00f0f, 0xe0e00f1e,
	0xe0e00f12, 0x18c0001f, 0x10006234, 0xc0c014c0, 0x17c07c1f, 0x1b00001f,
	0x3fffcfff, 0x19c0001f, 0x001c6bd7, 0x1800001f, 0x17cf0f3f, 0x1800001f,
	0x17ff0f3f, 0x19c0001f, 0x001823d7, 0xf0000000, 0x17c07c1f, 0x18c0001f,
	0x10006294, 0xc0c01560, 0x17c07c1f, 0x1800001f, 0x07cf0f1e, 0x1b80001f,
	0x20000a50, 0x1800001f, 0x07ce0f1e, 0x1b80001f, 0x20000300, 0x1800001f,
	0x078e0f1e, 0x1b80001f, 0x20000300, 0x1800001f, 0x038e0f1e, 0x1b80001f,
	0x20000300, 0x1800001f, 0x038e0e1e, 0x1800001f, 0x038e0e12, 0x1b80001f,
	0x200000ed, 0x18c0001f, 0x10006240, 0xe0e00f0d, 0x1b80001f, 0x2000000e,
	0x19c0001f, 0x000c4ba7, 0x19c0001f, 0x000c4ba5, 0xe8208000, 0x10006354,
	0xfffffa43, 0x19c0001f, 0x000d4ba5, 0x1b00001f, 0xbfffc7ff, 0xf0000000,
	0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x8880000d, 0x00000024, 0x1b00001f,
	0xbfffc7ff, 0xd80012a2, 0x17c07c1f, 0x1b00001f, 0x3fffc7ff, 0x1b80001f,
	0x20000004, 0xd80012ac, 0x17c07c1f, 0xe8208000, 0x10006354, 0xffffffff,
	0x19c0001f, 0x001c4be5, 0x1880001f, 0x10006320, 0xc0c01820, 0xe080000f,
	0xd80012a3, 0x17c07c1f, 0xe080001f, 0xc0c01940, 0x17c07c1f, 0x18c0001f,
	0x10006294, 0xe0f07ff0, 0xe0e00ff0, 0xe0e000f0, 0xe8208000, 0x10006294,
	0x000f00f0, 0x1800001f, 0x038e0e16, 0x1800001f, 0x038e0f16, 0x1800001f,
	0x07ce0f16, 0x1800001f, 0x17cf0f16, 0x1b00001f, 0x7fffd7ff, 0xf0000000,
	0x17c07c1f, 0xe0e00f16, 0x1380201f, 0xe0e00f1e, 0x1380201f, 0xe0e00f0e,
	0x1b80001f, 0x20000100, 0xe0e00f0f, 0xe0e00f0d, 0xe0e00e0d, 0xe0e00c0d,
	0xe0e0080d, 0xe0e0000d, 0xf0000000, 0x17c07c1f, 0xe0e00f0d, 0xe0e00f1e,
	0xe0e00f12, 0xf0000000, 0x17c07c1f, 0xe8208000, 0x10006294, 0x000e00f0,
	0xe8208000, 0x10006294, 0x000c00f0, 0xe8208000, 0x10006294, 0x000800f0,
	0xe8208000, 0x10006294, 0x000000f0, 0xe0e008f0, 0xe0e00cf0, 0xe0e00ef0,
	0xe0e00ff0, 0x1b80001f, 0x20000100, 0xe0f07ff0, 0xe0f07f00, 0xf0000000,
	0x17c07c1f, 0xa1d08407, 0x1b80001f, 0x200000ed, 0x80eab401, 0x1a00001f,
	0x10006814, 0xe2000003, 0xf0000000, 0x17c07c1f, 0x18c0001f, 0x80000000,
	0x1a10001f, 0x10002058, 0x1a80001f, 0x10002058, 0xa2000c08, 0xe2800008,
	0x1a10001f, 0x1000206c, 0x1a80001f, 0x1000206c, 0xa2000c08, 0xe2800008,
	0x1a10001f, 0x10002080, 0x1a80001f, 0x10002080, 0xa2000c08, 0xe2800008,
	0xf0000000, 0x17c07c1f, 0x1a00001f, 0x10006604, 0xd8001d23, 0x17c07c1f,
	0xe2200006, 0x1b80001f, 0x20000020, 0xd8201d83, 0x17c07c1f, 0xe2200005,
	0x1b80001f, 0x20000020, 0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
	0xa1d48407, 0x1990001f, 0x10006400, 0x1a40001f, 0x11008000, 0x1b00001f,
	0x3fffc7ff, 0x1b80001f, 0xd00f0000, 0x8880000c, 0x3fffc7ff, 0xd8003fc2,
	0x1140041f, 0xe8208000, 0x10006354, 0xfffffa43, 0xc0c03b00, 0x81471801,
	0xd80025c5, 0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200,
	0xc0c03c40, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001, 0x1b80001f,
	0x20000080, 0xc0c03c40, 0x1280041f, 0x18c0001f, 0x10006208, 0xc0c03c40,
	0x12807c1f, 0xe8208000, 0x10006248, 0x00000000, 0x1b80001f, 0x20000080,
	0xc0c03c40, 0x1280041f, 0xc0c03ba0, 0x81879801, 0x1b00001f, 0xffffdfff,
	0x1b80001f, 0x90010000, 0x8880000c, 0x3fffc7ff, 0xd80039e2, 0x17c07c1f,
	0x8880000c, 0x40000800, 0xd8002602, 0x17c07c1f, 0x19c0001f, 0x00044b25,
	0x1880001f, 0x10006320, 0xe8208000, 0x10006354, 0xffffffff, 0xc0c01820,
	0xe080000f, 0xd8002603, 0x17c07c1f, 0xe8208000, 0x10006310, 0x0b1600f8,
	0xe080001f, 0x19c0001f, 0x001c4be7, 0x1b80001f, 0x20000030, 0xc0c01940,
	0x17c07c1f, 0xd8002ae6, 0x17c07c1f, 0x18c0001f, 0x10006240, 0xc0c014c0,
	0x17c07c1f, 0x18c0001f, 0x10006294, 0xe0f07ff0, 0xe0e00ff0, 0xe0e000f0,
	0xe8208000, 0x10006294, 0x000f00f0, 0x1800001f, 0x00000036, 0x1800001f,
	0x00000f36, 0x1800001f, 0x07c00f36, 0x1800001f, 0x17cf0f36, 0xc0c01c00,
	0x10c07c1f, 0xd8002de6, 0x17c07c1f, 0x18c0001f, 0x10006234, 0xc0c014c0,
	0x17c07c1f, 0x19c0001f, 0x001c6bd7, 0x1800001f, 0x17cf0f3f, 0x1800001f,
	0x17ff0f3f, 0x19c0001f, 0x001823d7, 0x1b00001f, 0x3fffcfff, 0x1b80001f,
	0x90100000, 0x80c00400, 0xd8003043, 0x80980400, 0xd8003342, 0x17c07c1f,
	0xd8203802, 0x17c07c1f, 0x19c0001f, 0x001c4bd7, 0x1800001f, 0x17cf0f3f,
	0x1b80001f, 0x20000000, 0x1800001f, 0x17cf0f16, 0x19c0001f, 0x001c4be7,
	0xd8003246, 0x17c07c1f, 0x18c0001f, 0x10006234, 0xc0c012e0, 0x1200041f,
	0xd8003346, 0x17c07c1f, 0x18c0001f, 0x10006240, 0xe0e00f16, 0xe0e00f1e,
	0xe0e00f0e, 0xe0e00f0f, 0x18c0001f, 0x10006294, 0xc0c01560, 0x17c07c1f,
	0xc0c01c00, 0x10c0041f, 0x19c0001f, 0x001c4ba7, 0x1800001f, 0x07cf0f16,
	0x1b80001f, 0x20000a50, 0x1800001f, 0x07c00f16, 0x1b80001f, 0x20000300,
	0x1800001f, 0x04000f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000f16,
	0x1b80001f, 0x20000300, 0x1800001f, 0x00000016, 0x10007c1f, 0x1b80001f,
	0x2000049c, 0x1b80001f, 0x200000ed, 0x18c0001f, 0x10006240, 0xe0e00f0d,
	0x1b80001f, 0x2000000e, 0xd00039a0, 0x17c07c1f, 0x1800001f, 0x03800e12,
	0x1b80001f, 0x20000300, 0x1800001f, 0x00000e12, 0x1b80001f, 0x20000300,
	0x1800001f, 0x00000012, 0x10007c1f, 0x1b80001f, 0x2000079e, 0x19c0001f,
	0x00054b25, 0xe8208000, 0x10006354, 0xfffffa43, 0x19c0001f, 0x00014b25,
	0x19c0001f, 0x00014a25, 0xd0003fc0, 0x17c07c1f, 0xa1d10407, 0x1b80001f,
	0x20000020, 0xf0000000, 0x17c07c1f, 0xa1d40407, 0x1391841f, 0xa1d90407,
	0xf0000000, 0x17c07c1f, 0xd8003cca, 0x17c07c1f, 0xe2e0006d, 0xe2e0002d,
	0xd8203d6a, 0x17c07c1f, 0xe2e0002f, 0xe2e0003e, 0xe2e00032, 0xf0000000,
	0x17c07c1f, 0xd0003fc0, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0xd8004345, 0x17c07c1f, 0x18c0001f, 0x10006208,
	0x1212841f, 0xc0c04860, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000001,
	0x1b80001f, 0x20000080, 0xc0c04860, 0x1280041f, 0x18c0001f, 0x10006200,
	0x1212841f, 0xc0c04860, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000000,
	0x1b80001f, 0x20000080, 0xc0c04860, 0x1280041f, 0x19c0001f, 0x00415820,
	0x1ac0001f, 0x55aa55aa, 0x10007c1f, 0x80cab001, 0x808cb401, 0x80800c02,
	0xd82044a2, 0x17c07c1f, 0xa1d78407, 0x1240301f, 0xe8208000, 0x100063e0,
	0x00000001, 0x1b00001f, 0x00202000, 0x1b80001f, 0x80001000, 0x8880000c,
	0x00200000, 0xd80046c2, 0x17c07c1f, 0xe8208000, 0x100063e0, 0x00000002,
	0x1b80001f, 0x00001000, 0x809c840d, 0xd8204522, 0x17c07c1f, 0xa1d78407,
	0x1890001f, 0x10006014, 0x18c0001f, 0x10006014, 0xa0978402, 0xe0c00002,
	0x1b80001f, 0x00001000, 0xf0000000, 0xd800496a, 0x17c07c1f, 0xe2e00036,
	0x1380201f, 0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f, 0xd8204a6a,
	0x17c07c1f, 0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020,
	0xe2e0004d, 0xf0000000, 0x17c07c1f
};
#define PCM_SUSPEND_SIZE	597	/* palabras validas (como el stock) */

struct mt6582_spm {
	struct device *dev;
	void __iomem *base;
	void __iomem *bootvec;	/* INFRACFG_AO+0x800: vector de resume del BootROM */
	u32 *fw;		/* copia coherente (el IM la lee por DMA) */
	dma_addr_t fw_phys;
};

static struct mt6582_spm *gspm;

static inline u32 spm_r(struct mt6582_spm *s, u32 off)
{
	return readl(s->base + off);
}

static inline void spm_w(struct mt6582_spm *s, u32 off, u32 val)
{
	writel(val, s->base + off);
	/* el stock usa sync-write (dsb tras cada write) */
	dsb(sy);
}

static irqreturn_t mt6582_spm_irq(int irq, void *data)
{
	struct mt6582_spm *s = data;

	spm_w(s, SPM_SLEEP_ISR_STATUS, ISRC_ALL);
	return IRQ_HANDLED;
}

static void spm_hw_init(struct mt6582_spm *s)
{
	spm_w(s, SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | 1);
	spm_w(s, SPM_POWER_ON_VAL0, 0);
	spm_w(s, SPM_POWER_ON_VAL1, 0x00015820);
	spm_w(s, SPM_PCM_PWR_IO_EN, 0);
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY);
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);
	spm_w(s, SPM_PCM_CON1, CON1_CFG_KEY | CON1_SPM_SRAM_ISO_B |
	       CON1_SPM_SRAM_SLP_B | CON1_IM_NONRP_EN | CON1_MIF_APBEN);
	spm_w(s, SPM_PCM_IM_PTR, 0);
	spm_w(s, SPM_PCM_IM_LEN, 0);
	spm_w(s, SPM_CLK_CON, CC_SYSCLK0_EN_0 | CC_CXO32K_RM_EN_MD);
	spm_w(s, SPM_PCM_SRC_REQ, 0);
	spm_w(s, SPM_SLEEP_WAKEUP_EVENT_MASK, 0xffffffff);
	spm_w(s, SPM_SLEEP_ISR_MASK, ISRM_ALL);
	spm_w(s, SPM_SLEEP_ISR_STATUS, ISRC_ALL);
	spm_w(s, SPM_PCM_SW_INT_CLEAR, PCM_SW_INT_ALL);
}

static int mt6582_spm_finisher(unsigned long arg)
{
	/* corre tras el save de cpu_suspend. CLAVE (crash 0709): hay que
	 * DESHABILITAR la D-cache antes del WFI final — flush con cache
	 * activa deja sucios los pushes de stack posteriores y el MTCMOS
	 * los pierde (el downstream hace __disable_dcache()). El macro
	 * canonico lo hace todo en asm sin tocar stack: C-bit off + flush
	 * L1/L2 a PoC + salir de la coherencia SMP (ACTLR). */
	v7_exit_coherency_flush(all);
	wfi();
	return 1;
}

static int spm_suspend_enter(suspend_state_t state)
{
	struct mt6582_spm *s = gspm;
	u32 con0, con1, clk, isr, val1, timer;
	u32 dbg, r12, r13, raw, tout, evsta;
	int i;

	/* settle del 26M */
	spm_w(s, SPM_CLK_CON, spm_r(s, SPM_CLK_CON) | CC_SYSSETTLE_SEL);
	spm_w(s, SPM_CLK_SETTLE, 0);
	spm_w(s, SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE - spm_r(s, SPM_CLK_SETTLE));

	/* reset + init del PCM */
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY);
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);
	con1 = spm_r(s, SPM_PCM_CON1) & (CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
	spm_w(s, SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_SPM_SRAM_ISO_B |
	       CON1_SPM_SRAM_SLP_B | CON1_IM_NONRP_EN | CON1_MIF_APBEN);

	/* IM fetch del microcodigo */
	spm_w(s, SPM_PCM_IM_PTR, (u32)s->fw_phys);
	spm_w(s, SPM_PCM_IM_LEN, PCM_SUSPEND_SIZE - 1);
	con0 = spm_r(s, SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
	spm_w(s, SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_IM_KICK);
	spm_w(s, SPM_PCM_CON0, con0 | CON0_CFG_KEY);

	/* handshake UART a dormir */
	val1 = spm_r(s, SPM_POWER_ON_VAL1);
	spm_w(s, SPM_POWER_ON_VAL1, val1 | R7_UART_CLK_OFF_REQ);
	for (i = 0; !(spm_r(s, SPM_PCM_REG13_DATA) & R13_UART_CLK_OFF_ACK); i++) {
		if (i >= 10) {
			spm_w(s, SPM_POWER_ON_VAL1, val1);
			dev_err(s->dev, "sin ACK de UART a dormir\n");
			return -EBUSY;
		}
		udelay(10);
	}

	/* r0/r7 desde POWER_ON_VAL0/1 */
	spm_w(s, SPM_PCM_REG_DATA_INI, spm_r(s, SPM_POWER_ON_VAL0));
	spm_w(s, SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
	spm_w(s, SPM_PCM_PWR_IO_EN, 0);
	spm_w(s, SPM_PCM_REG_DATA_INI, spm_r(s, SPM_POWER_ON_VAL1));
	spm_w(s, SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	spm_w(s, SPM_PCM_PWR_IO_EN, 0);
	spm_w(s, SPM_PCM_REG_DATA_INI, 0);

	/* vectores de evento (v35rc1) */
	spm_w(s, SPM_PCM_EVENT_VECTOR0, EVENT_VEC(11, 1, 0, 0));
	spm_w(s, SPM_PCM_EVENT_VECTOR1, EVENT_VEC(12, 1, 0, 28));
	spm_w(s, SPM_PCM_EVENT_VECTOR2, EVENT_VEC(30, 1, 0, 59));
	spm_w(s, SPM_PCM_EVENT_VECTOR3, EVENT_VEC(31, 1, 0, 103));
	spm_w(s, SPM_PCM_EVENT_VECTOR4, 0);
	spm_w(s, SPM_PCM_EVENT_VECTOR5, 0);
	spm_w(s, SPM_PCM_EVENT_VECTOR6, 0);
	spm_w(s, SPM_PCM_EVENT_VECTOR7, 0);

	/* pwrctl: WFI de cada core enrutado al SPM */
	spm_w(s, SPM_APMCU_PWRCTL, 0);
	spm_w(s, SPM_AP_STANBY_CON, (0x3 << 19) | (1U << 4));
	spm_w(s, SPM_CORE0_WFI_SEL, 1);
	spm_w(s, SPM_CORE1_WFI_SEL, 1);
	spm_w(s, SPM_CORE2_WFI_SEL, 1);
	spm_w(s, SPM_CORE3_WFI_SEL, 1);

	/* wakeup: PCM_TIMER de seguridad + fuentes reales */
	timer = spm_wake_sec ? spm_wake_sec * 32768 : PCM_TIMER_MAX_FOR_WDT;
	spm_w(s, SPM_PCM_TIMER_VAL, timer);
	spm_w(s, SPM_PCM_CON1, spm_r(s, SPM_PCM_CON1) | CON1_CFG_KEY |
	       CON1_PCM_TIMER_EN);
	spm_w(s, SPM_SLEEP_WAKEUP_EVENT_MASK, ~WAKE_SRC_FOR_SUSPEND);
	isr = spm_r(s, SPM_SLEEP_ISR_MASK) & ISR_TWAM;
	spm_w(s, SPM_SLEEP_ISR_MASK, isr | ISRM_PCM_IRQ_AUX);

	/* kick: M3 = apagar CPU (dorm) si spm_cpu_pdn; INFRA sigue ON (M4).
	 * WDT del PCM en modo WAKE (sin el RGU configurado, un reset del PCM
	 * no nos resetearia) */
	clk = spm_r(s, SPM_CLK_CON) & ~(CC_DISABLE_DORM_PWR | CC_DISABLE_INFRA_PWR);
	if (!spm_cpu_pdn)
		clk |= CC_DISABLE_DORM_PWR;
	clk |= CC_DISABLE_INFRA_PWR;
	spm_w(s, SPM_CLK_CON, clk | CC_LOCK_INFRA_DCM);
	spm_w(s, SPM_PCM_MAS_PAUSE_MASK, 0xffffffff);
	spm_w(s, SPM_PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);
	con1 = spm_r(s, SPM_PCM_CON1) & ~(CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
	spm_w(s, SPM_PCM_CON1, con1 | CON1_CFG_KEY);
	spm_w(s, SPM_PCM_WDT_TIMER_VAL, spm_r(s, SPM_PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
	spm_w(s, SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_PCM_WDT_EN |
	       CON1_PCM_WDT_WAKE_MODE);
	con0 = spm_r(s, SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
	spm_w(s, SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_PCM_KICK);
	spm_w(s, SPM_PCM_CON0, con0 | CON0_CFG_KEY);

	/* dormir: M3 = apagar CPU0 (dormant); el BootROM resucita por el
	 * vector 0x10001800 -> cpu_resume (mainline restaura MMU/contexto;
	 * los notifiers de CPU_PM salvan GIC/VFP/arch-timer) */
	if (spm_cpu_pdn && s->bootvec) {
		/* activar el salto caliente SOLO durante el ciclo: con bit31
		 * activo, CUALQUIER core que pase por el BootROM salta al
		 * vector — si quedara activo tras el resume (con el vector ya
		 * a 0), los cores 1-3 re-onlineando saltarian a 0 = -5 o
		 * corrupcion (LOS DOS flecos del 0709) */
		writel(__pa_symbol(cpu_resume), s->bootvec);
		writel(readl(s->bootvec + 4) | BIT(31), s->bootvec + 4);
		dsb(sy);
		cpu_pm_enter();
		/* CLUSTER pm: dispara gic_dist_save/restore — sin esto el
		 * distribuidor del GIC vuelve EN BLANCO tras el MTCMOS del
		 * cluster (crash 2 del 0709: resume sin IRQs -> watchdog) */
		cpu_cluster_pm_enter();
		cpu_suspend(0, mt6582_spm_finisher);
		cpu_cluster_pm_exit();
		cpu_pm_exit();
		writel(readl(s->bootvec + 4) & ~BIT(31), s->bootvec + 4);
		writel(0, s->bootvec);
		dsb(sy);
	} else {
		isb();
		dsb(sy);
		wfi();
	}

	/* estado del wake */
	dbg = spm_r(s, SPM_PCM_REG_DATA_INI);
	r12 = spm_r(s, SPM_PCM_REG9_DATA);
	raw = spm_r(s, SPM_SLEEP_ISR_RAW_STA);
	tout = spm_r(s, SPM_PCM_TIMER_OUT);
	evsta = spm_r(s, SPM_PCM_EVENT_REG_STA);
	r13 = spm_r(s, SPM_PCM_REG13_DATA);

	/* limpieza post-wake */
	con1 = spm_r(s, SPM_PCM_CON1);
	spm_w(s, SPM_PCM_CON1, CON1_CFG_KEY | (con1 & ~CON1_PCM_WDT_EN));
	spm_w(s, SPM_POWER_ON_VAL1, spm_r(s, SPM_POWER_ON_VAL1) & ~R7_UART_CLK_OFF_REQ);
	spm_w(s, SPM_PCM_PWR_IO_EN, 0);
	spm_w(s, SPM_CLK_CON, spm_r(s, SPM_CLK_CON) & ~CC_LOCK_INFRA_DCM);
	spm_w(s, SPM_PCM_CON1, CON1_CFG_KEY |
	       (spm_r(s, SPM_PCM_CON1) & ~CON1_PCM_TIMER_EN));
	spm_w(s, SPM_SLEEP_CPU_WAKEUP_EVENT, 0);
	spm_w(s, SPM_SLEEP_WAKEUP_EVENT_MASK, ~WAKE_SRC_THERM);
	spm_w(s, SPM_SLEEP_ISR_MASK, spm_r(s, SPM_SLEEP_ISR_MASK) | ISRM_ALL_EXC_TWAM);
	spm_w(s, SPM_SLEEP_ISR_STATUS, ISRC_ALL_EXC_TWAM);
	spm_w(s, SPM_PCM_SW_INT_CLEAR, BIT(0));

	if (dbg)
		dev_err(s->dev, "PCM ASSERT: PC=%u r13=0x%x evsta=0x%x\n",
			dbg, r13, evsta);
	else
		dev_info(s->dev,
			 "wake: r12=0x%x%s%s%s raw=0x%x t=%u r13=0x%x ev=0x%x\n",
			 r12,
			 (r12 & WAKE_SRC_EINT) ? " EINT" : "",
			 (r12 & WAKE_SRC_KP) ? " KP" : "",
			 (r12 & BIT(0)) ? " TIMER" : "",
			 raw, tout, r13, evsta);
	return 0;
}

static int spm_suspend_valid(suspend_state_t state)
{
	return state == PM_SUSPEND_MEM;
}

static const struct platform_suspend_ops mt6582_spm_suspend_ops = {
	.valid = spm_suspend_valid,
	.enter = spm_suspend_enter,
};

static int mt6582_spm_probe(struct platform_device *pdev)
{
	struct mt6582_spm *s;
	int irq, ret;

	s = devm_kzalloc(&pdev->dev, sizeof(*s), GFP_KERNEL);
	if (!s)
		return -ENOMEM;
	s->dev = &pdev->dev;

	s->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(s->base))
		return PTR_ERR(s->base);

	/* copia coherente del microcodigo: el IM del SPM lo lee por DMA */
	s->fw = dmam_alloc_coherent(&pdev->dev, PCM_SUSPEND_SIZE * 4,
				    &s->fw_phys, GFP_KERNEL);
	if (!s->fw)
		return -ENOMEM;
	memcpy(s->fw, pcm_suspend_fw, PCM_SUSPEND_SIZE * 4);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;
	ret = devm_request_irq(&pdev->dev, irq, mt6582_spm_irq,
			       IRQF_NO_SUSPEND, "mt6582-spm", s);
	if (ret)
		return ret;

	/* vector de resume del BootROM: INFRACFG_AO+0x800 (addr) y +0x804
	 * (bit31 = habilitar el salto). Lo usa el dormant de CPU0 (M3). */
	s->bootvec = ioremap(0x10001800, 0x8);
	if (!s->bootvec)
		dev_warn(&pdev->dev, "sin bootvec: M3 (cpu_pdn) deshabilitado\n");

	spm_hw_init(s);
	gspm = s;
	suspend_set_ops(&mt6582_spm_suspend_ops);

	dev_info(&pdev->dev,
		 "SPM listo (PCM v35rc1 %u palabras @%pad) — 'mem' = SPM suspend (hito 1: WFI plano)\n",
		 (u32)PCM_SUSPEND_SIZE, &s->fw_phys);
	return 0;
}

static const struct of_device_id mt6582_spm_of_match[] = {
	{ .compatible = "mediatek,mt6582-spm" },
	{ }
};

static struct platform_driver mt6582_spm_driver = {
	.probe = mt6582_spm_probe,
	.driver = {
		.name = "mt6582-spm",
		.of_match_table = mt6582_spm_of_match,
	},
};
builtin_platform_driver(mt6582_spm_driver);
