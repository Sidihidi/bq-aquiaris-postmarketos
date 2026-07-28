# M119→M133 — MURO ROTO: launchd EJECUTA en el teléfono físico (2026-07-27)

> Continúa de `TRASPASO-CASA-0724-M116.md` y `HALLAZGO-M119-ASTLOOP.md` (ambos en la Pi `.123`,
> `~/darwin-krillin/`). Hardware real: BQ Aquaris E4.5 (MT6582, Cortex-A7 ARMv7).

## 🏆 TL;DR

Se rompió el muro que llevaba semanas: **launchd ejecuta su `main` en el teléfono físico, dyld
completa el bind entero, y HAY PROCESO 2 (launchctl)**. Dos bugs reales encontrados y arreglados
(M120 y **M128**, el grande), más dos de rendimiento/corrección (M130, M131) y uno abierto (M132/M133).

**Estado al cerrar la sesión**: launchd y launchctl arrancan pero ambos quedan colgados en un bucle
de *instrucción indefinida* en el MISMO punto de `libsystem_c` (`_tmcomp+0x2a`), donde el binario
tiene un `ldr.w r2,[r12,#4]` perfectamente legal. **M133 está construido y pendiente de flashear**:
vuelca los bytes reales de memoria en el fallo, y eso decide entre "la página tiene contenido
incorrecto" o "la CPU rechaza una instrucción legal".

---

## ★ M128 — EL BUG GRANDE: faltaba invalidar la TLB en los fallos de USUARIO

### El bug
`osfmk/arm/trap.c`, `sleh_abort()`. Todo el `switch` que contiene el `vm_fault` **y el fix de TLB de
REALVIEW/0723** vive dentro de `if (cpsr == 0x13)` → **solo fallos en modo KERNEL**. La rama de
**USUARIO** (`cpsr == 0x10`, mucho más abajo, ~línea 985) terminaba así:

```c
code = vm_fault(map, trunc(dfar), (dfsr & 0x800) ? RW : R, ...);
if ((code != KERN_SUCCESS) && (code != KERN_ABORTED)) {
    exception_type = EXC_BAD_ACCESS;      /* fallo legítimo */
} else {
    /* Retry execution of instruction. */
    ml_set_interrupts_enabled(TRUE);
    return;                               /* ← reintenta SIN invalidar la TLB */
}
```

**El fix de 0723 se aplicó solo al kernel; la rama de usuario se quedó sin él.** Por eso los fallos
de kernel funcionaban desde entonces (toda la saga del `copyout`, M102) y los de usuario entraban en
bucle infinito.

### Mecanismo exacto
1. dyld escribe en `0x94000` (página de cabecera de launchd) → **fallo de PERMISOS** (`FS=0xF`, `WnR=1`).
2. `vm_fault` lo resuelve y `pmap_enter` deja la PTE en **RW** — medido a mano en M127:
   `L2=0x846861bb` (página pequeña, válida, `AP[2:0]=0b011` = RW para usuario).
3. Se reintenta la instrucción **con la entrada vieja de SOLO LECTURA todavía en la TLB** → vuelve a
   fallar → para siempre.

**Por qué los primeros ~50 fallos SÍ progresaban**: eran de *traducción* (`FS=0x7`), sobre páginas
sin entrada previa en la TLB — no había nada obsoleto que estorbara. El bucle arranca exactamente
con el primer fallo de **permisos**, el único caso donde la TLB tiene algo que contradice a la PTE.

### El fix
En las **dos** rutas de usuario (data abort y prefetch abort), antes del reintento:
```c
uint32_t _va = (uint32_t)(dfar /* o arm_ctx->pc */) & ~0xFFFu;
__asm__ volatile ("dsb" ::: "memory");
__asm__ volatile ("mcr p15, 0, %0, c8, c7, 1" :: "r"(_va) : "memory"); /* TLBIMVA */
__asm__ volatile ("mcr p15, 0, %0, c8, c7, 0" :: "r"(0)  : "memory");  /* TLBIALL */
__asm__ volatile ("dsb\n\tisb" ::: "memory");
```
`TLBIALL` es **imprescindible**: `TLBIMVA` casa por ASID y aquí pasamos la VA con ASID 0 mientras el
proceso corre con otro (`CTXID=2` medido en M127).

### Resultado medido en HW
- Tormenta de fallos parada: `NF`/`NFR` congelados (antes +63 por tick, **todos en la misma página**).
- dyld completa **bind + lazy bind + inicializadores**:
  `dyld: bind: libbsm.0.dylib:0x0004A008 = libsystem_c.dylib:___stderrp`
- **launchd habla por su propio nombre**:
  `com.apple.launchd 1  *** Verbose boot, will log to /dev/console. ***`
