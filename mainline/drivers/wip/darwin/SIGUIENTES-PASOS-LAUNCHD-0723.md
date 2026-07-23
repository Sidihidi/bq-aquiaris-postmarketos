# XNU krillin — el muro de launchd: por dónde vamos y siguientes pasos (0723)

> **Continúa** `ESTADO-Y-HALLAZGOS-0722.md`. XNU monta raíz HFS+ y llega a `attempting to start init
> of /sbin/launchd`. Esta sesión (Windows + UART) ha desmontado ese "cuelgue" capa por capa hasta
> `vm_fault`. **No resuelto**, pero el frente está localizado con precisión. Builds M65→M85.

---

## 1. Estado en una frase

El `copyout` del argv a la página de usuario `0x1000` en `load_init_program` **falla correctamente**
(página lazy), pero el **manejo del fault** (`_fleh_dataabt` → `sleh_abort` → `vm_fault`) no llega a
poblar la página. Frontera actual: **`vm_fault(thread->map, 0x1000, WRITE)`** dentro de `sleh_abort`.

---

## 2. La cadena de diagnóstico (todo medido por UART / framebuffer, no supuesto)

| # | Prueba | Resultado | Conclusión |
|---|---|---|---|
| M68/69 | `init_addr` + lectura privilegiada de 0x1000 | `init_addr=0x1000`, **cuelga** | no es copyout/strt |
| M68 | detector de IRQ (`IRQ#`) | cero | no es tormenta de IRQ |
| M70/72 | leer 0x30000000 (TTBR0) vs 0x40000000 (TTBR1) | TTBR0 cuelga, TTBR1 va | es el rango de usuario |
| M71 | volcado de registros | `VBAR=0x800e3000` (kernel), `TTBCR=2`, `TTBR0=0x82fb405b`, `TTBR1=0x81e04018` | vector alcanzable |
| M74 | volcado L1 de usuario | `L1[0]=0x00000000` (inválido); `gPB=0x81000000 gVB=0x80000000` | página lazy, sin mapear |
| M75 | `sp` del kernel en el fault | `0xd04d3edc` (≥0x80000000, TTBR1) | pila de kernel OK |
| M77 | CBAR + desenmascarar `CPSR.A` | `CBAR=0x10210000` (SCU); sigue colgando | **no** es abort async enmascarado |
| **M78** | **`ATS1CPR(0x1000)`** | **`PAR=0x0000000b`** (bit0=1, FS=0x05 = **section translation fault LIMPIO**) | **EL WALK ESTÁ BIEN.** El MMU traduce 0x1000 y da fault limpio. No es coherencia ni descriptor podrido. |
| M79 | fault en 0xF0000000 (kernel sin mapear) | **cuelga igual** | **el despacho de aborts está roto EN GENERAL** (el 1er fault del arranque cuelga, sea kernel o user) |
| M80 | volcado de la página de vectores | `[0x10 dabt]=0xe59ff018`, target `_fleh_dataabt=0x800e7ad0` | **vectores PERFECTOS** |
| M83 | marcadores fb 'A'/'D' en `_fleh_dataabt` | **pantalla BLANCA** (ambos) | **`_fleh_dataabt` SE ENTRA** + guardado de registros OK, llega a `bl _sleh_abort` |
| **M84** | marcador fb en entrada de `sleh_abort` + **desactivar print `FALLO`** | **pantalla AZUL** (`0x00FF0000`), sin reset | **`sleh_abort` SE ENTRA.** El **print del `FALLO`** (diagnóstico del Mac, `PE_early_puts`) era parte del cuelgue → desactivado, saltamos a `vm_fault`. |
| **M85** | marcador fb AMARILLO tras `vm_fault` | **PENDIENTE DE FLASHEAR** | dirá si `vm_fault` retorna (amarillo) o cuelga por dentro (azul) |

### Lo confirmado
- El walk del MMU es **correcto** (ATS1CPR). No es coherencia SMP/SCU (esa hipótesis quedó descartada).
- Los vectores y el `_fleh_dataabt` (guardado de registros) **funcionan**.
- `sleh_abort` **se entra**; su print del `FALLO` estorbaba (desactivado en M84).
- El muro está ahora en **`vm_fault(thread->map, 0x1000, WRITE)`** (`osfmk/arm/trap.c`, caso
  `SLEH_ABORT_TYPE_DATA_ABORT`, ~línea 358).

### La hipótesis del Mac (documentada en el propio `trap.c` tras el `vm_fault`)
> *"vm_fault dice KERN_SUCCESS pero la instrucción vuelve a fallar en bucle, y FS=0x5 dice que la L1
> sigue VACÍA. El software escribe la L1 por `pm_l1_virt` y la MMU camina por `pm_l1_phys`: si esas dos
> NO son la misma página física, escribimos una tabla y el hardware lee otra."*

