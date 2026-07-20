/*
 * pexpert/arm/pe_mt6582.c — Platform Expert para MediaTek MT6582
 * (BQ Aquaris E4.5 "krillin"). Cortex-A7 quad + GIC-400.
 *
 * Basado en pe_apq8060.c (Qualcomm) por ser GIC + ARMv7 igual que este SoC
 * (el de OMAP3/N900 usa el INTC de TI, que no aplica).
 * Framebuffer y watchdog VALIDADOS EN HARDWARE por GenericBooter (M1).
 */
#if defined(BOARD_CONFIG_MT6582)

#include <mach/mach_types.h>
#include <IOKit/IOPlatformExpert.h>
#include <pexpert/pexpert.h>
#include <pexpert/arm/protos.h>
#include <pexpert/arm/boot.h>
#include <machine/machine_routines.h>
#include <vm/pmap.h>
#include <arm/pmap.h>
#include "pe_mt6582.h"

#define KPRINTF_PREFIX  "PE_MT6582: "
#define HwReg(x) *((volatile unsigned long*)(x))
#define barrier() __asm__ __volatile__("": : :"memory")

extern void rtclock_intr(arm_saved_state_t * regs);
extern void rtc_configure(uint64_t hz);

vm_offset_t gMT6582UartBase;
vm_offset_t gMT6582GICCPUBase;
vm_offset_t gMT6582GICDistBase;
vm_offset_t gMT6582TimerBase;
vm_offset_t gMT6582WdtBase;

static uint64_t clock_decrementer = 0;
static boolean_t clock_initialized = FALSE;
static boolean_t clock_had_irq = FALSE;
static uint64_t clock_absolute_time = 0;

static void timer_configure(void)
{
    uint64_t hz = MT6582_TIMER_RATE;
    gPEClockFrequencyInfo.timebase_frequency_hz = hz;
    clock_decrementer = hz / 100;          /* tick de 10ms */
    kprintf(KPRINTF_PREFIX "decrementer = %llu (rate %llu Hz)\n",
            clock_decrementer, hz);
    rtc_configure(hz);
    return;
}

/* ---------------- UART (8250, reg-shift 2) ----------------
 * El krillin no expone el UART fisicamente; se implementa por completitud,
 * pero la consola real de este port es el framebuffer. */
void mt6582_putc(int c)
{
    if (!gMT6582UartBase)
        return;
    if (c == '\n')
        mt6582_putc('\r');
    while (!(HwReg(gMT6582UartBase + UART_LSR) & UART_LSR_THRE))
        barrier();
    HwReg(gMT6582UartBase + UART_THR) = c;
}

int mt6582_getc(void)
{
    if (!gMT6582UartBase)
        return -1;
    while (!(HwReg(gMT6582UartBase + UART_LSR) & UART_LSR_DR))
        barrier();
    return (int) (HwReg(gMT6582UartBase + UART_RBR) & 0xff);
}

void mt6582_uart_init(void)
{
    gMT6582UartBase    = ml_io_map(MT6582_UART0_BASE,    PAGE_SIZE);
    gMT6582GICDistBase = ml_io_map(MT6582_GIC_DIST_BASE, PAGE_SIZE);
    gMT6582GICCPUBase  = ml_io_map(MT6582_GIC_CPU_BASE,  PAGE_SIZE);
    gMT6582TimerBase   = ml_io_map(MT6582_TIMER_BASE,    PAGE_SIZE);
    gMT6582WdtBase     = ml_io_map(MT6582_WDT_BASE,      PAGE_SIZE);
    return;
}

/* ---------------- GIC-400 ---------------- */
static void gic_dist_init(void)
{
    uint32_t i, num_irq, cpumask = 1;

    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;

    HwReg(gMT6582GICDistBase + GIC_DIST_CTRL) = 0;
    num_irq = HwReg(gMT6582GICDistBase + GIC_DIST_CTR) & 0x1f;
    num_irq = (num_irq + 1) * 32;

    for (i = 32; i < num_irq; i += 16)
        HwReg(gMT6582GICDistBase + GIC_DIST_CONFIG + i * 4 / 16) = 0;
    for (i = 32; i < num_irq; i += 4)
        HwReg(gMT6582GICDistBase + GIC_DIST_TARGET + i * 4 / 4) = cpumask;
    for (i = 0; i < num_irq; i += 4)
        HwReg(gMT6582GICDistBase + GIC_DIST_PRI + i) = 0xa0a0a0a0;
    for (i = 0; i < num_irq; i += 32)
        HwReg(gMT6582GICDistBase + GIC_DIST_ENABLE_CLEAR + i * 4 / 32) = 0xffffffff;

    HwReg(gMT6582GICDistBase + GIC_DIST_CTRL) = 0x1;
}

