# XNU en el krillin (MT6582): el Mach arranca entero, IOKit de plataforma vivo, y bsd_init hasta la red — M32→M42 (0721-0722)

> **Continuación de** `M3-XNU-ARRANCA-0721.md` (M3=ejecuta, M4=MMU/virtual, M5=código C,
> M6=consola de texto). Esta tanda lleva XNU **muchísimo más lejos**: el núcleo Mach completo,
> IOKit emparejando el platform expert **y el driver de CPU**, y `bsd_init` recorriendo ~30
> subsistemas hasta el stack de red. Frontera actual = `bsd_autoconf`.
>
> Patch completo: `xnu-mt6582-mach-iokit-bsd-M42.patch` (git diff sobre el árbol darwin-on-arm/xnu,
> 33 ficheros). Fuentes clave sueltas en `pexpert/`. Imagen: Pi `~/mainline/pkg/boot-xnu-M42-skipnet.img`.

## 1. Qué arranca ahora, en hardware real

```
arm_init → arm_vm_init (pmap real) → kernel_early_bootstrap → PE_init_platform ×2
  → machine_startup → kernel_bootstrap:
       vm_mem_bootstrap · sched_init · ipc_bootstrap · clock_init · thread_init
       kernel_thread_create · load_context   ← control cedido al scheduler
  → kernel_bootstrap_thread → banner "Darwin Kernel Version 13.0.0"
  → StartIOKit → runtime C++ + OSMetaClass + motor de matching
       → AppleARMPE::start  (platform expert genérico ARM)   ✅ arranca entero
       → AppleARMCPU::start (interrupt controller + ml_processor_register + processor_start) ✅
  → bsd_init: kmeminit · vfsinit · devsw_init · mbinit · ubc_init · IOKitInitializeTime
              · ... (~30 subsistemas) ... · [red saltada] · bsd_autoconf ← FRONTERA
```

**Memoria virtual, planificador, IPC, timer, runtime C++ de IOKit, emparejamiento de drivers,
el platform expert y el driver de CPU: TODO funciona.** El kernel de Apple inicializa su capa BSD
en un MediaTek MT6582 de 2014.

## 2. Los muros que se tiraron (uno por uno) y cómo

### M32 — IOKit no encontraba platform expert (`IOPanicPlatform`)
`iokit/KernelConfigTables.cpp`: `AppleARMPE` (platform expert genérico, `probe()` acepta cualquier
plataforma) solo se ofrecía si el `IONameMatch` casaba. La raíz de nuestro DeviceTree se llama
`mediatek,mt6582`, no casaba, y el único candidato restante era `IOPanicPlatform` (comodín score 0
cuyo `start()` solo entra en pánico). **Fix:** `'IONameMatch' = ('AppleARM', 'mediatek,mt6582');`.

### AppleARMPE + AppleARMCPU arrancan (M33-M34)
Con el match arreglado, `AppleARMPE::start` corre entero (`super::start`→`createNubs` del DT,
`registerService`, `populate_model_name` desde `model`). Luego IOKit empareja el nub `cpu` →
`AppleARMCPU::start`, que hace lo pesado: crea el controlador de interrupciones dummy
(`AppleARMGrandCentral`), `initCPU`, `ml_processor_register` (sobrevive al `start_paddr=0x100`
hardcoded) y `processor_start`. **Ambos completan.** Verificado con sondas `PE_early_puts`
(IOK8-11, CPU1-6).

### El "cuelgue tras DSM9" era un ESPEJISMO — lección de depuración
Instrumentando `IOService::probeCandidates`/`doServiceMatch` (M37) el matching del `cpu` completaba
limpio (`DSM9` = `_adjustBusy(-1)` incluido). Se probó neutralizar `processor_start` (M38) sin
efecto. **El error de método:** se perseguía el *config thread* (que hace el matching y termina
bien) mientras el **hilo principal ya había avanzado a `bsd_init`** — sus marcadores salían pero se
mezclaban en consola con los del config thread, y al leer solo "la última línea" parecía un cuelgue
en `DSM9`. Con dos hilos escribiendo el framebuffer sin lock, el orden visual engaña.
**Regla:** cuando dos hilos escriben la consola, silenciar el ruido de uno para leer el otro.
El `registerService` del root en `StartIOKit` **sí** tiene una rama síncrona que hace `thread_block`
esperando a que IOKit quede *quiet* (busy→0) — real, pero no era el bloqueo aquí.

