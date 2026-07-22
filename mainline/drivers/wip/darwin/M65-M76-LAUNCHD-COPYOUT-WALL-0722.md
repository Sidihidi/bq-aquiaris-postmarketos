# XNU en el krillin (MT6582): el muro de launchd — el `copyout` a `0x1000` y el walk de TTBR0 (M65→M76, 0722)

> **Continúa** `ESTADO-Y-HALLAZGOS-0722.md` (M65: raíz HFS+ montada, llega a `attempting to start init
> of /sbin/launchd`). Esta tanda instrumenta por **UART** el arranque de launchd y aísla el muro con
> precisión quirúrgica. **No resuelto** — es un problema de nivel hardware (walk de TTBR0 / coherencia
> SMP-SCU). Documentado para retomar. Imágenes `~/mainline/pkg/boot-xnu-M6x/M7x-*.img`.

## 1. El síntoma exacto

`load_init_program()` (`bsd/kern/kern_exec.c:3515`) imprime `attempting to start init` y **se cuelga en el
PRIMER `copyout`** (línea 3517) del nombre `/sbin/launchd` a la página de usuario `0x1000`
(`init_addr = VM_MIN_ADDRESS`, un `vm_allocate` lazy). **Cuelgue silencioso**: el log del UART **no crece**,
no hay `*** PANIC ***`, no hay `FALLO datos`, no hay `IRQ#`.

## 2. Aislamiento (todo medido por UART, no supuesto)

| Prueba (build) | Resultado | Descarta |
|---|---|---|
| `init_addr` + `copyout RET` (M68/M69) | `init_addr=0x1000 len=129`, no retorna | — |
| Lectura **privilegiada** directa de `0x1000` (M69) | **cuelga igual** (sin `LP0d`) | copyout / el `strt` unpriv |
| Detector de `IRQ#` (M68) | cero IRQ | tormenta de interrupciones |
| Lectura de `0x30000000` (rango TTBR0) (M70) | **cuelga** | (confirma: es el rango de usuario) |
| Lectura de `0x40000000` (rango TTBR1) (M72) | **lee 0x00, OK** | (confirma: TTBR1 va, TTBR0 no) |
| Volcado de registros (M71) | `VBAR=0x800e3000` (kernel), `TTBCR=2`, `TTBR0=0x82fb405b`, `TTBR1=0x81e04018` | vector inalcanzable |
| Atributos del walk `0x5b`→`0x18` (M73) | **cuelga igual** | atributos del table-walk |
| Volcado del descriptor L1 de usuario (M74) | `L1[0]=0x00000000` (inválido); `gPB=0x81000000 gVB=0x80000000` | L1 con basura *estática* |
| `sp` del kernel en el fault (M75) | `sp=0xd04d3ee8` (≥0x80000000, TTBR1) | pila de kernel en rango TTBR0 |
| Flush (DCCMVAC) de la L1 a RAM (M76) | **cuelga igual** | coherencia *solo* de la L1 |

**Conclusión medida:** cualquier acceso al rango de **TTBR0 (usuario, `<0x40000000`)** cuelga el
table-walk del MMU; **TTBR1 (kernel) funciona**. El descriptor L1 de usuario para `0x1000` es `0`
(inválido, página lazy) → **debería producir un translation fault limpio que despache a `sleh_abort`
(y de ahí `vm_fault` poblaría la página), pero el fault NO despacha: cuelga en silencio.**

## 3. La contradicción central (lo que hace esto un muro hardware)

Un descriptor L1 = `0` es, por arquitectura, un translation fault de sección. Debería:
`ldrb 0x1000` → data abort → vector `0x800e3000` (TTBR1, alcanzable) → `_fleh_dataabt`
(`osfmk/arm/traps_lo.s`) → `data_abort_crash_in_kernel` (escribe en la pila SVC `0xd04d3ee8`,
alcanzable) → `bl _sleh_abort` → `FALLO datos` (gate `nfault<8`, arranca en 0).

**Todo el camino es alcanzable y aun así no aparece `FALLO`.** Las dos únicas explicaciones que quedan,
ambas de nivel hardware:

