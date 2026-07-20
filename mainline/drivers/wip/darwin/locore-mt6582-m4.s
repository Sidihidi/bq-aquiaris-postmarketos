/*
 * Copyright 2013, winocm. <winocm@icloud.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 *   If you are going to use this software in any form that does not involve
 *   releasing the source to this project or improving it, let me know beforehand.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * ARM system startup
 */

#include <arm/arch.h>
#include <arm/asm_help.h>
#include <assym.s>
#include <mach/arm/asm.h>
#include <arm/PlatformConfigs.h>

/*
 * During system initialization, there are two possible methods of
 * initial bootstrap.
 *
 * The old BootKit loader prepared the initial virtual memory mappings
 * for the kernel. When we boot using a shim loader, we don't get this
 * luxury, so we have to do it ourselves. Isn't that quite fun?
 *
 *  - r0 = kernel boot-args structure.
 *
 * The boot-args structure will be updated to then be virtual.
 */

EnterARM(_start)
    /* PRUEBA DE VIDA (M3 debug): pintar el framebuffer del LK de BLANCO al entrar.
     * Fisico 0xBF400000, MMU aun off. Si la pantalla se pone blanca -> XNU EJECUTA. */
    movw    r4, #0x0000
    movt    r4, #0xBF40          /* r4 = 0xBF400000 */
    movw    r6, #0x0000
    movt    r6, #0x0020          /* r6 = 0x200000 palabras (~2M px) */
    mvn     r5, #0                /* r5 = 0xFFFFFFFF (blanco) */
Lprobe_fill:
    str     r5, [r4], #4
    subs    r6, r6, #1
    bne     Lprobe_fill
    /* fin prueba de vida */
    /* First, disable interrupts so that the BL doesn't get any. */
    LOAD_ADDR(lr, arm_init)
    cpsid   if

    /* If MMU is initialized, go the quick way. */
    mrc     p15, 0, r4, c1, c0, 0
    and     r4, #0x1
    cmp     r4, #0x1
    beq     mmu_initialized

mmu_reinitialize:
    /*
     * MMU initialization part begins here. -----------------------
     *
     * Basically, all of SDRAM gets remapped to the virtual base.
     */

    /* Adjust DACR register. */
    mov     r4, #0x1

    mcr     p15, 0, r4, c3, c0, 0
#ifdef _ARM_ARCH_7
    isb     sy
#endif

    /* Clean TLB and instruction cache. */
    mov     r4, #0
    mcr     p15, 0, r4, c8, c7, 0
    mcr     p15, 0, r4, c7, c5, 0
    mcr     p15, 0, r4, c2, c0, 2

    /*
     * Create a dumb mapping for right now. This mapping lies
     * at the top of kernel data.
     */
    ldr     r4, [r0, BOOT_ARGS_TOP_OF_KERNEL]
    ldr     r10, [r0, BOOT_ARGS_VIRTBASE]
    ldr     r11, [r0, BOOT_ARGS_PHYSBASE]

    /* Is it bootArgs revision 3? */
    ldrh    r12, [r0, BOOT_ARGS_VERSION]
    cmp     r12, #3

    /* Align the memory size to 1MB for compatibility. */
    ldreq   r5, [r0, BOOT_ARGS_MEMSIZE]
    andeq   r5, r5, #0xFFF000000
    streq   r5, [r0, BOOT_ARGS_MEMSIZE]

    /* Load memory size value after fixup. */
    ldr     r12, [r0, BOOT_ARGS_MEMSIZE]

    /* MMU cacheability value. */
    orr     r5, r4, #0x18

    /* Now, we have to set our TTB to this value. */
    mcr     p15, 0, r5, c2, c0, 0

    /* Make our section mappings now. */
    mov     r6, #0xE            /* This is a section descriptor */
    orr     r6, r6, #0x400      /* Permissions */

    /* Identity map UART for right now */
    LoadConstantToReg((0x7f600000), r7)
    mov     r7, r7, lsr#20
    add     r5, r4, r7, lsl#2
    mov     r7, r7, lsl#20
    orr     r8, r7, r6
    str     r8, [r5]

    mov     r7, pc, lsr#20
    add     r5, r4, r7, lsl#2   /* Make the TTE offset */

    /* God, I hope we're loaded at the beginning of SDRAM. */
    mov     r7, r7, lsl#20
    orr     r8, r7, r6

    /* Store our section mappings. */
    str     r8, [r5]

    /* Get the physical address... */
    mov     r1, r11
    add     r5, r4, r10, lsr#18
