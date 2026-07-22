# Darwin/XNU en el krillin — estado y hallazgos

**Última actualización:** 2026-07-22
**Máquina:** Pi de build `.123` · árbol `~/darwin-krillin/` · imágenes `~/mainline/pkg/boot-xnu-M*.img`

---

## 1. Estado actual en una frase

XNU arranca **Mach + IOKit + BSD completos**, monta una **raíz HFS+** desde disco en
memoria, y llega a la línea inmediatamente anterior al `exec` de `/sbin/launchd`.

Última traza buena (M65, por UART):

```
attempting to start init of /sbin/launchd     <- ULTIMA LINEA
```

**Sin ningún fallo de traducción.** El bucle infinito que nos bloqueó once builds
está resuelto (ver §3).

---

## 2. CONSOLA SERIE POR UART — funciona, y lo cambia todo

**El krillin SÍ expone UART0 por el micro-USB.** Confirmado en hardware.

### Cable

Cable USB sacrificado, código de colores estándar:

| Cable | Señal | Va a (Raspberry Pi) |
|---|---|---|
| **Verde** | D+ = TX del móvil | **RX**, GPIO15 (pin 10) |
| **Blanco** | D− = RX del móvil | **TX**, GPIO14 (pin 8) |
| **Negro** | GND | GND (pin 6/9/14…) |
| **Rojo** | VBUS +5 V | **NADA — aislar** |

Si no sale nada, **intercambiar verde y blanco**; el mapeo varía entre placas.

### Pi 5: dos trampas

1. **`/dev/serial0` NO son los pines GPIO.** En Pi 5 apunta al conector de
   depuración de 3 pines (`ttyAMA10`). Los GPIO14/15 son **`/dev/ttyAMA0`**.
2. Hace falta `enable_uart=1` en `/boot/firmware/config.txt` + reinicio.
   *(Ya aplicado en esta Pi, línea 54.)*

### Captura — UN SOLO lector

⚠️ **Dos procesos leyendo el mismo tty se reparten los bytes al azar** y
corrompen la captura. Nos pasó y costó dos arranques.

```sh
sudo pkill -x cat; sudo fuser -k /dev/ttyAMA0; sleep 1
sudo stty -F /dev/ttyAMA0 921600 raw -echo -crtscts
sudo rm -f /tmp/uart.log; sudo touch /tmp/uart.log; sudo chmod 666 /tmp/uart.log
sudo setsid sh -c "exec cat /dev/ttyAMA0 >> /tmp/uart.log 2>/dev/null" </dev/null >/dev/null 2>&1 &
```

Leer con `strings /tmp/uart.log`. Los primeros bytes suelen ser basura: es la
captura enganchando a mitad de carácter, no un problema de velocidad.

Se captura **preloader + LK + GenericBooter + XNU** enteros. 921600 8N1.

### Lado XNU

- `fbcon_uart_putc()` en `pe_bringup.c` escribe la dirección **física**
  `0x11002000` (THR=+0x00, LSR=+0x14, THRE=bit5), con espera **acotada**.
- Se engancha en `fbcon_putchar()`, el embudo único: todo lo que va a pantalla
  sale también por serie.
- ⚠️ **Se habilita con `PE_uart_enable()` desde `arm_vm_init`, tras el cambio de
  tablas.** Antes de eso el MMIO no está mapeado y el primer carácter mata el
  arranque (nos pasó en M64: pantalla roja, cero salida).
- `arm_vm_init` añade un **identity-map de sección para `0x11000000`**, junto al
  del framebuffer.

---

## 3. EL BUG GORDO, RESUELTO: alineación de la L1 de usuario

**Síntoma:** al ejecutar `/sbin/launchd`, `copyout()` del `argv` a la página de
usuario `0x1000` fallaba en bucle infinito:

```
FALLO datos pc=0x800e2xxx far=0x00001000 fsr=0x00000805 cpsr=0x200001d3
```

`fsr=0x805` = escritura + **fallo de traducción de SECCIÓN** (descriptor L1
inválido). Pero el descriptor era válido y estaba donde tocaba.

**Causa:** `pmap_create()` (`osfmk/arm/pmap.c`) reservaba la L1 con
`pmap_grab_page()` — una página suelta, alineada sólo a **4 KB**. El hardware
toma la base de TTBR0 con alineación de **16 KB**: con la tabla en
`0x82f92000`, la MMU caminaba desde `0x82f90000`, 8 KB más abajo, donde no hay
nada. Escribíamos descriptores impecables que el hardware no veía jamás.