### M41 — `IOKitInitializeTime` cuelga esperando el `IORTC`
`IOStartIOKit.cpp`: `IOService::waitForService(resourceMatching("IORTC"), &t)` con timeout de 2s.
El port no tiene driver de RTC, y el timeout necesita el calendario que se inicializa AQUÍ mismo
(huevo/gallina) → espera infinita. **Fix:** saltar el `waitForService(IORTC)`; el calendario arranca
en epoch 0, irrelevante para el boot.

### M42 — `dlil_init` cuelga; se salta el stack de red entero
El último subsistema alcanzado era `dlil_init` (base de red): crea el hilo de input con
`ml_thread_policy` + affinity de *processor set*, frágil en el bring-up de 1 CPU. Como la red **no
hace falta** para montar un root local, y `loopattach`/`ether_family_init`/`net_init_run` dependen de
`dlil`, se saltó el bloque entero (`dlil_init`, `proto_kpi_init`, `socketinit`, `domaininit`,
`loopattach`, `ether_family_init`, `net_init_run`, `utun/netsrc/nstat`). `bsd_init` avanzó hasta
`bsd_autoconf`.

## 3. La consola de framebuffer (pieza de infraestructura)

`PE_early_puts` original usa **semihosting** (debugger JTAG) → sin depurador no imprime. Re-cableada
a una consola de fb directa en `pe_bringup.c` (`pe_bringup-fbconsole.c`), portada del renderizador ya
probado del GenericBooter: fb físico `0xBF400000` (identity-mapped por el fix de M4), **32bpp
XRGB8888** (¡NO RGB565 como se anotó un rato — el texto blanco/negro se ve igual en cualquier orden
de canal, lo confirmó el propio renderizado!), fuente 8×8 escalada 2×. Los ~24 marcadores nativos de
`kernel_bootstrap_thread` y los 56 de `bsd_init` se reactivaron apuntando `*_kprintf` → `PE_early_puts`
= mapa del arranque gratis. **Técnica estrella sin UART:** sondas `PE_early_puts("MARCA\n")`; se lee
la última marca en pantalla.

## 4. Frontera actual y cómo reanudar

**`bsd_autoconf()`** (`bsd/kern/bsd_init.c:1028`): `kminit` → bucle `pseudo_inits` → `IOKitBSDInit()`.
Cuelga en uno de esos (sospechoso: un pseudo-device de red huérfano por el salto de M42, o
`IOKitBSDInit`). Siguiente paso: sondas `PE_early_puts` dentro de `bsd_autoconf` para localizar, luego
neutralizar. Tras él quedan `inittodr` (lee RTC otra vez), `setconf`/`IOFindBSDRoot` (busca el disco
root) y el bucle `mountroot`.

**El techo documentado está a 2-4 ciclos:** `mountroot` → panic **`"no root device"`** (no hay driver
de disco ni rootfs/userland Darwin-ARM). Ese panic es el **final honesto del proyecto**: el kernel de
Apple arranca ENTERO y solo se detiene por no tener un sistema de ficheros/userland Darwin para ARM.

**Reanudar:** aplicar el árbol XNU tal cual está en la Pi (commit local `3c7027d`), o el patch. Build
`~/darwin-krillin/build-xnu.sh`, empaquetar `~/darwin-krillin/package-xnu.sh`, flashear por BROM
(`mtk.py wo 0x2900000 0x1400000 boot-xnu-*.img`; fastboot NO funciona en este estado).

## 5. Veredicto sin adornos

Sigue siendo **research, no un SO usable**: IOKit es C++ y cada driver se **reescribe**, no se porta;
y aunque se llegue a `mountroot`, no existe userland Darwin-ARM que ejecutar. Lo reutilizable es el
conocimiento del hardware, ya documentado en este repo. **Pero el kernel de Apple arranca su Mach
completo, levanta IOKit y su platform expert, y corre la init de BSD en un MT6582** — territorio que,
que sepamos, nadie había pisado en darwin-on-arm para este SoC.

*0721-0722. Continúa M3-XNU-ARRANCA-0721.md. Sesión Windows, coordinada con la del Mac (que llevó
M1→M32).*
