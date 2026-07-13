# Módem H2 — mapa completo de registros + fórmula de remap (RE del downstream) — 0713

> Todo lo necesario para implementar H2 (memoria + mapeos + BANK-remap), extraído del downstream
> `dual_ccci/mt6582/`. El RE (la parte dura) está hecho; queda implementar+flashear.

## 1. BANK-remap de DRAM (BLOQUEANTE #1) — `set_ap_smem_remap` / `set_md_smem_remap`
Define la "vista" de DRAM que tiene el MD. Registros (offsets desde su base):
| Registro | Dirección física | Base |
|---|---|---|
| AP_BANK4_MAP0 | **0x10200200** | MCUSYS_CFGREG_BASE (0x10200000) = nuestro `mcusys` ya mapeado |
| AP_BANK4_MAP1 | **0x10200204** | idem |
| MD1_BANK4_MAP0 | **0x10001308** | INFRACFG_AO (0x10001000) |
| MD1_BANK4_MAP1 | **0x1000130C** | idem |
(AP_BANK4_MAP_UPDATE @ +0x90 = "no need for 6572", NO tocar.)

Constantes de la fórmula: `KERN_EMI_BASE=0x80000000`, `INVALID_OFFSET=0x02000000`,
`INVALID_ADDR[MD_SYS1]=0x3E000000` (hardcode; el otro camino = `round_up(get_max_DRAM_size(),INVALID_OFFSET)`).
Llamadas: `set_ap_smem_remap(md_id, 0x40000000, smem_addr)` y `set_md_smem_remap(md_id, 0x40000000, smem_addr)`
(en set_md, `des -= KERN_EMI_BASE` primero). `src=0x40000000` = base de la vista del MD; `des=smem_addr` = física del carveout.

Fórmula (para MD1, remap1/remap2 son 2×u32 empaquetados de 4 bytes cada uno):
```
remap1 = ((des>>24)|1)&0xFF | (((INVALID_ADDR+OFF*0)>>16)|1<<8)&0xFF00 |
         (((INVALID_ADDR+OFF*1)>>8)|1<<16)&0xFF0000 | (((INVALID_ADDR+OFF*2)>>0)|1<<24)&0xFF000000
remap2 = análogo con OFF*3..6
```
(AP usa OFF*14..20; ver ccci_platform.c:2508-2600.) Cada byte = un "bank" de 16MB con el bit de válido.

## 2. Boot-slave keys (release del MD a su vector) — ccci_platform.c:2298
| Qué | Física | Valor a escribir |
|---|---|---|
| Vector | **0x20190000** | 0x0 |
| Key | **0x2019379C** | 0x3567C766 |
| En | **0x20195488** | 0xA3B66175 |
| RGU (WDT del MD) | **0x20050000** (len 0x40) | WDT_MD_MODE: DEFAULT=0x3, KEY=0x22<<8, LENGTH=0x7FF<<5, LEN_KEY=0x8 |
(Direcciones en espacio de registros del MD 0x20xxxxxx — accesibles desde el AP tras power-on; leer/escribir con cuidado.)

## 3. CCIF (mailbox AP↔MD) — ccci_platform_cfg.h:106
Base AP **0x1020A000**, MD **0x1020B000**. Offsets: CON+0x0, BUSY+0x4, START+0x8, TCHNUM+0xC,
RCHNUM+0x10, ACK+0x14, TXCHDATA+0x100, RXCHDATA+0x100+128(0x180). Run-time data offset 0x140.
IRQ CCIF0_AP = **132** (GIC_SPI 100, level-low) → DT `<GIC_SPI 100 IRQ_TYPE_LEVEL_LOW>`.

## 4. Carveout de shared-mem (reserved-memory)
`arm_memblock_steal(size, SZ_32M)` = 24MB (MD1_MEM_SIZE 22MB + MD1_SMEM_SIZE 2MB) alineado a 32MB.
En mainline = nodo `reserved-memory` en el DTS (p.ej. `modem-region@bc000000 { reg=<0xbc000000 0x1800000>; no-map; }`,
bajo ramoops 0xBF300000). El `smem_addr` (la física del carveout) alimenta la fórmula del BANK-remap.

## 5. Estado H2 y siguiente
- ✅ RE completo (este doc). Registros y fórmula listos.
- ✅ **H2a VERIFICADO EN HW (menupick19, dump por sysfs `spm_md_dump`)**: CCIF@0x1020A000 y los
  BANK4_MAP son **accesibles desde mainline** — los reads dan `0x00000000` (idle/sin remap), NO
  `0xffffffff` (que sería fallo de bus/región no mapeada). Confirma que el **mailbox AP↔MD del CCIF es
  alcanzable** (CON=0=idle) y los BANK regs también → **de-riskea H3**. Móvil sano tras el dump.
  Dump: `CON=BUSY=TCHNUM=RCHNUM=0`, `AP0=AP1=MD1_0=MD1_1=0`.
- ⏳ Implementar: (a) reserved-memory en DTS [OJO: DTS compartido roto por el Mac — usar base buena +
  extraer dtb, ver [[H1]]], (b) ioremaps CCIF/boot-slave/RGU en el driver, (c) `set_{ap,md}_smem_remap`
  con la fórmula → escribir BANK4_MAP. (d) Verificable de verdad solo en H3 (cuando el MD arranque y use
  la vista de DRAM). Paso intermedio medible = dump por sysfs de CCIF+BANK (reads AP-space, sin riesgo).
- El firmware ya está (H0): `MOLY.WR8.W1449.MD.WG.MP.V1`. H1 (MTCMOS) validado.

## Ficheros downstream (Pi)
`~/mainline/downstream/drivers/misc/mediatek/dual_ccci/mt6582/src/ccci_platform.c` (remap 2508-2600,
boot-slave 2298-2301, bases 2911-2912), `.../mt6582/include/ccci_platform{_cfg.h,.h}` (defines CCIF/BANK/WDT).

*RE 2026-07-13. Base para implementar H2.*