Era **lotería**: sólo habría funcionado si la página caía por casualidad en un
múltiplo de 16 KB.

**Fix (M62):**

```c
l1ret = cpm_allocate(0x4000, &new_l1, 0, 3, FALSE, KMA_LOMEM);
our_pmap->pm_l1_phys = new_l1->phys_page << PAGE_SHIFT;
our_pmap->pm_l1_virt = phys_to_virt(our_pmap->pm_l1_phys);
bzero((void *) our_pmap->pm_l1_virt, 0x4000);
our_pmap->pm_l1_size = 0x4000;
if (our_pmap->pm_l1_phys & 0x3FFF) panic(...);
```

`cpm_allocate(size, &pages, max_pnum, pnum_mask, wire, flags)` — el 4º parámetro
es la **máscara de alineación en páginas**: 3 = 4 páginas = 16 KB.
Con `pm_l1_size = 0x4000` (máximo), `pmap_expand_ttb()` no vuelve a mover la
tabla a una dirección desalineada.

**Verificado:** cero `FALLO datos` en todo el arranque de M65.

### Árbol de descartes (todo medido, no supuesto)

Antes de dar con la alineación se eliminaron, con datos en pantalla:

| Hipótesis | Cómo se descartó |
|---|---|
| Falla `launchd` | `cpsr` bits de modo = `0x13` → **kernel**, no usuario |
| Coherencia de caché | `DCCMVAC` sobre TTE y PTE: sin cambio |
| TLB / ASID | `TLBIALL` + `BPIALL` completos: sin cambio |
| `TTBCR.PD0` | `TTBCR=0x2` → caminatas habilitadas |
| Dominios | `DACR=0x1` → dominio 0 en *client* |
| Access Flag | `SCTLR=0x00c5187d` → **AFE=0**, permisos clásicos |
| `ACTLR.SMP` | Activarlo **cuelga la CPU** tras `_start` (SCU sin inicializar). Camino cerrado. |
| pmap equivocado | `pm_l1_phys` == base de TTBR0 |
| `phys_to_virt` roto | Lectura por ventana de sección creada a mano: mismo valor |
| Desfase `sectionOffset` | Vale **0** en ejecución (el `0x1000` de la línea 67 es sólo el inicializador) |
| Tablas mal formadas | L1 `0x82fc6001` (tabla) y PTE `0x82fc51bb` (página, AP=11 RW) — correctas |

---

## 4. Otros hallazgos que costaron tiempo

### Framebuffer: es **RGB565 16bpp**, no 32bpp

Estaba anotado como 32bpp/stride 2176 y **es falso**. Costó ~15 builds de
"texto solapado" que no era tipográfico: cada palabra de 32 bits pintaba DOS
píxeles reales y cada fila lógica ocupaba DOS reales.

**Medido** con un patrón de calibración por **offset de byte crudo** (técnica
reutilizable para cualquier framebuffer de formato dudoso): 4 bandas de 100
filas con `u32 0x00FF0000`, `u32 0x000000FF`, `u16 0xF800`, `u16 0x001F`.
Resultado: *azul, azul idéntico, ROJO, azul claro*, ocupando el **83%** de la
pantalla. Que las dos primeras salgan **indistinguibles** sólo es posible en
16bpp.

Geometría real: base `0xBF400000`, RGB565, stride **544 px = 1088 bytes**,
visible 540×960.

⚠️ **Cabo suelto:** escribir píxeles de **16 bits** mata el arranque por razones
sin identificar. Se trabaja con escrituras de 32 bits y geometría lógica
declarada **270×480** (cada píxel lógico cubre 2×2 reales). Funciona.

### El tick del temporizador

`mt6582_timebase_init` acababa en `while (!clock_had_irq) barrier();` — espera
infinita. Dos causas:

1. **Polaridad `sysirq` de MediaTek.** El GPT es activo-BAJO
   (`IRQ_TYPE_LEVEL_LOW` en el DTS de pmOS) pero el GIC sólo entiende
   nivel-ALTO. MTK intercala un bloque de inversión en **`0x10200100`**:
   bit `(spi & 31)` de la palabra `(spi >> 5)`. Timer = SPI 112 → bit 16,
   palabra 3. Réplica de `drivers/irqchip/irq-mtk-sysirq.c`.
