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
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/kthread.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/tty_port.h>

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

/* --- Hito M1 (arranque del modem): MTCMOS del dominio del MD --- */
#define SPM_MD_PWR_CON		0x0284
#define SPM_MD_PWR_STATUS	0x060c
#define SPM_MD_PWR_STATUS_S	0x0610
#define MD_PWR_RST_B		BIT(0)
#define MD_PWR_ISO		BIT(1)
#define MD_PWR_ON		BIT(2)
#define MD_PWR_ON_S		BIT(3)
#define MD_PWR_CLK_DIS		BIT(4)
#define MD_SRAM_PDN		BIT(8)
#define MD1_PWR_STA_MASK	BIT(0)
#define MD1_PROT_MASK		0x00b8
#define INFRA_TOPAXI_PROT_EN	0x0220
#define INFRA_TOPAXI_PROT_STA1	0x0228

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

/* M4: apagar tambien INFRA/DDRPHY (DDR en self-refresh por HW, 26M fuera =
 * el suspend profundo completo del stock). Requiere spm_cpu_pdn=1. Default 0
 * hasta validar; los perifericos pierden estado (musb ya tiene rebind). */
static u32 spm_infra_pdn;
module_param(spm_infra_pdn, uint, 0644);

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
	void __iomem *mcusys;	/* MCUSYS_CFGREG 0x10200000 (CA7_CACHE_CONFIG @0) */
	u32 l2ctlr_boot;	/* L2CTLR del LK (latencias L2) — se pierde con el
				 * MTCMOS del cluster; el cpu_wake_up stock lo repone */
	u32 cacfg_boot;		/* CA7_CACHE_CONFIG del LK */
	u32 *fw;		/* copia coherente (el IM la lee por DMA) */
	dma_addr_t fw_phys;
};

static inline u32 read_l2ctlr(void)
{
	u32 v;

	asm volatile("mrc p15, 1, %0, c9, c0, 2" : "=r"(v));
	return v;
}

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

	pr_info("SPMDBG spm: enter\n");	/* bisect ciclo-5: ¿llega al driver? */

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

	pr_info("SPMDBG spm: im-fetch ok\n");

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
	if (!(spm_infra_pdn && spm_cpu_pdn))	/* M4 solo sobre M3 */
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
	pr_info("SPMDBG spm: kick ok, a dormir\n");

	if (spm_cpu_pdn && s->bootvec) {
		/* activar el salto caliente SOLO durante el ciclo: con bit31
		 * activo, CUALQUIER core que pase por el BootROM salta al
		 * vector — si quedara activo tras el resume (con el vector ya
		 * a 0), los cores 1-3 re-onlineando saltarian a 0 = -5 o
		 * corrupcion (LOS DOS flecos del 0709) */
		writel(__pa_symbol(cpu_resume), s->bootvec);
		writel(readl(s->bootvec + 4) | BIT(31), s->bootvec + 4);
		dsb(sy);
		pr_info("SPMDBG spm: pre-sleep cpu_pm_enter\n");
		cpu_pm_enter();
		/* CLUSTER pm: dispara gic_dist_save/restore — sin esto el
		 * distribuidor del GIC vuelve EN BLANCO tras el MTCMOS del
		 * cluster (crash 2 del 0709: resume sin IRQs -> watchdog) */
		cpu_cluster_pm_enter();
		pr_info("SPMDBG spm: cpu_suspend CPU0 dormant\n");
		cpu_suspend(0, mt6582_spm_finisher);
		/* HIPOTESIS ciclo-6 (0710): el MTCMOS del cluster resetea la
		 * config del L2 (latencias) y CA7_CACHE_CONFIG que puso el LK;
		 * correr con los valores de reset = inestabilidad marginal que
		 * estalla tras N ciclos. Restaurar YA, y loguear la deriva
		 * (si difiere => hipotesis CONFIRMADA). */
		{
			u32 l2 = read_l2ctlr(), ca;

			if (l2 != s->l2ctlr_boot) {
				pr_info("SPMDBG L2CTLR drift 0x%x -> LK 0x%x\n",
					l2, s->l2ctlr_boot);
				asm volatile("mcr p15, 1, %0, c9, c0, 2"
					     : : "r"(s->l2ctlr_boot));
				isb();
			}
			if (s->mcusys) {
				ca = readl(s->mcusys);
				if (ca != s->cacfg_boot) {
					pr_info("SPMDBG CA7_CACHE_CONFIG drift 0x%x -> LK 0x%x\n",
						ca, s->cacfg_boot);
					writel(s->cacfg_boot, s->mcusys);
				}
			}
		}
		pr_info("SPMDBG spm: RESUMED\n");
		cpu_cluster_pm_exit();
		cpu_pm_exit();
		pr_info("SPMDBG spm: exit ok\n");
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

	/* PARAR el PCM entre ciclos (0710): tras el wake, el tail "normal" del
	 * firmware (proteccion termica del stock) queda corriendo en el uP del
	 * SPM — sospechoso de la bomba retardada que mataba el sistema 1-3s
	 * despues del 6º ciclo (moria EN LA PAUSA, sin suspend en vuelo). No
	 * lo necesitamos: el thermal mainline ya cubre eso. Cada enter hace
	 * reset+refetch igualmente. */
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
	spm_w(s, SPM_PCM_CON0, CON0_CFG_KEY);

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

/*
 * Hito M1 del modem: power-on MTCMOS del dominio del MD.
 * ioremap PEREZOSO (local, solo al dispararse por sysfs) -> NADA del codigo
 * del MD toca el arranque (el probe queda identico al bueno). Secuencia 1:1
 * con spm_mtcmos_ctrl_mdsys1(STA_POWER_ON) del stock. Exito = SPM_MD_PWR_STATUS
 * bit0 -> 1. Disparar: /sys/module/mt6582_spm/parameters/spm_md_poweron
 */
static int spm_md_power_on(struct mt6582_spm *s)
{
	void __iomem *infracfg;
	u32 timeout;
	int ret = 0;

	infracfg = ioremap(0x10001000, 0x400);
	if (!infracfg) {
		dev_err(s->dev, "MD power-on: no pude mapear INFRACFG\n");
		return -ENOMEM;
	}

	dev_info(s->dev, "MD power-on: PWR_STATUS bit0 antes=%u CON=0x%x\n",
		 !!(spm_r(s, SPM_MD_PWR_STATUS) & MD1_PWR_STA_MASK),
		 spm_r(s, SPM_MD_PWR_CON));

	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) | MD_PWR_ON);
	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) | MD_PWR_ON_S);

	timeout = 100000;
	while ((!(spm_r(s, SPM_MD_PWR_STATUS) & MD1_PWR_STA_MASK) ||
		!(spm_r(s, SPM_MD_PWR_STATUS_S) & MD1_PWR_STA_MASK)) && --timeout)
		cpu_relax();
	if (!timeout) {
		dev_err(s->dev, "MD power-on: TIMEOUT (STA=0x%x STA_S=0x%x CON=0x%x)\n",
			spm_r(s, SPM_MD_PWR_STATUS), spm_r(s, SPM_MD_PWR_STATUS_S),
			spm_r(s, SPM_MD_PWR_CON));
		ret = -ETIMEDOUT;
		goto out;
	}

	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) & ~MD_PWR_CLK_DIS);
	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) & ~MD_PWR_ISO);
	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) | MD_PWR_RST_B);
	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) & ~MD_SRAM_PDN);

	writel(readl(infracfg + INFRA_TOPAXI_PROT_EN) & ~MD1_PROT_MASK,
	       infracfg + INFRA_TOPAXI_PROT_EN);
	timeout = 100000;
	while ((readl(infracfg + INFRA_TOPAXI_PROT_STA1) & MD1_PROT_MASK) && --timeout)
		cpu_relax();

	dev_info(s->dev, "MD power-on OK: PWR_STATUS=0x%x CON=0x%x PROT_STA1=0x%x -> bit0=%u\n",
		 spm_r(s, SPM_MD_PWR_STATUS), spm_r(s, SPM_MD_PWR_CON),
		 readl(infracfg + INFRA_TOPAXI_PROT_STA1),
		 !!(spm_r(s, SPM_MD_PWR_STATUS) & MD1_PWR_STA_MASK));
out:
	iounmap(infracfg);
	return ret;
}

/*
 * H6k: power-OFF MTCMOS del dominio del MD (inverso exacto del power-on).
 * Permite reintentar el arranque del MD en el MISMO boot:
 *   echo 1 > spm_md_poweroff ; echo 1 > spm_md_poweron ; load ; remap ; release ; hs2
 */
static int spm_md_power_off(struct mt6582_spm *s)
{
	void __iomem *infracfg;
	u32 timeout, val;
	int ret = 0;

	infracfg = ioremap(0x10001000, 0x400);
	if (!infracfg) {
		dev_err(s->dev, "MD power-off: no pude mapear INFRACFG\n");
		return -ENOMEM;
	}

	dev_info(s->dev, "MD power-off: PWR_STATUS bit0 antes=%u CON=0x%x\n",
		 !!(spm_r(s, SPM_MD_PWR_STATUS) & MD1_PWR_STA_MASK),
		 spm_r(s, SPM_MD_PWR_CON));

	/* 1) cortar el bus TOPAXI hacia el MD y esperar el ack */
	writel(readl(infracfg + INFRA_TOPAXI_PROT_EN) | MD1_PROT_MASK,
	       infracfg + INFRA_TOPAXI_PROT_EN);
	timeout = 100000;
	while (((readl(infracfg + INFRA_TOPAXI_PROT_STA1) & MD1_PROT_MASK) != MD1_PROT_MASK)
	       && --timeout)
		cpu_relax();
	if (!timeout)		/* el stock tampoco aborta aqui: solo avisa */
		dev_warn(s->dev, "MD power-off: TOPAXI prot sin ack (STA1=0x%x)\n",
			 readl(infracfg + INFRA_TOPAXI_PROT_STA1));

	/* 2..5) SRAM down -> ISO -> (~RST_B | CLK_DIS) -> quitar PWR_ON/PWR_ON_S */
	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) | MD_SRAM_PDN);
	spm_w(s, SPM_MD_PWR_CON, spm_r(s, SPM_MD_PWR_CON) | MD_PWR_ISO);
	val = spm_r(s, SPM_MD_PWR_CON);
	val = (val & ~MD_PWR_RST_B) | MD_PWR_CLK_DIS;
	spm_w(s, SPM_MD_PWR_CON, val);
	spm_w(s, SPM_MD_PWR_CON,
	      spm_r(s, SPM_MD_PWR_CON) & ~(MD_PWR_ON | MD_PWR_ON_S));

	/* 6) esperar a que el dominio quede REALMENTE apagado */
	timeout = 100000;
	while (((spm_r(s, SPM_MD_PWR_STATUS) & MD1_PWR_STA_MASK) ||
		(spm_r(s, SPM_MD_PWR_STATUS_S) & MD1_PWR_STA_MASK)) && --timeout)
		cpu_relax();
	if (!timeout) {
		dev_err(s->dev, "MD power-off: TIMEOUT (STA=0x%x STA_S=0x%x CON=0x%x)\n",
			spm_r(s, SPM_MD_PWR_STATUS), spm_r(s, SPM_MD_PWR_STATUS_S),
			spm_r(s, SPM_MD_PWR_CON));
		ret = -ETIMEDOUT;
	} else {
		dev_info(s->dev, "MD power-off OK: PWR_STATUS=0x%x CON=0x%x -> bit0=%u\n",
			 spm_r(s, SPM_MD_PWR_STATUS), spm_r(s, SPM_MD_PWR_CON),
			 !!(spm_r(s, SPM_MD_PWR_STATUS) & MD1_PWR_STA_MASK));
	}

	iounmap(infracfg);
	return ret;
}

static int spm_md_poweroff_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_power_off(gspm);
}

static const struct kernel_param_ops spm_md_poweroff_ops = {
	.set = spm_md_poweroff_set,
};
module_param_cb(spm_md_poweroff, &spm_md_poweroff_ops, NULL, 0200);
MODULE_PARM_DESC(spm_md_poweroff, "apaga el dominio MTCMOS del MD (permite reintentar sin reboot)");

static int spm_md_poweron_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_power_on(gspm);
}
static const struct kernel_param_ops spm_md_poweron_ops = { .set = spm_md_poweron_set };
module_param_cb(spm_md_poweron, &spm_md_poweron_ops, NULL, 0200);

/* H2a: dump de CCIF (mailbox AP-MD @0x1020A000) + BANK4_MAP (vista DRAM del MD).
 * SOLO LECTURAS en espacio AP (INFRACFG/MCUSYS/CCIF), sin escrituras -> seguro.
 * Confirma que el HW del CCIF es accesible desde mainline (de-risk H3).
 * Disparar: echo 1 > /sys/module/mt6582_spm/parameters/spm_md_dump
 */
static int spm_md_dump(struct mt6582_spm *s)
{
	void __iomem *ccif, *infracfg;
	u32 con, busy, tch, rch;
	u32 ap0, ap1, md0, md1;

	ccif = ioremap(0x1020A000, 0x100);
	infracfg = ioremap(0x10001000, 0x400);
	if (!ccif || !infracfg) {
		if (ccif) iounmap(ccif);
		if (infracfg) iounmap(infracfg);
		dev_err(s->dev, "H2 dump: ioremap fallo\n");
		return -ENOMEM;
	}
	con  = readl(ccif + 0x00);
	busy = readl(ccif + 0x04);
	tch  = readl(ccif + 0x0c);
	rch  = readl(ccif + 0x10);
	/* AP_BANK4_MAP0/1 = MCUSYS(0x10200000)+0x200/0x204 (s->mcusys mapea 0x10) -> local */
	{
		void __iomem *mcu = ioremap(0x10200000, 0x400);
		ap0 = mcu ? readl(mcu + 0x200) : 0xdeadbeef;
		ap1 = mcu ? readl(mcu + 0x204) : 0xdeadbeef;
		if (mcu) iounmap(mcu);
	}
	md0 = readl(infracfg + 0x308);
	md1 = readl(infracfg + 0x30c);

	dev_info(s->dev, "H2 CCIF@0x1020A000: CON=0x%08x BUSY=0x%08x TCHNUM=0x%08x RCHNUM=0x%08x\n",
		 con, busy, tch, rch);
	dev_info(s->dev, "H2 BANK4_MAP: AP0=0x%08x AP1=0x%08x MD1_0=0x%08x MD1_1=0x%08x (0=sin remap)\n",
		 ap0, ap1, md0, md1);
	iounmap(ccif);
	iounmap(infracfg);
	return 0;
}



static int spm_md_dump_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_dump(gspm);
}
static const struct kernel_param_ops spm_md_dump_ops = { .set = spm_md_dump_set };
module_param_cb(spm_md_dump, &spm_md_dump_ops, NULL, 0200);

/* ===== Hito M1 H3: arrancar el MD (modem processor) ===== */
/* Carveout del MD (DT modem-region@b8000000, 24MB) y su vista DRAM.
 * Layout stock (mtk_ccci_helper.c:475-476): firmware @ base, SMEM @ base+22MB. */
#define MD_MEM_PHYS	0xb8000000	/* firmware (md_region_phy): BANK0 = MD 0x0 -> aqui */
#define MD_MEM_SIZE	0x1600000	/* 22MB (MD1_MEM_SIZE) */
#define MD_SMEM_PHYS	0xb9600000	/* SMEM (base+22MB): BANK4 = MD 0x40000000 -> aqui */
#define MD_SMEM_SIZE	0x1800000	/* mapa de carga: 24MB (todo el carveout) */

/* H7r: buffers TTY del CCCI (shared_mem_tty_t de ccci_tty.h del bq-src) */
#define CCCI_TTY_BUF_SIZE	(16 * 1024)	/* rx_buffer = tx_buffer */
#define CCCI_TTY_SMEM_SIZE	(2 * 12 + 2 * CCCI_TTY_BUF_SIZE)	/* = 0x8018 */
/* H8d: el runtime declara UartShareMemBase[8] ("Current UART_MAX_PORT_NUM is 8"
 * en ccci_md.h), no 6.  El assert de UPS (ccci_uart_drv.c:2594) indexa los
 * puertos como (canal_CCCI - 37), asi que puede haber mas de 6.  Se sube el tope
 * a 8 y el numero declarado se barre con spm_md_uart_ports. */
#define CCCI_TTY_PORTS		8
#define CCCI_TTY_BASE		0x180000	/* tras las regiones de red */
#define CCCI_TTY_STRIDE		0x9000		/* > 0x8018, alineado */

/*
 * H6p: volcado de la MEMORIA COMPARTIDA del MD, buscando texto.
 *
 * HECHO (0729): el experimento de control H6o (modo 8, no contestar nada al
 * 0x1010) da EXACTAMENTE el mismo resultado que contestar: handshake OK, 2 ops
 * FS (100e + 1010) y silencio total (RCHNUM=0 BUSY=0 tras 8s).  O sea que
 * NUESTRA RESPUESTA NO ES EL DISPARADOR.  Ademas nuestra respuesta al GetDrive
 * es identica byte a byte a la del ccci_fsd real ([02][04][00][54]).
 *
 * Y el ground-truth (gd-boot-full.out) demuestra que el MD de Lineage pide EL
 * MISMO fichero (Z:\FAT3149C88D.log, op 1010) y SIGUE ADELANTE al 1001 OPEN
 * "Z:\NVRAM" 458us despues.  El nuestro se para justo ahi.
 *
 * Falta saber si el MD esta EXCEPCIONADO o simplemente ESPERANDO algo.  MOLY
 * escribe sus excepciones en la memoria compartida con texto legible (nombre de
 * fichero del assert, motivo...).  /dev/mem esta bloqueado por STRICT_DEVMEM,
 * asi que lo volcamos desde el kernel: se barre la SMEM buscando cadenas ASCII
 * imprimibles de >=8 caracteres.  Si aparece un assert de MOLY, dice
 * EXACTAMENTE de que se queja.
 *
 *   echo 1 > /sys/module/mt6582_spm/parameters/spm_md_smem
 */
