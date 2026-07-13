# Módem M1 — H3: bring-up del MD (CCCI boot-only) — SPEC DE IMPLEMENTACIÓN (2026-07-13)

> **RE completo del camino de arranque del MD del downstream.** Toda constante y la secuencia
> ordenada, lista para codificar. H3 es grande (~1.5-2 sem): esta sesión captura la receta entera +
> el nodo DTS del carveout + el andamiaje del módulo (`md1-boot.c`). El intento real (build+flash+
> captura del handshake) es la siguiente sesión, con el móvil a mano y kernel recuperable.

## Mapa de memoria y registros (TODO físico, confirmado)
| Recurso | Físico | Notas |
|---|---|---|
| Carveout MD | **0xBC000000** (32M-align, bajo ramoops) | 22MB img @0xBC000000 + 2MB SMEM @0xBD600000. `reserved-memory` en dts |
| boot-slave Vector | **0x20190000** (4B) | escribir `0x0` |
| boot-slave Key | **0x2019379C** (4B) | escribir `0x3567C766` |
| boot-slave En | **0x20195488** (4B) | escribir `0xA3B66175` (esto ARRANCA el MD CPU) |
| MD RGU (WDT) | **0x20050000** (0x40) | `WDT_MD_MODE` reg; disable = escribir `WDT_MD_MODE_KEY=0x2200` |
| AP_CCIF | **0x1020A000** (0x100) | CON+0x0, BUSY+0x4, START+0x8, TCHNUM+0xC, RCHNUM+0x10, ACK+0x14, TXCHDATA+0x100, RXCHDATA+0x180 |
| MD_CCIF | **0x1020B000** | lado MD del mailbox |
| CCIF IRQ | **GIC_SPI 100** (LEVEL_LOW) | =CCIF0_AP_IRQ_ID (132 = 32+100 downstream) |
| AP_BANK4_MAP0/1 | **0x10200200/204** (MCUSYS) | remap (H2) |
| MD1_BANK4_MAP0/1 | **0x10001308/30C** (INFRACFG) | remap (H2) |
| SPM MD1 MTCMOS | 0x10006284 | ya ON por el LK (H1) |

## Secuencia ORDENADA de arranque (de `boot_md`→`let_md_go`, downstream)
1. **Reservar carveout** (DTS `reserved-memory` `no-map`, 24MB@0xBC000000). El driver obtiene la base.
2. **Cargar firmware**: `request_firmware("modem_1_wg_n.img")` (5.100.832 B, en `/lib/firmware/`) →
   copiar a `ioremap_nocache(carveout_base, img_size)`. `load_addr = img->address = carveout_base`.
3. **Validar GFH**: `check_md_header()` — el GFH check-header (magic `"CHECK_HEADER"` / `0x4D4D4D`
   "MMM", `GFH_CHECK_HEADER_TYPE=0x104`, `m_size=0xc8`) va **al FINAL** del img (confirmado en H0).
   Parsear, no cripto (secure-boot OFF).
4. **Programar remap** (H2): `set_ap_smem_remap` → AP_BANK4_MAP0/1; `set_md_smem_remap` → MD1_BANK4_MAP0/1.
   `des = SMEM base (0xBD600000)`, granularidad 16MB, slots inválidos a 0x3E000000. Registros hoy =0.
   ⚠️ escribir AP_BANK4 corrompe el AP si `des`/slot0 pisa DRAM del AP → validar el cálculo.
5. **Init CCIF**: reset = `CCIF_CON=1` + `CCIF_ACK=0xFF`; entrar modo ARB (`CCIF_CON=CCIF_CON_ARB(1)`).
   `request_irq(GIC_SPI 100, handler)`.
6. **(HS1→HS2) runtime data**: tras HS1 el AP escribe `set_md_runtime` (geometría shared-mem, sin
   NVRAM/SIM: `filp_open("/data/mdl/mdl_config")` falla con gracia → `NORMAL_BOOT_ID`) en el SMEM y
   responde por CCIF. `runtime->BootingStartID = (mdlog_flag<<8)|NORMAL_BOOT_ID`. **Estructura grande
   = el grueso del port** (`ccci_md_main.c:1373-1656`, `set_md_runtime`).
7. **Kick** (`ungate_md1`): (a) `ccci_en_md1_clock()` [MTCMOS, ya ON]; (b) `WDT_MD_MODE=0x2200`;
   (c) los 3 boot-slave keys (Key/Vector/En arriba). El write de `En` suelta el MD CPU.
8. **Recibir handshake**: el MD corre MOLY y señaliza por CCIF → IRQ → `RCHNUM` da el canal →
   `ccci_logical` parsea `md_boot_stage`. **HS1** = `MD_INIT_START_BOOT`→stage 1. **HS2 = M1 OK** =
   `NORMAL_BOOT_ID`→stage 2 (`/sys/.../boot` = `md1:2`).

## Criterio de éxito / diagnóstico
- **HS1** (`"receive MD_INIT_START_BOOT"`): el MD ejecutó MOLY y habló por CCIF. Alcanzable con un
  módulo mínimo (pasos 1-5 + 7 + handler que loguea) — NO necesita runtime data (eso es HS1→HS2).
- **HS2** (`NORMAL_BOOT_ID`, stage 2) = **M1 COMPLETO** (necesita runtime data, paso 6).
- **Fallo**: timeout 5s → `"Time out at md_boot_stage_0"` → volcar CCIF CON/BUSY/RCHNUM + los 4
  bank4 regs (remap mal = 1er sospechoso) + EMI-MPU.

## Bloqueantes que quedan (por orden de riesgo)
1. **Remap bit-exacto** (H2) — 1er sospechoso si HS1 no llega. Falla observable.
2. **EMI-MPU** (`emi_mpu_set_region_protection`): para bring-up, `ENABLE_EMI_PROTECTION` OFF (memoria
   abierta) o portar región mínima para el carveout. Si va OFF, el MD accede libre.
3. **Reservar 24MB en 1GB** — reduce RAM de pmOS a ~1000MB; verificar que no rompe nada.
4. **check_md_header**: parsear bien el GFH del tail o el loader rechaza el img.
5. **Sin precedente mainline** — unknowns del firmware MOLY.

## Andamiaje: `md1-boot.c` (este dir)
Módulo de bring-up (SCAFFOLD, NO completo): mapea todo, `request_firmware`, remap, CCIF init +
IRQ handler que loguea RX, y el kick — disparado por debugfs. **Requiere `carveout_base=` (param) +
el nodo reserved-memory + kernel recuperable.** NO auto-arranca. Falta: `set_md_runtime` (HS2),
parse GFH robusto, EMI-MPU. Reutiliza `md1-mtcmos.c` (power) y `bank4read.c` (dump).

## Ficheros downstream (fuente de verdad, Pi)
`~/mainline/downstream/drivers/misc/mediatek/dual_ccci/`: `mt6582/src/ccci_platform.c`
(ungate_md1:2428, let_md_go:2488, set_ap/md_smem_remap:2508/2540, load_std_firmware:1556,
boot-slave ioremap:2298-2301), `ccci_md_main.c` (boot_md, set_md_runtime:1373-1656),
`ccci_logical.c` (md_boot_stage:517), `ccci_hw.c` (CCIF), `mt6582/include/ccci_platform_cfg.h`,
`ccci_platform.h` (GFH/WDT), `arch/arm/mach-mt6582/mtk_ccci_helper.c` (reserve, MD1_MEM_SIZE 22M).

*Sesión Mac (Fable 5), 2026-07-13. H0✅ H1✅ H2✅caracterizado. H3 = spec completo; sigue el port CCCI + build/flash.*
