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

## 4. Carveout de shared-mem (reserved-memory) — ✅ HECHO Y VERIFICADO EN HW (menupick22)
Nodo (dentro de `reserved-memory {}`, junto a framebuffer/ramoops):
```dts
modem-region@b8000000 {
    reg = <0xb8000000 0x1800000>;   /* 24MB = 22 MD + 2 SMEM, alineado a 32MB */
    no-map;
};
```
**Verificado**: `OF: reserved mem: 0xb8000000..0xb97fffff (24576 KiB) nomap non-reusable modem-region@b8000000`.
⚠️ **GOTCHAs de colocación** (2 iteraciones costaron):
1. **NO @0xbc000000** (el 1er intento): **cae DENTRO del CMA** (`cma: Reserved 64 MiB at 0xbb000000` →
   0xbb000000-0xbf000000) → el kernel lo ignora silenciosamente. Hay que ir **por debajo del CMA**.
2. **0xb8000000** (32MB-aligned, bajo CMA, termina en 0xb9800000) = OK, se reserva. (fb@0xBF400000 +
   ramoops@0xBF300000 arriba; CMA 0xbb-0xbf; libre por debajo.)
3. Al editar el DTS **decompilado**: insertar el nodo con ancla INEQUÍVOCA (`framebuffer-region@bf400000`),
   NO `ranges;` (hay varios `ranges;`; uno en un `simple-bus` que se lo tragó → nodo en el bus equivocado
   = no reservado). Es `smem_addr = 0xb8000000` el que alimenta la fórmula del BANK-remap (des en H3).
DTS de trabajo: `ganador.dts` en la Pi (decompilado de `ganador.dtb` + este nodo). **Pendiente**: portar
el nodo al DTS COMPARTIDO cuando se coordine con el Mac (ahora aislado en nuestro dtb).

## 5. Estado H2 y siguiente
- ✅ RE completo (este doc). Registros y fórmula listos.
- ✅ **H2a VERIFICADO EN HW (menupick19, dump por sysfs `spm_md_dump`)**: CCIF@0x1020A000 y los
  BANK4_MAP son **accesibles desde mainline** — los reads dan `0x00000000` (idle/sin remap), NO
  `0xffffffff` (que sería fallo de bus/región no mapeada). Confirma que el **mailbox AP↔MD del CCIF es
  alcanzable** (CON=0=idle) y los BANK regs también → **de-riskea H3**. Móvil sano tras el dump.
  Dump: `CON=BUSY=TCHNUM=RCHNUM=0`, `AP0=AP1=MD1_0=MD1_1=0`.
- ✅ **H2b VERIFICADO EN HW (menupick22)**: carveout de 24MB @0xb8000000 reservado (§4). El móvil arranca
  con la memoria del MD reservada. **H2 sustancialmente completo** (memoria + mapeos verificados).
- ⏳ Falta para H2/H3: escribir el BANK-remap con la fórmula (§1) usando `des=0xb8000000` + ioremaps
  boot-slave/RGU/CCIF en el driver + el handshake CCCI. Va junto con H3 (donde el MD usa la vista DRAM).
- ⏳ Implementar: (a) reserved-memory en DTS [OJO: DTS compartido roto por el Mac — usar base buena +
  extraer dtb, ver [[H1]]], (b) ioremaps CCIF/boot-slave/RGU en el driver, (c) `set_{ap,md}_smem_remap`
  con la fórmula → escribir BANK4_MAP. (d) Verificable de verdad solo en H3 (cuando el MD arranque y use
  la vista de DRAM). Paso intermedio medible = dump por sysfs de CCIF+BANK (reads AP-space, sin riesgo).
- El firmware ya está (H0): `MOLY.WR8.W1449.MD.WG.MP.V1`. H1 (MTCMOS) validado.

## Ficheros downstream (Pi)
`~/mainline/downstream/drivers/misc/mediatek/dual_ccci/mt6582/src/ccci_platform.c` (remap 2508-2600,
boot-slave 2298-2301, bases 2911-2912), `.../mt6582/include/ccci_platform{_cfg.h,.h}` (defines CCIF/BANK/WDT).

*RE 2026-07-13. Base para implementar H2.*
