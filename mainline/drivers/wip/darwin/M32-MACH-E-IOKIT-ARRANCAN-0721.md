# Darwin/XNU en el krillin — el Mach arranca entero y IOKit se pone en marcha

**Fecha:** 2026-07-21
**Punto de partida:** XNU se colgaba dentro de `arm_vm_init` (M7).
**Punto de llegada:** el núcleo de Mach arranca **completo**, cede el control a su propio
planificador, y **IOKit se inicializa** hasta el emparejamiento del platform expert.

Árbol: Pi `~/darwin-krillin/` · imágenes en `~/mainline/pkg/boot-xnu-M*.img`.

---

## 1. Resumen de lo conseguido

XNU atraviesa ahora, en hardware real:

```
arm_init → arm_vm_init (pmap_bootstrap) → kernel_early_bootstrap
        → PE_init_platform ×2 → machine_startup → kernel_bootstrap:
             vm_mem_bootstrap · vm_mem_init · sched_init · wait_queue_bootstrap
             ipc_bootstrap · ipc_init · PMAP_ACTIVATE_KERNEL · mapping_free_prime
             machine_init · clock_init · task_init · thread_init
             kernel_thread_create · load_context   ← control cedido al scheduler
        → kernel_bootstrap_thread → banner "Darwin Kernel Version 13.0.0"
        → StartIOKit → runtime C++ + OSMetaClass + motor de matching VIVOS
```

Es decir: **memoria virtual, planificador, IPC, temporizador y el runtime C++ de
IOKit funcionan.** La frontera se ha movido de "no sabemos dónde muere" a
"IOKit busca driver de plataforma".

---

## 2. El bug que desbloqueó el Mach: el tick del temporizador

`clock_init()` no volvía nunca. La causa, en `pexpert/arm/pe_mt6582.c`:

```c
    while (!clock_had_irq)      /* espera INFINITA al primer tick */
        barrier();
```

Si el IRQ del timer no llega, el arranque muere mudo. **Misma familia que el
`putc` del UART** (M8): esperas sin límite sobre hardware que puede no
responder. Regla para este puerto: *ninguna espera sobre hardware sin cota*.

Y no llegaba, por dos motivos independientes:

### 2.1 Polaridad: el bloque `sysirq` de MediaTek

El GIC de ARM sólo entiende **nivel-ALTO / flanco-ascendente**. El GPT del
MT6582 es **activo-BAJO** — el DTS de pmOS que funciona en este mismo teléfono
lo declara `IRQ_TYPE_LEVEL_LOW`. MediaTek resuelve esto con un bloque de
inversión (`mediatek,mt6582-sysirq`) en **`0x10200100`**, que nosotros no
programábamos en ningún sitio: **el GIC no podía ver el tick jamás.**

Réplica exacta de `drivers/irqchip/irq-mtk-sysirq.c`:

```c
static void mt6582_sysirq_set_low(uint32_t spi)
{
    uint32_t word = spi >> 5;
    uint32_t bit  = 1u << (spi & 0x1f);
    HwReg(gMT6582SysirqBase + word * 4) |= bit;
}
```

Para el timer, SPI 112 → **bit 16 de la palabra 3** (`0x1020010C`).

### 2.2 El GPT es un bloque COMPARTIDO

Los seis timers del GPT cuelgan de **una sola línea de interrupción**. Si el LK
dejó cualquiera con su IRQ habilitada y el estado pendiente, la línea queda
activa; al habilitar interrupciones entra de inmediato y, al ser de nivel, se
re-dispara para siempre. Hay que limpiarlos antes de tocar nada:

```c
    HwReg(gMT6582TimerBase + GPT_IRQ_EN_REG)  = 0;      /* enmascarar los 6 */
    HwReg(gMT6582TimerBase + GPT_IRQ_ACK_REG) = 0x3f;   /* reconocer todo */
```

### 2.3 Constantes, verificadas contra el DTS de pmOS

| | |
|---|---|
| Timer | `GIC_SPI 112` → hwirq 144 |
| GIC dist / cpu | `0x10211000` / `0x10212000` |
| GPT | `0x10008000` |
| sysirq | `0x10200100` |

---

## 3. La consola: el framebuffer es **RGB565 de 16 bits**