2. **El GPT es un bloque COMPARTIDO** (6 timers, una línea IRQ). Hay que
   enmascarar los 6 (`GPT_IRQ_EN_REG = 0`) y reconocer lo pendiente
   (`GPT_IRQ_ACK_REG = 0x3f`) antes de habilitar interrupciones.

### Base de tiempo no monótona

`mt6582_get_timebase()` reconstruía el tiempo desde **GPT1** (el one-shot del
planificador) con `clock_absolute_time + (decrementer - contador)`. Como GPT1
cuenta hacia arriba, el tiempo **retrocedía** entre ticks → `microuptime()` no
progresaba → `prngForceReseed()` (en `random_init`) giraba eternamente.

**Fix:** leer **GPT2**, que ya estaba en free-run para eso, con la parte alta a
mano (32 bits a 13 MHz dan la vuelta cada ~5,5 min).

> El `System uptime in nanoseconds: 0` del informe de pánico era ese síntoma
> exacto, y lo tuvimos delante un día entero.

### Regla del port: **ninguna espera sobre hardware sin cota**

Nos mordió tres veces: el `putc` del UART, el tick del timer, y el reloj del
sistema. Cualquier `while` esperando a un registro debe llevar límite.

### Otros escritores del framebuffer que hubo que callar

1. `vc_` desde `mt6582_framebuffer_init` → gateado tras `-xnu-vc`.
2. `vc_` desde **`ml_install_interrupt_handler()`** (`osfmk/arm/machine_routines.c`)
   — el que se escapaba.
3. `vcputc()` → redirigido a `PE_fb_putc`.
4. **`draw_panic_dialog()`** (`osfmk/arm/model_dep.c`) → repinta la pantalla
   ENTERA y borra el mensaje de pánico. Gateado.
5. **`vcattach()`** (`osfmk/console/video_console.c`) → con `vc_` off, `gc_ops`
   queda a NULL y revienta en `gc_reset_vt100`. Retorno inmediato.
6. **Residuo de D-cache**: las primeras líneas se pintan con el fb aún cacheable
   → `PE_fb_cache_flush()` (DCCIMVAC a mano; `flush_dcache()` está declarada en
   `arm/pmap.h` pero **no implementada para ARM**).

### Cómo se lee un `panic()`

1. **`panicstr` es la cadena de FORMATO sin expandir** (`"%s:%d Assertion failed: %s"`).
2. El texto formateado está en **`debug_buf[]`**, pero ese búfer acumula TODO el
   `kdb_printf` del arranque → buscar la **ÚLTIMA** aparición de `"panic("`.
3. **`consdebug_putc()` llama a `cnputc()` Y a `PE_kputc()`**: si ambas acaban en
   la consola, cada carácter sale **doblado**.

### Resolver un PC a símbolo

El `mach_kernel-mt6582` empaquetado está **stripped** — `nm` da 0 y engaña
(así concluimos erróneamente que HFS no estaba compilado). Usar el binario sin
strippear y el `nm` **de la toolchain Darwin** (el de GNU no lee Mach-O):

```sh
/opt/Developer/Toolchains/DarwinARM.toolchain/usr/bin/nm -n \
  ~/darwin-krillin/xnu/BUILD/obj/DEBUG_ARM_MT6582/mach_kernel
```

### IOKit

- `AppleARMPE` es el platform expert genérico y acepta cualquier plataforma
  (`probe()` devuelve `this`), pero sólo se le ofrece si el nombre casa.
  En `iokit/KernelConfigTables.cpp`:
  `'IONameMatch' = ('AppleARM', 'mediatek,mt6582');`
- Sin eso caía en `IOPanicPlatform` (IOProbeScore 0, comodín que sólo sabe
  entrar en pánico).

### Raíz y ramdisk

- Faltaba **`rd=md0`** en la cmdline del MT6582 en
  `GenericBooter/init/main.c` — las otras plataformas ya lo llevaban. Sin él,
  `IOFindBSDRoot()` espera un `IOMedia` con `Content=Apple_HFS` que no puede
  existir (no hay driver de almacenamiento).
- **HFS SÍ está compilado.** La traza lo demuestra (`hfs_mountfs`,
  `hfs_mounthfsplus`).