#define SPM_SMEM_SCAN	0x200000	/* barrer los primeros 2MB */
#define SPM_SMEM_MINRUN	5		/* longitud minima de cadena */
#define SPM_SMEM_MAXHIT	80		/* tope de cadenas impresas */

/*
 * H6r: LIMPIAR el area de excepcion de la SMEM.
 *
 * FALLO DE METODO detectado el 0729: la SMEM NO se limpia entre ciclos, asi que
 * el registro de excepcion que leiamos podia ser el de la corrida ANTERIOR.  Por
 * eso el modo 8 (sin responder) parecia dar el mismo registro que el modo 0: era
 * el mismo registro, rancio.  Sin esto ninguna comparacion vale.
 */
static int spm_md_smem_clear(struct mt6582_spm *s)
{
	void __iomem *sm;
	u32 off;

	sm = ioremap(MD_SMEM_PHYS + 0x800, 0x200);
	if (!sm)
		return -ENOMEM;
	for (off = 0; off < 0x200; off += 4)
		writel(0, sm + off);
	wmb();
	iounmap(sm);
	dev_info(s->dev, "H6r: area de excepcion de la SMEM (+0x800..+0xa00) limpiada\n");
	return 0;
}

/*
 * H7p: LEER el registro de excepcion del MD, decodificado.
 *
 * Formato (EX_LOG_T, ccci_md.h del bq-src):
 *   +0    ex_type(1) ex_nvram(1) ex_serial(2)
 *   +16   envinfo: boot_mode(1), reservado(8), execution_unit(8) <- TAREA en texto
 *   +216  content: ASSERT   -> filename[24] + linenumber + 3 parametros
 *                  FATALERR -> code1, code2
 *
 * Lo escribe el MD en ExceShareMemBase (SMEM+0x800), que es donde se lo dijimos
 * en el runtime del HS2.  /dev/mem no alcanza la zona: se lee desde el kernel.
 */
static const char *spm_ex_name(u8 t)
{
	switch (t) {
	case 0:  return "INVALID";
	case 1:  return "UNDEF";
	case 2:  return "SWI";
	case 3:  return "PREFETCH ABORT";
	case 4:  return "DATA ABORT";
	case 5:  return "ASSERT";
	case 6:  return "FATALERR TASK";
	case 7:  return "FATALERR BUF";
	case 8:  return "LOCKUP";
	case 9:  return "ASSERT DUMP";
	case 10: return "ASSERT FAIL";
	case 99: return "EMI CHECK";
	default: return "?";
	}
}

/*
 * H11: volcado de los anillos TTY del CCCI.
 *
 * Los canales TTY (lch 6/10/38 y sus ACK) no llevan datos en el mensaje: son un
 * timbre, y el contenido esta en la memoria compartida que anunciamos en el
 * runtime.  Puertos del stock: 0 = UART1 (meta), 1 = UART2 (modem, comandos AT),
 * 5 = IPC_UART.
 */
static uint spm_md_uart_ports;	/* H8c, definido con valor mas abajo */

/*
 * H12: enviar un mensaje CCCI (el "timbre").  Formato del stock: cuatro palabras
 * {data0, data1, canal_logico, reserved} en un canal fisico TX libre, y luego
 * TCHNUM para dispararlo.  Es el mismo camino que usa la respuesta del FS.
 */
static int spm_ccci_send(struct mt6582_spm *s, void __iomem *ccif,
			 u32 d0, u32 d1, u32 lch, u32 resv)
{
	u32 busy = readl(ccif + 0x04);
	int tch;

	for (tch = 0; tch < 8; tch++)
		if (!(busy & (1 << tch)))
			break;
	if (tch >= 8) {
		dev_warn(s->dev, "H12: sin canal TX libre (BUSY=%02x)\n", busy);
		return -EBUSY;
	}
	writel(1 << tch, ccif + 0x04);
	writel(d0,   ccif + 0x100 + tch * 16 + 0);
	writel(d1,   ccif + 0x100 + tch * 16 + 4);
	writel(lch,  ccif + 0x100 + tch * 16 + 8);
	writel(resv, ccif + 0x100 + tch * 16 + 12);
	wmb();
	writel(tch, ccif + 0x0c);
	return 0;
}

/*
 * H12: mandarle una linea al modem por el puerto TTY 1 (UART2 = canal AT).
 *   echo "AT" > /sys/module/mt6582_spm/parameters/spm_tty_at
 * Se le anade CR, que es lo que termina un comando AT.
 */
static char spm_tty_at_buf[128];

static int spm_tty_at_send(struct mt6582_spm *s, const char *txt)
{
	u32 base = MD_SMEM_PHYS + CCCI_TTY_BASE + 1 * CCCI_TTY_STRIDE;
	void __iomem *t, *ccif;
	u32 rr, rw, tw, tl, n, i;
	int ret = 0;

	t = ioremap(base, CCCI_TTY_SMEM_SIZE);
	ccif = ioremap(0x1020A000, 0x200);
	if (!t || !ccif) {
		if (t) iounmap(t);
		if (ccif) iounmap(ccif);
		return -ENOMEM;
	}

	/* 1) consumir lo que el MD tenga pendiente y ACKear (canal 11, id 1) */
	rr = readl(t + 0x00);
	rw = readl(t + 0x04);
	if (rw != rr) {
		writel(rw, t + 0x00);
		wmb();
		spm_ccci_send(s, ccif, 0xffffffff, 1, 11, 0);
		dev_info(s->dev, "H12 RX drenado: read %u -> %u, ACK por ch11\n", rr, rw);
	}

	/* 2) copiar al tx_buffer y avanzar tx.write */
	tw = readl(t + 0x10);
	tl = readl(t + 0x14);
	n = strlen(txt);
	if (!tl || n == 0 || n > CCCI_TTY_BUF_SIZE / 2) {
		ret = -EINVAL;
		goto fuera;
	}
	for (i = 0; i < n; i++)
		writeb(txt[i], t + 0x4018 + ((tw + i) % tl));
	writel((tw + n) % tl, t + 0x10);
	wmb();

	/* 3) timbre: stream {addr=0, len} en CCCI_UART2_TX = 12 */
	ret = spm_ccci_send(s, ccif, 0, n, 12, 0);
	dev_info(s->dev, "H12 TX '%s' (%u B) tx.write %u -> %u, timbre ch12 -> %d\n",
		 txt, n, tw, (tw + n) % tl, ret);

fuera:
	iounmap(ccif);
	iounmap(t);
	return ret;
}

static int spm_tty_dump_read(struct mt6582_spm *s)
{
	static const char * const nombre[CCCI_TTY_PORTS] = {
		"UART1/meta", "UART2/modem-AT", "?", "?", "?", "IPC_UART", "?", "?"
	};
	int i, n = min_t(int, spm_md_uart_ports, CCCI_TTY_PORTS);

	/* H11c: solo los puertos DECLARADOS.  CCCI_TTY_PORTS es el tope del array
	 * del runtime (8); del 6 en adelante no se inicializan y salia basura. */
	for (i = 0; i < n; i++) {
		u32 base = MD_SMEM_PHYS + CCCI_TTY_BASE + i * CCCI_TTY_STRIDE;
		void __iomem *t = ioremap(base, CCCI_TTY_SMEM_SIZE);
		u32 rr, rw, rl, tr, tw, tl, n, off;
		char hx[100], as[40];

		if (!t)
			continue;
		rr = readl(t + 0x00); rw = readl(t + 0x04); rl = readl(t + 0x08);
		tr = readl(t + 0x0c); tw = readl(t + 0x10); tl = readl(t + 0x14);
		dev_info(s->dev, "H11 TTY%d %-14s rx[r=%u w=%u len=%u] tx[r=%u w=%u len=%u]\n",
			 i, nombre[i], rr, rw, rl, tr, tw, tl);

		/* datos pendientes del MD: de rx.read a rx.write (sin dar la vuelta) */
		if (rw == rr || rw > CCCI_TTY_BUF_SIZE) {
			iounmap(t);
			continue;
		}
		n = (rw > rr) ? rw - rr : rl - rr;
		if (n > 256)
			n = 256;
		for (off = 0; off < n; off += 16) {
			u32 k, m = min_t(u32, 16, n - off);
			int hn = 0, an = 0;

			for (k = 0; k < m; k++) {
				u8 c = readb(t + 0x18 + rr + off + k);

				hn += scnprintf(hx + hn, sizeof(hx) - hn, "%02x ", c);
				an += scnprintf(as + an, sizeof(as) - an, "%c",
						(c >= 32 && c < 127) ? c : '.');
			}
			hx[hn] = 0; as[an] = 0;
			dev_info(s->dev, "H11   +%04x: %-48s |%s|\n", rr + off, hx, as);
		}
		iounmap(t);
	}
	return 0;
}

static int spm_md_ex_read(struct mt6582_spm *s)
{
	void __iomem *ex;
	char b[40];
	u32 off;
	u8 type, nvram;
	u16 serial;

	ex = ioremap(MD_SMEM_PHYS + 0x800, 0x400);
	if (!ex)
		return -ENOMEM;

	type   = readb(ex + 0);
	nvram  = readb(ex + 1);
	serial = readw(ex + 2);
	dev_info(s->dev, "H7p EXCEPCION del MD: tipo=%u [%s] nvram=%u serie=%u\n",
		 type, spm_ex_name(type), nvram, serial);

	memcpy_fromio(b, ex + 16 + 9, 8);	/* envinfo.execution_unit */
	b[8] = 0;
	dev_info(s->dev, "H7p   tarea='%s' boot_mode=%u\n", b, readb(ex + 16));

	if (type == 5 || type == 9 || type == 10) {
		memcpy_fromio(b, ex + 216, 24);	/* content.assert.filename */
		b[24] = 0;
		dev_info(s->dev, "H7p   ASSERT en %s:%u  p=%08x %08x %08x\n",
			 b, readl(ex + 216 + 24), readl(ex + 216 + 28),
			 readl(ex + 216 + 32), readl(ex + 216 + 36));
	} else if (type == 6 || type == 7) {
		dev_info(s->dev, "H7p   FATAL code1=%08x code2=%08x\n",
			 readl(ex + 216), readl(ex + 220));
	}

	for (off = 0; off < 0x120; off += 16)
		dev_info(s->dev, "H7p   +%03x: %08x %08x %08x %08x\n", off,
			 readl(ex + off), readl(ex + off + 4),
			 readl(ex + off + 8), readl(ex + off + 12));

	iounmap(ex);
	return 0;
}

static int spm_md_smem_scan(struct mt6582_spm *s)
{
	void __iomem *sm;
	char buf[96];
	u32 off, run = 0;
	int hits = 0;

	sm = ioremap(MD_SMEM_PHYS, SPM_SMEM_SCAN);
	if (!sm) {
		dev_err(s->dev, "H6p: no pude mapear la SMEM\n");
		return -ENOMEM;
	}

	dev_info(s->dev, "H6p SMEM @0x%08x, primeros 64B:\n", MD_SMEM_PHYS);
	for (off = 0; off < 64; off += 16)
		dev_info(s->dev, "  +%04x: %08x %08x %08x %08x\n", off,
			 readl(sm + off), readl(sm + off + 4),
			 readl(sm + off + 8), readl(sm + off + 12));

	dev_info(s->dev, "H6p: buscando texto en %d KB...\n", SPM_SMEM_SCAN / 1024);
	for (off = 0; off < SPM_SMEM_SCAN && hits < SPM_SMEM_MAXHIT; off++) {
		u8 c = readb(sm + off);

		if (c >= 0x20 && c < 0x7f) {
			if (run < sizeof(buf) - 1)
				buf[run] = c;
			run++;
			continue;
		}
		if (run >= SPM_SMEM_MINRUN) {
			buf[min_t(u32, run, sizeof(buf) - 1)] = 0;
			dev_info(s->dev, "H6p +0x%06x: %s\n", off - run, buf);
			hits++;
		}
		run = 0;
	}
	dev_info(s->dev, "H6p: %d cadenas encontradas\n", hits);

	iounmap(sm);
	return 0;
}

/*
 * H6q: volcado hex+ASCII de una zona concreta de la SMEM.
 *
 * HALLAZGO (0729, kernel #38): el barrido de texto encontro "fs_ccci.c" en
 * +0x8d8 -> firma de un ASSERT de MOLY (escribe fichero y linea al petar).  O
 * sea que el MD SI EXCEPCIONA, y lo hace en su capa de FS/CCCI.  Falta leer el
 * registro entero (linea, codigo, contexto), que vive alrededor de +0x800.
 *
 *   echo 1   > .../spm_md_smem    -> barrido de cadenas (como antes)
 *   echo 800 > .../spm_md_smem    -> volcado hex+ASCII de 512B desde +0x800
 */
static int spm_md_smem_hex(struct mt6582_spm *s, u32 off)
{
	void __iomem *sm;
	u32 i, j;

	off &= ~0xFu;
	sm = ioremap(MD_SMEM_PHYS + off, 0x200);
	if (!sm) {
		dev_err(s->dev, "H6q: no pude mapear SMEM+0x%x\n", off);
		return -ENOMEM;
	}
	dev_info(s->dev, "H6q SMEM +0x%06x (512B):\n", off);
	for (i = 0; i < 0x200; i += 16) {
		char asc[17];

		for (j = 0; j < 16; j++) {
			u8 c = readb(sm + i + j);

			asc[j] = (c >= 0x20 && c < 0x7f) ? c : '.';
		}
		asc[16] = 0;
		dev_info(s->dev, "  +%04x: %08x %08x %08x %08x  |%s|\n", off + i,
			 readl(sm + i), readl(sm + i + 4),
			 readl(sm + i + 8), readl(sm + i + 12), asc);
	}
	iounmap(sm);
	return 0;
}

static int spm_md_smem_set(const char *val, const struct kernel_param *kp)
{
	u32 off = 0;

	if (!gspm)
		return -ENODEV;
	if (val && kstrtou32(strim((char *)val), 16, &off))	/* se lee en HEX */
		off = 0;					/* invalido -> barrido */
	if (off >= 0x100)
		return spm_md_smem_hex(gspm, off);
	return spm_md_smem_scan(gspm);
}
static const struct kernel_param_ops spm_md_smem_ops = { .set = spm_md_smem_set };
module_param_cb(spm_md_smem, &spm_md_smem_ops, NULL, 0200);
MODULE_PARM_DESC(spm_md_smem, "vuelca/busca texto en la memoria compartida del MD (excepciones de MOLY)");

static int spm_md_smem_clr_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_smem_clear(gspm);
}
static const struct kernel_param_ops spm_md_smem_clr_ops = { .set = spm_md_smem_clr_set };
module_param_cb(spm_md_smem_clr, &spm_md_smem_clr_ops, NULL, 0200);
MODULE_PARM_DESC(spm_md_smem_clr, "H6r: limpia el registro de excepcion de la SMEM (hacerlo ANTES de cada ciclo)");

static int spm_md_ex_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_ex_read(gspm);
}
static const struct kernel_param_ops spm_md_ex_ops = { .set = spm_md_ex_set };
module_param_cb(spm_md_ex, &spm_md_ex_ops, NULL, 0200);
MODULE_PARM_DESC(spm_md_ex, "H7p: lee y decodifica el registro de excepcion del MD");

static int spm_tty_dump_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_tty_dump_read(gspm);
}
static const struct kernel_param_ops spm_tty_dump_ops = { .set = spm_tty_dump_set };
module_param_cb(spm_tty_dump, &spm_tty_dump_ops, NULL, 0200);
MODULE_PARM_DESC(spm_tty_dump, "H11: vuelca los anillos TTY del CCCI (lo que el MD ha escrito)");

static int spm_tty_at_set(const char *val, const struct kernel_param *kp)
{
	int n;

	if (!gspm)
		return -ENODEV;
	n = snprintf(spm_tty_at_buf, sizeof(spm_tty_at_buf) - 2, "%s", val);
	if (n < 0)
		return -EINVAL;
	if (n > (int)sizeof(spm_tty_at_buf) - 3)
		n = sizeof(spm_tty_at_buf) - 3;
	while (n > 0 && (spm_tty_at_buf[n - 1] == '\n' || spm_tty_at_buf[n - 1] == '\r'))
		spm_tty_at_buf[--n] = 0;
	spm_tty_at_buf[n++] = '\r';		/* un comando AT termina en CR */
	spm_tty_at_buf[n] = 0;
	return spm_tty_at_send(gspm, spm_tty_at_buf);
}
static const struct kernel_param_ops spm_tty_at_ops = { .set = spm_tty_at_set };
module_param_cb(spm_tty_at, &spm_tty_at_ops, NULL, 0200);
MODULE_PARM_DESC(spm_tty_at, "H12: manda una linea por el canal AT del modem (UART2)");

/*
 * H13d: POR DEFECTO DESACTIVADO.  El tty cuelga el sistema al usarlo (cuelgue
 * duro, sin oops ni registro en pstore => watchdog) y todavia no se ha
 * localizado.  El arranque del modem y el canal AT por sysfs (spm_tty_at) SI
 * funcionan, asi que no deben quedar a merced de una pieza sin verificar:
 *   echo 1 > .../spm_tty_enable   ANTES del ciclo, para experimentar con el tty.
 */
static uint spm_tty_enable;
module_param(spm_tty_enable, uint, 0644);
MODULE_PARM_DESC(spm_tty_enable, "H13: registrar /dev/ttyCCCI0 y el hilo de servicio (0 = no, defecto)");


#define KERN_EMI_BASE	0x80000000
#define MD_INVALID_ADDR	0x3E000000
#define MD_INVALID_OFF	0x02000000

