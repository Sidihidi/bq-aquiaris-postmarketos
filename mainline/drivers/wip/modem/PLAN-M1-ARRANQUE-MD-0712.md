# Plan M1 — arrancar el MD (modem processor) del krillin MT6582 — 0712

> Investigación de planificación (read-only sobre downstream + repo). Reconcilia el NO-GO de
> `FEASIBILITY-MODEM-0707.md` (que evalúa el módem COMPLETO = M3 llamadas/SMS, sigue NO-GO) con
> el arranque del MD solo (M1). **VEREDICTO M1: CONDITIONAL-GO ~2-4 semanas a la señal READY.**
> El arranque del MD es tan autónomo como el WiFi (blob MOLY stock + glue kernel, CERO RIL),
> secure-boot OFF, y el MTCMOS es una secuencia SPM que ya dominamos.

## Arquitectura AP↔MD (verificada, direcciones reales)
- **Driver = `dual_ccci` (CONFIG_MTK_CCCI_EXT)**, NO `eccci`/CLDMA (eso es SoCs nuevos).
  `drivers/misc/mediatek/dual_ccci/` core ~18.6K LOC; glue `mt6582/` ~3.9K.
- **CCIF (mailbox AP↔MD)** — confirmado el "0xF020A000/IRQ132" de la estrategia:
  - AP_CCIF físico **0x1020A000**, MD_CCIF físico **0x1020B000** (`mt_reg_base.h:115,118`).
  - **IRQ CCIF0_AP = 132**, level/active-low → DT `<GIC_SPI 100 IRQ_TYPE_LEVEL_LOW>` (`x_define_irq.h:55`).
  - Regs: CON+0x0 BUSY+0x4 START+0x8 TCHNUM+0xC RCHNUM+0x10 ACK+0x14; TXCHDATA+0x100 RXCHDATA+0x180.
- **Shared-mem = carveout DINÁMICO (no dirección fija)**: 22MB MD + 2MB SMEM = **24MB alineados a 32MB**
  vía `arm_memblock_steal(size, SZ_32M)` (`mtk_ccci_helper.c:193`, `ccci_helper.c:416`). El MD NO asume
  dirección fija; el AP se la comunica por registros de remap + run-time data. `KERN_EMI_BASE=0x80000000`.