map:
    /* Just map all of SDRAM. */
    orr     r8, r1, r6
    str     r8, [r5], #4
    add     r1, r1, #_1MB
    subs    r12, r12, #_1MB
    bne     map

    /* M4 FIX: RE-escribir el mapeo IDENTITY del PC actual DESPUES del bucle del
     * mapa principal. En el MT6582 la DRAM fisica esta en 0x80000000 = virtBase,
     * asi que el bucle sobreescribe la seccion del PC (0x810). Sin esto, al activar
     * la MMU el fetch de la siguiente instruccion cae en una direccion mal mapeada
     * y cuelga. r4=TTE base, r6=flags de seccion. */
    mov     r7, pc, lsr#20
    add     r5, r4, r7, lsl#2
    mov     r7, r7, lsl#20
    orr     r8, r7, r6
    str     r8, [r5]

    /* M4: mapear IDENTITY el framebuffer del LK (0xBF400000, 16 secciones=16MB)
     * para que la consola de XNU pueda escribir tras la MMU. r4=TTE base, r6=flags. */
    LoadConstantToReg((0xBF400000), r1)
    mov     r7, r1, lsr#20
    add     r5, r4, r7, lsl#2
    mov     r9, #16
Lmap_fb:
    orr     r8, r1, r6
    str     r8, [r5], #4
    add     r1, r1, #_1MB
    subs    r9, r9, #1
    bne     Lmap_fb

    /* M4 PRUEBA PRE-MMU (fisico): ROJO justo antes de activar la MMU.
     * Si se ve rojo -> la tabla de paginas se monto sin fallar. */
    push    {r4, r5, r6}
    movw    r4, #0x0000
    movt    r4, #0xBF40
    movw    r6, #0x0000
    movt    r6, #0x0020
    movw    r5, #0x0000
    movt    r5, #0x00FF          /* rojo 0x00FF0000 */
Lprobe_red:
    str     r5, [r4], #4
    subs    r6, r6, #1
    bne     Lprobe_red
    pop     {r4, r5, r6}

    /* Start MMU. */
    mrc     p15, 0, r3, c1, c0, 0
    orr     r3, r3, #1
    mcr     p15, 0, r3, c1, c0, 0

    /* M4 PRUEBA POST-ENABLE (MMU ya ON, ANTES del salto a virtual): VERDE-AZUL.
     * fb mapeado identity. Si se ve -> la MMU activo bien; el fallo es el bx a virtual. */
    push    {r4, r5, r6}
    movw    r4, #0x0000
    movt    r4, #0xBF40
    movw    r6, #0x0000
    movt    r6, #0x0020
    movw    r5, #0xFFFF
    movt    r5, #0x0000          /* 0x0000FFFF (cian en BGR) */
Lprobe_cyan:
    str     r5, [r4], #4
    subs    r6, r6, #1
    bne     Lprobe_cyan
    pop     {r4, r5, r6}

    /*
     * Hopefully, if we got here, things are looking good and we
     * are running in VM mode.
     */

     /*
      * xxx KASLR: we need to jump to a trampoline.
      * The address in r3 is relative, we convert it to a KVA and jump.
      */
    ldr     r3, =start_trampoline
    /* M4 FIX: el kernel esta LINKADO en virtual (0x80000000-based) y el mapa
     * principal mapea virtBase->physBase, asi que "ldr =start_trampoline" ya da
     * la direccion VIRTUAL correcta. La conversion phys->virt original
     * (sub physBase / add virtBase) la corrompe -> bx directo. */
    bx      r3
start_trampoline:
    nop

    /* M4 PRUEBA POST-MMU: pintar el fb de VERDE ya en modo virtual (MMU on).
     * Si se ve verde -> MMU funciona, fb mapeado y XNU corre en virtual. */
    push    {r0, r4, r5, r6}
    movw    r4, #0x0000
    movt    r4, #0xBF40
    movw    r6, #0x0000
    movt    r6, #0x0020
    movw    r5, #0xFF00
    movt    r5, #0x0000          /* verde 0x0000FF00 */
Lprobe_green:
    str     r5, [r4], #4
    subs    r6, r6, #1
    bne     Lprobe_green
    pop     {r0, r4, r5, r6}

fix_boot_args_hack_for_bootkit:
    /* Fix up boot-args */
    sub     r0, r0, r11
    add     r0, r0, r10

    /* Goddamn section offset. */
    LOAD_ADDR(r12, sectionOffset)
    mov     sp, #0
    str     sp, [r12]