/* H3a: cargar el firmware MOLY al inicio del carveout (raw copy, como load_std_firmware) */
static int spm_md_load(struct mt6582_spm *s)
{
	const struct firmware *fw;
	void __iomem *dst;
	u32 w0, wlast;
	int ret;

	ret = request_firmware(&fw, "modem.img", s->dev);
	if (ret) {
		dev_err(s->dev, "H3 load: request_firmware(modem.img) fallo %d\n", ret);
		return ret;
	}
	dst = ioremap(MD_MEM_PHYS, MD_SMEM_SIZE);
	if (!dst) {
		release_firmware(fw);
		dev_err(s->dev, "H3 load: ioremap carveout fallo\n");
		return -ENOMEM;
	}
	if (fw->size > MD_SMEM_SIZE) {
		iounmap(dst); release_firmware(fw);
		dev_err(s->dev, "H3 load: firmware %zu > carveout\n", fw->size);
		return -EFBIG;
	}
	memcpy_toio(dst, fw->data, fw->size);
	w0 = readl(dst);
	wlast = readl(dst + ((fw->size & ~0x3u) - 4));
	dev_info(s->dev, "H3 load: modem.img %zu B -> 0x%08x; w0=0x%08x (esperado 0xe59ff018) wlast=0x%08x\n",
		 fw->size, MD_MEM_PHYS, w0, wlast);
	iounmap(dst);
	release_firmware(fw);
	return 0;
}

/* H3b: BANK-remap = la vista de DRAM del MD apunta al carveout. Formula 1:1 del downstream. */
static int spm_md_remap(struct mt6582_spm *s)
{
	void __iomem *mcu, *infra;
	const u32 IA = MD_INVALID_ADDR, O = MD_INVALID_OFF;
	u32 apd = MD_SMEM_PHYS;			/* AP/MD BANK4 -> SMEM (0xb9600000) */
	u32 mdd = MD_SMEM_PHYS - KERN_EMI_BASE;
	u32 b0d = MD_MEM_PHYS - KERN_EMI_BASE;	/* MD BANK0 -> FIRMWARE (0xb8000000) */
	u32 ap0, ap1, md0, md1, b0m0, b0m1;

	/* BANK4 (set_ap_smem_remap / set_md_smem_remap): vista 0x40000000 del MD = SMEM.
	 * AP usa slots invalidos 14-20, MD usa 0-6. */
	ap0 = (((apd>>24)|0x1)&0xFF) + ((((IA+O*14)>>16)|(1<<8))&0xFF00)
	    + ((((IA+O*15)>>8)|(1<<16))&0xFF0000) + ((((IA+O*16))|(1<<24))&0xFF000000);
	ap1 = ((((IA+O*17)>>24)|0x1)&0xFF) + ((((IA+O*18)>>16)|(1<<8))&0xFF00)
	    + ((((IA+O*19)>>8)|(1<<16))&0xFF0000) + ((((IA+O*20))|(1<<24))&0xFF000000);
	md0 = (((mdd>>24)|0x1)&0xFF) + ((((IA+O*0)>>16)|(1<<8))&0xFF00)
	    + ((((IA+O*1)>>8)|(1<<16))&0xFF0000) + ((((IA+O*2))|(1<<24))&0xFF000000);
	md1 = ((((IA+O*3)>>24)|0x1)&0xFF) + ((((IA+O*4)>>16)|(1<<8))&0xFF00)
	    + ((((IA+O*5)>>8)|(1<<16))&0xFF0000) + ((((IA+O*6))|(1<<24))&0xFF000000);

	/* ⚡ H3 v3 (0717): BANK0 (set_md_rom_rw_mem_remap, src=0x0) — LA PIEZA QUE FALTABA.
	 * Mapea la dir 0x0 del MD (donde arranca su boot ROM: boot-slave Vector=0x0) al
	 * FIRMWARE en DRAM. Sin esto el MD, al soltarlo, ejecuta memoria vacia -> nunca
	 * llega a hablar por CCIF (RCHNUM=0). Slots invalidos 7-13. Reg MD1_BANK0_MAP0/1
	 * = INFRACFG+0x300/0x304 (BANK4 iba a +0x308/0x30C). */
	b0m0 = (((b0d>>24)|0x1)&0xFF) + ((((IA+O*7)>>16)|(1<<8))&0xFF00)
	     + ((((IA+O*8)>>8)|(1<<16))&0xFF0000) + ((((IA+O*9))|(1<<24))&0xFF000000);
	b0m1 = ((((IA+O*10)>>24)|0x1)&0xFF) + ((((IA+O*11)>>16)|(1<<8))&0xFF00)
	     + ((((IA+O*12)>>8)|(1<<16))&0xFF0000) + ((((IA+O*13))|(1<<24))&0xFF000000);

	mcu = ioremap(0x10200000, 0x400);
	infra = ioremap(0x10001000, 0x400);
	if (!mcu || !infra) {
		if (mcu) iounmap(mcu);
		if (infra) iounmap(infra);
		return -ENOMEM;
	}
	writel(b0m0, infra + 0x300);		/* MD1_BANK0_MAP0 -> firmware */
	writel(b0m1, infra + 0x304);		/* MD1_BANK0_MAP1 */
	writel(ap0, mcu + 0x200);
	writel(ap1, mcu + 0x204);
	writel(md0, infra + 0x308);		/* MD1_BANK4_MAP0 -> SMEM */
	writel(md1, infra + 0x30c);
	dev_info(s->dev, "H3 v3 remap: BANK0(fw 0x%08x)=%08x/%08x  BANK4(smem 0x%08x) AP=%08x/%08x MD=%08x/%08x\n",
		 MD_MEM_PHYS, b0m0, b0m1, apd, ap0, ap1, md0, md1);
	iounmap(mcu);
	iounmap(infra);
	return 0;
}

/* H3c: soltar el MD (WDT off + boot-slave keys) y sondear el CCIF por actividad del MD. */
static int spm_md_release(struct mt6582_spm *s)
{
	void __iomem *rgu, *vec, *key, *en, *ccif;
	u32 con, busy, rch, rx;
	int i, seen = 0;

	rgu = ioremap(0x20050000, 0x40);
	vec = ioremap(0x20190000, 0x4);
	key = ioremap(0x2019379C, 0x4);
	en  = ioremap(0x20195488, 0x4);
	ccif = ioremap(0x1020A000, 0x100);
	if (!rgu || !vec || !key || !en || !ccif) {
		if (rgu)
			iounmap(rgu);
		if (vec)
			iounmap(vec);
		if (key)
			iounmap(key);
		if (en)
			iounmap(en);
		if (ccif)
			iounmap(ccif);
		return -ENOMEM;
	}
	/* H3 v2 (0717): INIT del CCIF ANTES de soltar el MD — la pieza que faltaba.
	 * El stock (__ccif_v1_init, ccci_hw.c:263) pone el mailbox AP en modo
	 * ARBITRACIÓN (CCIF_CON=+0x0=CCIF_CON_ARB=0x1) + ACKea todos los canales
	 * (CCIF_ACK=+0x14=0xFF) en el probe, ANTES del let_md_go. Sin ARB el AP no
	 * refleja las escrituras del MD → el handshake no registra (H3 v1: CON/RCHNUM=0,
	 * RX0=0xffffffff = lectura flotante del CCIF sin init). */
	writel(0x1, ccif + 0x00);		/* CCIF_CON = CCIF_CON_ARB (arbitración) */
	writel(0xFF, ccif + 0x14);		/* CCIF_ACK = ACK todos los canales */
	(void)readl(ccif + 0x00);		/* flush posted write */
	dev_info(s->dev, "H3 v2: CCIF init -> CON=0x%08x (esperado 0x1) BUSY=0x%08x RCHNUM=0x%08x RX0=0x%08x\n",
		 readl(ccif+0), readl(ccif+4), readl(ccif+0x10), readl(ccif+0x180));

	writel(0x2200, rgu + 0x00);		/* WDT_MD_MODE = KEY = disable */
	writel(0x3567C766, key);
	writel(0x0, vec);
	writel(0xA3B66175, en);
	dev_info(s->dev, "H3 release: MD soltado (WDT off + keys). Sondeando CCIF@0x1020A000 5s...\n");
	for (i = 0; i < 50; i++) {
		con = readl(ccif + 0x00);
		busy = readl(ccif + 0x04);
		rch = readl(ccif + 0x10);
		rx = readl(ccif + 0x180);
		/* handshake REAL = cambia CON (deja de ser 0x1=ARB nuestro), BUSY o RCHNUM.
		 * NO mirar RX0: lee 0xffffffff flotante y da falso positivo. */
		if ((con && con != 0x1) || busy || rch) {
	dev_info(s->dev, "H3 v2: *** HANDSHAKE! @t=%dms CON=0x%08x BUSY=0x%08x RCHNUM=0x%08x RX0=0x%08x ***\n",
		 i*100, con, busy, rch, rx);
			seen = 1;
			break;
		}
		msleep(100);
	}
	if (!seen)
		dev_info(s->dev, "H3 release: SIN actividad tras 5s. CON=0x%08x BUSY=0x%08x RCHNUM=0x%08x RX0=0x%08x\n",
		 readl(ccif+0), readl(ccif+4), readl(ccif+0x10), readl(ccif+0x180));
	iounmap(rgu); iounmap(vec); iounmap(key); iounmap(en); iounmap(ccif);
	return 0;
}

/* H4: HS2 — responder al handshake HS1 del MD con la runtime data + el mensaje de
 * arranque, para que el MD avance a stage 2 = M1 completo. Todo RE'd del stock
 * (ccci_send_run_time_data / set_md_runtime / __ccif_v1_write_phy_ch_data).
 * Ver H4-HS2-SPEC-0717.md. Primer intento: runtime minimo (share-mems a 0 salvo
 * MiscInfo). El MD lee el TAG en la SRAM del CCIF@0x140 -> apunta al runtime en SMEM. */
#define MD_AP_OFF	0x78000000	/* md_2_ap_phy_addr_offset_fixed = (smem&0xFE000000)-0x40000000 */
#define RT_NINTS	70		/* sizeof(modem_runtime_t)/4 */

/* ===== H6 PROXY FS: servidor mínimo (kernel) que sirve la NVRAM del MD =====
 * Protocolo (RE del ccci_fsd stock, docs H6b..H6f): el MD escribe una peticion en
 * fs_buffer[idx] = {u32 fs_ops; u8 buf[16384]}; nosotros hacemos el file-op sobre
 * /data/nvram/md y escribimos la respuesta en el mismo buffer. Op-codes (low16 de
 * fs_ops; high16 request=0x0000): OPEN=0x1001, WRITE=0x1004 (confirmados por poller
 * + strace); READ/CLOSE/GETSIZE por confirmar del propio log de este proxy en pmOS.
 * Respuesta: word0 = 0xffff0000|op (marca "hecho", ~(~(op<<16)>>16) del ccci_fsd);
 * payload {u32 len; data} desde buf+... ; SEND: data1 = length+4.
 * Devuelve la 'length' para el FS_TX (o 0 = solo cabecera). */
#define FS_NR_HANDLES	16
static struct file *g_fs_h[FS_NR_HANDLES];

/*
 * H10a: traza dirigida a UN fichero.  Se sigue el handle que devuelve el OPEN
 * cuyo path contenga esta subcadena, y se loguean sus ops (siempre visibles,
 * aunque spm_fs_quiet silencie la cascada).  Vacio o "" = traza desactivada.
 */
static char *spm_fs_trace = "MP0B";
module_param(spm_fs_trace, charp, 0644);
MODULE_PARM_DESC(spm_fs_trace, "H10a: subcadena del path a trazar op a op (por defecto MP0B)");

static int g_fs_trace_h = -1;

/* convierte "Z:\NVRAM\NVD_DATA\MT48_001" (UTF-16LE en el buffer) a
 * "/data/nvram/md/NVRAM/NVD_DATA/MT48_001" (ASCII). plen = bytes UTF-16. */
static void spm_fs_p0(void __iomem *fs, u32 boff, u32 val);	/* H7j: se usa antes de su definicion */

/* ===== H7j: enumeracion REAL de la NVRAM (FindFirst / FindNext) ===== */
#define FS_ENUM_MAX	96
#define FS_ENUM_PLEN	72

static char g_enum[FS_ENUM_MAX][FS_ENUM_PLEN];	/* rutas "Z:\\NVRAM\\DIR\\FICHERO" */
static int  g_enum_n;				/* cuantas hay */
static int  g_enum_i;				/* cursor del FindNext */

static uint spm_fs_enum = 1;
module_param(spm_fs_enum, uint, 0644);
MODULE_PARM_DESC(spm_fs_enum, "1 = 1010/1011 enumeran la NVRAM de verdad; 0 = comportamiento anterior");

static uint spm_fs_find_nf = 1;
module_param(spm_fs_find_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_find_nf, "campos de la respuesta de FindFirst/FindNext (1 o 2)");

static uint spm_fs_dumpresp = 1;
module_param(spm_fs_dumpresp, uint, 0644);
MODULE_PARM_DESC(spm_fs_dumpresp, "H7n: volcar las 20 primeras palabras de cada respuesta FS");

static uint spm_fs_find_mode;		/* 0 = escalar (H7q), 1 = ruta (H7j) */
module_param(spm_fs_find_mode, uint, 0644);
MODULE_PARM_DESC(spm_fs_find_mode, "H7q: 0 = responder escalar al 1010/1011 de la NVRAM, 1 = responder ruta");

static uint spm_fs_find_val;
module_param(spm_fs_find_val, uint, 0644);
MODULE_PARM_DESC(spm_fs_find_val, "H7q: valor del escalar de 4 bytes (barrer y mirar la secuencia de ops)");

static uint spm_fs_find_attr = 0x700;
module_param(spm_fs_find_attr, uint, 0644);
MODULE_PARM_DESC(spm_fs_find_attr, "H7m: campo 1 de FindFirst/FindNext (0x700 en el ground-truth)");

struct spm_enum_ctx {
	struct dir_context ctx;
	const char *sub;
};

static bool spm_enum_filldir(struct dir_context *ctx, const char *name, int nlen,
			     loff_t off, u64 ino, unsigned int dt)
{
	struct spm_enum_ctx *c = container_of(ctx, struct spm_enum_ctx, ctx);

	if (nlen == 1 && name[0] == '.')
		return true;
	if (nlen == 2 && name[0] == '.' && name[1] == '.')
		return true;
	if (dt == DT_DIR)
		return true;
	if (g_enum_n < FS_ENUM_MAX)
		scnprintf(g_enum[g_enum_n++], FS_ENUM_PLEN,
			  "Z:\\NVRAM\\%s\\%.*s", c->sub, nlen, name);
	return true;
}

/* Orden alfabetico de subdirectorios: es el que sigue el fsd real (su FindFirst
 * devolvio NVD_CORE\MT00A000, el primer fichero del primer subdir con contenido). */
static void spm_fs_enum_build(struct mt6582_spm *s)
{
	static const char * const subs[] = {
		/* H7l: los NVD_* PRIMERO.  El fsd real, preguntado por NVD_DATA,
		 * respondio NVD_CORE\MT00A000: enumera los NVD_* en orden alfabetico
		 * (NVD_CORE < NVD_DATA).  Empezar por CALIBRAT hacia excepcionar al MD. */
		"NVD_CORE", "NVD_DATA", "NVD_IMEI", "CALIBRAT", "IMPORTNT"
	};
	char path[80];
	int k;

	g_enum_n = 0;
	g_enum_i = 0;
	for (k = 0; k < ARRAY_SIZE(subs); k++) {
		struct spm_enum_ctx c = {
			.ctx.actor = spm_enum_filldir,
			.ctx.pos = 0,
			.sub = subs[k],
		};
		struct file *d;

		scnprintf(path, sizeof(path), "/data/nvram/md/NVRAM/%s", subs[k]);
		d = filp_open(path, O_RDONLY | O_DIRECTORY, 0);
		if (IS_ERR(d))
			continue;
		iterate_dir(d, &c.ctx);
		filp_close(d, NULL);
	}
	dev_info(s->dev, "H7j: enumeracion NVRAM = %d ficheros%s%s\n", g_enum_n,
		 g_enum_n ? ", primero: " : "", g_enum_n ? g_enum[0] : "");
}

/* Escribe una ruta ASCII como UTF-16 en el buffer de respuesta y deja los campos.
 * Devuelve la 'length' (4 del contador + 4 del len + los bytes del path). */
static u32 spm_fs_reply_path(void __iomem *fs, u32 boff, const char *ap, u32 nf)
{
	u32 n = strlen(ap), i;
	u32 blen = (n + 1) * 2;			/* bytes UTF-16, terminador incluido */
	u32 pad  = ALIGN(blen, 4);		/* el MD camina con align4(len)+4 */
	u32 off;

	for (i = 0; i < n; i++)
		writew((u16)(u8)ap[i], fs + boff + 0xc + i * 2);
	for (i = blen - 2; i < pad + 2; i += 2)	/* terminador + relleno */
		writew(0, fs + boff + 0xc + i);

	spm_fs_p0(fs, boff, nf);
	writel(blen, fs + boff + 8);		/* len_0 = bytes del path */

	/*
	 * H7m: EL SEGUNDO CAMPO.  Decodificado del ground-truth (gd-boot-full.out
	 * linea 12), la respuesta real del fsd a un FindFirst es:
	 *
	 *   +4    = 2			<- DOS campos
	 *   +8    = 0x36, path		<- 54 bytes, rellenados hasta 56
	 *   +0x44 = 4, 0x700		<- atributos/tipo de la entrada
	 *
	 * Declarabamos 2 campos y escribiamos 1: el MD camina hasta +0x44, lee
	 * basura como len_1 y muere en la comprobacion de capacidad (fs_ccci.c:547)
	 * -> data abort.  Es el mismo bug que mato al 0x1004.  Ademas el path no
	 * se rellenaba a multiplo de 4 (62 en vez de 64) y descuadraba el paseo.
	 */
	off = 8 + 4 + pad;			/* donde cae el campo 1 */
	for (i = 1; i < nf; i++) {
		writel(4, fs + boff + off);
		writel(i == 1 ? spm_fs_find_attr : 0, fs + boff + off + 4);
		off += 8;
	}
	return off - 4;				/* length = contador + campos */
}

