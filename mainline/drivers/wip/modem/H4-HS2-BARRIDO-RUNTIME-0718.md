# Módem H4 — barrido EXHAUSTIVO del runtime: las regiones SÍ importan (0718)

> Barrido campo-a-campo de los 70 ints del `modem_runtime_t` vs el source, + test empírico
> "dar TODO lo que da el stock". **Hallazgo: el runtime completo CAMBIA el comportamiento del MD**
> (contradice la conclusión previa "las regiones no importan"), pero el data abort PERSISTE, más
> profundo. La búsqueda avanza: la causa NO es un campo faltante, es algo en el uso de las regiones.

## Diff campo-a-campo (los 70 ints) — todo lo que dábamos era CORRECTO
`modem_runtime_t` (ccci_md.h:296): 70 ints = 280B. Comparado uno a uno contra `set_md_runtime` +
`platform_set_runtime_data` (que solo pone Platform_L/H "MT65"/"82E1" + DriverVersion 0x20121001 —
nada extra) + las condiciones `if(smem_size)` de cada región:
- **Correctos (coinciden con stock/Lineage)**: Prefix, Platform_L/H, DriverVersion, BootChannel=0,
  BootingStartID=0, BootAttributes=0, BootReadyID=0, CheckSum=0, Postfix. Exce(35/36), MDExExpInfo
  (62/63), Misc(66/67), Pcm(10/11), Rpc(31/32), Fs(29/30), Ipc(39/40).
- **Correctamente a 0 (el stock tampoco los aloca)**: Sys(37/38), Pmic(33/34), IPCMDIlm(64/65).
- **Faltaban (el stock SÍ los pone no-cero)**: **Mdlog(8/9), Uart(12=PortNum + 13-15/21-23 x3 TTY),
  Net(41/42 MDUL, 43/44 MDDL, 45=PortNum, 46-61 ctrl CCMNI)**.

## Test empírico: añadir Mdlog + Uart x3 + Net (kernel #23) → el abort CAMBIA
Con TODAS las regiones no-cero puestas (bases MD-view no-solapadas en el SMEM, tamaños placeholder):
```
Antes (runtime minimo):  ch1 = ffffffff 00000004 ... 45584350   (data abort inmediato)
Ahora (runtime completo): 
  RCHNUM=0x0C
  ch1 = 4160e000 00000020 0000000e   <-- NUEVO: msg canal 0xe=14=CCCI_FS_RX, data0=base FS (0x4160e000)
  ch2 = ffffffff 00000004            <-- la excepcion data-abort SIGUE
```
**El MD ahora emite un mensaje del canal FS referenciando la región FS que le dimos** — algo que NO
hacía con el runtime mínimo. → **las regiones del runtime SÍ las usa el MD** (refuta el "no importan"
de las iteraciones previas, que no habían probado Uart/Net/Mdlog juntos). Pero el data abort persiste.

## Interpretación y siguiente paso
Dos lecturas (ambiguo sin más datos):
1. **Progreso real**: el MD avanza más (procesa FS) y aborta en un paso POSTERIOR — probablemente
   por un **tamaño de región incorrecto** (usamos placeholders 0x1000-0x8000; los reales son
   `sizeof(structs)` complejos; si el MD calcula offsets de buffer con el size y se sale de la
   región → abort). O por una región cuyo size real necesita.
2. **El msg FS es parte del dump de excepción** (el handler de EE del MD podría volcar por el canal
   FS). Menos probable (data0 = exactamente nuestra base FS, no un log).

**Siguiente (bisección barata):** (a) quitar SOLO la región FS y ver si el msg FS desaparece y el
abort se mueve (confirma que el MD usa FS). (b) Poner los TAMAÑOS reales de las regiones (extraer
`sizeof(fs_stream_buffer_t)`, `shared_mem_tty_t`, `CCCI_CCMNI_SMEM_*` del source) en vez de
placeholders. (c) Si con tamaños correctos sigue → RE del parser (~0x6148) para ver el paso posterior.

## Estado
El runtime YA NO es "obviamente incompleto" — dárselo entero avanza al MD. Driver de test con el
barrido = `spm-sweep` (en scratchpad; NO commiteado al `mt6582-spm-H1.c` compartido para no pisar al
Mac — los writels extra están aquí documentados). Test #23 `boot-modem-hs2.img`. Móvil restaurado a
la imagen de diario (#14). Fichero compartido `spm-H1.c`: LIBRE (no lo toqué; el test fue sobre una
copia local).

## Los writels del test (para reproducir / integrar)
```c
/* tras el bloque IPC/PCM/RPC/FS existente, antes de MiscInfo: */
writel((MD_SMEM_PHYS+0x10000)-MD_AP_OFF, smem+8*4);  writel(0x8000, smem+9*4);   /* Mdlog */
writel(3, smem+12*4);                                                            /* UartPortNum */
writel((MD_SMEM_PHYS+0x18000)-MD_AP_OFF, smem+13*4); /* +0x19000 smem+14, +0x1A000 smem+15 */
writel(0x1000, smem+21*4); /* +22,+23 = 0x1000 */                               /* Uart sizes */
writel((MD_SMEM_PHYS+0x1C000)-MD_AP_OFF, smem+41*4); writel(0x2000, smem+42*4);  /* MDULNet */
writel((MD_SMEM_PHYS+0x1E000)-MD_AP_OFF, smem+43*4); writel(0x2000, smem+44*4);  /* MDDLNet */
writel(3, smem+45*4);                                                            /* NetPortNum */
writel((MD_SMEM_PHYS+0x20000..0x22000)-MD_AP_OFF, smem+46..48*4); writel(0x1000, smem+50..52*4);
writel((MD_SMEM_PHYS+0x24000..0x26000)-MD_AP_OFF, smem+54..56*4); writel(0x1000, smem+58..60*4);
```

*2026-07-18, sesión Windows (Fable 5). Barrido exhaustivo: runtime byte-correcto + regiones SÍ usadas;
el abort persiste más profundo (tamaños o paso posterior). Toca bisección de tamaños o RE del parser.*