⚠️ En el proyecto estaba anotado "32bpp, stride 2176". **Es FALSO**, y ese dato
equivocado costó ~15 builds persiguiendo un "solape" que no era tipográfico:
cada palabra de 32 bits pintaba DOS píxeles reales y cada fila lógica ocupaba
DOS reales, así que todo salía al doble y las líneas de más de ~34 caracteres
derramaban su mitad derecha sobre la fila siguiente.

**Medido**, no deducido, con un patrón de calibración escrito por **offset de
byte crudo** (técnica reutilizable para cualquier framebuffer de formato
dudoso): 4 bandas de 100 filas con `u32 0x00FF0000`, `u32 0x000000FF`,
`u16 0xF800`, `u16 0x001F`.

| Resultado observado | Conclusión |
|---|---|
| 3ª banda **roja** | RGB565 (en 32bpp habría salido verde) |
| 1ª y 2ª **indistinguibles** | sólo posible en 16bpp: una da negro+azul y la otra azul+negro |
| bandas ocupan **83%** | stride 1088 bytes (con 2176 habría sido 41%) |

**Geometría real:** base `0xBF400000`, RGB565, stride **544 px = 1088 bytes**,
visible 540×960.

> **Cabo suelto:** escribir píxeles de **16 bits** mata el arranque (pantalla roja
> antes de A3) por razones aún sin identificar; con escrituras de 32 bits y
> geometría lógica declarada como **270×480** (cada píxel lógico cubre 2×2
> reales) todo funciona y nada se sale de pantalla. Es la configuración actual.

---

## 4. Otros escritores del framebuffer que hubo que callar

La traza aparecía cizallada y con texto fantasma porque **había más de una
consola pintando sobre el mismo framebuffer**:

1. **`vc_` desde `mt6582_framebuffer_init`** → gateado tras `-xnu-vc`.
2. **`vc_` desde `ml_install_interrupt_handler()`** (`osfmk/arm/machine_routines.c`)
   — este era el que se escapaba; arranca `vc_` al registrar el manejador de IRQ.
3. **`vcputc()`** → redirigido a `PE_fb_putc` para que `vc_` no toque el fb.
4. **`draw_panic_dialog()`** (`osfmk/arm/model_dep.c`) → repinta la pantalla
   ENTERA y borra el mensaje de pánico. Gateado.
5. **Residuo de caché**: las primeras líneas se pintan con el fb aún mapeado
   cacheable por el booter; esas líneas quedan sucias en la D-cache y se
   vuelcan más tarde, según la caché se llena, corrompiendo pantalla ya
   actualizada. Se resuelve con `PE_fb_cache_flush()` (DCCIMVAC a mano **antes**
   del `set_mmu_ttb`, mientras el mapeo cacheable sigue vivo).
   `flush_dcache()` está declarada en `arm/pmap.h` pero **no implementada para
   ARM** en este árbol — sólo existe la versión x86_64.

Consola actual: buffer de **texto** en DRAM (no de píxeles) + redibujado desde
él; fuente **ISO 8×16 de XNU** (`osfmk/console/iso_font.c`), **bits LSB primero**
(bit 0 = píxel izquierdo), al revés que la 8×8 — lo fija `video_console.c`
(`mask = 1`, `mask <<= 1`).

---

## 5. Cómo se lee un `panic()` en este puerto

Tres trampas, las tres resueltas:

1. **`panicstr` es la cadena de FORMATO sin expandir** (`"%s:%d Assertion failed: %s"`).
   Dice el tipo de fallo pero no el fichero, la línea ni la condición.
2. El texto formateado lo acumula `debug_putc()` en **`debug_buf[]`** — pero ese
   búfer recoge **todo** el `kdb_printf` del arranque desde el primer instante,
   así que su comienzo es el volcado de `boot_args`. Hay que buscar la
   **ÚLTIMA** aparición de `"panic("` y volcar desde ahí.
3. **`consdebug_putc()` llama a `cnputc()` Y a `PE_kputc()`**: si las dos rutas
   acaban en la consola del pexpert, cada carácter sale **doblado**.

---

## 6. Los dos pánicos de IOKit

### 6.1 `Assertion failed: gc_ops.update_color` — autoinfligido

`video_console.c:1280`, pila `_Assert → gc_update_color → gc_reset_vt100`.