/*
 * H8f: la traduccion de ruta era ciega al DISCO.
 *
 * Medido 0730 con spm_fs_quiet=0: tras montar la NVRAM el MD pide ficheros de
 * OTROS volumenes, no solo de Z:
 *     path=[003a0058 005c005c 0050004d]  ->  "X:\\MP0D_000"
 *     path=[003a0058 005c005c 00540053]  ->  "X:\\ST33A004"
 *     path=[003a0059 005c005c 00540053]  ->  "Y:\\ST33B004"
 * Descartabamos los 2 primeros caracteres y lo metiamos TODO en /data/nvram/md,
 * asi que (a) confundiamos tres volumenes en un solo directorio y (b) la barra
 * doble producia rutas con "//" (de ahi el "/data/nvram/md//MP0D_000" del log).
 *
 * Ahora: Z: -> /data/nvram/md (como antes, la NVRAM), y cualquier otro disco a
 * su propio subdirectorio (/data/nvram/md/X, .../Y).  Ademas se colapsan las
 * barras repetidas.
 */
static void spm_fs_path(void __iomem *src, int plen, char *out, int outsz)
{
	int i, o;
	u8 drive = readb(src);			/* letra de volumen: 'Z', 'X', 'Y'... */

	strscpy(out, "/data/nvram/md", outsz);
	o = strlen(out);
	if (drive != 'Z' && drive != 'z' && o < outsz - 3) {
		out[o++] = '/';
		out[o++] = drive;
	}
	/* saltar "<letra>:" (2 chars UTF-16 = 4 bytes) */
	for (i = 4; i + 1 < plen && o < outsz - 1; i += 2) {
		u8 c = readb(src + i);		/* low byte del char UTF-16 */

		if (c == '\\')
			c = '/';
		if (c == '/' && o > 0 && out[o - 1] == '/')
			continue;		/* colapsar barras repetidas */
		out[o++] = c;
	}
	out[o] = 0;
}

/*
 * H6j: encoding del NOT-FOUND del FS (0x1010/0x1011), seleccionable en caliente
 * via /sys/module/mt6582_spm/parameters/spm_fs_nf_mode.  Ver H6j doc.
 */
/* H6m: el bucle de servicio sondeaba cada 25ms; el ccci_fsd real contesta en ~0.5ms
 * (gd.out). Si el MD tiene deadline para el mount FS, 25ms lo revienta. Fase rapida
 * de sondeo (us) configurable en caliente; 0 = busy-poll puro. */
/* H6n: los dev_info del camino FS costaban 20-37ms por op (consola) mientras el
 * ccci_fsd real responde en ~0.5ms. Con spm_fs_quiet=1 se acumulan los op-codes y
 * se vuelcan DESPUES del bucle, dejando el servicio a la velocidad del hardware. */
static uint spm_fs_quiet = 1;
module_param(spm_fs_quiet, uint, 0644);
MODULE_PARM_DESC(spm_fs_quiet, "1 = sin logs durante el servicio FS (volcado al final)");
/*
 * H8k: traza de ops del FS. Antes era un buffer lineal que se llenaba y dejaba
 * de grabar, asi que (a) el contador saturaba en 512 aunque se sirvieran ~900 y
 * (b) los "ultimos" que imprimia eran en realidad los del medio del arranque.
 * Ahora es un ANILLO (guarda los ULTIMOS 512) + un total sin tope.
 */
#define SPM_FSLOG_SZ 512
static u32 spm_fslog[SPM_FSLOG_SZ];
static int spm_fslog_n;			/* cuantos hay en el anillo (<= SZ) */
static int spm_fslog_w;			/* siguiente posicion de escritura */
static int spm_fs_total;		/* total servido en el ciclo, sin tope */
#define fs_dbg(...) do { if (!spm_fs_quiet) dev_info(__VA_ARGS__); } while (0)

static uint spm_fs_fastpoll_us = 50;
module_param(spm_fs_fastpoll_us, uint, 0644);
MODULE_PARM_DESC(spm_fs_fastpoll_us, "periodo de sondeo rapido del bucle FS en us (0=busy)");
static uint spm_fs_slow_iters = 320;	/* H7f: iteraciones de 25ms (ceden CPU) */
module_param(spm_fs_slow_iters, uint, 0644);
MODULE_PARM_DESC(spm_fs_slow_iters, "iteraciones de la fase lenta (x25ms) del bucle FS");
/*
 * H9a: iteraciones que se SIGUEN sirviendo despues del HS2. El bucle salia en
 * cuanto detectaba NORMAL_BOOT_ID, asi que el MD se quedaba sin servicio FS
 * justo al entrar en L1 (la capa de radio) y el registro de excepcion salia
 * limpio porque no le daba tiempo a llegar. 0 = comportamiento historico
 * (salir en el hito, deja la medida de M1 intacta).
 */
static uint spm_fs_post_hs2_iters;
module_param(spm_fs_post_hs2_iters, uint, 0644);
MODULE_PARM_DESC(spm_fs_post_hs2_iters,
		 "iteraciones que se siguen sirviendo tras el HS2 (0 = parar en el hito)");
static uint spm_fs_fastpoll_iters = 10000;
module_param(spm_fs_fastpoll_iters, uint, 0644);
MODULE_PARM_DESC(spm_fs_fastpoll_iters, "iteraciones de la fase rapida antes de pasar a 25ms");

#define SPM_FS_NOREPLY	0xFFFFFFFFu	/* H6o: no contestar (experimento de control) */

/*
 * H6s: que escribimos en el campo +4 de la respuesta.
 *
 * HIPOTESIS: el MD valida el buffer de respuesta con DOS comprobaciones
 * (fs_ccci.c, desensamblado):
 *    linea 520:  buffer[+0] == 0xffff0000|op     <- ESTA LA PASAMOS
 *    linea 528:  buffer[+4] == r6                <- AQUI PETA (leido=2)
 * Nosotros escribimos 2 en +4 (drive-type en el GetDrive, "found" en el 0x1010).
 * El MD pone 1 ahi en su peticion (campo c).  Puede que espere que NO se toque.
 *   0 = comportamiento actual
 *   1 = NO TOCAR +4 (dejar el valor que puso el MD)   <- candidato principal
 *   2 = escribir 0
 *   3 = escribir 1 (eco del c de la peticion)
 */
/*
 * H6v: valor de 4 bytes que devolvemos en el UNICO campo de la respuesta al 0x1010.
 *
 * El desensamblado del bucle de fs_ccci.c (0729) demostro que el buffer de
 * respuesta es una LISTA DE CAMPOS con longitud:
 *      +0 = 0xffff0000|op | +4 = c = NUMERO DE CAMPOS | +8 = len_0 | +0xc = datos_0 | ...
 * y que para cada campo el MD tiene reservada una CAPACIDAD fija (fp[i]) sobre la
 * que hace memcpy: exige capacidad >= len.  El 0x1010 llega con c=1 y capacidad 4
 * -> la respuesta es UN campo de 4 bytes, no un path.  Devolvemos 0 y el MD acaba
 * recorriendo la NVRAM en circulo hasta el op 0x1012 (que no existe en el arranque
 * real).  Este parametro permite barrer ese valor sin recompilar:
 *   echo 0xfffff057 > /sys/module/mt6582_spm/parameters/spm_fs_1010_val
 * (acepta hex con prefijo 0x; candidatos: 0, 1, 0xffffffff y la serie de codigos
 *  del modulo FS 0xfffff055..0xfffff05d = -4011..-4003)
 */
/*
 * H7a: numero de campos de la respuesta generica (ops sin handler).
 * El 0x1012 asserta en la linea 528 con valor 1 leido -> espera otro contador.
 * Como 0x1012 no existe en el arranque real, se barre: 0,1,2,3.
 * 0xff = no tocar +4 (dejar el de la peticion).
 */
/*
 * H7b: contador de campos PROPIO del op 0x1012.
 * Barrido del kernel #49: el 0x1007 SOLO acepta 1 campo y el 0x1012 rechaza
 * tanto 1 como 4 -> el contador es POR OP, no global.
 */
/*
 * H7c: override GENERICO del contador de campos para UN op cualquiera.
 * Cada op nuevo que aparece pide su propio numero de campos (medido: 1007->1,
 * 1012->3), asi que en vez de un parametro por op, dos parametros que apuntan
 * a cualquiera:
 *   echo 0x1004 > spm_fs_ov_op ; echo 2 > spm_fs_ov_nf
 */
static uint spm_fs_ov_op = 0x1004;
module_param(spm_fs_ov_op, uint, 0644);
MODULE_PARM_DESC(spm_fs_ov_op, "op al que aplicar el override del contador de campos");
static uint spm_fs_ov_nf = 1;
module_param(spm_fs_ov_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_ov_nf, "campos de la respuesta para spm_fs_ov_op (0xff = no tocar)");

static uint spm_fs_1012_nf = 3;
module_param(spm_fs_1012_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_1012_nf, "campos de la respuesta del op 0x1012 (0xff = no tocar)");

static uint spm_fs_def_nf = 1;
module_param(spm_fs_def_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_def_nf, "campos de la respuesta generica (0..3; 0xff = no tocar)");

static uint spm_fs_1010_val;
module_param(spm_fs_1010_val, uint, 0644);
MODULE_PARM_DESC(spm_fs_1010_val, "valor de 4 bytes del campo de respuesta del 0x1010 (acepta 0x...)");

/*
 * H6x: +4 = NUMERO DE CAMPOS DE LA RESPUESTA (no el 'c' de la peticion).
 *
 * Medido 0729 (kernel #44): poner "no tocar" por defecto rompio el OPEN, que
 * antes funcionaba escribiendo 1.  La peticion del OPEN trae c=2 pero su
 * respuesta lleva UN solo campo (el handle) -> +4 = 1.  El GetDrive coincide en
 * 2 por casualidad (peticion c=2, respuesta 2 campos).  El 0x1010 pide 1 campo,
 * y por eso "no tocar" (dejaba el 1 de la peticion) tambien funcionaba.
 * Asi que cada handler declara los suyos:  0 = handler, 1 = no tocar, ...
 */
static uint spm_fs_p0_mode;
module_param(spm_fs_p0_mode, uint, 0644);
MODULE_PARM_DESC(spm_fs_p0_mode, "campo +4 de la respuesta: 0=actual 1=no-tocar 2=cero 3=eco del c");

/*
 * H8a: modo 9 POR DEFECTO — es el que arranca el MD.
 *
 * Medido 0730 en HW: con el default anterior (0) el arranque se quedaba en 2
 * operaciones; con el modo 9 (respuesta de forma corta: 1 campo de 4 bytes, sin
 * tocar +4) el MD monta su NVRAM ENTERA y alcanza
 *   *** HS2 LOGRADO: NORMAL_BOOT_ID (stage 2 = M1 COMPLETO) ***
 * Con spm_fs_1010_val=0 (el default) hace 894 peticiones, contra las 883 del
 * ccci_fsd real (gd-boot-full.out) -> el mount es equivalente al de fabrica.
 * Con 0xffffffff tambien llega a HS2 pero solo con 476 (mount a medias).
 */
/*
 * H8b: support_mask del misc_info — el bloqueo de L1 (m12100.c:9064).
 *
 * Desensamblado (0730): el assert de la tarea L1 llama a una funcion en 0x6148
 * con (r0=1, r1=0, r2=0) y exige que devuelva 1 o 2.  Esa funcion hace:
 *     r1 = runtime[+0x108]        -> MiscInfoBase   (offset 264 = indice 66 ✓)
 *     if (*r1 != 'CCIF') return 0                   <- el prefijo SI lo escribimos
 *     return (*(r1+4) >> (r0*2)) & 3                <- support_mask, 2 BITS POR FEATURE
 * Con r0=1 mira los bits [3:2] = MISC_32K_LESS.  Escribiamos support_mask=0x1,
 * asi que esos bits valian 00 = FEATURE_NOT_EXIST -> devolvia 0 -> ASSERT.
 *
 * Enums del stock (ccci_common.h):
 *   estado : NOT_EXIST=0  NOT_SUPPORT=1  SUPPORT=2  PARTIALLY_SUPPORT=3
 *   feature: MISC_DMA_ADDR=0  MISC_32K_LESS=1  MISC_RAND_SEED=2  MD_COCLK=3
 * El MD necesita SABER si hay cristal de 32kHz; "no me consta" no le vale.
 * Candidatos (feature1 en bits[3:2]):
 *   0x5 = DMA_ADDR:1  32K_LESS:1(NOT_SUPPORT)   <- hay 32kHz externo
 *   0x9 = DMA_ADDR:1  32K_LESS:2(SUPPORT)       <- el sistema es "32k-less"
 *   0x6 / 0xa = idem con DMA_ADDR:2
 */
/*
 * H8c: puertos TTY del CCCI — el bloqueo de la tarea UPS (ccci_uart_drv.c:2594).
 *
 * Desensamblado (0730) del assert en 0x230c: para un puerto cuyo bit NO esta en
 * la mascara de activos, el MD EXIGE que su descriptor de memoria compartida
 * este A CERO:
 *     r1 = tabla[puerto].descriptor
 *     if (*(r1+0) != 0) ASSERT      <- rx.read   (ya era 0)
 *     if (*(r1+8) != 0) ASSERT      <- rx.LENGTH (poniamos 16K)
 * H7r inicializaba rx.length/tx.length = 16K, lo que avanzo el assert de la
 * linea 793 a la 2594, pero el MD no quiere longitudes en puertos que el AP no
 * ha abierto.  Dos salidas posibles, ambas conmutables:
 *   spm_md_uart_ports = 0  -> no declarar ningun puerto
 *   spm_md_uart_len   = 0  -> declararlos con el descriptor entero a cero
 */
/*
 * H8e: ZERAR las regiones de control de RED — el assert de UPS
 * (ccci_uart_drv.c:2594).
 *
 * Localizado el inicializador en 0x225a del firmware:
 *     r2 = 0xf0873eb8  (la copia interna del runtime, la misma que mira L1)
 *     r0 = *(r2 + 0xb8)   = NetULCtrlShareMemBase[0]   (offset 184)
 *     r6 = *(r2 + 0xd8)   = NetDLCtrlShareMemBase[0]   (offset 216)
 *     str r0, [r3, #4]    -> tabla[puerto].campo_4 = NetULCtrlBase
 * Y el assert exige que *(campo_4 + 0) y *(campo_4 + 8) sean CERO para un canal
 * que no esta activo.  Declarabamos las 6 regiones (indices 46-48 y 54-56) pero
 * NUNCA las zerabamos -> basura del carveout -> assert.  Mismo patron que el
 * misc_info (ver el comentario del memset ahi).
 *
 * Nota: el fichero se llama ccci_uart_drv.c pero es el driver GENERICO de
 * puertos del CCCI; la tabla es de red, no de UART.  Por eso tocar los
 * descriptores TTY (H8c/H8d) no cambio nada.
 */
static uint spm_md_zero_net = 1;
module_param(spm_md_zero_net, uint, 0644);
MODULE_PARM_DESC(spm_md_zero_net, "1 = zerar las regiones NetUL/DLCtrl antes de arrancar el MD");

static uint spm_md_uart_ports = 6;
module_param(spm_md_uart_ports, uint, 0644);
MODULE_PARM_DESC(spm_md_uart_ports, "puertos TTY del CCCI a declarar (0..6)");
static uint spm_md_uart_len = CCCI_TTY_BUF_SIZE;	/* como el stock; medido: no influye en el assert de UPS */
module_param(spm_md_uart_len, uint, 0644);
MODULE_PARM_DESC(spm_md_uart_len, "longitud declarada en rx/tx del TTY (0 = a cero)");

static uint spm_md_misc_mask = 0x5;
module_param(spm_md_misc_mask, uint, 0644);
MODULE_PARM_DESC(spm_md_misc_mask, "support_mask del misc_info: 2 bits por feature (acepta 0x...)");

static uint spm_fs_1010_mode = 9;	/* H8a: 9 = el que arranca el MD */
module_param(spm_fs_1010_mode, uint, 0644);
MODULE_PARM_DESC(spm_fs_1010_mode, "formato de respuesta del op 0x1010: 0=actual 1=eco+len 2=len-corto 3=c1 4=getdrive-like 5=vacio 6=eco+NUL 7=ground-truth");

static uint spm_fs_nf_mode = 1;
module_param(spm_fs_nf_mode, uint, 0644);
MODULE_PARM_DESC(spm_fs_nf_mode, "FS not-found encoding: 0=orig 1=limpio 2=cnt1 3=cnt2 4=ENOENT 5=fake-found 6=bit31");

/* H6s: escribe el campo +4 de la respuesta segun spm_fs_p0_mode. */
static void spm_fs_p0(void __iomem *fs, u32 boff, u32 val)
{
	switch (spm_fs_p0_mode) {
	case 1:				/* NO TOCAR: dejar lo que puso el MD */
		return;
	case 2:				/* cero */
		writel(0, fs + boff + 4);
		return;
	case 3:				/* eco del c de la peticion (=1 tipicamente) */
		writel(1, fs + boff + 4);
		return;
	default:
		writel(val, fs + boff + 4);
		return;
	}
}

