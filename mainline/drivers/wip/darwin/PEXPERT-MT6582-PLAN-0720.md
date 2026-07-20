# Qué hace falta para el pexpert MT6582 (XNU) — plan concreto, 0720

> El **pexpert** ("Platform Expert") es la capa de XNU que abstrae el SoC. Es **sorprendentemente
> pequeño**: los ports existentes van de **165 a 595 líneas** (`pe_sun4i.c` 165, `pe_apq8060.c` 376,
> `pe_realview.c` 415, `pe_omap3530.c` 595). No es un port de kernel: es una **tabla de callbacks**
> (`gPESocDispatch`) + una cabecera con las direcciones del SoC.

## 1. El mapa hardware — YA LO TENEMOS ENTERO

Esto es lo grande: cada dirección que pide un pexpert **ya la conocemos** del port de Linux, y las
dos más delicadas están **validadas en hardware** por el M1 de GenericBooter.

| Pieza | MT6582 / krillin | Origen |
|---|---|---|
| GIC distributor | **`0x10211000`** (0x1000) | DTS (`arm,cortex-a7-gic`) |
| GIC CPU interface | **`0x10212000`** (0x2000) | DTS |
| GIC vctrl / vcpu | `0x10214000` / `0x10216000` | DTS |
| Timer (MTK GPT) | **`0x10008000`** (0x80), IRQ **SPI 112** | DTS (`mediatek,mt6577-timer`) |
| UART0 | **`0x11002000`**, IRQ SPI 51, 8250 reg-shift 2 | DTS + M1 |
| **Framebuffer** | **`0xBF400000`**, 540x960, stride **2176** (544*4), 32bpp | **✅ M1 en HW** |
| **Watchdog / restart** | **`0x10007000`**, key `0x22000000`, restart `+0x08` key `0x1971` | **✅ M1 en HW** |
| sysirq (polaridad) | `0x10200100` (0x1c) | DTS |

## 2. Ficheros a crear/tocar — solo 4

### a) `pexpert/arm/pe_mt6582.h` (nuevo, ~40 líneas)
Los `#define` de la tabla de arriba. Plantilla: `pe_apq8060.h`.

### b) `pexpert/arm/pe_mt6582.c` (nuevo, ~250-400 líneas) — **plantilla: `pe_apq8060.c`**
Se eligió el de Qualcomm porque es **GIC + ARMv7**, igual que el MT6582 (el de OMAP3/N900 usa el
INTC de TI, distinto). Hay que implementar el dispatch:

```c
void PE_init_SocSupport_mt6582(void) {
    gPESocDispatch.uart_getc         = mt6582_getc;
    gPESocDispatch.uart_putc         = mt6582_putc;
    gPESocDispatch.uart_init         = mt6582_uart_init;
    gPESocDispatch.interrupt_init    = mt6582_interrupt_init;
    gPESocDispatch.timebase_init     = mt6582_timebase_init;
    gPESocDispatch.get_timebase      = mt6582_get_timebase;
    gPESocDispatch.handle_interrupt  = mt6582_handle_interrupt;
    gPESocDispatch.timer_value       = mt6582_timer_value;
    gPESocDispatch.timer_enabled     = mt6582_timer_enabled;
    gPESocDispatch.framebuffer_init  = mt6582_framebuffer_init;
    mt6582_framebuffer_init();
    mt6582_uart_init();
    PE_halt_restart = mt6582_halt_restart;
}
```
Trabajo por bloque:
- **framebuffer_init** → 🟢 **casi copiar-pegar de nuestro `plat-mt6582/fb.c` del M1** (misma base,
  mismo stride, ya probado en pantalla). El más fácil y ya resuelto.
- **halt_restart** → 🟢 el watchdog del M1 (`0x10007000`), ya validado.
- **putc/getc/uart_init** → 🟢 8250 estándar (RBR/THR 0x0, LSR 0x14, reg-shift 2). Ojo: **sin acceso
  físico al UART**, así que en la práctica la consola útil será el framebuffer.
- **interrupt_init / handle_interrupt** → 🟡 GIC-400: init del distributor + CPU iface, leer IAR,
  despachar, escribir EOI. `pe_apq8060.c` lo hace casi igual (GIC de Qualcomm); adaptar bases.
- **timebase / timer_value / timer_enabled** → 🔴 **lo más laborioso**: el GPT de MediaTek no se
  parece al de Qualcomm. Hay que portar del downstream Linux (`mtk_timer`): GPT en free-run como
  timebase + un GPT en modo one-shot para el tick del scheduler. Referencia:
  `drivers/clocksource/timer-mediatek.c` de nuestro árbol mainline.

### c) `makedefs/MakeInc.def` (2 líneas)
```make
export SUPPORTED_ARM_MACHINE_CONFIGS := ... IMX53 MT6582
ARCH_FLAGS_ARM_MT6582 = -arch armv7
```

### d) `pexpert/conf/files.arm` (1 línea)
```
pexpert/arm/pe_mt6582.c		standard
```

Build: `make TARGET_CONFIGS="debug arm MT6582" NO_DTRACE_SYMS=YES`

## 3. Lo que NO cubre el pexpert (y también hará falta)

- **Apple DeviceTree**: XNU espera un device tree en formato Apple (≠ FDT). Repos
  `darwin-on-arm/dtc-AppleDeviceTree` y `DeviceTrees` (hay plantillas por plataforma).
- **`osfmk/arm/`**: puede requerir tocar el mapa de memoria/arranque (`arm_vm_init`) si la RAM del
  krillin no encaja en las asunciones (DRAM en `0x80000000`, 1GB).
- **Empaquetado**: `image3maker` (✅ ya construido) para hacer el `mach.img3` que GenericBooter carga,
  + `rdsk.img3` (ramdisk).
- **Userland**: aunque XNU arranque, sin ramdisk con un launchd/dyld de Darwin no hay sistema usable.

## 4. Orden de ataque sugerido

1. **Registrar la máquina** (c + d) y crear `pe_mt6582.{c,h}` **stub** copiado de apq8060 con las
   direcciones nuestras → intentar `make TARGET_CONFIGS="debug arm MT6582"`. Objetivo: **que XNU
   COMPILE** (M2b). Los timers pueden ser stubs que devuelvan 0 al principio.
2. Rellenar framebuffer (copiar del M1) y halt_restart → consola visible.
3. GIC (init + dispatch).
4. Timer MTK (el bloque duro).
5. DeviceTree Apple + `image3maker` → intentar arrancar con GenericBooter (M3).

## 5. Valoración honesta

**Lo bueno**: el pexpert es pequeño, tenemos TODO el mapa hardware, y las dos piezas críticas
(framebuffer y watchdog) están **probadas en el móvil**. El toolchain ya compila (M2a ✅).

**Lo caro**: (1) el timer de MediaTek hay que portarlo a mano; (2) XNU de 2014 compilado con
clang-19 dará su propia cosecha de errores (como pasó con el SDK, pero a mayor escala: XNU son
~1M líneas frente a las 31 cabeceras del SDK); (3) aunque arranque, sin userland Darwin no hay
sistema usable — sería un "XNU arranca y hace panic/consola", que es el hito honesto a perseguir.

*0720, sesión Windows. Toolchain en `/opt/Developer/Toolchains/DarwinARM.toolchain`; XNU clonado en
la Pi `~/darwin-krillin/xnu` (66M). Ver `FEASIBILITY-DARWIN-0719.md` para M0/M1/M2a.*
