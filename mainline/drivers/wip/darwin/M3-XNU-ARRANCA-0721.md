# 🎉 M3: XNU EJECUTA en el krillin (MT6582) — 0721

> **Probado en hardware.** GenericBooter carga el kernel XNU, parsea el device tree, prepara los
> boot-args y **transfiere el control a XNU, que EJECUTA sus primeras instrucciones** en el BQ
> Aquaris E4.5. Prueba definitiva: una escritura al framebuffer inyectada como primera instrucción
> de `__start` de XNU **pone la pantalla en blanco** → el kernel de Apple corre código en el móvil.

## La cadena de boot completa (todo validado en HW)

```
LK MTK  →  GenericBooter (0x80008000, M1)  →  carga Mach-O XNU + device tree XML + boot-args
        →  _locore_jump_to(entry)  →  XNU __start (0x800e7910 VM / 0x811e7910 fis)  →  EJECUTA
```

Secuencia vista en pantalla (consola de framebuffer con scroll):
`[1]bootargs [2]kernel → Loading kernel → [K1]rebase [K2]filemap [K3]entry [K4]ok entry=0x800e7910
→ [3]devtree [4]devtree2 [5]finalargs [6]JUMP → "transferring control out of booter now!"` →
**pantalla BLANCA (XNU __start ejecutando).**

## Lo que hizo falta para M3 (patches en `genericbooter-m3-xnu.patch` + `devicetree/`)

1. **Los 3 Image3 embebidos en GenericBooter** (van tras `__end`, `get_image3` los recorre):
   - `Mach.img3` (tag `krnl`) = el `mach_kernel` XNU (7MB) — con `image3maker -t krnl`.
   - `DeviceTree.img3` (tag `xmdt`) = device tree en **XML plist** (`devicetree/mt6582-krillin.xml`).
   - `Ramdisk.img3` (tag `rdsk`) = de momento un buffer vacío (64KB).
   - Cada img3 → `.o` con `ld -r -b binary` + `objcopy --rename-section` a `.mach`/`.rdsk`/`.xmdt`,
     que el linkerscript coloca contiguos tras `__end`.
2. **`CONFIG_DT_SUPPORT_XML=y`** (venía `CONFIG_DT_SUPPORT_APPLE_FDT`, que busca un DT binario Apple
   con tag `dtre`; nuestra vía es el XML `xmdt`).
3. **Device tree = XML plist MÍNIMO** que empiece DIRECTO por `<dict>` (sin `<?xml?>`, sin
   comentarios, sin `<plist>` wrapper — el parser `XMLParseNextTag` no encuentra el dict de nivel
   superior si hay prólogo, y falla `prepare_devicetree()`). Nodos hijos = key `@children` con
   valor `<array>` de `<dict>`.
4. **Bloque de arranque MT6582 en `init/main.c`** (LA pieza que faltaba tras M1): el booter solo
   inicializaba `malloc` para HTC_HD2/HPTOUCHPAD → en el krillin `panic("malloc not inited")` justo
   tras el banner (eso era el "solo banner" de M1). Añadido bloque `#ifdef CONFIG_BOARD_MT6582`:
   `gBootArgs.physBase = 0x81000000; memSize = 0x20000000; malloc_init(...); commandLine="-v -s"`.
   ⚠️ **`physBase` DEBE estar por encima del booter+kernel embebido** (0x80008000..~0x80700000):
   `start_darwin` hace `bzero(physBase, 64MB)` y copia el kernel a `physBase+0x1000`; con
   physBase=0x80000000 se auto-borraría. Y el bucle de ATAGS se SALTA para MT6582 (el LK pasa un
   DTB, no atags).
5. **Watchdog TOPRGU desarmado** (heredado de M1): sin ello, reset a los segundos.
6. **Consola de framebuffer con SCROLL** (`plat-mt6582/fb.c`) + `__assert_func` que limpia la
   pantalla y muestra `fichero/linea/expr` aislado — imprescindible para depurar sin UART.

## Estado y lo que queda (M4: XNU hasta la consola)

**XNU ejecuta `__start`** pero tras `_start → arm_init` no llega (aún) a imprimir por la consola.
Siguiente frontera (territorio profundo de bring-up de XNU, muchas iteraciones esperables):
- `arm_init` monta MMU/page-tables, vectores de excepción, y espera un **`boot_args` con el layout
  EXACTO** que XNU quiere (virtBase/physBase/memSize/deviceTreeP/topOfKernelData...). Nuestro
  `prepare_finalized_boot_args` da uno básico; probablemente haya que afinarlo.
- La **consola de XNU** (`initialize_screen` + `vc_`) escribe a `PE_state.video.v_baseAddr` como
  dirección VIRTUAL tras la MMU; nuestro pexpert la pone en el físico `0xBF400000` → hay que
  mapearla (o identity-map temprano) para ver texto.
- Técnica de depuración probada: **inyectar escrituras al framebuffer** (0xBF400000, físico, MMU
  off) en puntos del arranque de XNU para localizar hasta dónde llega antes de fallar
  (`xnu-lifeprobe.patch` = la que pone la pantalla blanca en `__start`).

## Imágenes guardadas (Pi `~/darwin-krillin/`)
- `boot-xnu-M3.img` — llega a "transferring control" (sin la prueba de vida).
- `boot-xnu-M3-alive.img` — con la prueba: **pantalla blanca = XNU ejecuta**.
- `mach_kernel-mt6582` — el kernel XNU armv7 (7MB).

*0721, sesión Windows. Culminación del hilo Darwin: M0(compila)→M1(corre en HW)→M2(toolchain+XNU
enlazado)→M3(XNU EJECUTA). Ver FEASIBILITY-DARWIN-0719.md y PEXPERT-MT6582-PLAN-0720.md.*

