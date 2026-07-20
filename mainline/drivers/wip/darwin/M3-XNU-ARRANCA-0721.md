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