Candidatos del Mac: (a) se falta sobre un pmap distinto del que tiene el HW en TTBR0
(`pm_l1_phys != base de TTBR0`); (b) es el pmap correcto pero `vm_fault` no llega a `pmap_enter`.

---

## 3. Siguiente paso inmediato: flashear M85

`boot-xnu-M85-vmfmark.img` (BROM). **Mirar el color de la pantalla:**
- **AZUL** → `vm_fault(0x1000)` **cuelga por dentro** → atacar `vm_fault`/`pmap_enter` (lock, o el
  page-in sobre el pmap de usuario). Instrumentar dentro de `vm_fault` / `pmap_enter`.
- **AMARILLO** → `vm_fault` **retorna** → confirmar la hipótesis del Mac: volcar (por **framebuffer**,
  que el UART/`PE_early_puts` en ese contexto no imprime fiable) `pm_l1_phys`, `TTBR0` actual, y la
  TTE releída por `pm_l1_virt` **y** por `phys_to_virt(pm_l1_phys)`. Si `pm_l1_virt != phys_to_virt
  (pm_l1_phys)` → **cazado**: arreglar `pmap_create`/`pm_l1_virt` para que apunten a la misma física.

### Después (si se resuelve el page-in)
El `copyout` completará → `LP 1`/`LP 2`/`LP 3` → `set_security_token` → `execve` de launchd (`LI0`/`LI1`).
Cada uno puede traer su propio muro (los mismos métodos: sondas, ATS1CPR, marcadores fb).

---

## 4. Cambios experimentales EN EL ÁRBOL ahora mismo (hay que decidir qué queda)

Sin commitear sobre `0c92c32`. Todo son **diagnósticos temporales**, no fixes definitivos:

- `bsd/kern/kern_exec.c` (`load_init_program`): sondas `LP0`/`REGS`/`L1DUMP`/`KSTACK`/`ATS1CPR`/`VEC`
  antes del `copyout`. Ruido; quitar cuando se resuelva.
- `osfmk/arm/traps_lo.s` (`data_abort_crash_in_kernel`): marcadores fb 'A' (mitad sup. blanca) y 'D'
  (mitad inf. blanca). Quitar al final.
- `osfmk/arm/trap.c` (`sleh_abort`):
  - marcador fb AZUL al entrar (a prueba de fault).
  - **`if (0)` en el bloque del print `FALLO`** ← esto NO es diagnóstico: el print del FALLO colgaba
    el handler; si se quiere el FALLO de vuelta, hay que arreglar su `PE_early_puts` en contexto de
    abort (o dejarlo desactivado). **Ojo: revisar por qué `PE_early_puts` cuelga ahí** (funciona en
    `load_init_program` pero no tras el abort — puede ser recursión: el print faltea → nested abort).
  - marcador fb AMARILLO tras `vm_fault` (M85).
- `osfmk/arm/trap.c` (`irq_iokit_dispatch`): detector `IRQ#` (M68). Quitar.

> **Sub-hallazgo importante:** `PE_early_puts` (consola por UART+fb) **cuelga dentro del handler de
> abort** aunque funcione en código normal. Sospecha: el print faltea (¿el `str` al UART 0x11002000?
> ¿fbcon reentrante?) → nested abort → recursión → reset (M81/M83 reseteaban; M84 con el print
> desactivado ya no). Esto merece su propia investigación: **la consola no es segura desde el handler
> de abort.** Regla nueva del port: en `sleh_abort`, marcar por **framebuffer directo**, no por
> `PE_early_puts`.

---

## 5. Procedimiento (recordatorio)

```sh
# build + empaquetar
cd ~/darwin-krillin && ./build-xnu.sh && ./package-xnu.sh
cp ~/mainline/pkg/boot-xnu.img ~/mainline/pkg/boot-xnu-MXX-nombre.img

# UART (UN solo lector). Ver en vivo:
sudo ~/uart-capture.sh
tail -n0 -f /tmp/uart.log | tr -c '[:print:]\n' '.'

# flashear por BROM (bateria fuera/dentro + Vol- + USB de datos; luego cable UART para arrancar):
sudo systemctl stop ModemManager
sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py \
     wo 0x2900000 0x1400000 ~/mainline/pkg/boot-xnu-MXX-nombre.img
```

Volcar tablas desde C: `phys_to_virt(p) = (p - gPhysBase) + gVirtBase` con
`gPhysBase=0x81000000, gVirtBase=0x80000000`.

---

## 6. ⚠️ Batería

Darwin **NO carga** (sin driver FAN5405). Cada arranque drena. Umbral del LK = **3450 mV** (por debajo:
`Can not Boot Linux Kernel`, bucle `mt_power_off: check charger`). **El cable UART NO da corriente**
(VBUS aislado). Para cargar: cargador externo de batería, o conectar un USB de datos real al móvil (el
LK carga en su modo power-off). Empezamos la sesión a 3788 mV; a ~3648 tras varios arranques. **Si el
móvil deja de arrancar de golpe, mirar VBAT en el UART antes de sospechar del build.**