static u32 spm_fs_serve(struct mt6582_spm *s, void __iomem *fs, u32 idx)
{
	u32 nf_extra = 0;			/* H6j: bits extra para la marca done */
	u32 boff = idx * 0x4004;
	u32 fs_ops = readl(fs + boff + 0);
	u32 op = fs_ops & 0xffff;
	/*
	 * H8j: ESTRUCTURA DE LA PETICION (y de la respuesta) — lista de campos
	 * con longitud, confirmada contra el ground-truth del fsd de Lineage:
	 *
	 *   +0    op                marca; en la respuesta 0xffff0000|op, escrita AL FINAL
	 *   +4    nfields           numero de campos
	 *   +8    len_0
	 *   +0xc  datos_0           (alineado a 4)
	 *         len_1  datos_1    ... y asi nfields veces
	 *
	 * Por eso, con un primer campo de 4 bytes (un handle): datos_0 en +0xc,
	 * len_1 en +0x10 y datos_1 en +0x14. Leer +0x10 esperando datos es el
	 * error que costo tres bugs (H6z, H7b, H8i) — solo el WRITE lee +0x10
	 * a proposito, porque ahi el len_1 ES la longitud de los datos.
	 */
	u32 len0 = readl(fs + boff + 8);	/* len_0: long. del 1er campo */
	u32 length = 0;				/* bytes de payload -> data1 = length+4 */

	spm_fs_total++;				/* H6n/H8k: traza compacta, anillo */
	spm_fslog[spm_fslog_w] = op;
	spm_fslog_w = (spm_fslog_w + 1) % SPM_FSLOG_SZ;
	if (spm_fslog_n < SPM_FSLOG_SZ)
		spm_fslog_n++;

	switch (op) {
	case 0x1001: {				/* OPEN */
		char path[160];
		struct file *f;
		int h, oflag = O_RDWR;
		u32 fl = 0;
		/*
		 * H8i: los FLAGS del OPEN vienen en el SEGUNDO CAMPO, detras del path
		 * — no en +4, que es el CONTADOR DE CAMPOS (vale 2 y por eso los bits
		 * nunca se activaban: jamas pasabamos O_CREAT).
		 *
		 * Medido 0730 volcando la peticion entera (H8h):
		 *   w0=00001001 w1=00000002 w2=0000001a  <- op, nfields, len_0
		 *   w3..w9 = "X:\\MP0D_000"             (26 bytes, align4 -> 28)
		 *   w10=00000004                        <- len_1
		 *   w11=01010400                        <- FLAGS  (bit 0x10000 = crear)
		 * Asi que el offset depende de la longitud del path:
		 *   flags_off = 0xc + align4(len_0) + 4
		 * Las constantes del mapeo (0x10000 / 0x20000) ya eran correctas; lo
		 * que estaba mal era de donde se leia el valor.
		 */
		{
			u32 plen0 = readl(fs + boff + 8);
			u32 foff = 0xc + ((plen0 + 3) & ~3u);

			if (plen0 <= 144 && readl(fs + boff + foff) == 4)
				fl = readl(fs + boff + foff + 4);
		}
		/* mapeo flags FS->oflag (Ghidra H6d) */
		if (fl & 0x10000)  oflag = O_RDWR | O_CREAT;
		if (fl & 0x20000)  oflag = O_RDWR | O_CREAT | O_TRUNC;
		spm_fs_path(fs + boff + 0xc, len0, path, sizeof(path));	/* path desde +0xc */
		f = filp_open(path, oflag, 0660);
		/*
		 * H6w FIX B: el MD hace OPEN sobre DIRECTORIOS ("Z:\\NVRAM"), y con
		 * O_RDWR eso falla con EISDIR.  Reintentar en solo lectura.
		 */
		if (IS_ERR(f))
			f = filp_open(path, O_RDONLY | O_DIRECTORY, 0);
		if (IS_ERR(f))
			f = filp_open(path, O_RDONLY, 0);
		for (h = 1; h < FS_NR_HANDLES; h++)
			if (!g_fs_h[h])
				break;
		if (IS_ERR(f) || h >= FS_NR_HANDLES) {
			/*
			 * H6w FIX A: NO marcar el error con el bit31.
			 * Medido 0729: la comprobacion de la linea 520 de fs_ccci.c exige
			 * buffer[+0] == 0xffff0000|op SIEMPRE.  Con el bit31 el MD leia
			 * 0x80001001 donde esperaba 0xffff1001 -> assert inmediato (params
			 * del registro: esperado ffff1001, recibido 80001001).  El error se
			 * senaliza en el PAYLOAD, no en la marca.
			 */
			if (spm_fs_trace && spm_fs_trace[0] && strstr(path, spm_fs_trace))
				dev_info(s->dev, "H10a OPEN '%s' fl=%08x oflag=%x -> FALLO %ld\n",
					 path, fl, oflag,
					 IS_ERR(f) ? PTR_ERR(f) : 0L);
			dev_warn(s->dev, "H6 FS OPEN %s fallo (err en payload)\n", path);
			spm_fs_p0(fs, boff, 1);			/* H6x: 1 campo */
			writel(4, fs + boff + 8);
			writel(spm_fs_1010_val, fs + boff + 0xc);
			length = 12;
			break;
		}
		g_fs_h[h] = f;
		if (spm_fs_trace && spm_fs_trace[0] && strstr(path, spm_fs_trace)) {
			g_fs_trace_h = h;
			dev_info(s->dev, "H10a OPEN '%s' fl=%08x oflag=%x -> h=%d\n",
				 path, fl, oflag, h);
		}
		/* ground-truth: resp = [count=1][len=4][handle] en +4/+8/+0xc (NO handle en +4) */
		spm_fs_p0(fs, boff, 1);	/* H6w FIX C: +4 = numero de campos */
		writel(4, fs + boff + 8);
		writel(h, fs + boff + 0xc);
		length = 12;
		fs_dbg(s->dev, "H6 FS OPEN %s -> h=%d\n", path, h);
		break;
	}
	case 0x1009: {				/* STAT/GetSize: handle@+0xc -> tamaño 64b en +0x14/+0x18 */
		u32 hnd = readl(fs + boff + 0xc);
		u64 sz = 0;
		if (hnd < FS_NR_HANDLES && g_fs_h[hnd]) {
			struct kstat st;
			if (vfs_getattr(&g_fs_h[hnd]->f_path, &st, STATX_SIZE, 0) == 0)
				sz = st.size;
		}
		/*
		 * H6z: este handler escribe DOS campos (handle y tamaño) pero declaraba
		 * UNO en +4, y ademas ponia len=4 para un tamaño de 64 bits.  Medido
		 * 0729 (kernel #46): el MD asserta en la linea 528 con valor 1 leido.
		 *   +4    = 2      numero de campos
		 *   +8    = 4      len_0  -> +0xc  = handle
		 *   +0x10 = 8      len_1  -> +0x14 = tamaño (64b, dos palabras)
		 */
		spm_fs_p0(fs, boff, 2);			/* 2 campos */
		writel(4, fs + boff + 8);
		writel(hnd, fs + boff + 0xc);
		/* H6z-b: la capacidad del 2o campo es 4, no 8 (medido: assert 547 con
		 * params 4 y 8) -> el tamaño va en 32 bits. */
		writel(4, fs + boff + 0x10);		/* len_1 = 4 */
		writel((u32)sz, fs + boff + 0x14);	/* tamaño (32b) */
		length = 0x14;
		if (g_fs_trace_h >= 0 && hnd == (u32)g_fs_trace_h)
			dev_info(s->dev, "H10a STAT h=%u -> size=%llu\n", hnd, sz);
		fs_dbg(s->dev, "H6 FS STAT h=%u -> size=%llu\n", hnd, sz);
		break;
	}
	/*
	 * SEEK: handle@+0xc, offset@+0x14, whence@+0x1c -> newpos en +0xc.
	 * H8j: el whence es 1 en las 97 peticiones del ground-truth y significa
	 * "desde el principio" (la respuesta real devuelve el offset absoluto
	 * pedido), asi que SEEK_SET es correcto y no hace falta leerlo.
	 * La respuesta lleva 1 campo con la posicion — igual que el fsd real.
	 */
	case 0x1002: {
		u32 hnd = readl(fs + boff + 0xc);
		u32 off = readl(fs + boff + 0x14);
		loff_t np = off;
		if (hnd < FS_NR_HANDLES && g_fs_h[hnd])
			np = vfs_llseek(g_fs_h[hnd], off, SEEK_SET);
		spm_fs_p0(fs, boff, 1);	/* H6w FIX C: +4 = numero de campos */
		writel(4, fs + boff + 8);
		writel((u32)np, fs + boff + 0xc);	/* nueva posicion */
		if (g_fs_trace_h >= 0 && hnd == (u32)g_fs_trace_h)
			dev_info(s->dev, "H10a SEEK h=%u off=%u -> pos=%lld\n",
				 hnd, off, (long long)np);
		length = 0x1c;
		break;
	}
	case 0x1003: {				/* READ: handle@+0xc, len@+0x14 -> nread + datos @+0x20 */
		u32 hnd = readl(fs + boff + 0xc);
		u32 rlen = readl(fs + boff + 0x14);
		int nread = 0;
		if (rlen > 0x3f00) rlen = 0x3f00;	/* cabe en el buffer (16388) */
		if (hnd < FS_NR_HANDLES && g_fs_h[hnd]) {
			u8 *tmp = kmalloc(rlen, GFP_KERNEL);	/* NO en el stack (8KB) */
			if (tmp) {
				nread = kernel_read(g_fs_h[hnd], tmp, rlen, &g_fs_h[hnd]->f_pos);
				/*
				 * H10b: los datos van en +0x1c, NO en +0x20.  El 3er campo
				 * tiene su longitud en +0x18, asi que el MD empieza a leer
				 * en +0x1c (camina con align4(len)+4).  Ground-truth:
				 *   ffff1003 | 3 | 4,0 | 4,0x35a | 0x35a, 1234abcd...
				 *                                          ^ datos en +0x1c
				 * Con el desplazamiento el MD leia 4 ceros delante y perdia
				 * los ultimos 4 bytes.  Casi nada del arranque comprueba el
				 * contenido, por eso la cascada funcionaba igual; lo destapa
				 * la validacion de L4 (nvram_io.c:1202, error 10).
				 */
				if (nread > 0)
					memcpy_toio(fs + boff + 0x1c, tmp, nread);
				if (g_fs_trace_h >= 0 && hnd == (u32)g_fs_trace_h) {
					char hx[64];
					int q, hn = 0;

					for (q = 0; q < nread && q < 12; q++)
						hn += scnprintf(hx + hn, sizeof(hx) - hn,
								"%02x ", tmp[q]);
					hx[hn] = 0;
					dev_info(s->dev, "H10a READ h=%u len=%u pos=%lld -> %d [%s]\n",
						 hnd, rlen,
						 (long long)g_fs_h[hnd]->f_pos, nread, hx);
				}
				kfree(tmp);
			}
		}
		if (nread < 0) nread = 0;
		spm_fs_p0(fs, boff, 3);	/* H6w FIX C: +4 = numero de campos */
		writel(4, fs + boff + 8);
		writel(0, fs + boff + 0xc);		/* result OK */
		writel(4, fs + boff + 0x10);
		writel(nread, fs + boff + 0x14);
		writel(nread, fs + boff + 0x18);	/* len_2 -> datos en +0x1c (H10b) */
		length = 0x1c + nread;			/* cabecera 0x1c + datos */
		fs_dbg(s->dev, "H6 FS READ h=%u len=%u -> %d\n", hnd, rlen, nread);
		break;
	}
	case 0x1004: {				/* WRITE: handle@+0xc, len@+0x10, datos@+0x14 */
		u32 hnd  = readl(fs + boff + 0xc);
		u32 wlen = readl(fs + boff + 0x10);
		int nwr = 0;

		if (wlen > 0x3f00)			/* tope: cabe en el buffer (16388) */
			wlen = 0x3f00;
		if (hnd < FS_NR_HANDLES && g_fs_h[hnd] && wlen) {
			u8 *tmp = kmalloc(wlen, GFP_KERNEL);	/* NO en el stack */

			if (tmp) {
				memcpy_fromio(tmp, fs + boff + 0x14, wlen);
				nwr = kernel_write(g_fs_h[hnd], tmp, wlen,
						   &g_fs_h[hnd]->f_pos);
				kfree(tmp);
			}
		}
		if (nwr < 0)
			nwr = 0;
		/*
		 * H7e: formato exacto del ground-truth (gd-boot-full.out):
		 *   c=2 | len=4, status(0=OK) | len=4, bytes escritos
		 */
		spm_fs_p0(fs, boff, 2);
		writel(4, fs + boff + 8);
		writel(0, fs + boff + 0xc);		/* status OK */
		writel(4, fs + boff + 0x10);
		writel(nwr, fs + boff + 0x14);		/* bytes escritos */
		length = 0x14;				/* 4 + 2*8 */
		fs_dbg(s->dev, "H6 FS WRITE h=%u len=%u -> %d\n", hnd, wlen, nwr);
		break;
	}
	case 0x1005: {				/* CLOSE: handle@+0xc */
		u32 hnd = readl(fs + boff + 0xc);
		if (g_fs_trace_h >= 0 && hnd == (u32)g_fs_trace_h) {
			dev_info(s->dev, "H10a CLOSE h=%u\n", hnd);
			g_fs_trace_h = -1;
		}
		if (hnd < FS_NR_HANDLES && g_fs_h[hnd]) {
			filp_close(g_fs_h[hnd], NULL);
			g_fs_h[hnd] = NULL;
		}
		spm_fs_p0(fs, boff, 1);	/* H6w FIX C: +4 = numero de campos */
		writel(4, fs + boff + 8);
		writel(0, fs + boff + 0xc);
		length = 0xc;
		break;
	}
	case 0x100e: {				/* GetDrive (mount de "Z:\") — GROUND-TRUTH capturado
		 * del ccci_fsd de Lineage en boot FRIO (wrapper+poller, sesion Windows 0720):
		 * respuesta = [result][02][04][00][54]. El default (length 0) hacia EXCP el MD. */
		/*
		 * H7o: el 0x54 NO es un escalar.  Con la regla de campos ya conocida
		 * ([len_i][data_i], el MD camina con align4(len)+4) la respuesta real
		 * del fsd (gd-boot-full.out linea 7) es:
		 *
		 *   +4    = 2			<- dos campos
		 *   +8    = 4, 0		<- status OK
		 *   +0x10 = 0x54, 84 bytes A CERO	<- info de la unidad
		 *   length = 4 + (4+4) + (4+84) = 100
		 *
		 * Se dedujo de una captura parcial como "[02][04][00][54] len=16": el
		 * len_1 = 0x54 casaba de milagro (era un resto de la peticion) pero los
		 * 84 bytes quedaban con BASURA y anunciabamos solo 16.  El MD se guarda
		 * esa info de unidad corrupta y aborta cuatro ops despues, al procesar
		 * el FindFirst (cuya respuesta ya es identica byte a byte al GT).
		 */
		spm_fs_p0(fs, boff, 2);
		writel(4, fs + boff + 8);		/* len_0 */
		writel(0, fs + boff + 0xc);		/* status OK */
		writel(0x54, fs + boff + 0x10);		/* len_1 = 84 */
		memset_io(fs + boff + 0x14, 0, 0x54);	/* info de unidad, a cero */
		length = 4 + 8 + 4 + 0x54;		/* = 100 */
		fs_dbg(s->dev, "H6 FS GetDrive Z:\\ -> GT: 2 campos, len=%u\n", length);
		break;
	}
	case 0x1010:
	case 0x1011: {
		/* GetFullPath/FileExists: chequeo REAL. Si el path existe -> found (2) + eco;
		 * si NO (p.ej. "Z:\FAT..log" de recovery, ausente en NVRAM limpia) -> NOT-FOUND
		 * (error bit31) para que el MD siga el mount limpio. */
		char path[160];
		struct file *f;
		u32 plen = readl(fs + boff + 8);
		spm_fs_path(fs + boff + 0xc, plen, path, sizeof(path));

		/*
		 * H7j: FindFirst (0x1010) / FindNext (0x1011) de VERDAD.
		 * El fsd real responde con la RUTA COMPLETA del fichero encontrado y el
		 * MD sigue con GetSize+READ; devolver un error (lo que haciamos) le mete
		 * en un bucle de recuperacion que nunca llega a NORMAL_BOOT_ID.
		 */
		/* H7k: la enumeracion SOLO vale para consultas de la NVRAM.  El primer
		 * 1010 del arranque pregunta por "Z:\FAT....log" (log de recuperacion
		 * ausente) y el fsd real contesta NO ENCONTRADO; si le devolvemos una ruta
		 * de la NVRAM, el MD excepciona al instante. */
		if (spm_fs_enum && strstr(path, "/NVRAM")) {
			if (op == 0x1010)
				spm_fs_enum_build(s);	/* FindFirst: (re)construir y cursor a 0 */
			else if (g_enum_i < g_enum_n)
				g_enum_i++;		/* FindNext: avanzar */

			if (spm_fs_find_mode == 0) {
				/*
				 * H7q: escalar de 4 bytes.  Es lo unico que el MD
				 * acepta aqui (capacidad 4, ver arriba); el valor
				 * se barre en caliente con spm_fs_find_val.
				 */
				spm_fs_p0(fs, boff, 1);
				writel(4, fs + boff + 8);
				writel(spm_fs_find_val, fs + boff + 0xc);
				length = 12;
				fs_dbg(s->dev, "H7q FIND op=%x -> escalar %08x\n",
				       op, spm_fs_find_val);
			} else if (g_enum_i < g_enum_n) {
				length = spm_fs_reply_path(fs, boff,
							   g_enum[g_enum_i],
							   spm_fs_find_nf);
				fs_dbg(s->dev, "H7j FIND op=%x [%d/%d] -> %s\n",
				       op, g_enum_i, g_enum_n, g_enum[g_enum_i]);
			} else {			/* agotado: no hay mas ficheros */
				spm_fs_p0(fs, boff, 1);	/* H7m: 1 campo declarado = 1 escrito */
				writel(4, fs + boff + 8);
				writel(spm_fs_1010_val, fs + boff + 0xc);
				length = 12;
				fs_dbg(s->dev, "H7j FIND op=%x -> AGOTADO (%d)\n", op, g_enum_n);
			}
			break;
		}

		/* H6l: formatos alternativos de respuesta, seleccionables en caliente.
		 * El modo 0 conserva el comportamiento actual (found/not-found real). */
		if (spm_fs_1010_mode) {
			u32 m = spm_fs_1010_mode;

			if (m == 8) {		/* H6o: control -> no contestar nada */
				dev_info(s->dev, "H6 FS DIR op=%x %s -> SIN RESPUESTA (control)\n",
					 op, path);
				return SPM_FS_NOREPLY;
			}

			/*
			 * H6t: respuesta con la FORMA que el MD acepta.
			 *
			 * Del desensamblado de fs_ccci.c (0729) salen DOS reglas, ambas
			 * confirmadas midiendo con el registro de excepcion limpio:
			 *   linea 528: buffer[+4] debe conservar el 'c' de la PETICION.
			 *              (GetDrive llega con c=2 y escribiamos 2 -> pasaba;
			 *               el 0x1010 llega con c=1 y escribiamos 2 -> petaba)
			 *   linea 547: buffer[+8] (longitud) NO puede exceder lo que el MD
			 *              reservo -> aqui midio 4; nosotros devolviamos 0x26.
			 * Asi que: NO tocar +4 y devolver una longitud corta.
			 *   9  = +8=4, +0xc=0, length=12
			 *   10 = clon exacto de la forma del GetDrive (+8=4, +0xc=0,
			 *        +0x10=0x54, length=16), que es la unica que el MD acepta.
			 */
			/*
			 * H6u: RESOLUCION DE RUTAS (lo que hace el ccci_fsd real).
			 *
			 * Ground-truth (gd-boot-full.out, unica respuesta 1010 capturada):
			 *    REQ  1010 c=1 len=0x24 "Z:\NVRAM\NVD_DATA"
			 *    RESP 1010 c=2 len=0x36 "Z:\NVRAM\NVD_CORE\MT00A000"
			 * O sea: el fsd NO hace eco, TRADUCE el nombre logico a un fichero
			 * fisico.  Contestando "vacio" (modos 9/10) el MD recorre toda la
			 * NVRAM y acaba en el op 0x1012, que NO existe en el arranque real
			 * -> se va por una rama que el original nunca pisa.
			 *
			 *   11 = replicar el ground-truth (mapeo fijo), +4 INTACTO
			 *   12 = idem pero escribiendo c=2 en +4 (como el fsd real).  OJO:
			 *        escribir 2 nos hacia petar en la linea 528, asi que la
			 *        expectativa del MD depende de su estado -> hay que medirlo.
			 *   13 = generico: si la ruta pedida es un DIRECTORIO que existe,
			 *        devolver el primer fichero que contenga.
			 */
			if (m == 11 || m == 12 || m == 13) {
				static const char *gt = "Z:\\NVRAM\\NVD_CORE\\MT00A000";
				char res[128];
				const char *out = NULL;
				int n;

				if (m == 13) {
					/* primer fichero dentro del directorio pedido */
					struct file *d = filp_open(path, O_RDONLY | O_DIRECTORY, 0);

					if (!IS_ERR(d)) {
						filp_close(d, NULL);
						/* sin readdir en este contexto: usamos el mapeo
						 * conocido si la ruta acaba en NVD_DATA */
						if (strstr(path, "NVD_DATA"))
							out = gt;
					}
				} else if (strstr(path, "NVD_DATA")) {
					out = gt;
				}

				if (!out) {		/* sin resolucion -> forma corta (modo 9) */
					spm_fs_p0(fs, boff, 1);	/* H6x: 1 campo */
					writel(4, fs + boff + 8);
					writel(spm_fs_1010_val, fs + boff + 0xc);	/* H6v */
					length = 12;
					dev_info(s->dev, "H6u op=%x %s -> sin resolucion (corta)\n",
						 op, path);
					break;
				}

				/* escribir la ruta resuelta en UTF-16LE a partir de +0xc */
				n = 0;
				while (out[n] && n < 120) {
					writew((u16) out[n], fs + boff + 0xc + n * 2);
					n++;
				}
				writew(0, fs + boff + 0xc + n * 2);	/* NUL */
				writel((n + 1) * 2, fs + boff + 8);	/* len en BYTES, con NUL */
				if (m == 12)
					spm_fs_p0(fs, boff, 2);	/* H6w FIX C: +4 = numero de campos */	/* como el fsd real */
				length = 8 + (n + 1) * 2;
				scnprintf(res, sizeof(res), "%s", out);
				dev_info(s->dev, "H6u op=%x %s -> RESUELTO '%s' len=%d (modo %u)\n",
					 op, path, res, (n + 1) * 2, m);
				break;
			}

			if (m == 9 || m == 10) {
				spm_fs_p0(fs, boff, 1);		/* H6x: 1 campo en la respuesta */
				writel(4, fs + boff + 8);	/* len_0 = 4 (capacidad del MD) */
				writel(spm_fs_1010_val, fs + boff + 0xc);	/* H6v */
				if (m == 10) {
					writel(0x54, fs + boff + 0x10);
					length = 16;
				} else {
					length = 12;
				}
				dev_info(s->dev, "H6t op=%x %s -> forma corta (modo %u, +4 intacto)\n",
					 op, path, m);
				break;
			}

			if (m == 7) {		/* ground-truth exacto del gd.out */
				static const char gt[] = "Z:\\NVRAM\\NVD_CORE\\MT00A000";
				u32 i, n = sizeof(gt) - 1;	/* 26 chars, sin NUL */

				for (i = 0; i < n; i++)
					writew((u16)gt[i], fs + boff + 0xc + i * 2);
				writew(0, fs + boff + 0xc + n * 2);
				spm_fs_p0(fs, boff, 2);	/* H6w FIX C: +4 = numero de campos */
				writel((n + 1) * 2, fs + boff + 8);	/* 0x36 */
				length = 8 + (n + 1) * 2;
			} else if (m == 4) {	/* formato corto tipo GetDrive */
				spm_fs_p0(fs, boff, 2);	/* H6w FIX C: +4 = numero de campos */
				writel(4, fs + boff + 8);
				writel(0, fs + boff + 0xc);
				length = 16;
			} else if (m == 5) {	/* vacio limpio */
				spm_fs_p0(fs, boff, 0);	/* H6w FIX C: +4 = numero de campos */
				writel(0, fs + boff + 8);
				length = 8;
			} else {		/* 1,2,3,6: variaciones sobre el eco */
				spm_fs_p0(fs, boff, (m == 3) ? 1 : 2);	/* H6w FIX C: +4 = numero de campos */
				writel(plen, fs + boff + 8);
				if (m == 6 && plen >= 2)
					writew(0, fs + boff + 0xc + plen - 2);
				length = (m == 2) ? plen : 8 + plen;
			}
			fs_dbg(s->dev, "H6 FS DIR op=%x %s -> MODO %u (len=%u)\n",
				 op, path, m, length);
			break;
		}

		f = filp_open(path, O_RDONLY, 0);
		if (IS_ERR(f)) {
			/* H6j: variantes del encoding de NOT-FOUND (spm_fs_nf_mode). */
			switch (spm_fs_nf_mode) {
			case 0:		/* ORIGINAL: +4=0 y +8 SIN TOCAR (conserva el len de la peticion) */
				spm_fs_p0(fs, boff, 0);	/* H6w FIX C: +4 = numero de campos */
				length = 0;
				break;
			case 2:		/* count=1, len=0 */
				spm_fs_p0(fs, boff, 1);	/* H6w FIX C: +4 = numero de campos */
				writel(0, fs + boff + 8);
				length = 8;
				break;
			case 3:		/* count=2 (como el FOUND) pero sin datos */
				spm_fs_p0(fs, boff, 2);	/* H6w FIX C: +4 = numero de campos */
				writel(0, fs + boff + 8);
				length = 8;
				break;
			case 4:		/* ENOENT (-2) en el status */
				spm_fs_p0(fs, boff, (u32)-2);	/* H6w FIX C: +4 = numero de campos */
				writel(0, fs + boff + 8);
				length = 8;
				break;
			case 5:		/* fingir FOUND: eco del path pedido */
				spm_fs_p0(fs, boff, 2);	/* H6w FIX C: +4 = numero de campos */
				length = 8 + plen;
				break;
			case 6:		/* limpio + bit31 de error en la marca done */
				spm_fs_p0(fs, boff, 0);	/* H6w FIX C: +4 = numero de campos */
				writel(0, fs + boff + 8);
				length = 8;
				nf_extra = 0x80000000;
				break;
			case 1:		/* LIMPIO (defecto): +4=0 y +8=0 -> sin len fantasma */
			default:
				spm_fs_p0(fs, boff, 0);	/* H6w FIX C: +4 = numero de campos */
				writel(0, fs + boff + 8);
				length = 8;
				break;
			}
			fs_dbg(s->dev, "H6 FS DIR op=%x %s -> NOT FOUND (nf_mode=%u len=%u)\n",
				 op, path, spm_fs_nf_mode, length);
			break;
		}
		filp_close(f, NULL);
		spm_fs_p0(fs, boff, 2);			/* found (H6s) */
		length = 8 + plen;
		fs_dbg(s->dev, "H6 FS DIR op=%x %s -> FOUND\n", op, path);
		break;
	}
	case 0x1012: {
		/* H7b: mismo esquema que la respuesta generica pero con su PROPIO
		 * contador de campos.  Barrido del kernel #49: el 0x1007 solo acepta 1
		 * campo y el 0x1012 rechaza 1 y 4 -> el contador es POR OP. */
		u32 k, nf = (spm_fs_1012_nf == 0xff) ? 1 : spm_fs_1012_nf;

		if (spm_fs_1012_nf != 0xff)
			spm_fs_p0(fs, boff, spm_fs_1012_nf);
		length = 4;
		for (k = 0; k < nf && k < 6; k++) {
			writel(4, fs + boff + 8 + k * 8);
			writel(k ? 0 : spm_fs_1010_val, fs + boff + 0xc + k * 8);
			length = 4 + (k + 1) * 8;
		}
		fs_dbg(s->dev, "H7b op=1012 -> nf=%u len=%u\n", spm_fs_1012_nf, length);
		break;
	}
	default:
		/*
		 * H6y: RESPUESTA MINIMA VALIDA para cualquier op sin handler.
		 *
		 * Antes no se tocaba nada, asi que en +8 quedaba la longitud de la
		 * PETICION y el MD reventaba en la comprobacion de la linea 547
		 * (capacidad >= longitud).  Medido 0729 con el op 0x1007: esperado 4,
		 * devolviamos 0x24.
		 *
		 * Como la regla es capacidad >= longitud y la capacidad minima que
		 * hemos observado es 4, devolver UN campo de 4 bytes es seguro para
		 * cualquier op: si la capacidad fuese mayor, 4 sigue cabiendo.
		 */
		{
			u32 nf = (op == spm_fs_ov_op) ? spm_fs_ov_nf : spm_fs_def_nf;
			u32 i;

			if (nf == 0xff)			/* 0xff = no tocar el contador */
				nf = spm_fs_def_nf;
			else
				spm_fs_p0(fs, boff, nf);	/* H7a/H7c */

			/*
			 * H7d: rellenar TODOS los campos DECLARADOS.
			 *
			 * Antes esto miraba spm_fs_def_nf en vez de `nf`, asi que con el
			 * override (0x1004 -> nf=2) se declaraban 2 campos y solo se
			 * escribia 1: el segundo se quedaba con la longitud de la
			 * PETICION (medido 0x358) y el MD moria en la comprobacion de
			 * capacidad (fs_ccci.c:547, error -4010).
			 *
			 * Cada campo son 8 bytes: [len=4][dato].  length = 4 (la palabra
			 * del contador) + nf*8, que reproduce 12/0x14/0x1c para nf=1/2/3.
			 */
			if (nf > 8)
				nf = 8;			/* tope de seguridad para el buffer */
			for (i = 0; i < nf; i++) {
				writel(4, fs + boff + 8 + i * 8);		/* len_i = 4 */
				writel(i ? 0 : spm_fs_1010_val,
				       fs + boff + 0xc + i * 8);	/* dato_i */
			}
			length = 4 + nf * 8;

			fs_dbg(s->dev, "H6y op=%x sin handler -> minima (nf=%u len=%u)\n",
			       op, nf, length);
		}
		break;
	}
	/* marca "hecho": conserva op, high16 = 0xffff (o bit31 si error, ya puesto arriba) */
	writel((0xffff0000 | op) | nf_extra, fs + boff + 0);	/* H6j: nf_extra */
	return length;
}