Al gatear `initialize_screen()` para que `vc_` no pintara encima, `gc_ops` quedó
a NULL. Pero `PE_init_printf(vm_initialized=TRUE)` llama igualmente a
`vcattach()`, que arranca la maquinaria de `vc_` y revienta contra el aserto.
Con `vc_` desactivada por diseño, **`vcattach()` no tiene nada que hacer** →
retorno inmediato bajo `#ifdef BOARD_CONFIG_MT6582`.

### 6.2 `Unable to find driver for this platform: "mediatek,mt6582"`

`iokit/Kernel/IOPlatformExpert.cpp:1512`, pila
`IOPanicPlatform::start → IOService::startCandidate`.

**Este es el bueno**: los símbolos C++ desmangling prueban que el runtime de C++
del kernel, `OSMetaClass` y el motor de matching de IOKit están **vivos**.

La tabla de personalidades integradas (`iokit/KernelConfigTables.cpp`) declara:

```
'IOClass' = IOPanicPlatform;  'IOProviderClass' = IOPlatformExpertDevice;  'IOProbeScore' = 0;
'IOClass' = AppleARMPE;       'IONameMatch' = 'AppleARM';                  'IOProbeScore' = 1;
```

`AppleARMPE` es el platform expert genérico de ARM y **acepta cualquier
plataforma** (`probe()` devuelve `this` sin condiciones), pero sólo se le ofrece
si el nombre casa. La raíz de nuestro DeviceTree se llama `mediatek,mt6582`, no
casaba, y el único candidato restante era `IOPanicPlatform` — comodín de
puntuación 0 cuyo `start()` sólo sabe entrar en pánico.

**Fix (M32):**

```c
"     'IONameMatch'     = ('AppleARM', 'mediatek,mt6582');"
```

---

## 7. Técnica de depuración que funciona

- **Sondas `PE_early_puts("MARCA\n")`** en puntos del arranque; se lee la última
  marca en pantalla. Los `kdb_printf`/`kprintf` de XNU **no** están cableados al
  fb por defecto (se silencian salvo panic o `-xnu-verbose`).
- **`kernel_bootstrap` ya trae 24 marcadores propios** (`kernel_bootstrap_kprintf`),
  comentados en el fuente de XNU. Reactivados contra la consola = mapa completo
  del bootstrap gratis.
- **Parada de diagnóstico**: `PE_fb_refresh(); for(;;);` en el punto a
  investigar. Congela la pantalla y garantiza que nada posterior tape la traza.
  Fue lo único que dio pantallas legibles cuando había pintores compitiendo.
- **Simulador del renderizador en el Mac**: se replica `fbcon_*` en Python con
  la fuente real y se vuelca a PNG. Cazó dos bugs antes de compilar (la fuente
  ISO con los bits invertidos, y que el renderizador ya era correcto y el
  problema era el tamaño físico del texto).
- **Cada build con nombre único** (`boot-xnu-M32-iokit.img`) + banner del build
  en la primera línea de pantalla: sin eso no se distingue "el cambio no
  funcionó" de "se flasheó la imagen anterior".

---

## 8. Estado y siguiente paso

**Frontera actual:** `AppleARMPE::start()` — el platform expert genérico ya
emparejado. A partir de aquí IOKit intentará instanciar los nodos del
DeviceTree (`arm-io`, `cpu`, …) y previsiblemente aparecerán más asertos por
subsistemas que este puerto no tiene.

Cada uno se lee igual (`*** MOTIVO DEL PANIC ***`) y se resuelve igual: o se
neutraliza el camino, o se implementa el mínimo imprescindible.

**Veredicto sin adornos:** sigue siendo *research*, no un SO usable. IOKit es
C++ y cada driver se **reescribe**, no se porta; lo reutilizable es el
conocimiento del hardware que ya está documentado en este repo. Pero el kernel
de Apple arranca su Mach completo en un MT6582, que era la pregunta.

## 9. Flasheo

```bash
sudo systemctl stop ModemManager
sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py \
     wo 0x2900000 0x1400000 ~/mainline/pkg/boot-xnu-M32-iokit.img
```

`wo`, **no** `wf`; por offset, no por nombre. BROM = batería fuera/dentro + Vol− + USB.