---

# 🎉 M4 (0721): XNU CORRE EN MODO VIRTUAL (MMU activada)

**Probado en HW.** Tras M3 (XNU ejecuta `__start`), se depuró el arranque temprano inyectando
escrituras de color al framebuffer en 4 puntos. **Los 4 dispararon en orden** → XNU pasa `__start`,
monta la tabla de páginas, **activa la MMU**, y **salta a modo virtual** (`start_trampoline`).
(Los colores salen "raros" —blanco/azul/blanco/amarillo— porque el formato de píxel del framebuffer
no es el ARGB de 32bpp asumido; las escrituras funcionan igual, cada etapa cambia la pantalla.)

## Dos bugs REALES del `_start` de XNU con la memoria del MT6582 (fix en `xnu-m4-mmu-virtual.patch`)

1. **La sección del PC se sobreescribía → cuelgue al ACTIVAR la MMU.** `_start` mapea el PC físico
   identity (para seguir ejecutando tras encender la MMU), PERO en el MT6582 la DRAM física está en
   **0x80000000 = virtBase**, así que el bucle del mapa principal (virtBase→physBase, 512MB)
   sobreescribe la sección del PC (0x810). Al activar la MMU, el fetch de la siguiente instrucción
   cae en una dirección mal mapeada → prefetch abort. (En realview NO pasa porque su DRAM física
   está en direcciones BAJAS, lejos de virtBase=0x80000000.) **Fix: RE-escribir el mapeo identity
   del PC DESPUÉS del bucle del mapa principal** (para que gane).
2. **La conversión del trampolín corrompía el salto a virtual.** `ldr r3, =start_trampoline` da ya
   la dirección VIRTUAL (el kernel está linkado en 0x80000000-based), pero el código original hacía
   `sub r3,physBase; add r3,virtBase` (conversión phys→virt) que la corrompe → `bx` a una dirección
   basura. **Fix: `bx r3` DIRECTO** sin la conversión.

## Estado y lo que queda

**XNU ejecuta en modo virtual con la MMU activada.** Tras `start_trampoline` sigue hacia
`fix_boot_args`, vectores de excepción y la llamada a `arm_init` (C). Se cuelga en algún punto
posterior (la 4ª prueba, en start_trampoline, es la última que se ve). Siguiente:
- Averiguar el **formato de píxel real del framebuffer** (los colores salen mal → la consola de XNU
  también saldría mal). Probable RGB565 (16bpp) o ABGR; testear con un patrón conocido.
- Inyectar una prueba **dentro de `arm_init` (C)** para ver si llega al código C del kernel.
- Wire-up de la consola de XNU real (initialize_screen ya tiene el fb mapeado identity por el M4).

Imagen: `~/darwin-krillin/boot-xnu-M4-virtual.img`.

*M4 0721. XNU corre virtual en el MT6582 — extraordinario para un SoC sin precedente en darwin-on-arm.*

---

# 🎉 M5 (0721): XNU EJECUTA CÓDIGO C — `arm_init()` corriendo

**Probado en HW.** Tras M4 (XNU en modo virtual), se inyectaron dos pruebas de color:
- **ROJO puro** (RGB565 `0xF800F800`) en el `_start` de `locore.s`, justo antes del `bx lr` que
  salta a `arm_init` (sp ya montado, `lr=arm_init`).
- **VERDE puro** (RGB565 `0x07E007E0`) como PRIMERA acción dentro de `arm_init()` en C
  (`osfmk/arm/arm_init.c`), escribiendo directo al fb identity-mapped por el fix de M4.

**La pantalla queda VERDE** → XNU no solo completó todo el `_start` en ensamblador, sino que
**cruzó a `arm_init()` y ejecuta código C del kernel**. La cadena completa validada en hardware:

```
__start (M3) → tabla de páginas + MMU ON + salto a modo virtual (M4) → arm_init() en C (M5)
```

## Formato de píxel confirmado: RGB565 (16bpp), NO 32bpp

Los colores "raros" de M4 (blanco/azul/blanco/amarillo) se explican por completo si el framebuffer
del LK es **RGB565 de 16 bits** (cada `str` de 32 bits pinta 2 píxeles):
`0x00FF0000`→azul, `0x0000FF00`→amarillo, `0xFFFFFFFF`→blanco. Con valores RGB565 puros
(`0xF800`=rojo, `0x07E0`=verde, `0x001F`=azul, duplicados en la palabra de 32 bits) los colores
salen limpios y predecibles. **Esto es clave para la consola**: cuando se conecte
`initialize_screen`/`vc_` de XNU habrá que declararle el fb como 16bpp RGB565 (no ARGB de 32).

## Lo que queda (M6: consola de XNU con TEXTO real)

`arm_init` corre, pero la prueba verde **sobreescribe** cualquier texto. Siguiente frontera:
- Quitar el relleno verde y dejar que `PE_early_puts("arm_init: starting up\n")` (la 1ª línea real
  de `arm_init`) escriba por el pexpert al fb → **primer texto de XNU en pantalla**.
- El pexpert `pe_mt6582.c` ya tiene consola de fb (de M1); hay que asegurar que `PE_early_puts`
  está cableado a ella y que el fb se declara **RGB565 16bpp** (hallazgo de arriba).
- Bisegar cuánto de `arm_init` corre (cpu_bootstrap → arm_vm_init → …) si se cuelga antes de la
  consola.

Imagen: `~/darwin-krillin/boot-xnu-M5-arm_init.img`.

*M5 0721. XNU ejecuta C en el MT6582. De M0(compila) a M5(código C corriendo) en una sesión.*