/* H13k: spm_ccci_pass() va antes del bloque del tty y necesita ambas. */
static void spm_tty_rx(struct mt6582_spm *s, void __iomem *ccif);
static bool spm_tty_serve;	/* H13j: solo el hilo atiende el TTY */

/*
 * H13m: discriminador.  Permite arrancar el hilo sin que atienda el TTY, para
 * separar "el hilo" de "el empuje al tty_port + el ACK", que hasta ahora se
 * probaban juntos.
 */
static uint spm_tty_rx_on = 1;
module_param(spm_tty_rx_on, uint, 0644);
MODULE_PARM_DESC(spm_tty_rx_on, "H13m: 0 = el hilo corre pero NO atiende el TTY");

/*
 * H13o: las dos mitades de spm_tty_rx, cada una con su interruptor.  El drenaje
 * del anillo se hace siempre, para que la prueba solo cambie lo que se mide.
 */
static uint spm_tty_push = 1;
module_param(spm_tty_push, uint, 0644);
MODULE_PARM_DESC(spm_tty_push, "H13o: 0 = no empujar al tty_port");

static uint spm_tty_ack = 1;
module_param(spm_tty_ack, uint, 0644);
MODULE_PARM_DESC(spm_tty_ack, "H13o: 0 = no mandar el ACK por CCCI");

/*
 * H13: UNA pasada de servicio del CCCI (leer RCHNUM, despachar y ACKear).
 *
 * Extraido del bucle de spm_md_hs2 sin tocar su contenido, para que lo usen
 * los dos: el bucle de arranque y el hilo que mantiene vivo el tty.  Devuelve
 * 1 si en esta pasada ha llegado el boot-ready.
 */
static int spm_ccci_pass(struct mt6582_spm *s, void __iomem *ccif,
			 void __iomem *fs, int *done)
{
	u32 rch;
	int k, hs2_ahora = 0;

	rch = readl(ccif + 0x10);
	for (k = 0; k < 8 && rch; k++) {
		u32 d0, d1, lch, rsv;

		if (!(rch & (1 << k)))
			continue;
		d0  = readl(ccif + 0x180 + k * 16);
		d1  = readl(ccif + 0x184 + k * 16);
		lch = readl(ccif + 0x188 + k * 16);
		rsv = readl(ccif + 0x18c + k * 16);
		fs_dbg(s->dev, "CCCI RX ch%d: d0=%08x id=%08x lch=%08x rsv=%08x\n",
			 k, d0, d1, lch, rsv);
		/* H7i: los canales que NO son el FS(14) son raros y CLAVE
		 * (control, IPC, excepciones): siempre visibles, aunque
		 * spm_fs_quiet silencie la cascada de ficheros. */
		if (lch != 0x0e)
			dev_info(s->dev, "CCCI RX no-FS ch%d: d0=%08x id=%08x lch=%08x rsv=%08x\n",
				 k, d0, d1, lch, rsv);
		/* boot-ready HS2: canal de control (lch=0) + id=NORMAL_BOOT_ID(0),
		 * distinto del HS1 (que trae rsv=MD_INIT_CHK_ID=0x5555FFFF). */
		if (lch == 0 && d1 == 0 && rsv != 0x5555FFFF) {
			if (!*done) {
				dev_info(s->dev, "*** HS2 LOGRADO: NORMAL_BOOT_ID (stage 2 = M1 COMPLETO) ***\n");
				hs2_ahora = 1;	/* H9a: desde aqui cuentan las post-HS2 */
			}
			*done = 1;
		}
		/* H6 PROXY FS: el MD manda su peticion de EFS/NVRAM por CCCI_FS_RX(14).
		 * fs_stream_buffer_t = {u32 fs_ops; u8 buf[16384]} (16388B), buffer idx
		 * = rsv. Volcamos la peticion (op + path) y respondemos por CCCI_FS_TX(15)
		 * con {data0=MD-view del buffer, data1=len+4, rsv=idx} (ccci_fs_send).
		 * NOTA: el CONTENIDO de la respuesta (resultado del fs_op) lo define
		 * ccci_fsd userspace -> HIPOTESIS aqui (result=0); ground-truth pendiente
		 * de la snoop de LineageOS (fs_rx/tx_debug_enable). */
		/*
		 * H13j: el TTY lo atiende SOLO el hilo (20 ms), no la cascada.
		 * Medido: el mismo kernel con spm_tty_enable=0 completa el ciclo
		 * (HS2 + 1572 ops) y con =1 reinicia el movil.  En la fase rapida
		 * este bucle corre a ~5000 pasadas/s y no aguanta el empuje al
		 * tty_port mas un ACK por evento.  El canal AT solo importa tras el
		 * HS2, que es cuando arranca el hilo.
		 */
		if (lch == 10 && spm_tty_serve && spm_tty_rx_on)
			spm_tty_rx(s, ccif);
		if (lch == 14) {
			u32 idx = rsv & 0xff;
			/* stride = sizeof(fs_stream_buffer_t) = {u32 fs_ops; u8 buf[16384]}
			 * = 16388 = 0x4004 (ccci_fs.h). El 0x14014 era el TOTAL de los 5
			 * buffers (bug: fuera de region para idx>0). */
			u32 boff = idx * 0x4004;
			u32 fs_ops, fs_len;
			int tch;
			u32 busy;

			fs_ops = readl(fs + boff + 0);
			fs_dbg(s->dev, "H6 FS REQ idx=%u op=%08x [%08x %08x] path=[%08x %08x %08x]\n",
				 idx, fs_ops, readl(fs + boff + 4),
				 readl(fs + boff + 8), readl(fs + boff + 12),
				 readl(fs + boff + 16), readl(fs + boff + 20));
			/*
			 * H8h: volcado COMPLETO de la peticion del OPEN.
			 * El handler leia los flags de +4, que es el CONTADOR DE
			 * CAMPOS (vale 2) -> nunca activaba O_CREAT.  Los flags
			 * deben venir en el 2o campo, detras del path:
			 *   +8 = len_0 | +0xc = path | +0xc+align4(len_0) = len_1
			 * Se volcan 14 palabras para localizarlo con datos, no
			 * por deduccion.
			 */
			if ((fs_ops & 0xffff) == 0x1001 && !spm_fs_quiet) {
				char rb[200];
				int rn = 0, rw;

				for (rw = 0; rw < 14 && rn < 190; rw++)
					rn += scnprintf(rb + rn, 200 - rn, "%08x ",
							readl(fs + boff + rw * 4));
				rb[rn] = 0;
				dev_info(s->dev, "H8h OPEN req: %s\n", rb);
			}
			/* servidor FS: file-op sobre /data/nvram/md + respuesta en el buffer;
			 * devuelve la length del payload (spm_fs_serve, H6f). */
			fs_len = spm_fs_serve(s, fs, idx);
			wmb();
			if (fs_len == SPM_FS_NOREPLY) {	/* H6o: control, no se contesta */
				dev_info(s->dev, "H6 FS: control -> NO se envia FS_TX\n");
				goto fs_noreply;
			}
			busy = readl(ccif + 0x04) & 0xff;
			for (tch = 0; tch < 8; tch++)
				if (!(busy & (1 << tch)))
					break;
			if (tch < 8) {
				writel(1 << tch, ccif + 0x04);	/* ocupar canal fisico TX */
				writel((MD_SMEM_PHYS + 0xE000) - MD_AP_OFF + boff,
				       ccif + 0x100 + tch * 16 + 0);	/* data0 = MD-view buffer */
				writel(fs_len + 4, ccif + 0x100 + tch * 16 + 4); /* data1 = length+4 */
				writel(15, ccif + 0x100 + tch * 16 + 8);	/* channel = CCCI_FS_TX */
				writel(idx, ccif + 0x100 + tch * 16 + 12);	/* reserved = idx */
				wmb();
				writel(tch, ccif + 0x0c);	/* TCHNUM -> dispara */
				fs_dbg(s->dev, "H6 FS RESP idx=%u -> FS_TX ch%d data0=%08x\n",
					 idx, tch, (MD_SMEM_PHYS + 0xE000) - MD_AP_OFF + boff);
				/*
				 * H7n: volcado literal de la respuesta.  Hay que diffear palabra a
				 * palabra contra el ground-truth (gd-boot-full.out linea 12) en vez
				 * de deducir el formato: es lo unico que distingue "casi igual" de
				 * "igual".  Se imprimen 20 palabras desde +0 del buffer.
				 */
				if (spm_fs_dumpresp) {
					char db[220];
					int dn = 0, dw;

					for (dw = 0; dw < 20; dw++)
						dn += scnprintf(db + dn, sizeof(db) - dn, "%08x ",
								readl(fs + boff + dw * 4));
					dev_info(s->dev, "H7n RESP len=%u | %s\n", fs_len, db);
				}
			} else {
				dev_warn(s->dev, "H6 FS: sin canal TX libre (BUSY=%02x)\n", busy);
			}
		}
fs_noreply:
		writel(1 << k, ccif + 0x14);	/* ACK canal k -> libera el TX del MD */
	}

	return hs2_ahora;
}

