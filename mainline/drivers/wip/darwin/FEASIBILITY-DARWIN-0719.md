# Darwin/XNU en el krillin — estudio de viabilidad + M0 CONSEGUIDO (0719)

> Referencia: [darwin-on-arm/xnu](https://github.com/darwin-on-arm/xnu) (portaron XNU al
> Nokia N900 = OMAP3, ARMv7 Cortex-A8). El krillin = MT6582, ARMv7 **Cortex-A7** → misma
> arquitectura base. Estudio de qué haría falta y hasta dónde se llega hoy.

## ✅ M0 CONSEGUIDO: el bootloader (GenericBooter) COMPILA para el krillin

`GenericBooter` (el 2º-stage que carga XNU+ramdisk+devicetree y salta al kernel) **compila
limpio** con nuestro port. Toolchain: `arm-none-eabi-gcc` 14.2 + `mkimage` (ambos apt, sin
nada de Apple). Artefacto: uImage 64.75 KiB, entry `0x80008040`, `_start` con el protocolo
de boot ARM estándar (r2 = dtb/atags).

**Lo que hizo falta (mínimo, ~180 líneas):**
- `arch/arm/plat-mt6582/` (copiado de plat-omap3): `debug.c` = UART putc del MT6582
  (16550 en `0x11002000`, reg-shift=2 → LSR=0x14/THR=0x0, idéntico a OMAP3 salvo la base)
  + Makefile (`obj-y = debug.o`).
- Kconfig: `config BOARD_MT6582 depends on PROC_CORTEX_A7`.
- `arch/arm/Makefile`: platname "MT6582", **textofs `0x80008040`** (DRAM 0x80000000 + 0x8000
  estándar ARM + 0x40 header), core → plat-mt6582/.
- `core/malloc.c`: añadido wrapper `calloc` (faltaba; lo usa el device-tree DT__AddProperty;
  wrap de `tlsf_calloc`). Único parche fuera del port — probablemente por newlib/gcc nuevos.

Patch reproducible: `genericbooter-mt6582.patch` (sobre GenericBooter @ b2f0298). Port en
`plat-mt6582/`.

## 🎉 M1 CONSEGUIDO EN HW (0720): GenericBooter EJECUTA en el krillin y PINTA EN PANTALLA

**Probado en hardware.** El LK del MTK carga y salta a nuestro binario bare-metal y el banner
("GenericBooter for MT6582" + build style + fecha) **aparece en la pantalla del móvil**, dibujado
por nuestro `plat-mt6582/fb.c` sobre el framebuffer del LK. Primer código no-Linux propio
corriendo en el krillin. Confirma de golpe: (a) la dirección de carga `0x80008000` deducida del
`zreladdr` era correcta; (b) el LK **no exige magic de zImage** (arranca un binario raw); (c) el
framebuffer del LK persiste hasta el salto y las escrituras físicas se ven.

**GOTCHA resuelto y VERIFICADO EN HW — el WATCHDOG TOPRGU reiniciaba el móvil:** (tras el fix,
confirmado 0720: pantalla limpiada a negro + banner blanco + **estable indefinidamente**, sin reset) primer intento = banner en pantalla
y **reset a los pocos segundos**. Causa: el LK deja el watchdog ARMADO y Linux lo va pateando;
GenericBooter no lo patea → reset. (Mismo watchdog que la saga del suspend.) Fix en `init_debug()`:
```c
#define MTK_WDT_MODE      0x10007000   /* AP_RGU/TOPRGU fisico (= AP_RGU_PHYS de mt6582-consys.c) */
#define MTK_WDT_MODE_KEY  0x22000000
*(volatile unsigned int *)MTK_WDT_MODE = MTK_WDT_MODE_KEY;  /* key + ENABLE(0x1)=0 -> desarmado */
```
**Cualquier código bare-metal que pongamos en el krillin necesita esto** (o patear
`MTK_WDT_RESTART` @+0x08 con key `0x1971`).

Otro detalle: en el 1er intento `mt6582_console_init()` no llegó a insertarse, así que el texto
salió dibujado ENCIMA del logo de BQ — lo que probó que las escrituras al FB funcionan aunque no
se limpie la pantalla. Ya corregido (limpia a negro primero).

## Detalle del port del framebuffer (M1)

Sin acceso al UART del krillin, la salida de M1 va por **framebuffer**. El LK del MTK deja el
panel hx8389 (qHD 540x960, DSI modo vídeo, **se auto-refresca**) con su FB en **`0xBF400000`**,
**stride 2176 = 544*4** (32bpp) — confirmado en el DTS mainline (`fb_region@bf400000` +
comentario del simple-framebuffer). GenericBooter corre bare-metal → escritura física directa
a 0xBF400000 = píxeles en pantalla, **sin tocar el DSI** (ni riesgo del negro-tras-modeset).