- Syscalls reales: `getuid`, `gettimeofday`, `socket`, `getifaddrs`… (181 → 700+).
- **HAY PROCESO 2**: launchd forkea y el hijo carga libobjc/libicucore/libstdc++/CF = **launchctl**.
  launchd queda en `TH_WAIT` con continuation (esperando, sano).
- La consola de userland llega al UART **de forma nativa** con `serial=3` (el mensaje de launchd
  salió **sin** el prefijo `>>W:` de mi interceptor de `write()`).

---

## M120 — `csw_check()` era un stub hardcodeado de upstream

`osfmk/kern/sched_prim.c`. La función que decide *"¿hay que expulsar al hilo actual?"* empezaba con:

```c
ast_t csw_check(processor_t processor)
{
    return AST_PREEMPT | AST_URGENT;   /* ← STUB: SIEMPRE "expulsa" */
    ast_t result = AST_NONE;           /* (todo lo de abajo, inalcanzable) */
    ...
}
```

**Es de upstream (darwin-on-arm/xnu), no nuestro**: `sched_prim.c` conserva la fecha del import
(Jul 20 20:22, igual que los ficheros vírgenes; los parcheados en local tienen fecha posterior), y
está indentado con 4 espacios en un fichero que usa tabs.

Consecuencia: `ast_check()` (una vez por tick) hacía `ast_on(AST_PREEMPT|AST_URGENT)`
incondicionalmente y el `astloop` de `ml_set_interrupts_enabled` giraba sin fin.
**Fix**: `#define CSW_CHECK_ALWAYS_PREEMPT 0` + `#if` alrededor del return (fácil de revertir).
Fix real y necesario, aunque **no era el bloqueo de launchd**.

---

## M130 — el sondeo del UART se comía la CPU

`pexpert/arm/pe_mt6582.c`:
```c
#define MT6582_UART_SPIN_MAX 100000
int mt6582_getc(void) {
    for (spin = 0; spin < MT6582_UART_SPIN_MAX; spin++)   /* 100.000 lecturas MMIO */
        if (HwReg(...UART_LSR) & UART_LSR_DR) break;      /* esperando una tecla */
    ...
}
```
`serial_keyboard_poll()` lo llama **cada 16 ms desde un hilo a prioridad 95**; si nadie teclea (lo
normal) quema las 100.000 vueltas (~10 ms de accesos al bus APB). **Medido: el 85% de las muestras
del tick caían ahí**, y a prioridad 95 expulsaba a launchd (prioridad 31).

**Fix**: `getc` pasa a sondeo instantáneo (mirar `LSR` una vez y volver). La espera acotada solo
tiene sentido al **transmitir** (esperar `THRE`); `serial_keyboard_poll` ya llama en bucle hasta
recibir `-1` — literalmente *"get a character if there is one"*.

---

## M131 — la hora del sistema era basura

`iokit/Kernel/IOPlatformExpert.cpp`:
```c
long IOPlatformExpert::getGMTTimeOfDay(void) { return(0); }   /* AppleARMPE no lo sobreescribe */
```
El krillin no tiene RTC cableado en este port → `clock_initialize_calendar()` hace
`epoch = 0 - uptime` (`TIME_SUB`), o sea un `time_t` **negativo**. (El propio `commpage.c` del port
avisa: *"Time is still broken though, willfix"*, con `TimeBaseTicks_per_sec = 0`.)

**Fix provisional**: `PEGetGMTTimeOfDay()` devuelve `1785110400` (2026-07-27 00:00:00 UTC) cuando la
plataforma da 0. Cuando se porte el RTC del MT6323, esto sobra.

**Verificado en HW**: `CAL=0x6a669f94` (= 1785110420, 27-jul-2026 00:00:20) y **avanza ~1/s**
siguiendo al uptime. Instrumentación `CAL=`/`UP=` añadida al TICK (`osfmk/arm/model_dep.c`).

---

## M132 — `pmap_sync_page_data_phys()` era un stub vacío

`osfmk/arm/pmap.c`:
```c
void pmap_sync_page_data_phys(__unused ppnum_t pa)
{
    Debugger("pmap_sync_page_data_phys");   /* ← no hacía NADA */
}
```
Esa es justamente la función que XNU llama para **sincronizar cachés tras cargar una página de
código**. Además `pmap_enter_options` limpiaba la D-cache (`arm_dcache_wbinv_all`) pero **nunca
invalidaba la I-cache**, y `invalidate_icache` no existe en todo `osfmk/arm`.