/* ===================== H13: /dev/ttyCCCI0 ===================== */

#define TTY_MODEM_PORT	1		/* puerto TTY del canal AT (UART2) */

static struct tty_driver *spm_tty_drv;
static bool spm_tty_ready;	/* H13b: el tty_port ya existe */

/*
 * H13h: los logs del camino de RX, apagados por defecto.  Cada dev_info cuesta
 * decenas de ms; dentro de la fase rapida del bucle eso estanca el servicio y
 * dispara el watchdog.  Se enciende para medir, no para funcionar.
 */
static uint spm_tty_debug;
module_param(spm_tty_debug, uint, 0644);
MODULE_PARM_DESC(spm_tty_debug, "H13h: logs del camino de RX del tty (lento: solo para medir)");

/*
 * H13e: mapeados UNA sola vez al registrar el tty.  Antes se hacia ioremap por
 * evento dentro del camino caliente (spm_ccci_pass corre a ~200 us en la fase
 * rapida), que es caro y toma cerrojos: candidato al cuelgue duro observado.
 */
static void __iomem *spm_tty_ring;	/* anillo del puerto TTY_MODEM_PORT */
static void __iomem *spm_tty_ccif;
static struct tty_port spm_tty_p;
static DEFINE_MUTEX(spm_ccif_tx);	/* el CCIF lo tocan el hilo y el write */
static struct task_struct *spm_ccci_task;

/* Copia al tx_buffer del puerto y toca el timbre.  Devuelve los bytes puestos. */
static int spm_tty_ring_write(struct mt6582_spm *s, const u8 *buf, size_t n)
{
	void __iomem *t = spm_tty_ring, *ccif = spm_tty_ccif;
	u32 tw, tl, i;
	int ret;

	if (!n)
		return 0;
	if (!t || !ccif)			/* H13e: mapeados al registrar */
		return -ENODEV;
	mutex_lock(&spm_ccif_tx);
	tw = readl(t + 0x10);
	tl = readl(t + 0x14);
	if (!tl) {
		mutex_unlock(&spm_ccif_tx);
		return -EIO;
	}
	if (n > tl / 2)
		n = tl / 2;
	for (i = 0; i < n; i++)
		writeb(buf[i], t + 0x4018 + ((tw + i) % tl));
	writel((tw + n) % tl, t + 0x10);
	wmb();
	ret = spm_ccci_send(s, ccif, 0, n, 12, 0);	/* CCCI_UART2_TX */
	mutex_unlock(&spm_ccif_tx);
	return ret ? ret : (int)n;
}

/*
 * Llega el timbre del MD por lch 10: vaciar el anillo de RX hacia el tty y
 * ACKear (mailbox con id=1 por CCCI_UART2_RX_ACK=11).  Si no se consume, el
 * anillo se llena y el MD deja de emitir.
 */
static void spm_tty_rx(struct mt6582_spm *s, void __iomem *ccif)
{
	void __iomem *t = spm_tty_ring;		/* H13e: mapeado al registrar */
	u32 rr, rw, rl, i, n;

	if (!t)
		return;
	rr = readl(t + 0x00);
	rw = readl(t + 0x04);
	rl = readl(t + 0x08);
	/*
	 * H13n: los tres punteros los escribe el MD en memoria compartida, asi
	 * que no se pueden dar por buenos.  Sin esta comprobacion, un rr mayor
	 * que rl hace que "rl - rr" desborde (son u32) y n quede en miles de
	 * millones: el bucle de abajo se sale del mapeo leyendo con readb.
	 */
	if (!rl || rl > CCCI_TTY_BUF_SIZE || rr >= rl || rw >= rl) {
		if (spm_tty_debug)
			dev_info(s->dev, "H13n rx: punteros invalidos rr=%u rw=%u rl=%u\n",
				 rr, rw, rl);
		return;
	}
	if (rr == rw)
		return;
	if (spm_tty_debug)
		dev_info(s->dev, "H13g rx: timbre  rr=%u rw=%u rl=%u tty=%d\n",
			 rr, rw, rl, spm_tty_ready);

	n = (rw > rr) ? rw - rr : rl - rr;	/* hasta el final; la vuelta, al proximo timbre */
	if (n > CCCI_TTY_BUF_SIZE)		/* H13n: cinturon y tirantes */
		return;
	/*
	 * H13b: el tty_port puede no existir todavia (el MD empieza a emitir en
	 * cuanto arranca).  Se drena y se ACKea igual —que es lo que evita que el
	 * MD se atasque— y solo se empuja si hay tty.
	 *
	 * H13g: con tty_insert_flip_string, que ademas dice cuantos bytes acepto
	 * la capa de linea.  Si acepta menos de los que sacamos del anillo, la
	 * diferencia se perderia en silencio.
	 */
	if (spm_tty_ready && spm_tty_push) {
		/*
		 * H13i: empuje caracter a caracter, como en el #91, que era el que
		 * sobrevivia al ciclo.  La version por trozos con buffer en pila
		 * (H13g) reiniciaba el movil DURANTE la cascada y sin dejar rastro
		 * -- queda anotada como sospechosa para cuando se retome.
		 */
		for (i = 0; i < n; i++)
			tty_insert_flip_char(&spm_tty_p,
					     readb(t + 0x18 + rr + i), TTY_NORMAL);
		tty_flip_buffer_push(&spm_tty_p);
		if (spm_tty_debug)
			dev_info(s->dev, "H13i rx: empujados %u\n", n);
	}

	writel((rr + n) % rl, t + 0x00);
	wmb();
	if (spm_tty_ack) {
		mutex_lock(&spm_ccif_tx);
		spm_ccci_send(s, ccif, 0xffffffff, 1, 11, 0);	/* CCCI_UART2_RX_ACK */
		mutex_unlock(&spm_ccif_tx);
	}
	if (spm_tty_debug)
		dev_info(s->dev, "H13g rx: fin, read %u -> %u, ACK enviado\n",
			 rr, (rr + n) % rl);
}

static int spm_tty_op_open(struct tty_struct *tty, struct file *f)
{
	int r;

	if (gspm)
		if (spm_tty_debug)
			dev_info(gspm->dev, "H13f open: entrando\n");
	r = tty_port_open(&spm_tty_p, tty, f);
	if (gspm)
		if (spm_tty_debug)
			dev_info(gspm->dev, "H13f open: -> %d\n", r);
	return r;
}

static void spm_tty_op_close(struct tty_struct *tty, struct file *f)
{
	tty_port_close(&spm_tty_p, tty, f);
}

static ssize_t spm_tty_op_write(struct tty_struct *tty, const u8 *buf, size_t n)
{
	int r;

	if (!gspm)
		return -ENODEV;
	if (spm_tty_debug)
		dev_info(gspm->dev, "H13f write: %u bytes\n", (unsigned int)n);
	r = spm_tty_ring_write(gspm, buf, n);
	if (spm_tty_debug)
		dev_info(gspm->dev, "H13f write: -> %d\n", r);
	/*
	 * H13f: avisar a la capa de linea de que ya hay sitio.  Sin esto
	 * n_tty_write se queda esperando un despertar que nadie manda.
	 */
	if (r > 0)
		tty_wakeup(tty);
	return r;
}

static unsigned int spm_tty_op_write_room(struct tty_struct *tty)
{
	if (gspm)
		if (spm_tty_debug)
			dev_info(gspm->dev, "H13f write_room\n");
	return CCCI_TTY_BUF_SIZE / 2;
}

/*
 * H13c: el tty_port NECESITA su ops.  tty_port_init() lo deja a NULL de un
 * memset y tty_port_open() hace port->ops->activate sin comprobarlo -> oops al
 * abrir el nodo.  Vacio es suficiente: no hay init especifica ni lineas de
 * modem reales, y sin ->carrier_raised se considera que siempre hay portadora.
 */
static const struct tty_port_operations spm_tty_port_ops = { };

static const struct tty_operations spm_tty_ops = {
	.open       = spm_tty_op_open,
	.close      = spm_tty_op_close,
	.write      = spm_tty_op_write,
	.write_room = spm_tty_op_write_room,
};

static int spm_tty_register(struct mt6582_spm *s)
{
	int ret;

	if (spm_tty_drv)
		return 0;
	spm_tty_drv = tty_alloc_driver(1, TTY_DRIVER_REAL_RAW |
					  TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(spm_tty_drv))
		return PTR_ERR(spm_tty_drv);

	spm_tty_drv->driver_name = "mt6582-ccci";
	spm_tty_drv->name        = "ttyCCCI";
	spm_tty_drv->major       = 0;		/* dinamico */
	spm_tty_drv->type        = TTY_DRIVER_TYPE_SERIAL;
	spm_tty_drv->subtype     = SERIAL_TYPE_NORMAL;
	spm_tty_drv->init_termios = tty_std_termios;
	spm_tty_drv->init_termios.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
	tty_set_operations(spm_tty_drv, &spm_tty_ops);

	/* H13e: los mapeos, una sola vez y para siempre */
	spm_tty_ring = ioremap(MD_SMEM_PHYS + CCCI_TTY_BASE +
			       TTY_MODEM_PORT * CCCI_TTY_STRIDE, CCCI_TTY_SMEM_SIZE);
	spm_tty_ccif = ioremap(0x1020A000, 0x200);
	if (!spm_tty_ring || !spm_tty_ccif) {
		if (spm_tty_ring) iounmap(spm_tty_ring);
		if (spm_tty_ccif) iounmap(spm_tty_ccif);
		spm_tty_ring = NULL;
		spm_tty_ccif = NULL;
		tty_driver_kref_put(spm_tty_drv);
		spm_tty_drv = NULL;
		return -ENOMEM;
	}

	tty_port_init(&spm_tty_p);
	spm_tty_p.ops = &spm_tty_port_ops;	/* H13c: imprescindible */
	ret = tty_register_driver(spm_tty_drv);
	if (ret) {
		tty_port_destroy(&spm_tty_p);
		tty_driver_kref_put(spm_tty_drv);
		spm_tty_drv = NULL;
		return ret;
	}
	tty_port_register_device(&spm_tty_p, spm_tty_drv, 0, NULL);
	spm_tty_ready = true;
	dev_info(s->dev, "H13: /dev/ttyCCCI0 registrado (canal AT del modem)\n");
	return 0;
}

/*
 * El hilo de servicio: mantiene el CCCI atendido cuando el bucle de arranque ya
 * ha terminado.  Sin esto el tty solo funcionaria mientras alguien tuviera vivo
 * el bucle sincrono de spm_md_hs2.
 */
static int spm_ccci_fn(void *arg)
{
	struct mt6582_spm *s = arg;
	void __iomem *ccif = ioremap(0x1020A000, 0x200);
	void __iomem *fs   = ioremap(MD_SMEM_PHYS + 0xE000, 0x15000);
	int done = 1;			/* el HS2 ya paso: solo servir */

	if (!ccif || !fs) {
		if (ccif) iounmap(ccif);
		if (fs) iounmap(fs);
		return -ENOMEM;
	}
	spm_tty_serve = true;		/* H13j: desde aqui si */
	dev_info(s->dev, "H13: hilo de servicio CCCI en marcha\n");
	while (!kthread_should_stop()) {
		spm_ccci_pass(s, ccif, fs, &done);
		msleep(20);
	}
	iounmap(fs);
	iounmap(ccif);
	return 0;
}