- **Remap DRAM (RIESGO #1)**: `AP_BANK4_MAP0/1`, `MD1_BANK4_MAP0/1` en INFRACFG/MCU
  (`set_ap_smem_remap`/`set_md_smem_remap`, `ccci_platform.c:~2500-2600`). Indocumentado; define la
  "vista" de DRAM del MD. Replicar bit-a-bit.

## MTCMOS del MD = SPM puro, ~40 líneas, sin microcódigo (confirma la estrategia)
`ccci_power_on_md()` es NO-OP; el power real está en `let_md_go()→ungate_md1()` (`ccci_platform.c:2488,2426`):
1. `ccci_en_md1_clock()→md_power_on(0)` → subsys `SYS_MD1` (`SPM_MD_PWR_CON`, `mt_clkmgr.c:1136`) →
   **`spm_mtcmos_ctrl_mdsys1(STA_POWER_ON)`** (`mt_spm_mtcmos.c:799`).
2. Deshabilita WDT del MD (`WDT_MD_MODE` en `md1_rgu_base=ioremap(0x20050000)`).
3. **Boot-slave keys**: `Key(0x2019379C)=0x3567C766`, `Vector(0x20190000)=0x0`, `En(0x20195488)=0xA3B66175`.

Secuencia MTCMOS (spm_write/read, físicos) — **mismo patrón que nuestro `drivers/done/spm/mt6582-spm.c`**:
```
SPM_MD_PWR_CON = 0x10006284   PWR_ON(b2)→PWR_ON_S(b3)→wait SPM_PWR_STATUS(_S) bit0
                              →clr PWR_CLK_DIS(b4)→clr PWR_ISO(b1)→set PWR_RST_B(b0)→clr MD_SRAM_PDN(b8)
TOPAXI_PROT_EN  = 0x10001220  clr MD1_PROT_MASK(0x00B8); wait TOPAXI_PROT_STA1(0x10001228) clear
SPM_PWR_STATUS  = 0x1000660c  SPM_PWR_STATUS_S = 0x10006610  MD1_PWR_STA_MASK = (1<<0)
```

## Kernel-vs-blob: autónomo como el WiFi (ángulos escépticos cerrados)
- **¿Necesita RIL/daemon para arrancar?** NO. `boot_md` lo dispara un atributo sysfs tonto
  (`register_ccci_attr_func("boot")`, `ccci_md_main.c:2536`) → `echo 0 > /sys/kernel/ccci/boot`. In-kernel.
- **¿run-time data necesita NVRAM/SIM?** NO. `set_md_runtime` (`:1405`) = pura geometría shared-mem.
  El `filp_open("/data/mdl/mdl_config")` **falla con gracia** → `NORMAL_BOOT_ID`. Sin SIM/calibración.
- **¿Secure boot?** OFF: `ENABLE_MD_IMG_SECURITY_FEATURE` comentado ("disable for bring up");
  `load_std_firmware` (`ccci_platform.c:1556`) = copia RAW (ioremap+read del .img) + `check_md_header`
  (parse GFH, no cripto). Idéntico al modelo WiFi.
- **¿Dirección física fija?** NO (carveout dinámico). **¿DSP aparte?** NO (un solo modem.img).
- **¿Estado PMIC/clocks del LK?** El MD gestiona sus PLLs (MD_TOPSM interno); el AP solo MTCMOS+keys.

## Bloqueantes reales
1. **Remap DRAM (ALTO)** — replicar `*_BANK4_MAP` bit-a-bit; 1er sospechoso si HS1 no llega. Falla observable (timeout 5s vuelca CCIF).
2. **EMI-MPU** (`emi_mpu_set_region_protection`) — para bring-up: portar mínima o `ENABLE_EMI_PROTECTION` OFF (memoria abierta OK para traer-arriba).
3. **Sin precedente mainline** (ningún MT65xx con el MD arrancando) → unknown-unknowns del firmware. Gate temporal.
4. **Carveout 32M-align en DT** — nodo `reserved-memory` 24MB (p.ej. @0xBC000000, bajo ramoops 0xBF300000); sustituir `arm_memblock_steal`.

## PLAN por hitos
- **H0 — extracción+inventario (~2 días, bajo)**: dd de la partición del módem (read-only), confirmar GFH, transcribir regs (ya casi todos aquí).
- **H1 — MTCMOS del MD [PRIMER HITO BARATO Y MEDIBLE] (~3-4 días, bajo)**: añadir dominio `SYS_MD1` a `mt6582-spm.c` (secuencia arriba). Medible SIN CCCI: `SPM_PWR_STATUS` bit0=1 tras power-on. Análogo directo a `SPM_CONN_PWR_CON` (CONNSYS).
- **H2 — carveout + mapeos (~1 sem, medio)**: nodo reserved-memory 24MB/32M-align; ioremaps CCIF/boot-slave/RGU; replicar BANK-remap (bloqueante #1).
- **H3 — port CCCI boot-only + arranque (~1.5-2 sem, medio-alto)**: subset dual_ccci (init/ccci_hw/ccci_md_main boot/ccci_logical IRQ/glue/util loader); `request_firmware` en vez de filp_open; EMI-MPU mínima o OFF; IRQ 132; `let_md_go`; capturar handshake.

## Criterio de éxito M1 (dmesg, sin RIL)
- **HS1**: `"receive MD_INIT_START_BOOT"` → `md_boot_stage=1` (el MD ejecutó MOLY y habló por CCIF).
- **HS2 = M1 COMPLETO**: `"receive NORMAL_BOOT_ID"` → `md_boot_stage=2`. `/sys/.../boot` imprime `md1:2`.
- **Fallo diagnóstico**: timeout 5s → `"Time out at md_boot_stage_0"` + volcado CCIF (remap/MPU/keys mal).

## HW necesario
- H0 (read-only): `ls -l /dev/block/by-name/ | grep -iE 'md1img|modem'` + `dd if=<part> of=/tmp/modem.img` → a la Pi (magic GFH). Confirmar nombre real (`modem_1_wg_n.img`?).
- H1: kernel con dominio MD en SPM → flash → readback `SPM_PWR_STATUS` bit0 (devmem/debugfs).
- H3: firmware en `/lib/firmware/` + captura dmesg por USB/netconsole (`~/krillin-kmsg-stream.sh`).
  Botón de abortar: si en ~1-2 sem de iterar remap/MPU no aparece `MD_INIT_START_BOOT`, escalar/parar (sigue NO-GO para M3).

## Ficheros clave
Downstream (Pi): `dual_ccci/` (core), `dual_ccci/mt6582/src/ccci_platform.c` (glue),
`arch/arm/mach-mt6582/mtk_ccci_helper.c` (mem-reserve), `mt_spm_mtcmos.c:799`, `mt_clkmgr.c:1136,1644`.
Local: `drivers/done/spm/mt6582-spm.c` (base para el dominio MD), `dts/mt6582-bq-krillin.dts:38`
(reserved-memory), `drivers/wip/modem/FEASIBILITY-MODEM-0707.md` (veredicto M3, sigue vigente).

*Investigación 2026-07-12. M1 = CONDITIONAL-GO ~2-4 sem; NO-GO de 0707 vigente SOLO para M3 (llamadas).*