1. **El table-walk del MMU no lee `0`** que sí ve la CPU (lectura coherente) → **incoherencia de las
   tablas**. Con **SMP/SCU OFF** (blocker del doc §8: `ACTLR.SMP` cuelga porque el SCU no está init),
   los walks cacheables y la D-cache **no son coherentes**. El MMU leería un descriptor basura →
   apunta a física muerta → el `ldrb` a esa física da un **external abort asíncrono** que, si `CPSR.A`
   está enmascarado, **no se toma → cuelga**. (El flush de la L1 en M76 no bastó: faltarían L2/páginas,
   o el flush por VA no cubre lo que lee el walk.)
2. Un fallo del propio walk de TTBR0 con `TTBCR.N=2` (tabla de 4KB/1024 entradas, base `0x82fb4000`)
   que lo lleva a leer descriptores de una ubicación equivocada. *(Menos probable: el cálculo de la
   dirección del descriptor da `0x82fb4000` y ahí hay `0`.)*

**Sospechoso principal = coherencia (SMP/SCU).** El kernel (TTBR1) funciona porque sus tablas se
montaron en `arm_vm_init` con mantenimiento de caché; la L1 de usuario, creada luego por `pmap_create`,
no queda coherente para el walk.

## 4. Candidatos a probar (siguiente sesión)

1. **Flush COMPLETO de la D-cache** antes del `copyout` (no solo los 4KB de la L1). Si desbloquea →
   coherencia confirmada, y da un workaround (flushear tablas de usuario tras crearlas).
2. **Inicializar el SCU y activar `ACTLR.SMP`** = coherencia real. Es el arreglo de fondo. El SCU del
   Cortex-A7 está en `PERIPHBASE` (leer CBAR: `mrc p15,4,rX,c15,c0,0`); habilitar bit 0 de
   `SCU_CONTROL` (PERIPHBASE+0x0) ANTES de tocar `ACTLR.SMP`. El doc cerró SMP por colgar, pero
   probablemente sin init previo del SCU.
3. **`TTBCR.N=0`** (tabla única, sin split TTBR0/TTBR1): evita el walk de TTBR0 por completo, pero la
   L1 de usuario tendría que copiar las mapas altas del kernel. Cambio arquitectónico mayor.
4. Comprobar `CPSR.A` (bit 8): si los aborts asíncronos están enmascarados, un external abort del walk
   quedaría pendiente sin tomarse (cuelgue). Desenmascararlo haría VISIBLE el fallo (aunque no lo cure).

## 5. Cómo se depura esto (infra ya montada — vale oro)

- **UART**: cable a GPIO14/15 de la Pi = `/dev/ttyAMA0`, 921600 8N1, UN solo lector. Helper
  `~/uart-capture.sh`; leer `sudo strings /tmp/uart.log`. Lado XNU: `fbcon_uart_putc` (físico
  `0x11002000`) enganchado en `fbcon_putchar`. Log completo preloader+LK+GenericBooter+XNU.
- **Sondas** `kprintf("MARCA\n")` / `PE_early_puts` (M67 pone `fbcon_quiet=0` = kprintf verboso).
- **Volcado de tablas desde C**: `phys_to_virt(p) = (p - gPhysBase) + gVirtBase` (macro en `arm/pmap.h`);
  con `gPhysBase=0x81000000, gVirtBase=0x80000000`.
- Flasheo **BROM**: `mtk.py wo 0x2900000 0x1400000 boot-xnu-*.img` (fastboot NO va). Batería
  fuera/dentro + Vol− + USB. **Cable de flasheo y cable UART son distintos** (uno al puerto USB, otro
  a los GPIO): tras flashear, cambiar de cable para arrancar y capturar.

*0722. Continúa ESTADO-Y-HALLAZGOS-0722.md. El kernel de Apple monta raíz HFS+ y llega al `exec` de
launchd; el último muro es el walk de TTBR0 / coherencia SMP-SCU. Diagnóstico cerrado, fix pendiente.*
