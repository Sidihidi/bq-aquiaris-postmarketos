# Módem M1 — H2: carveout + remap BANK4 (riesgo #1) — CARACTERIZADO (2026-07-13)

> **El "riesgo #1" (remap DRAM indocumentado) queda ESPECIFICADO byte a byte + verificado en HW.**
> RE completo del downstream + lectura read-only de los registros reales. Los 4 registros del remap
> están **a 0** (el LK NO los toca) → lienzo limpio: escribir con los valores calculados. La
> integración (nodo DTS + escritura del remap) es rebuild+flash y va acoplada a H3.

## Registros del remap (direcciones FÍSICAS confirmadas)
| Registro | Físico | Base downstream |
|---|---|---|
| `AP_BANK4_MAP0` / `MAP1` | **0x10200200 / 0x10200204** | `MCUSYS_CFGREG_BASE(0xF0200000→0x10200000) + 0x200/0x204` |
| `MD1_BANK4_MAP0` / `MAP1` | **0x10001308 / 0x1000130C** | `INFRACFG_AO_BASE(0x10001000) + 0x308/0x30C` |
| `MD1_BANK0_MAP0` / `MAP1` | 0x10001300 / 0x10001304 | (bank0, no se toca para SMEM) |

## Lectura en HW (pmOS #284, módulo `bank4read.c`, READ-ONLY)
```
AP_BANK4_MAP0=0x00000000  MAP1=0x00000000     <- reset, el LK NO programa el remap
MD1_BANK4_MAP0=0x00000000 MAP1=0x00000000     <- reset
MD1_BANK0_MAP0=0x00000000 MAP1=0x00000000
TOPAXI_PROT_EN=0x00000000  STA1=0x0000e000    <- protección MD ya liberada (coherente con H1)
AP_CCIF CON/BUSY/START = 0/0/0                 <- CCIF idle
```
→ El remap lo hace CCCI en runtime (cuando Android arranca el MD), no el LK. En nuestro port lo
hacemos nosotros. **Nada que preservar.**

## Carveout (de `mtk_ccci_helper.c` + `ccci_platform.h`)
- **`MD1_MEM_SIZE = 22MB` (imagen MD) + `MD1_SMEM_SIZE = 2MB` (shared) = 24MB.**
- `arm_memblock_steal(24MB, SZ_32M)` → **alineado a 32MB**. Constraints (assert downstream):
  addr MD `& (32MB-1) == 0`, addr SMEM `& (2MB-1) == 0`. `KERN_EMI_BASE = 0x80000000` (base DRAM).
- **Nodo `reserved-memory` propuesto** (DRAM 0x80000000..0xC0000000; ramoops en 0xBF300000):
  ```dts
  reserved-memory {
      md1_mem: modem@bc000000 {          /* 32MB-aligned, bajo ramoops */
          reg = <0xBC000000 0x1800000>;   /* 24MB: 22MB MD @0xBC000000 + 2MB SMEM @0xBD600000 */
          no-map;
      };
  };
  ```
  (Sustituye el `arm_memblock_steal`; el driver toma la base por phandle. Verificar que no pisa
  ni ramoops ni el rango de otros reserved-mem del dts.)

## Fórmula del remap (portar VERBATIM — corta, del downstream)
`ccci_platform.c` MT6582. `INVALID_OFFSET=0x02000000`, `INVALID_ADDR[MD_SYS1]=0x3E000000`.
Granularidad = **16MB** (`>>24`): cada byte de MAP0/MAP1 es un slot de 16MB; slot0 = shared-mem real,
slots 1-7 = direcciones inválidas (0x3E000000 + n·32MB) para atrapar accesos fuera de rango.
```c
/* AP side: set_ap_smem_remap(md_id, src, des) -> AP_BANK4_MAP0/1(MCUSYS+0x200/0x204) */
remap1 = ((des>>24)|0x1)&0xFF)
       + ((((INVALID_ADDR + INVALID_OFFSET*14)>>16)|1<<8)&0xFF00)
       + ((((INVALID_ADDR + INVALID_OFFSET*15)>>8)|1<<16)&0xFF0000)
       + ((((INVALID_ADDR + INVALID_OFFSET*16)>>0)|1<<24)&0xFF000000);
remap2 = ... (INVALID_OFFSET*17..20) ...
/* MD side: set_md_smem_remap -> des -= KERN_EMI_BASE; MD1_BANK4_MAP0/1(INFRACFG+0x308/0x30C) */
des -= 0x80000000;
remap1 = ((des>>24)|0x1)&0xFF) + (INVALID_OFFSET*0..2 empaquetados);
remap2 = (INVALID_OFFSET*3..6 empaquetados);
```
(Fuente exacta: `~/mainline/downstream/.../mt6582/src/ccci_platform.c:2508` (AP) y `:2540` (MD).)
⚠️ **Peligro**: escribir `AP_BANK4_MAP` en pmOS VIVO puede remapear DRAM que el AP usa → NO probar
por poke en caliente. Va en el flujo H3 integrado (carveout reservado + CCCI dueño), sobre un kernel
recuperable.

## Estado H2 y siguiente
- ✅ Direcciones, tamaños, alineación, fórmula y estado HW (reset) **caracterizados**. Riesgo #1
  desmitificado.
- ⏳ **Integración (rebuild+flash, va con H3)**: (1) nodo `reserved-memory` en el dts; (2) portar
  las 2 funciones de remap + ioremaps CCIF(0x1020A000)/RGU(0x20050000)/boot-slave(0x2019xxxx);
  (3) EMI-MPU mínima o `ENABLE_EMI_PROTECTION` OFF; (4) el subset CCCI boot-only + IRQ 132 + keys.
- Coordinar con casa: toca dts + (opcional) `mt6582-spm.c` (dominio MD defensivo) — árbol caliente.

## Ficheros
- `bank4read.c` (este dir) — módulo read-only del dump (reutilizable).
- Pi: `~/modem-h0/bank4-test/`. Downstream: `~/mainline/downstream/.../dual_ccci/`.

*Sesión Mac (Fable 5), 2026-07-13. H0 ✅ H1 ✅ H2 caracterizado → H3 (CCCI boot + carveout+remap juntos).*