static void gic_cpu_init(void)
{
    HwReg(gMT6582GICCPUBase + GIC_CPU_PRIMASK) = 0xf0;
    HwReg(gMT6582GICCPUBase + GIC_CPU_CTRL) = 0x1;
}

void mt6582_interrupt_init(void)
{
    assert(gMT6582GICDistBase && gMT6582GICCPUBase);
    gic_dist_init();
    gic_cpu_init();
    return;
}

/* ---------------- Timer GPT de MediaTek ---------------- */
uint64_t mt6582_timer_value(void);
void mt6582_timer_enabled(int enable);

void mt6582_timebase_init(void)
{
    uint32_t reg, bit;

    timer_configure();
    mt6582_timer_enabled(FALSE);

    /* GPT2 en free-run como fuente del timebase */
    HwReg(gMT6582TimerBase + GPT_CTRL_REG(MT6582_TIMER_CLK_SRC)) = GPT_CTRL_CLEAR;
    HwReg(gMT6582TimerBase + GPT_CLK_REG(MT6582_TIMER_CLK_SRC))  =
        GPT_CLK_SRC_SYS13M | GPT_CLK_DIV1;
    HwReg(gMT6582TimerBase + GPT_CTRL_REG(MT6582_TIMER_CLK_SRC)) =
        GPT_CTRL_OP(GPT_CTRL_OP_FREERUN) | GPT_CTRL_ENABLE;

    /* habilitar el IRQ del timer en el GIC */
    reg = GIC_DIST_ENABLE_SET + (MT6582_INT_TIMER / 32) * 4;
    bit = 1 << (MT6582_INT_TIMER & 31);
    HwReg(gMT6582GICDistBase + reg) = bit;

    ml_set_interrupts_enabled(TRUE);
    mt6582_timer_enabled(TRUE);
    clock_initialized = TRUE;

    while (!clock_had_irq)
        barrier();

    return;
}

void mt6582_handle_interrupt(void *context)
{
    uint32_t irq_no = HwReg(gMT6582GICCPUBase + GIC_CPU_INTACK);

    if (irq_no >= NR_IRQS) {
        kprintf(KPRINTF_PREFIX "IRQ espuria %u\n", irq_no);
        return;
    }

    if (irq_no == MT6582_INT_TIMER) {
        HwReg(gMT6582TimerBase + GPT_IRQ_ACK_REG) = GPT_IRQ_ACK(MT6582_TIMER_CLK_EVT);
        mt6582_timer_enabled(FALSE);
        clock_absolute_time += (clock_decrementer - (int64_t) mt6582_timer_value());
        rtclock_intr((arm_saved_state_t *) context);
        mt6582_timer_enabled(TRUE);
        clock_had_irq = TRUE;
    } else {
        irq_iokit_dispatch(irq_no);
    }

    HwReg(gMT6582GICCPUBase + GIC_CPU_EOI) = irq_no;
    return;
}

uint64_t mt6582_get_timebase(void)
{
    uint32_t timestamp;

    if (!clock_initialized)
        return 0;

    timestamp = mt6582_timer_value();
    if (timestamp) {
        uint64_t v = clock_absolute_time;
        v += (uint64_t) (((uint64_t) clock_decrementer) - (uint64_t) (timestamp));
        return v;
    } else {
        clock_absolute_time += clock_decrementer;
        return clock_absolute_time;
    }
}

uint64_t mt6582_timer_value(void)
{
    if (!gMT6582TimerBase)
        return 0;
    return (uint64_t) HwReg(gMT6582TimerBase + GPT_CNT_REG(MT6582_TIMER_CLK_EVT));
}