*0723, sesión Windows. Frontera: `vm_fault(0x1000)` en `sleh_abort`. A 1-2 capas de que launchd
ejecute. Repo: `M65-M76-LAUNCHD-COPYOUT-WALL-0722.md` + este doc.*

---

## ADENDA 0723 (sesión Mac, lectura de M85)

**M85 flasheado → pantalla AZUL** = `vm_fault(0x1000)` **se cuelga por dentro**, no llega al marcador amarillo.

Del log de M85, confirmado:
- `l1va=0x81fb4000`, `TTBR0=82fb405b` (base `0x82fb4000`). `phys_to_virt(0x82fb4000)=0x81fb4000` = `l1va` → **`pm_l1_virt` y `pm_l1_phys` SON la misma página**. La hipótesis "escribimos una tabla y el HW lee otra" queda DEFINITIVAMENTE muerta.
- `ATS1CPR(0x1000)=PAR 0xb` (fault limpio, walk correcto). `ATS1CPR(0x80000000)=PAR 0x81000068` (kernel OK).
- `L1[0]=0` → página lazy, sin poblar. Lo esperado.

### Causa raíz localizada: IRQs deshabilitadas durante el fault

- `traps_lo.s`, `data_abort_crash_in_kernel`: llama a `sleh_abort` con **`cpsid i`** (IRQs OFF).
- El `cpsr` guardado del fallo = `0x200001d3` → **bit7 (I) = 1**: el propio `copyout` de `load_init_program` **ya corría con IRQs apagadas** (además F=1, A=1).
- `vm_fault` necesita poder BLOQUEARSE (esperar página, ceder al scheduler). Con IRQs off el timer no dispara → nada avanza → cuelga eterno. **Un page-fault se atiende SIEMPRE con IRQs habilitadas.**

### M86: prueba

`osfmk/arm/trap.c`, caso `SLEH_ABORT_TYPE_DATA_ABORT`: `__asm__ volatile("cpsie i")` justo antes de `vm_fault`.
- **AMARILLO** (vm_fault retorna) → CONFIRMADO. Siguiente: ver por qué `copyout`/`load_init_program` corre con IRQs off (mirar `bsd_utaskbootstrap`, el fork del proceso 1, y si `_fleh_dataabt` debería restaurar el estado de IRQs del contexto interrumpido en vez de forzar `cpsid`). Arreglo definitivo probablemente en `traps_lo.s`: no forzar `cpsid i`, o re-habilitar según la SPSR guardada.
- **AZUL** (sigue colgado) → no era (solo) las IRQs; instrumentar dentro de `vm_fault` con marcadores de color GATED (verde=entrada, cian=pre `vm_map_lock_read`, magenta=post lock, blanco=post `vm_map_lookup_locked`).

> Recordatorio: en `sleh_abort` marcar SOLO por framebuffer directo a `0xBF400000` (TTBR1, a prueba de fault). `PE_early_puts` cuelga en contexto de abort (nested abort → recursión).

---

## ★ CAUSA RAÍZ ENCONTRADA (0723, análisis de código + agente) — M94

**El table walker de USUARIO camina CACHEABLE+SHAREABLE, pero las tablas se escriben WRITE-THROUGH/non-shareable. Con ACTLR.SMP=0 no son coherentes: el walker cachea el descriptor rancio (L1[0]=0 del primer fault) y lo reusa para siempre.**

### La asimetría exacta
- `arm_context_switch` (`osfmk/arm/armv/cpufunc-v7.s:49`) escribe TTBR0 de USUARIO con `orrlt r0,r0,#0x5b` (Cortex-A7 tiene MP ext → MPIDR bit31=1 → rama LT) = IRGN=WB inner + RGN=WB outer + **S=1**. Caminatas cacheables+shareable. → TTBR0 medido = `0x82fb405b`. ✔ coincide.
- `set_mmu_ttb` del KERNEL (`machine_routines_asm.s:253`) usa `orr r0,r0,#0x18` = RGN=WB outer, **IRGN=0 (inner NO-cacheable)**, S=0. Por eso el kernel funciona y sólo el usuario falla.
- Las tablas se escriben por `phys_to_virt` (mapa lineal de `arm_vm_init`, `l2_map_linear_range`) = Normal **Write-Through, non-shareable** (`MMU_TEXCB_CA_WT_NWA`, sin S-bit).