Port `plat-mt6582/fb.c` (consola de framebuffer, fuente 8x8, `setPixel` que SÍ escribe —el de
hptouchpad era un stub—) + `consolefont.c` (copiada) + `debug.c` engancha `putc_wrapper` →
UART **y** framebuffer. **Compila.** `mt6582_console_init()` limpia la pantalla y cada `printf`
del booter se dibuja.

**Dirección de carga CLAVE:** `zreladdr-y := 0x80008000` (downstream `Makefile.boot`) = donde
salta el LK del MT6582. GenericBooter es de posición fija → `_start` re-linkado a **exactamente
`0x80008000`** (textofs, sin el +0x40 de otras plats, que lo desalinearía 64B → crash).
Empaquetado `boot-genericbooter.img` (mtk_hdr KERNEL + ganador dtb + initrd menupick, como el
zImage). **Pendiente: el flash-test** (requiere mirar la pantalla física + recuperación fastboot).

Riesgos del 1er intento (recuperable por fastboot): (a) que el LK del MTK exija magic de zImage
y no arranque un binario raw; (b) MMU/caches (GenericBooter debería correr con MMU off/flat);
(c) que el FB del LK no persista exactamente hasta el salto. Si no aparece nada: reflashear
`boot-menupick24-consys.img` por fastboot.

## Estado de las 3 piezas de un boot de Darwin

| pieza | qué es | estado |
|---|---|---|
| **GenericBooter** | 2nd-stage: UART + carga XNU/ramdisk/dtb + salto | ✅ **compila** (M0) |
| **XNU (mach kernel)** | el kernel Darwin en sí | ❌ pendiente — necesita `darwin-sdk` |
| **Apple DeviceTree** | árbol de HW estilo Apple (NO el DTB de Linux) | pendiente (repo DeviceTrees + dtc-AppleDeviceTree) |
| ramdisk | rootfs Darwin mínimo | pendiente (repo ramdisk) |

## El hurdle real: compilar XNU necesita el toolchain Apple

XNU NO compila con gcc/none-eabi. Necesita `darwin-sdk` = **clang cross (target
`arm-apple-darwin`) + cctools-port (ld64/as/lipo, formato Mach-O) + ctf tools
(ctfconvert/merge de dtrace)**. Build: `make TARGET_CONFIGS="debug arm <board>"
NO_DTRACE_SYMS=YES`. Es el paso finicky (montar el SDK Apple en Linux). Además **hay que
crear una config de máquina nueva** en XNU (`pexpert/pe_arm_mt6582` estilo el
`pe_arm_omap3` del N900): mapa de memoria, GIC (interrupt controller), timer del SoC, UART.

## Sobre portar NUESTROS drivers a IOKit (lo que preguntaste)

**No se portan, se reescriben.** Los nuestros son C de kernel Linux (platform_driver, regmap,
DTS, subsistemas iio/cfg80211/ALSA). IOKit es **C++** con otro modelo entero (clases
`IOService`, matching por el Apple DeviceTree, familias propias `IOUARTFamily`/`IONetworking`
/etc.). Lo REUTILIZABLE es el **conocimiento del hardware** que ya tenemos documentado (qué
registros, en qué orden, la secuencia de power/clock) — eso ES la especificación y vale oro.
Pero cada driver = reescritura desde cero en el modelo IOKit. Es la fase LEJANA; primero XNU
tiene que bootear con lo mínimo (UART + timer + interrupts).

## Veredicto y ruta

- **Viable en teoría** (misma arquitectura que el N900 que lo logró), pero es un proyecto
  **grande y de recompensa incierta** (Darwin en el móvil = sin apps, sin utilidad práctica;
  valor = reto/aprendizaje).
- **Siguiente filtro barato (M1, sin coste de XNU):** bootear GenericBooter en el krillin y
  ver si imprime por el UART. Bloqueante: el UART (`ttyS0`, 921600) del krillin necesita
  **acceso físico** (test points / cable USB-serial) — sin eso, alternativa = portar un
  `fb.c` (framebuffer, como plat-hptouchpad) y escribir a pantalla, o escribir a una
  dirección de DRAM y leerla de vuelta. Riesgo: flashear código bare-metal (recuperable por
  fastboot, pero cuidado).
- **XNU (M2+):** montar el `darwin-sdk` en la Pi (ojo disco: hoy 1.3G libres, XNU es pesado)
  + crear `pe_arm_mt6582`. Semanas de trabajo antes del primer "hello" de XNU.

**Recomendación:** M0 demuestra que la cadena de build del booter funciona para nuestro HW.
Antes de invertir en XNU (el hurdle grande), decidir si hay acceso al UART para un M1 barato
que valide el boot handoff. Si no, es un proyecto de fondo de baja prioridad frente a
módem/GPS (que sí dan utilidad).

*0719, sesión Windows (Fable 5). GenericBooter compilando para MT6582 = primer artefacto
Darwin-side del krillin.*