#ifdef _ARM_ARCH_7
    /*
     * VBAR Note:
     * The exception vectors are mapped high also at 0xFFFF0000 for compatibility purposes.
     */

    /* Set low vectors. */
    mrc     p15, 0, r4, c1, c0, 0
    bic     r4, r4, #(1 << 13)
    mcr     p15, 0, r4, c1, c0, 0

    /* Set NS-VBAR to ExceptionVectorsBase */
    LOAD_ADDR(r4, ExceptionVectorsBase)
    mcr     p15, 0, r4, c12, c0, 0

#else

    /* Now, the vectors could be mapped low. Fix that. */
    mrc     p15, 0, r4, c1, c0, 0
    orr     r4, r4, #(1 << 13)
    mcr     p15, 0, r4, c1, c0, 0

#endif

    /*
     * MMU initialization end. ------------------------------------
     */

mmu_initialized:
    /*
     * Zero out the frame pointer so that the kernel fp tracer
     * doesn't go farther than it needs to.
     */
    mov     r7, #0

#if __ARM_PROCESSOR_CLASS_CORTEX_A9__
    /* Enable automatic-clock gating. */
    mrc     p15, 0, r4, c15, c0, 0
    orr     r4, r4, #1
    mcr     p15, 0, r4, c15, c0, 0
#endif

    /* Enable unaligned memory access and caching */
    mrc     p15, 0, r4, c1, c0, 0
    orr     r4, r4, #(1 << 22)  /* Force unaligned accesses, fixes OMAP boot. */
    bic     r4, r4, #(1 << 1)
    orr     r4, r4, #(1 << 23)  /* Unaligned memory access */
    orr     r4, r4, #(1 << 12)  /* Enable I-cache */
    mcr     p15, 0, r4, c1, c0, 0

    /* Invalid Data/Inst TLB */
    mov     r4, #0
    mcr     p15, 0, r4, c8, c7, 0

    /* Invalidate caches */
    mcr     p15, 0, r4, c7, c5, 0

    /* Set CONTEXIDR to 0, kernel ASID. */
    mcr     p15, 0, r4, c13, c0, 1

    /* Set up initial sp. */
    LOAD_ADDR(sp, intstack_top)

    /* Boot to ARM init. */
    bx      lr

/**
 * sleep_test
 */
EnterARM(sleep_test)
    /* Get physical base. */
    ldr     r8, [r1, BOOT_ARGS_PHYSBASE]
    ldr     r9, [r1, BOOT_ARGS_VIRTBASE]
    ldr     r4, [r1, BOOT_ARGS_TOP_OF_KERNEL]

    /* Set new page tables. (kernel bootstrap page table) */
    orr     r6, r4, #0x18
    mcr     p15, 0, r6, c2, c0, 0
    mcr     p15, 0, r6, c2, c0, 1

    sub     r4, r4, r8
    add     r4, r4, r9

    /* Create boot page table entry for trampoline. */
    ldr     r10, [r1, BOOT_ARGS_MEMSIZE]
    mov     r6, #0xE
    mov     r1, #1
    orr     r6, r6, r1, lsl#10
    add     r5, r4, r8, lsr#18
    orr     r11, r8, r6
    str     r11, [r5]

    /* Clear unified TLB */
    mov     r1, #0
    mcr     p15, 0, r1, c8, c7, 0
#ifdef _ARM_ARCH_7
    isb     sy
#endif

    /* Clear MMU-EN bit in SCTLR */
    mrc     p15, 0, r11, c1, c0, 0
    bic     r11, r11, #1
#ifdef _ARM_ARCH_7
    isb     sy
#endif

    /* Jump to physical trampoline. */
    ldr     r4, =sleep_tramp
    sub     r4, r4, r9
    add     r4, r4, r8
    bx      r4
sleep_tramp:
    cpsid   if, #0x13
    mcr     p15, 0, r11, c1, c0, 0
    nop
    nop
    nop
    nop
    bx      r2


/*
 * Initial stack
 */
.data
.align 4
.globl _intstack_top
.globl _intstack
_intstack:
.space (8192), 0
_intstack_top:

/*
 * ARM SMP stack.
 */
.globl _debstack_top
.globl _debstack
_debstack:
.space (8192), 0
_debstack_top:

LOAD_ADDR_GEN_DEF(ExceptionVectorsBase)
LOAD_ADDR_GEN_DEF(arm_init)
LOAD_ADDR_GEN_DEF(intstack_top)
LOAD_ADDR_GEN_DEF(intstack)
LOAD_ADDR_GEN_DEF(sectionOffset)