**Fix**: `ICIALLU` + `BPIALL` + `dsb/isb` tras escribir la PTE en `pmap_enter_options`, y las dos
funciones `pmap_sync_page_*_phys` implementadas de verdad (`DCCMVAU` línea a línea sobre la página +
`ICIALLU` + `BPIALL`).

**Estaba mal objetivamente y el fix se queda**, pero **NO resolvió el cuelgue** (ver abajo).

---

## ⚠️ ABIERTO — bucle de instrucción indefinida en `_tmcomp+0x2a`

### Los hechos
```
INSTR ILEGAL pc=0x001a3bba cpsr=0xc0070130     ←→   USRPC=0x001a3bba
```
La dirección del fallo y la del muestreo son **la misma**: no es que "gire en `mktime`", es que
**reintenta sin fin una instrucción indefinida**. (El print está topado en 6/8, por eso solo se ven
unas pocas líneas.)

Simbolizado reconstruyendo las bases de carga a partir de las direcciones que el propio log de bind
resolvió, contra los binarios del ramdisk montado en el Mac:
```
libsystem_c → 0x108000    libsystem_info → 0x1CA000    libsystem_kernel → 0x1FC000
USRPC 0x159bba → libsystem_c + 0x51bba → _tmcomp (+0x2a)
```
Y en el binario, esa dirección contiene:
```
00051bba   f8dc2004   ldr.w  r2, [r12, #0x4]
```
Un **`ldr` normal de Thumb-2**, que el Cortex-A7 ejecuta sin problema. `cpsr` confirma modo usuario
Thumb (bit T=1), sin bloque IT activo.

Tras M132 quedan **dos** direcciones colgadas, `0x1a2bba` y `0x1a3bba`, separadas exactamente una
página y con el **mismo offset `0xbba`**: son **los dos procesos** (launchd y launchctl), cada uno
con su desplazamiento de carga, parados **en el mismo punto del código**. Esa reproducibilidad
**descarta basura aleatoria de I-cache** (si lo fuera, cada proceso fallaría en un sitio distinto).

### M133 — la medición que decide (CONSTRUIDA, PENDIENTE DE FLASHEAR)
`boot-xnu-M133-dumpinstr.img`, md5 `0b61de56eaadccb85c09e44ee150fea2`.
En `sleh_undef` hace `copyin` de la memoria del proceso y vuelca las tres palabras alrededor del PC:
```
INSTR ILEGAL pc=… cpsr=… mem[-4]=… [0]=… [+4]=…
```
Dos ramas que no se solapan:
- **`[0]` ≠ `f8dc2004`** → el contenido de la página es **incorrecto**: la instrucción es inocente y
  el fallo está en la paginación (qué página se trae y con qué contenido). Mirar el pager del ramdisk.
- **`[0]` = `f8dc2004`** → la memoria es correcta y **la CPU rechaza una instrucción legal** → no es
  un problema de datos sino de estado del procesador: enrutado del vector de excepciones o CP15.
  (Encajaría con que sea siempre el mismo punto del código.)

### Pistas adicionales para retomar
- `sleh_undef` hace *lazy enable* de VFP y **reintenta** para cualquier instrucción que no sea el
  trap de dyld (`0xe7ffdefe`). Si la instrucción no es VFP, el reintento es un bucle infinito por
  construcción → conviene revisar ese camino y que entregue `EXC_BAD_INSTRUCTION` cuando toque.
- Ojo con el **PC guardado**: en un undef, `LR_und` apunta a la instrucción *siguiente* (+2 Thumb /
  +4 ARM). Si el handler no ajusta bien, `state->pc` puede estar desplazado 2 ó 4 bytes respecto a
  la instrucción culpable — verificarlo antes de sacar conclusiones del volcado.

---

## 🔬 Método (cadena M119→M128, cada paso refutando al anterior)

Merece la pena conservarlo porque el camino fue un embudo de hipótesis descartadas:

| Paso | Qué midió | Qué refutó/estableció |
|---|---|---|
| M119 (casa) | muestreo de PC en el tick | todo en `enable_interrupts` (`astloop`) |
| M120 | lógica de `csw_check` | stub de upstream → fix, pero **no era el bloqueo** |
| M121 | traps mach **a la entrada** (hook en `traps_lo.s` antes del `bx r1`) | el último `mach_msg` **sí retornaba** → no bloqueado ahí |
| M122 | `gLaunchdThread` volcado en el tick | `st=0x04` = **TH_RUN** → no dormido, **corriendo** |
| M123 | ramas de `csw_check` + `ast_taken` | el AST se limpia y hay context switch real → planificador **sano** |
| M124 | quién corre + `lr` | `CUR == launchd` y `lr` constante → **`sleh_abort`** |
| M125 | detalle de cada fallo | páginas **distintas** (demand paging normal) → aún no era bucle |
| M126 | contadores `NF`/`NFR` | en régimen permanente crecen **igual** → **sí hay bucle, en UNA página** |
| M127 | caminata de tabla a mano | `fsr=0x80f` (permisos) pero `L2` con AP=RW → software y hardware **discrepan** |
| M128 | por qué no imprimía `VF` | el print estaba dentro de `if (cpsr == 0x13)` → **la rama de usuario no invalidaba la TLB**. **FIX** |