### Por qué fallaron los intentos previos (lo CONFIRMA)
- **M90 (`DCCMVAC`)**: sólo hace *clean*; la copia rancia del walker es *limpia* (sólo la leyó) → no-op. No arregla.
- **M93 (`TLBIALL`/`TLBIMVAA`)**: invalida el TLB, no la caché de datos donde está el descriptor rancio. No arregla.
- **No-determinismo** M91(rojo)/M92(blanco)/ATS1CPR: que la línea rancia sobreviva al reintento depende de las evicciones de caché, que varían por build. Huella de coherencia, no de bug estático.

### EL FIX (M94)
`arm_context_switch` en `cpufunc-v7.s`: quitar los `orr r0,r0,#0x5b`/`#0x1b` → TTBR0 de usuario NO-cacheable (base sin atributos; ya viene alineada a 16KB, bits[13:0]=0). El walker lee siempre RAM; las escrituras WT+`DSB` (M90) siempre visibles. Determinista.

### Bugs latentes a arreglar aparte (NO son este muro, confirmado por M93):
- `pmap_flush_tlbs`: `flush_self` sólo TRUE si `armreg_ttbr_read()&0xFFFFFF00==pm_l1_phys`; si lee TTBR1 (kernel), nunca casa para user → TLB de user no se invalida. Verificar.
- `pmap_create` deja `pm_asid=0` y `pmap_asid_alloc_fast` bajo `#ifdef _NOTYET_` → no se asigna ASID; con `pmap_asid_ncpus==0` cae a `arm_tlb_flushID()` (total), funciona pero es parche.

Análisis completo: agente sobre pmap.c/vm_fault.c/trap.c/arm_vm_init.c/cpufunc-v7.s.

---

## ★★ MURO DE LAUNCHD RESUELTO (0723 tarde, sesión Mac) — M94→M102

**Eran DOS bugs encadenados, no uno:**

### Bug 1: caminatas de tabla de USUARIO cacheables (M94) — parcial
`arm_context_switch` (cpufunc-v7.s) ponía TTBR0 de usuario a `|0x5b` (WB inner + S). Se cambió a no-cacheable (base sin atributos). **NO era el muro** pero es correcto dejarlo (el kernel usa 0x18).

### Bug 2 (EL MURO): el "bucle de copyout" era en realidad el UART

Diagnóstico definitivo con volcado EN PANTALLA (PE_fb_hexline, pixel directo — `PE_early_puts` CUELGA en contexto de abort). Capturando 1er y 2o fallo (M101):
- **1er fallo**: `dfar=0x1000 dfsr=0x805(write) pc=0x800e2774` (copyout). vm_fault mapea, reintenta, **copyout FUNCIONA**.
- **2o fallo**: `dfar=0x11002014` (¡registro LSR del UART!) `dfsr=0x5(read) pc=0x8047a110` (fbcon_uart_putc).

Antes de mapear 0x1000 se verificó (M99/M100) que tras vm_fault: `L1[0]=0x82fc6001` válido, `PTE=0x82fc51bb` (AP=011 RW PL0+PL1), `ATS1CPR=ATS1CPW=0x82fc5068` (éxito lectura Y escritura). O sea el mapeo de usuario era PERFECTO — el "bucle" no era copyout.

**Causa del bug 2**: `fbcon_uart_putc` accede al UART por la física `0x11002000`, que está identity-mapeada en el rango **TTBR0** (por-pmap, sólo en el pmap del KERNEL). Al imprimir el marcador `LP1` justo tras el éxito de copyout, corremos con el pmap de USUARIO activo (TTBR0=user) → el UART no está mapeado → fallo → vm_fault no puede mapear MMIO → segundo fallo mudo = el "bucle azul".

El **framebuffer** (0xBF400000) nunca dio problema porque está en **TTBR1** (kernel, compartido por todos los pmaps). El UART estaba en TTBR0.

**Fix (M102)**: mapear el UART TAMBIÉN en TTBR1 — VA alta `0xF1000000` → PA `0x11000000` (sección device en cpu_ttb, arm_vm_init, junto al fb). `fbcon_uart_putc` usa `0xF1002000`. Accesible desde cualquier contexto.

### Descartes por el camino (todos medidos, útil para futuros bugs de MMU):
- NO era coherencia de caché: M96 (arm_dcache_wbinv_all TOTAL) + walker no-cacheable (M94) seguía "fallando".
- NO era phys_to_virt: M97 verde (pm_l1_virt traduce a pm_l1_phys correctamente).
- NO era formato ni permisos: PTE válida, AP=RW, ATS1CPR y ATS1CPW éxito.
- La técnica que desbloqueó todo: **volcar valores hex EN PANTALLA con escritura de pixel directa** (PE_fb_hexline), porque el UART y PE_early_puts cuelgan en contexto de abort.

### Regla nueva del port:
Todo MMIO que se acceda desde contexto de USUARIO (consola, etc.) debe mapearse en **TTBR1** (VA >= 0x40000000), no en el identity TTBR0, o desaparece bajo el pmap de usuario.
