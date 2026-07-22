/*
 * pexpert/arm/pe_mt6582.h — MediaTek MT6582 (BQ Aquaris E4.5 "krillin")
 * Cortex-A7 quad, GIC-400. Direcciones tomadas del port de Linux mainline
 * del krillin; el framebuffer y el watchdog estan VALIDADOS EN HARDWARE
 * (GenericBooter M1 pinta en pantalla y el WDT reinicia si no se desarma).
 */
#ifndef _PEXPERT_MT6582_H_
#define _PEXPERT_MT6582_H_

/* ---- GIC-400 (offsets estandar ARM; solo cambian las bases) ---- */
#define MT6582_SYSIRQ_BASE       0x10200000   /* pagina del bloque sysirq */
#define MT6582_SYSIRQ_INTPOL     0x100        /* registros de polaridad: 0x10200100 */
#define MT6582_GIC_DIST_BASE     0x10211000
#define MT6582_GIC_CPU_BASE      0x10212000
#define GIC_CPU_REG(off)         ((off))
#define GIC_DIST_REG(off)        ((off))
#define GIC_CPU_CTRL             GIC_CPU_REG(0x00)
#define GIC_CPU_PRIMASK          GIC_CPU_REG(0x04)
#define GIC_CPU_BINPOINT         GIC_CPU_REG(0x08)
#define GIC_CPU_INTACK           GIC_CPU_REG(0x0c)
#define GIC_CPU_EOI              GIC_CPU_REG(0x10)
#define GIC_CPU_RUNNINGPRI       GIC_CPU_REG(0x14)
#define GIC_CPU_HIGHPRI          GIC_CPU_REG(0x18)
#define GIC_DIST_CTRL            GIC_DIST_REG(0x000)
#define GIC_DIST_CTR             GIC_DIST_REG(0x004)
#define GIC_DIST_ENABLE_SET      GIC_DIST_REG(0x100)
#define GIC_DIST_ENABLE_CLEAR    GIC_DIST_REG(0x180)
#define GIC_DIST_PRI             GIC_DIST_REG(0x400)
#define GIC_DIST_TARGET          GIC_DIST_REG(0x800)
#define GIC_DIST_CONFIG          GIC_DIST_REG(0xc00)

/* ---- Timer GPT de MediaTek (drivers/clocksource/timer-mediatek.c) ---- */
#define MT6582_TIMER_BASE        0x10008000
#define GPT_IRQ_EN_REG           0x00
#define GPT_IRQ_ACK_REG          0x08
#define GPT_IRQ_ENABLE(n)        (1 << ((n) - 1))
#define GPT_IRQ_ACK(n)           (1 << ((n) - 1))
#define GPT_CTRL_REG(n)          (0x10 * (n))
#define GPT_CLK_REG(n)           (0x04 + (0x10 * (n)))
#define GPT_CNT_REG(n)           (0x08 + (0x10 * (n)))
#define GPT_CMP_REG(n)           (0x0C + (0x10 * (n)))
#define GPT_CTRL_OP(v)           (((v) & 0x3) << 4)
#define GPT_CTRL_OP_ONESHOT      (0)
#define GPT_CTRL_OP_FREERUN      (3)
#define GPT_CTRL_CLEAR           (2)
#define GPT_CTRL_ENABLE          (1)
#define GPT_CLK_SRC_SYS13M       (0)
#define GPT_CLK_DIV1             (0x0)
#define MT6582_TIMER_CLK_EVT     (1)    /* GPT1: tick del scheduler (one-shot) */
#define MT6582_TIMER_CLK_SRC     (2)    /* GPT2: timebase (free-run) */
#define MT6582_TIMER_RATE        13000000   /* system_clk = 13MHz */

/* IRQ del timer: DTS dice GIC_SPI 112 -> numero GIC = 32 + 112 */
#define MT6582_INT_TIMER         (32 + 112)
#define NR_IRQS                  (32 + 224)

/* ---- UART0 (8250, reg-shift 2). Sin acceso fisico al UART en el krillin:
 *      la consola util es el framebuffer. ---- */
#define MT6582_UART0_BASE        0x11002000
#define UART_RBR                 0x00
#define UART_THR                 0x00
#define UART_LSR                 0x14
#define UART_LSR_DR              0x01
#define UART_LSR_THRE            0x20

/* ---- Framebuffer que deja el LK (VALIDADO en HW por GenericBooter M1):
 *      panel Himax hx8389 qHD, DSI modo video (se auto-refresca). ---- */
#define MT6582_FB_BASE           0xBF400000
#define MT6582_FB_WIDTH          540
#define MT6582_FB_HEIGHT         960
#define MT6582_FB_STRIDE         2176   /* lo que se le DICE a XNU (ver nota) */
#define MT6582_FB_DEPTH          32     /* lo que se le DICE a XNU (ver nota) */

/*
 * NOTA: el panel es RGB565 de 16 bits, stride 1088 (medido con un patron de
 * calibracion).  Nuestra consola pinta asi.  Pero estos dos valores son solo
 * lo que se copia a PE_state.video, o sea lo que XNU CREE.  Poniendolos a
 * 16/1088 el arranque se muere y la pantalla queda roja; con 32/2176 llegaba
 * mucho mas lejos.  Como toda la salida de video propia de XNU esta
 * desactivada, declararlo como 32bpp es inocuo y evita ese camino.
 */

/* ---- Watchdog TOPRGU (VALIDADO en HW: sin desarmarlo, reset a los segundos) ---- */
#define MT6582_WDT_BASE          0x10007000
#define MT6582_WDT_MODE          0x00
#define MT6582_WDT_RESTART       0x08
#define MT6582_WDT_MODE_KEY      0x22000000
#define MT6582_WDT_RESTART_KEY   0x1971
#define MT6582_WDT_MODE_ENABLE   0x0001
#define MT6582_WDT_MODE_EXTPOL   0x0004
#define MT6582_WDT_MODE_EXTEN    0x0008

#endif /* _PEXPERT_MT6582_H_ */