void mt6582_timer_enabled(int enable)
{
    if (!gMT6582TimerBase)
        return;

    if (enable) {
        HwReg(gMT6582TimerBase + GPT_CTRL_REG(MT6582_TIMER_CLK_EVT)) = GPT_CTRL_CLEAR;
        HwReg(gMT6582TimerBase + GPT_CLK_REG(MT6582_TIMER_CLK_EVT))  =
            GPT_CLK_SRC_SYS13M | GPT_CLK_DIV1;
        HwReg(gMT6582TimerBase + GPT_CMP_REG(MT6582_TIMER_CLK_EVT))  =
            (uint32_t) clock_decrementer;
        HwReg(gMT6582TimerBase + GPT_IRQ_EN_REG) |= GPT_IRQ_ENABLE(MT6582_TIMER_CLK_EVT);
        HwReg(gMT6582TimerBase + GPT_CTRL_REG(MT6582_TIMER_CLK_EVT)) =
            GPT_CTRL_OP(GPT_CTRL_OP_ONESHOT) | GPT_CTRL_ENABLE;
    } else {
        HwReg(gMT6582TimerBase + GPT_IRQ_EN_REG) &= ~GPT_IRQ_ENABLE(MT6582_TIMER_CLK_EVT);
        HwReg(gMT6582TimerBase + GPT_CTRL_REG(MT6582_TIMER_CLK_EVT)) = 0;
        barrier();
        HwReg(gMT6582TimerBase + GPT_IRQ_ACK_REG) = GPT_IRQ_ACK(MT6582_TIMER_CLK_EVT);
        barrier();
    }
    return;
}

/* ---------------- Framebuffer (el que deja el LK; validado en M1) ---------------- */
void vcputc(__unused int l, __unused int u, int c);

static void _fb_putc(int c)
{
    if (c == '\n')
        vcputc(0, 0, '\r');
    vcputc(0, 0, c);
}

void mt6582_framebuffer_init(void)
{
    char tempbuf[16];

    PE_state.video.v_baseAddr = (unsigned long) MT6582_FB_BASE;
    PE_state.video.v_rowBytes = MT6582_FB_STRIDE;   /* 2176, NO width*4 */
    PE_state.video.v_width    = MT6582_FB_WIDTH;
    PE_state.video.v_height   = MT6582_FB_HEIGHT;
    PE_state.video.v_depth    = MT6582_FB_DEPTH;

    kprintf(KPRINTF_PREFIX "framebuffer %ux%u stride %u @ 0x%08x\n",
            MT6582_FB_WIDTH, MT6582_FB_HEIGHT, MT6582_FB_STRIDE, MT6582_FB_BASE);

    if (PE_parse_boot_argn("-early-fb-debug", tempbuf, sizeof(tempbuf)))
        initialize_screen((void *) &PE_state.video, kPEAcquireScreen);

    if (PE_parse_boot_argn("-graphics-mode", tempbuf, sizeof(tempbuf)))
        initialize_screen((void *) &PE_state.video, kPEGraphicsMode);
    else
        initialize_screen((void *) &PE_state.video, kPETextMode);

    return;
}

/* ---------------- Halt / restart (watchdog TOPRGU, validado en M1) ---------------- */
int mt6582_halt_restart(int type)
{
    if (!gMT6582WdtBase)
        return -1;

    if (type == kPERestartCPU) {
        /* watchdog armado con reset externo -> reinicia el SoC */
        HwReg(gMT6582WdtBase + MT6582_WDT_MODE) =
            MT6582_WDT_MODE_KEY | MT6582_WDT_MODE_ENABLE | MT6582_WDT_MODE_EXTEN;
        HwReg(gMT6582WdtBase + MT6582_WDT_RESTART) = MT6582_WDT_RESTART_KEY;
        while (1)
            barrier();
    }

    /* halt: desarmar el watchdog (si no, resetea a los segundos) y parar */
    HwReg(gMT6582WdtBase + MT6582_WDT_MODE) = MT6582_WDT_MODE_KEY;
    while (1)
        barrier();

    return 0;
}

void PE_init_SocSupport_mt6582(void)
{
    gPESocDispatch.uart_getc        = mt6582_getc;
    gPESocDispatch.uart_putc        = mt6582_putc;
    gPESocDispatch.uart_init        = mt6582_uart_init;
    gPESocDispatch.interrupt_init   = mt6582_interrupt_init;
    gPESocDispatch.timebase_init    = mt6582_timebase_init;
    gPESocDispatch.get_timebase     = mt6582_get_timebase;
    gPESocDispatch.handle_interrupt = mt6582_handle_interrupt;
    gPESocDispatch.timer_value      = mt6582_timer_value;
    gPESocDispatch.timer_enabled    = mt6582_timer_enabled;
    gPESocDispatch.framebuffer_init = mt6582_framebuffer_init;

    mt6582_uart_init();          /* mapea los MMIO ANTES de tocar el fb */
    mt6582_framebuffer_init();

    PE_halt_restart = mt6582_halt_restart;
}

void PE_init_SocSupport_stub(void)
{
    PE_early_puts("PE_init_SocSupport: Initializing for MediaTek MT6582 (krillin)\n");
    PE_init_SocSupport_mt6582();
}

#endif /* !BOARD_CONFIG_MT6582 */
