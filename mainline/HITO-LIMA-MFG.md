# lima / Mali-400 MP2 (GPU) — INICIADO, falta encender el MFG (2026-06-18)

Objetivo: aceleración GPU (Mesa lima) para que Phosh pase de pixman (software) a acelerado.
El DRM real (mtk_drm) ya funciona = prerequisito cumplido.

## ESTADO (kernel #34, boot-lima.img)
- ✅ `CONFIG_DRM_LIMA=y` (+ DRM_GEM_SHMEM_HELPER + DRM_SCHED). lima acepta `"arm,mali-400"` directo.
- ✅ Nodo DT `gpu@13010000` añadido (en mt6582-bq-krillin.dts):
  - reg = `<0x13010000 0x10000>` (MALI_BASE downstream 0xF3010000; G3D config en 0x13000000).
  - interrupts = **GIC_SPI 170-175** LEVEL_LOW = gp/gpmmu/pp0/ppmmu0/pp1/ppmmu1
    (downstream `MT_MFG_IRQ_*` = GIC_PRIVATE_SIGNALS+170.. → SPI=N).
  - clocks = fixed-clock stubs bus(312MHz)+core(416MHz), clock-names "bus","core".
- ✅ lima **encuentra el GPU**: `lima 13010000.gpu` proba.
- ❌ **`lima 13010000.gpu: gpmmu dte write test fail` → -5 (EIO)**: el **power-domain del MFG está
  apagado** → los registros del Mali no responden. (El LK no deja el GPU encendido para mainline.)
  El display+Phosh siguen OK (lima falla sin romper nada).

## LO QUE FALTA: encender el MFG **antes** de que lima probe
Secuencia (de downstream `mt_spm_mtcmos.c:spm_mtcmos_ctrl_mfg(STA_POWER_ON)`).
SPM físico = **0x10006000** (downstream SPM_BASE 0xF0006000). Regs:
- `SPM_POWERON_CONFIG_SET` = 0x10006000  (unlock)
- `SPM_MFG_PWR_CON`        = 0x10006214
- `SPM_PWR_STATUS`         = 0x1000660c   (bit4 = MFG_PWR_STA_MASK)
- `SPM_PWR_STATUS_S`       = 0x10006610
Bits de SPM_MFG_PWR_CON: PWR_RST_B=1<<0, PWR_ISO=1<<1, PWR_ON=1<<2, PWR_ON_S=1<<3,
PWR_CLK_DIS=1<<4, MFG_SRAM_ACK=1<<12, SRAM_PDN=(confirmar bit, ~1<<8). Unlock key=`0x0b160001`.

Power-ON:
1. write 0x0b160001 -> 0x10006000   (desbloquea escrituras SPM)
2. MFG_PWR_CON |= PWR_ON; |= PWR_ON_S; esperar (PWR_STATUS & 1<<4) && (PWR_STATUS_S & 1<<4)
3. MFG_PWR_CON &= ~PWR_CLK_DIS; &= ~PWR_ISO; |= PWR_RST_B; &= ~SRAM_PDN; esperar !(MFG_PWR_CON & 1<<12)
(Además: revisar el **clock del MFG** en downstream `mt_clkmgr.c` — quizá haya que ungate un clock G3D/MFG aparte del power-domain.)

## CÓMO IMPLEMENTARLO (2 opciones)
- **A (rápido para probar):** `CONFIG_DRM_LIMA=m`, un script userspace con `devmem`/poke que haga
  la secuencia SPM, y luego `modprobe lima`. Si el GPU responde → permanente con un init script.
- **B (limpio):** poke del MFG en el kernel **antes** de lima — early_initcall o un mini
  power-domain (genpd) `mediatek,mt6582-mfg-power` que lima referencie con `power-domains`.
  (El scpsys mainline `mtk-pm-domains` no cubre mt6582 → variante mínima o el poke.)

Tras encender MFG: esperar `lima ... mali-400 ... gp ... pp` OK + `/dev/dri/renderD128`; probar
GL con `eglinfo`/`glmark2-es2-wayland` bajo Phosh.

## Recuperación / pendientes
- El display+Phosh funcionan en boot-disp7/boot-lima (card0=mediatek-drm). Recuperación: boot-simpledrm.img.
- **Color rojizo leve** del display: pendiente (probable formato OVL RGB_SWAP/BYTE_SWAP en mtk_disp_ovl.c, o gamma del panel) — necesita iteración visual.