static int spm_md_hs2(struct mt6582_spm *s)
{
	void __iomem *smem, *ccif, *fs;
	u32 misc_phys = MD_SMEM_PHYS + 0x400;	/* misc_info tras el runtime struct */
	u32 rch;
	int i;

	smem = ioremap(MD_SMEM_PHYS, 0x2000);
	ccif = ioremap(0x1020A000, 0x200);
	fs   = ioremap(MD_SMEM_PHYS + 0xE000, 0x15000);	/* region FS: 5 buffers de 16388B (H6) */
	if (!smem || !ccif || !fs) {
		if (smem) iounmap(smem);
		if (ccif) iounmap(ccif);
		if (fs) iounmap(fs);
		return -ENOMEM;
	}
	rch = readl(ccif + 0x10);
	dev_info(s->dev, "H4 HS2: pre RCHNUM=0x%08x (HS1 esperado bit0)\n", rch);
	/* DIAG: el mensaje HS1 que envio el MD (RXCHDATA ch0 @0x180, 4 ints) — confirma
	 * el protocolo y revela el formato exacto que el MD espera de vuelta. */
	dev_info(s->dev, "H4 DIAG HS1-msg (RX ch0): %08x %08x %08x %08x | START=%08x CON=%08x BUSY=%08x\n",
		 readl(ccif + 0x180), readl(ccif + 0x184), readl(ccif + 0x188), readl(ccif + 0x18c),
		 readl(ccif + 0x08), readl(ccif + 0x00), readl(ccif + 0x04));
	if (rch & 0x1)
		writel(0x1, ccif + 0x14);	/* ACK canal 0 (el HS1 del MD) */

	/* 1) runtime struct (RT_NINTS ints, memset 0 + esenciales) en SMEM@0 */
	for (i = 0; i < RT_NINTS; i++)
		writel(0, smem + i * 4);
	writel(0x46494343, smem + 0 * 4);	/* Prefix "CCIF" */
	writel(0x3536544d, smem + 1 * 4);	/* Platform_L "MT65" */
	writel(0x31453238, smem + 2 * 4);	/* Platform_H "82E1" (ground-truth Lineage 0717) */
	writel(0x20121001, smem + 3 * 4);	/* DriverVersion (CCCI1_DRIVER_VER) */
	writel(0, smem + 4 * 4);		/* BootChannel = CCCI_CONTROL_RX */
	writel(0, smem + 5 * 4);		/* BootingStartID = NORMAL_BOOT_ID */
	/* Regiones que set_md_runtime SIEMPRE rellena (size constante no-cero) — el MD
	 * las exige. Las colocamos en el SMEM tras el runtime(280)+misc(0x400). */
	writel((MD_SMEM_PHYS + 0x800) - MD_AP_OFF, smem + 35 * 4);	/* ExceShareMemBase (MD-view, tras misc) */
	writel(0x800, smem + 36 * 4);					/* ExceShareMemSize = MD_EX_LOG_SIZE */
	writel((MD_SMEM_PHYS + 0x1000) - MD_AP_OFF, smem + 62 * 4);	/* MDExExpInfoBase (MD-view) */
	writel(12, smem + 63 * 4);					/* MDExExpInfoSize = sizeof(exp_t) */
	/* Regiones que el stock SIEMPRE asigna no-cero (ccci_settings.c): el MD monta
	 * estos canales al boot y desreferencia su base — si es 0 -> DATA ABORT. Les
	 * damos bases validas en el SMEM (2MB, el MD accede fisico; no escribimos, el
	 * MD las inicializa). Zonas distintas: IPC@+0x2000 PCM@+0x4000 RPC@+0xC000 FS@+0xE000. */
	writel((MD_SMEM_PHYS + 0x4000) - MD_AP_OFF, smem + 10 * 4);	/* PcmShareMemBase */
	writel(0x8000, smem + 11 * 4);					/* PcmShareMemSize (16*2K) */
	writel((MD_SMEM_PHYS + 0xE000) - MD_AP_OFF, smem + 29 * 4);	/* FileShareMemBase (FS) */
	writel(0x14014, smem + 30 * 4);					/* FileShareMemSize = 16388*5 REAL (era 0x2000 -> overrun) */
	writel((MD_SMEM_PHYS + 0xC000) - MD_AP_OFF, smem + 31 * 4);	/* RpcShareMemBase */
	writel(0x2000, smem + 32 * 4);					/* RpcShareMemSize */
	writel((MD_SMEM_PHYS + 0x2000) - MD_AP_OFF, smem + 39 * 4);	/* IPCShareMemBase */
	writel(0x2000, smem + 40 * 4);					/* IPCShareMemSize */
	/* BARRIDO EXHAUSTIVO 0718: las ULTIMAS regiones no-cero del stock que aun no
	 * dabamos (Mdlog, Uart x3 TTY, Net CCMNI). Si el MD las desreferencia sin
	 * respetar los PortNum, base=0 -> abort. Bases MD-view en el SMEM (2MB), no
	 * solapan con las anteriores (hasta +0x10000). Test: si el abort PERSISTE con
	 * TODO puesto = runtime descartado def. */
	/* LAYOUT NO-SOLAPADO con tamaños REALES (v2): FS acaba en ~0x22014, así que lo de
	 * aquí va de 0x30000 en adelante, cada región con hueco >= su size real.
	 * Mdlog@0x30000, Uart@0x40/50/60000, MDULNet@0x70000(300K), MDDLNet@0xC0000(320K),
	 * NetCtrl@0x120/130/140000. Todo < 2MB. */
	writel((MD_SMEM_PHYS + 0x30000) - MD_AP_OFF, smem + 8 * 4);	/* MdlogShareMemBase */
	writel(0x8000, smem + 9 * 4);					/* MdlogShareMemSize (mdlog off, chico) */
	/*
	 * H7r: puertos TTY del CCCI.  Con el FS ya resuelto el MD pasa a montarlos
	 * y muere en ccci_uart_drv.c:793.  El AP stock (ccci_tty.c:709-718) anuncia
	 * el tamano EXACTO sizeof(shared_mem_tty_t) e INICIALIZA el bloque de
	 * control al principio de cada region:
	 *
	 *   +0x00 rx.read  +0x04 rx.write  +0x08 rx.length = 16K
	 *   +0x0c tx.read  +0x10 tx.write  +0x14 tx.length = 16K
	 *   +0x18 rx_buffer[16K]  +0x4018 tx_buffer[16K]
	 *
	 * Antes: 3 regiones de 0xA000 con el contenido a basura.  Ahora: los 6
	 * puertos del stock (CCCI_TTY_PORT_COUNT), tamano 0x8018 y control puesto.
	 * Van a +0x180000 para no pisar las regiones de red, que empiezan en +0x70000.
	 */
	writel(spm_md_uart_ports, smem + 12 * 4);			/* H8c: UartPortNum */
	for (i = 0; i < spm_md_uart_ports && i < CCCI_TTY_PORTS; i++) {
		u32 base = MD_SMEM_PHYS + CCCI_TTY_BASE + i * CCCI_TTY_STRIDE;
		void __iomem *tty = ioremap(base, 0x20);

		writel(base - MD_AP_OFF, smem + (13 + i) * 4);		/* UartShareMemBase[i] */
		writel(CCCI_TTY_SMEM_SIZE, smem + (21 + i) * 4);	/* UartShareMemSize[i] */
		if (tty) {
			writel(0, tty + 0x00);				/* rx.read */
			writel(0, tty + 0x04);				/* rx.write */
			writel(spm_md_uart_len, tty + 0x08);		/* H8c: rx.length */
			writel(0, tty + 0x0c);				/* tx.read */
			writel(0, tty + 0x10);				/* tx.write */
			writel(spm_md_uart_len, tty + 0x14);		/* H8c: tx.length */
			wmb();
			iounmap(tty);
		}
	}
	writel((MD_SMEM_PHYS + 0x70000) - MD_AP_OFF, smem + 41 * 4);	/* MDULNetShareMemBase */
	writel(0x4B000, smem + 42 * 4);					/* MDULNetShareMemSize = 300K REAL */
	writel((MD_SMEM_PHYS + 0xC0000) - MD_AP_OFF, smem + 43 * 4);	/* MDDLNetShareMemBase */
	writel(0x50000, smem + 44 * 4);					/* MDDLNetShareMemSize ~320K */
	writel(3, smem + 45 * 4);					/* NetPortNum = 3 */
	writel((MD_SMEM_PHYS + 0x120000) - MD_AP_OFF, smem + 46 * 4);	/* NetULCtrlBase[0] */
	writel((MD_SMEM_PHYS + 0x130000) - MD_AP_OFF, smem + 47 * 4);	/* [1] */
	writel((MD_SMEM_PHYS + 0x140000) - MD_AP_OFF, smem + 48 * 4);	/* [2] */
	writel(0x8000, smem + 50 * 4); writel(0x8000, smem + 51 * 4); writel(0x8000, smem + 52 * 4); /* ULCtrlSize[0-2] */
	writel((MD_SMEM_PHYS + 0x150000) - MD_AP_OFF, smem + 54 * 4);	/* NetDLCtrlBase[0] */
	writel((MD_SMEM_PHYS + 0x160000) - MD_AP_OFF, smem + 55 * 4);	/* [1] */
	writel((MD_SMEM_PHYS + 0x170000) - MD_AP_OFF, smem + 56 * 4);	/* [2] */
	writel(0x8000, smem + 58 * 4); writel(0x8000, smem + 59 * 4); writel(0x8000, smem + 60 * 4); /* DLCtrlSize[0-2] */

	/* H8e: zerar las 6 regiones de control de red (el MD exige leer ceros ahi). */
	if (spm_md_zero_net) {
		static const u32 netoff[6] = { 0x120000, 0x130000, 0x140000,
					       0x150000, 0x160000, 0x170000 };
		int z;

		for (z = 0; z < 6; z++) {
			void __iomem *nr = ioremap(MD_SMEM_PHYS + netoff[z], 0x8000);
			u32 w;

			if (!nr) {
				dev_warn(s->dev, "H8e: no pude mapear net+0x%x\n", netoff[z]);
				continue;
			}
			for (w = 0; w < 0x8000; w += 4)
				writel(0, nr + w);
			iounmap(nr);
		}
		wmb();
		dev_info(s->dev, "H8e: 6 regiones NetUL/DLCtrl zeradas (6 x 32KB)\n");
	}
	writel(misc_phys - MD_AP_OFF, smem + 66 * 4);	/* MiscInfoBase (MD-view) */
	writel(1024, smem + 67 * 4);		/* MiscInfoSize */
	writel(0x46494343, smem + 69 * 4);	/* Postfix "CCIF" */
	wmb();
	/* DIAG: read-back del SMEM — confirma que la escritura fisica cuajo (descarta
	 * problema de ioremap/mapeo del SMEM que el MD tambien veria). */
	dev_info(s->dev, "H4 DIAG SMEM rb: [0]=%08x [2]=%08x [66]=%08x [69]=%08x (esp CCIF/82E1/miscMDbase/CCIF)\n",
		 readl(smem + 0), readl(smem + 2 * 4), readl(smem + 66 * 4), readl(smem + 69 * 4));

	/* 2) misc_info (config_misc_info) en SMEM@0x400. Layout misc_info_t:
	 * prefix@0, support_mask@4, index@8, next@12, feature_0..15_val[4]@16,
	 * reserved_2[3]@272, postfix@284. El stock hace memset(0) primero — sin él,
	 * feature_1..15/reserved quedan con basura del carveout y el MD puede leer un
	 * puntero basura -> DATA ABORT. Zeramos los 0x400 bytes de la region misc. */
	for (i = 0; i < 256; i++)
		writel(0, smem + 0x400 + i * 4);
	writel(0x46494343, smem + 0x400 + 0);		/* prefix "CCIF" */
	writel(spm_md_misc_mask, smem + 0x400 + 4);	/* H8b: 2 bits por feature */
	writel(0, smem + 0x400 + 8);			/* index */
	writel(0, smem + 0x400 + 12);			/* next */
	writel(MD_MEM_PHYS, smem + 0x400 + 16);		/* feature_0_val[0] = md_mem_start */
	writel(0x46494343, smem + 0x400 + 284);		/* postfix "CCIF" (config_misc_info lo pone) */

	/* 3) TAG (modem_runtime_info_tag_t, 7 ints) en la SRAM del CCIF @ 0x140 */
	writel(0x46494343, ccif + 0x140 + 0);		/* prefix */
	writel(0x3536544d, ccif + 0x140 + 4);		/* platform_L */
	writel(0x31453238, ccif + 0x140 + 8);		/* platform_H "82E1" */
	writel(0x20121001, ccif + 0x140 + 12);		/* driver_version */
	writel(MD_SMEM_PHYS - MD_AP_OFF, ccif + 0x140 + 16);	/* runtime_data_base (MD-view) */
	writel(RT_NINTS * 4, ccif + 0x140 + 20);	/* runtime_data_size */
	writel(0x46494343, ccif + 0x140 + 24);		/* postfix */

	/* 4) mensaje de arranque por CCIF TX canal fisico 0 (TXCHDATA@0x100) */
	if (readl(ccif + 0x04) & 0x1)
		dev_warn(s->dev, "H4 HS2: CCIF ch0 BUSY antes de TX (0x%08x)\n", readl(ccif + 0x04));
	writel(0x1, ccif + 0x04);			/* CCIF_BUSY = 1<<0 */
	writel(0xFFFFFFFF, ccif + 0x100 + 0);		/* data0 = magic */
	writel(0x00000000, ccif + 0x100 + 4);		/* data1 = MD_INIT_START_BOOT */
	writel(0x00000001, ccif + 0x100 + 8);		/* channel = CCCI_CONTROL_TX */
	writel(0x5555FFFF, ccif + 0x100 + 12);		/* reserved = MD_INIT_CHK_ID */
	wmb();
	writel(0, ccif + 0x0c);				/* CCIF_TCHNUM = 0 -> dispara */
	udelay(100);
	/* DIAG: tras disparar el TX — si el MD consume el mensaje, BUSY bit0 se limpia
	 * (el MD leyo TXCHDATA). START refleja canales disparados. */
	dev_info(s->dev, "H4 DIAG post-TX: START=%08x BUSY=%08x RCHNUM=%08x (BUSY b0=0 => MD leyo el msg)\n",
		 readl(ccif + 0x08), readl(ccif + 0x04), readl(ccif + 0x10));
	dev_info(s->dev, "H4 HS2: runtime+tag+msg enviados. Bucle de servicio CCCI 8s...\n");

	/* 5) BUCLE DE SERVICIO CCCI (v2): el MD manda mensajes por canales fisicos y su
	 * canal TX queda OCUPADO hasta que el AP lo ACKea (CCIF_ACK=1<<ch, ccci_hw.c
	 * __ccif_v1_ack). Sin ACK el MD no puede mandar el siguiente -> se bloquea antes
	 * del boot-ready. Aqui: leer cada canal con dato (RCHNUM), loguearlo, ACKear, y
	 * buscar el boot-ready = mensaje de control (lch=0) con id=NORMAL_BOOT_ID(0) que
	 * NO sea el HS1 (rsv!=MD_INIT_CHK_ID). */
	{
		int done = 0, hs2_i = -1;

		for (i = 0; i < (int)(spm_fs_fastpoll_iters + spm_fs_slow_iters) &&
			    (!done || i - hs2_i < (int)spm_fs_post_hs2_iters); i++) {	/* fase rapida + 8s */
			if (spm_ccci_pass(s, ccif, fs, &done))
				hs2_i = i;
			/* H6m: fase rapida al principio (mount FS del MD), luego relajado. */
			if (i < (int)spm_fs_fastpoll_iters) {
				if ((i & 0x3ff) == 0x3ff)	/* H7f: no monopolizar la CPU */
					cond_resched();
				if (spm_fs_fastpoll_us)
					udelay(spm_fs_fastpoll_us);
				else
					cpu_relax();
			} else {
				msleep(25);
			}
		}
		{	int z, z0; static char b[3200]; int o = 0;
			/* H7h: imprimir la COLA (donde se para), no la cabeza:
			 * el volcado se truncaba y ocultaba el punto de parada.
			 * H8k: se imprime SIEMPRE, no solo al fallar. El if (!done)
			 * que introdujo H8g escondia el contador de operaciones
			 * justo cuando el arranque salia bien, que es cuando mas
			 * interesa (es la metrica de progreso del hito). */
			z0 = (spm_fslog_n < 48) ? spm_fslog_n : 48;
			for (z = z0; z > 0 && o < 3100; z--) {
				int k = (spm_fslog_w - z + SPM_FSLOG_SZ) % SPM_FSLOG_SZ;

				o += scnprintf(b + o, 3200 - o, "%04x ", spm_fslog[k]);
			}
			b[o] = 0;
			dev_info(s->dev, "H6 FS: %d ops servidos, ultimos %d: %s\n",
				 spm_fs_total, z0, b);
			spm_fslog_n = 0;
			spm_fslog_w = 0;
			spm_fs_total = 0;
		}
			/* H8g: antes esto se imprimia SIEMPRE, incluso tras un HS2
			 * correcto, dejando dos mensajes contradictorios en el log
			 * ("HS2 LOGRADO" y "sin boot-ready").  El stock decide la
			 * etapa 2 con la misma condicion que nosotros
			 * (ccci_md_main.c:2140: msg.id == NORMAL_BOOT_ID en etapa 1),
			 * asi que el HS2 era bueno y el que sobraba era este aviso. */
			/*
			 * H13: con el arranque hecho, el tty y su hilo toman el
			 * relevo para que el canal AT siga vivo cuando este bucle
			 * termine.  Solo si se alcanzo el HS2: sin modem no hay
			 * nada que servir.
			 */
			if (done && spm_tty_enable && spm_tty_ready && !spm_ccci_task)
				spm_ccci_task = kthread_run(spm_ccci_fn, s,
							    "mt6582-ccci");
			dev_info(s->dev, done
				 ? "H4 HS2: fin del bucle (etapa 2 alcanzada). RCHNUM=%08x BUSY=%08x\n"
				 : "H4 HS2: sin boot-ready. RCHNUM=%08x BUSY=%08x\n",
				 readl(ccif + 0x10), readl(ccif + 0x04));
	}
	iounmap(smem);
	iounmap(ccif);
	iounmap(fs);
	return 0;
}

static int spm_md_load_set(const char *v, const struct kernel_param *k)
{ return gspm ? spm_md_load(gspm) : -ENODEV; }
static int spm_md_remap_set(const char *v, const struct kernel_param *k)
{ return gspm ? spm_md_remap(gspm) : -ENODEV; }
static int spm_md_release_set(const char *v, const struct kernel_param *k)
{ return gspm ? spm_md_release(gspm) : -ENODEV; }
static int spm_md_hs2_set(const char *v, const struct kernel_param *k)
{ return gspm ? spm_md_hs2(gspm) : -ENODEV; }
static const struct kernel_param_ops spm_md_load_ops = { .set = spm_md_load_set };
static const struct kernel_param_ops spm_md_remap_ops = { .set = spm_md_remap_set };
static const struct kernel_param_ops spm_md_release_ops = { .set = spm_md_release_set };
static const struct kernel_param_ops spm_md_hs2_ops = { .set = spm_md_hs2_set };
module_param_cb(spm_md_load, &spm_md_load_ops, NULL, 0200);
module_param_cb(spm_md_remap, &spm_md_remap_ops, NULL, 0200);
module_param_cb(spm_md_release, &spm_md_release_ops, NULL, 0200);
module_param_cb(spm_md_hs2, &spm_md_hs2_ops, NULL, 0200);

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

	/* capturar la config de cache del LK (referencia para restaurar
	 * tras cada ciclo dormant — se pierde con el MTCMOS del cluster) */
	s->mcusys = ioremap(0x10200000, 0x10);
	s->l2ctlr_boot = read_l2ctlr();
	if (s->mcusys)
		s->cacfg_boot = readl(s->mcusys);
	dev_info(&pdev->dev, "LK: L2CTLR=0x%x CA7_CACHE_CONFIG=0x%x\n",
		 s->l2ctlr_boot, s->cacfg_boot);

	spm_hw_init(s);
	gspm = s;
	suspend_set_ops(&mt6582_spm_suspend_ops);

	/*
	 * H13l: el tty se registra AQUI, no dentro del manejador de sysfs del
	 * arranque del modem.  Registrarlo alli creaba el dispositivo y disparaba
	 * uevents en mitad de la cascada, y era lo unico que quedaba por descartar
	 * cuando spm_tty_enable=1 reiniciaba el movil (con 0 el ciclo completaba).
	 * Aqui no hay trafico CCCI ni modem, y un fallo se ve al arrancar.
	 * El nodo existe siempre; el SERVICIO sigue detras de spm_tty_enable.
	 */
	spm_tty_register(s);

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