**Lección de herramientas**: los símbolos del `mach_kernel` hay que resolverlos **en el Mac**
(`nm -n`, `otool -tV -arch armv7`): es **Mach-O** y los binutils `arm-none-eabi` **no lo leen**
(`file format not recognized`). Lo mismo para simbolizar userland: montar el ramdisk en el Mac
(`hdiutil attach`) y usar `nm`/`otool` sobre las dylibs.

---

## 🧰 Artefactos (Pi `.123`)

| Qué | Ruta |
|---|---|
| **Imagen pendiente de probar** | `~/mainline/pkg/boot-xnu-M133-dumpinstr.img` (md5 `0b61de56…`) |
| Imágenes de la cadena | `~/mainline/pkg/boot-xnu-M12{0,1,2,3,4,5,6,7,8,9}-*.img`, `M13{0,1,2}-*.img` |
| Ramdisk 12 MB (cierre completo) | `~/darwin-krillin/ramdisk/ramdisk-shell12.dmg` (md5 `219fb559…`) |
| Scripts de build | `~/darwin-krillin/wip/darwin/build-m1{19..133}.sh` |
| Parches aplicados (python) | `~/darwin-krillin/wip/darwin/patch_*.py` |
| Doc de la cadena | `~/darwin-krillin/HALLAZGO-M119-ASTLOOP.md` |
| Traspaso anterior | `~/darwin-krillin/TRASPASO-CASA-0724-M116.md` |

### Instrumentación viva (LIMPIAR cuando arranque a shell)
- `bsd/dev/arm/unix_syscalls.c` — trace de syscalls **apagado** (`if (0 && ...)`, poner a 1 para reactivar).
- `bsd/kern/kern_exec.c` — `gMachTrace` apagado; entorno de launchd sin `DYLD_PRINT_*`;
  captura de `gLaunchdThread` en LI1.
- `osfmk/arm/model_dep.c` — `dump_sched_state()` / `dump_launchd_thread()` (TICK: MAT/CUR/pri/NF/NFR/CAL/UP).
- `osfmk/arm/trap.c` — `FLT`/`VF`/`PGW` (gated por `gMachTrace`), contadores de fallos, volcado de bytes en `sleh_undef`.
- `osfmk/kern/ast.c`, `osfmk/kern/sched_prim.c` — trazas `AT`/`CSW` (gated).
- `osfmk/arm/traps_lo.s` — hook `mach_trap_enter_trace` antes del `bx r1`.

### Receta de build y GOTCHAS
Ver `TRASPASO-CASA-0724-M116.md`. Los que más cuestan si se olvidan:
- **Swap de `.config`**: MT6582 y realview comparten `.config` → `cp .config.mt6582 .config` antes,
  restaurar realview después.
- **El initrd necesita cabecera MTK ROOTFS** (`mtk_hdr.py ROOTFS`) o el LK lo rechaza.
- **Presupuesto 20 MB** (`bootsize=0x1400000`): kernel ~7 MB + ramdisk 12 MB + initrd mínimo (35 B).
- **physBase 0x82000000**: con 12 MB de ramdisk, `.xmdt` cae en `0x812dbdc0`, por debajo de physBase.

---

## ▶️ Siguientes pasos

1. **Flashear M133** y leer `mem[0]` en el UART → decide la rama (contenido de página vs CPU).
2. Según eso: mirar el pager del ramdisk, o el enrutado de excepciones / `sleh_undef`.
3. Cuando arranque: getty → `/etc/rc.boot` (*"Welcome to Darwin/BSD on ARM platforms"*) → login → **shell**.
4. Limpiar toda la instrumentación y dejar un build limpio.
5. (Opcional) Portar el RTC del MT6323 para quitar la fecha fija de M131.

### Recordatorios operativos
- Flasheo por BROM (batería fuera/dentro + Vol− + USB). **Darwin NO carga la batería**; cargar aparte;
  el cable rojo del USB aislado; el LK corta si VBAT < 3450 mV.
- Lectura UART: `sudo ~/uart-capture.sh` (reset limpio) → `/tmp/uart.log`, 921600 8N1 en `/dev/ttyAMA0`.
- Pi `.123` = árbol del kernel/build. Pi `.38` = casa (QEMU realview).