- El ramdisk con userland Darwin/ARM estaba en `~/darwin-krillin/ramdisk/`
  (clon de `github.com/darwin-on-arm/ramdisk`) y **nunca se había embebido**.
  21 MB usados no caben con el kernel en 20 MB de partición → recortado a 6 MB:

```sh
sudo apt-get install -y hfsprogs
dd if=/dev/zero of=ramdisk-min.dmg bs=1M count=6
/usr/sbin/mkfs.hfsplus -v DarwinRD ramdisk-min.dmg
# montar original RO + nuevo RW y copiar:
#   SE QUEDA: sbin/ etc/ var/ tmp/ dev/ bin/sh
#             usr/lib/dyld, usr/lib/libSystem.B.dylib, usr/lib/system/
#             resto de usr/lib/*.dylib, System/Library/LaunchDaemons
#   FUERA:    System/Library, usr/local, libicucore, libobjc,
#             libiconv, libstdc++, libncurses
sudo /usr/sbin/fsck.hfsplus -f -y ramdisk-min.dmg     # dejarlo LIMPIO
image3maker -f ramdisk-min.dmg -t rdsk -o GenericBooter/images/Ramdisk.img3
```

---

## 5. La batería es un recurso escaso

**Darwin NO carga.** El cargador es un **FAN5405** por I²C y XNU no tiene driver
(sólo `AppleARMSoftIICController`, un esqueleto vacío). El chip tiene watchdog de
~32 s: sin refresco por I²C deja de cargar.

Peor: el LK imprime `Bypass Kernel Power off charging mode` — **se salta el modo
de carga con el móvil apagado** y arranca Darwin, que consume.

Corte del LK:

```
is_low_battery, TRUE
[BATTERY] battery voltage(3445mV) <= CLV ! Can not Boot Linux Kernel !!
mt_power_off : check charger        (en bucle)
```

**Cargar con cargador externo de batería** (es extraíble y ya se saca para el
BROM). Alternativa: flashear pmOS, cargar, reflashear Darwin.

> Si el móvil deja de arrancar de repente, **mirar la tensión en el log del UART
> antes de sospechar del último build.**

---

## 6. Frontera actual y siguiente paso

Última línea: `attempting to start init of /sbin/launchd`
(`bsd/kern/kern_exec.c:3513`, dentro de `load_init_program`).

Orden real de la función:

```
vm_allocate → kprintf("attempting…")   <- ULTIMA LINEA VISTA
  → copyout(nombre)      [LP 1]
  → copyout("-s")        [LP 2]        (la cmdline lleva -s)
  → copyout(argv)        [LP 3]
  → set_security_token   [LP 4]
  → LI0 → execve → LI1
```

**M66** instrumenta ese tramo con los cuatro marcadores `LP`. El último que
aparezca señala la llamada que no retorna. Sospechosa principal:
`set_security_token()`, que es lo único no trivial entre el último `copyout` y
`LI0`.

**M67** (imagen de diagnóstico) añade, aprovechando que el UART no tiene límite
de pantalla:
- `fbcon_quiet = 0` → **kprintf verboso de XNU desde el primer instante**
- traza de syscalls hasta 60 (antes 14) y **sin congelar** la consola
- marcador de fin de pánico

---

## 7. Flasheo

```sh
sudo systemctl stop ModemManager
sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py \
     wo 0x2900000 0x1400000 ~/mainline/pkg/boot-xnu-M67-diag.img
```

`wo`, **no** `wf`; por offset, no por nombre. BROM = batería fuera/dentro +
Vol− + USB.

> El preloader dice `part BOOTIMG 0x3180000 0x1400000` y nosotros flasheamos en
> `0x2900000`. El tamaño coincide, el offset no — funciona (readback+md5 lo
> demostró), pero merece una comprobación tranquila.

---

## 8. Ideas descartadas, para no repetirlas

- **Portar el driver USB de Linux a IOKit**: no se porta, se **reescribe**.
  IOKit es C++ con otro modelo, y `IOUSBFamily` **no está en el árbol de XNU**.
  Sería una pila USB de dispositivo desde cero. El UART por USB da lo mismo por
  céntimos.
- **Montar la raíz desde la SD**: exige un driver de almacenamiento para IOKit.
  El ramdisk lo esquiva precisamente por eso.
- **`ACTLR.SMP`**: cuelga la CPU. Cerrado.
