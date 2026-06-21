All critical code facts are confirmed:

1. **RDMA `MEM_START_ADDR = 0x0f00` hardcoded** (line 53, used line 285) — NOT parameterized by `.data`. Confirms the mappings: a code change is mandatory for MT6582 (needs 0x28).
2. **DSI `clk_set_rate(dsi->hs_clk, ...)` at line 695** — unconditional in `mtk_dsi_poweron`. A `fixed-clock` returns `-EINVAL` here, which fails poweron. Confirms the "hs" clock is the hardest single blocker.
3. **`dma_dev = mtk_crtc_dma_dev_get(crtc)`** (line 561) + `drm_gem_prime_import_dev(..., private->dma_dev)` — the OVL becomes the DMA device; no IOMMU code path exists in this flow, which is exactly what the no-M4U approach needs. With `DMA_CMA` the GEM allocator yields contiguous physical buffers.

I have everything needed. Here is the roadmap.

---

# ROADMAP DE IMPLEMENTACIÓN — Port mtk_drm al MT6582 (krillin)

Reemplaza el hack `mt6582-dispfix.c` (simplefb + fixup one-shot) por el driver DRM real. Objetivo concreto: **flujo de frames por DSI → MUTEX sincroniza el latch con el SOF → backlight controlable sin descuadre + vsync**.

---

## 1. Tabla resumen de componentes

| # | Componente | Veredicto | Esfuerzo | Compatible DT | Cambio de código C |
|---|-----------|-----------|----------|---------------|--------------------|
| 1 | **mmsys** (router clk/reset + lanza `mediatek-drm`) | crear-variante | bajo | `mediatek,mt6582-mmsys` *(o reusar mt2701 + parche)* | Tabla routing propia (OVL_MOUT_EN=**0x030**, no 0x040) + `clk_driver=NULL` |
| 2 | **OVL** (overlay, capa de entrada) @0x14007000 | **reusar** | bajo | `mediatek,mt2701-disp-ovl` | Ninguno |
| 3 | **RDMA** (read-DMA) @0x14008000 | crear-variante | **medio** | `mt6582-disp-rdma`, `mt2701-disp-rdma` | **Parametrizar `MEM_START_ADDR` (0x0f00→0x28)** ⚠ |
| 4 | **COLOR** (color-correction) @0x1400B000 | **reusar** | bajo | `mediatek,mt2701-disp-color` | Ninguno (posible quirk CFG_MAIN si tinta) |
| 5 | **MUTEX** (sync SOF) @0x1400E000 | **reusar** *(o variante `.no_clk`)* | bajo | `mediatek,mt2701-disp-mutex` | Ninguno con fixed-clock stub |
| 6 | **DSI + PHY + panel** @0x1400C000 / @0x10010000 | crear-variante | **ALTO** | `mt2701-dsi` + `mt2701-mipi-tx` + **panel nuevo** | **Driver panel HX8389 nuevo (~250 líneas)** + clk "hs" real ⚠ |
| 7 | **BLS/PWM** (backlight) @0x1400A000 | crear-variante | medio | `mt2701-disp-pwm` *(DT)* | **Variante `bls_debug_mask=0`** (bug colores) + config dither 565 ⚠ |
| 8 | **CRTC/drv + clocks + IOMMU** (capa transversal) | crear-variante | **muy-alto** | — | Path mt6582 en `mtk_drm_drv` + clk "hs" set_rate-able |

**Correcciones de hecho ya validadas** (que el prompt traía mal): DSI base = **0x1400C000** (NO 0x14012000); MIPI-TX = 0x10010000; OVL_MOUT_EN = **0x030**; panel reset = **GPIO112**, power = **VGP2 2.8V** (no VGP1); panel formato = **RGB888** (el OVL/RDMA pueden ir en 565, el DSI emite 888 con PS_SEL=3).

---

## 2. Grafo de dependencias y orden de implementación

```
                    ┌─────────────────────────────────────────────┐
                    │  FASE 0: Recon en el dispositivo (sin build) │
                    │  - IRQ raw de cust/mt_irq.h (OVL/RDMA/DSI…)  │
                    │  - dump nvram/protect_*/md1img defensivo     │
                    └───────────────────┬─────────────────────────┘
                                        │
              ┌─────────────────────────▼─────────────────────────┐
              │  FASE 1: fixed-clocks (todos) + mmsys + MUTEX      │  ← cimientos
              │  mmsys lanza mediatek-drm; mutex se bindea         │
              └─────────────────────────┬─────────────────────────┘
                                        │ (component framework)
              ┌─────────────────────────▼─────────────────────────┐
              │  FASE 2: OVL + RDMA + COLOR                        │  ← pipeline ddp
              │  (reusan/variante; entran al MUTEX por add_comp)   │
              └─────────────────────────┬─────────────────────────┘
                                        │ el CRTC exige un "output"
              ┌─────────────────────────▼─────────────────────────┐
              │  FASE 3: DSI + PHY mipi-tx + panel HX8389  ★MURO★  │  ← el más caro
              │  sin esto el CRTC RECHAZA el path (no hay frames)  │
              └─────────────────────────┬─────────────────────────┘
                                        │
              ┌─────────────────────────▼─────────────────────────┐
              │  FASE 4: CRTC/drv ensambla todo + CMA (no-M4U)     │  ← integración
              │  /dev/dri/card0 = mediatek-drm; primer modeset     │
              └─────────────────────────┬─────────────────────────┘
                                        │ ya hay SOF → latch sincronizado
              ┌─────────────────────────▼─────────────────────────┐
              │  FASE 5: BLS reintegrado como DDP_COMPONENT_BLS    │  ← el objetivo
              │  backlight latcheado por frame, sin descuadre      │
              └─────────────────────────┬─────────────────────────┘
                                        │
              ┌─────────────────────────▼─────────────────────────┐
              │  FASE 6: retirar dispfix + simplefb del chosen     │  ← cleanup
              └───────────────────────────────────────────────────┘
```

**Por qué este orden (relaciones de dependencia duras):**

- **mmsys es la raíz**: su probe hace `platform_device_register_data("mediatek-drm")`. Sin mmsys, NADA se bindea (todo queda en `-EPROBE_DEFER`). Va primero junto con los fixed-clocks (que todos los sub-bloques consumen).
- **MUTEX antes que OVL/RDMA/COLOR**: el `mtk_crtc` obtiene el mutex y hace `add_comp()` de cada componente; conviene tenerlo bindeado temprano. Con `.no_clk=true` (variante) o fixed-clock stub no bloquea.
- **OVL/RDMA/COLOR son intercambiables en orden entre sí** (todos cuelgan del component framework), pero el **CRTC no se crea hasta que el path completo está presente** — y el path mt2701 termina en **DSI0**.
- **DSI es bloqueante del CRTC**: `mtk_drm_drv` exige DSI0/DPI0 como último componente del path (`ml_mtk_drm_drv.c` L982-996). Aunque OVL/RDMA/COLOR bindeen, **sin DSI no hay `card0`**. Por eso el DSI+panel (el componente más caro) es prerequisito de ver cualquier frame.
- **BLS al final**: necesita que el SOF del DSI exista para latchear. Reintegrarlo antes de tener SOF reproduce exactamente el bug actual ("colores raros" del hito #26).

---

## 3. Bloqueantes críticos y cómo sortearlos

### BLOQUEANTE A — `clk_set_rate("hs")` sobre fixed-clock = `-EINVAL` ★ el más insidioso
**Confirmado en código** (`ml_mtk_dsi.c` L695): `mtk_dsi_poweron()` hace `clk_set_rate(dsi->hs_clk, dsi->data_rate)` **incondicionalmente**. Un `compatible="fixed-clock"` devuelve `-EINVAL` → poweron falla → DSI no arranca → CRTC sin frames. Los otros dos relojes del DSI ("engine", "digital") sí pueden ser fixed (solo `clk_prepare_enable`, que es no-op).

**Sortear (en orden de preferencia):**
1. **Que el "hs" venga del PHY mt2701-mipi-tx** (la solución correcta y la que asume el diseño): el PHY implementa `mtk_mipi_tx_pll_ops` con `.set_rate`/`.round_rate` reales calculando PCW desde su ref 26MHz. Referenciar `clocks = <…>, <…>, <&mipi_tx0>` y `clock-output-names="mipi_tx0_pll"`. Esto **resuelve A y la dependencia del PHY a la vez**. Requiere `CONFIG_PHY_MTK_MIPI_DSI=y` + `CONFIG_COMMON_CLK=y`.
2. **Plan B si el PLL no engancha** (porque el LK ya dejó el D-PHY configurado y reprogramarlo glitchea): parchear `mtk_dsi_poweron` para saltar `clk_set_rate` la primera vez (heredar estado del LK) y solo arrancar el engine. 3 líneas con un flag.
3. **Plan C**: registrar un `clk_hw` a medida con `.set_rate` no-op que devuelva 0 (acepta cualquier rate, no programa nada). Funciona si confiamos en que el LK dejó el rate correcto.

### BLOQUEANTE B — RDMA `MEM_START_ADDR` hardcoded a 0x0f00 ⚠
**Confirmado** (`ml_mtk_disp_rdma.c` L53 `#define`, usado L285): NO está parametrizado por `.data`. En modo memory-input el RDMA escribe la física del FB a 0xf00 → lee basura → **pantalla negra silenciosa**. (El MT6582 lo quiere en **0x28**.)
**Sortear**: añadir campo `mem_start_addr_reg` a `struct mtk_disp_rdma_data`, inicializar a 0x0f00 en los SoCs existentes y 0x28 en `mt6582_rdma_driver_data`. Reemplazar el `#define` por `priv->data->mem_start_addr_reg` en `mtk_rdma_layer_config`. **Es el único cambio de código verdaderamente obligatorio del pipeline** (junto con el panel).

### BLOQUEANTE C — IOMMU/M4U ausente → ¿el OVL lee el FB?
**Confirmado favorable** (`ml_mtk_drm_drv.c` L561): el `dma_dev` del DRM **es el OVL** (`mtk_crtc_dma_dev_get`), y el GEM hace `drm_gem_prime_import_dev(…, private->dma_dev)`. **No hay path de scatter-gather/IOMMU en este flujo.** El hack ya demuestra que el OVL lee física directa (0xBF400000).
**Sortear**: `CONFIG_MTK_IOMMU=n`, **NO declarar `iommus`/`mediatek,larb`** en ningún nodo, `CONFIG_DMA_CMA=y` + reserva CMA (`cma=8M` o nodo `linux,cma`). Para la primera prueba, **apuntar el FB inicial a la MISMA región reservada del dispfix** para descartar variables. Riesgo residual: **coherencia de cache** (el A7 es no-coherente) — usar buffers WC/coherent; el panel video-mode auto-refresca, así que un flush al primer frame basta. **Este NO es un muro** — es trabajo conocido del contenedor.

### BLOQUEANTE D — IRQ raw numbers no están en el árbol local
**Confirmado**: `ds_ddp_drv.c` usa los símbolos `MT6582_DISP_*_IRQ_ID` como casos de enum, pero los valores numéricos viven en `cust/mt_irq.h` **en el dispositivo**, no en los archivos locales. `mtk_disp_rdma_probe` y `mtk_disp_ovl_probe` **abortan si `platform_get_irq < 0`**, y el vblank del CRTC se engancha al IRQ del OVL → un número erróneo = page-flips colgados.
**Sortear (Fase 0, en el dispositivo)**: `grep -rhnE 'X_DEFINE_IRQ\(MT6582_DISP_(OVL|RDMA|COLOR|BLS|MUTEX)_IRQ_ID' arch/arm/mach-mt6582/ cust/` → el 2º arg es el raw GIC id → **`GIC_SPI = raw − 32`** (patrón ya confirmado en este equipo: EINT raw145→SPI113, USB0 raw64→SPI32). Anotar polaridad `LEVEL_LOW`. Hasta tenerlos, los nodos llevan placeholders que **NO deben probarse**.

### BLOQUEANTE E — BLS_DEBUG=0x3 rompe colores en cada apply
**Confirmado por comportamiento** (hito #26 + `ddp_bls.c`): `pwm-mtk-disp.c` (data mt2701, sin `has_commit`) hace OR `0x3` en `BLS_DEBUG@0xB0` en cada cambio de brillo → desactiva el doble-buffer → descuadra color. **No se cura solo** dentro del DRM.
**Sortear**: variante `mt6582_disp_pwm_data` con `bls_debug_mask=0` (o flag `.skip_debug`); el latch lo da el MUTEX. Cambio mínimo (~5 líneas + entry en la match table). Además, **alguien** debe escribir el `config_full` RGB565 una vez (dither 565: `DITHER(15)@0xE3C=0x50500001`, `(16)@0xE40=0x50504040`, `(0)@0xE00=0x1` + luminance LUT@0x300) — mtk_drm trata el BLS solo como entrada del MUTEX, NO programa dither/LUT. Reusar provisionalmente el bloque BLS del dispfix hasta validar.

### BLOQUEANTE F (riesgo, no muro) — mmsys intenta registrar clk-driver CCF inexistente
`mtk-mmsys.c` (~L429) hace `platform_device_register_data(clk_driver mt2701)`. Con `clk_driver=NULL` debería ser inofensivo, pero **verificar que `register_data(name=NULL)` no devuelve error en 7.0.12**. Si lo hace: parche defensivo de 3 líneas `if (data->clk_driver) { … }`.

---

## 4. PRIMER PASO concreto y de bajo riesgo (próxima sesión)

**Objetivo de la sesión: lograr que el subsistema mmsys+mutex bindee y lance `mediatek-drm`, SIN tocar el panel ni arriesgar la imagen actual.** Esto valida los cimientos (component framework + fixed-clocks + ausencia de bloqueo CCF) de forma incremental y reversible.

**Pasos accionables:**

1. **(En el dispositivo, 10 min) Recolectar los IRQ raw** del DISP. Es prerequisito de TODO y no requiere build:
   ```
   grep -rhnE 'X_DEFINE_IRQ\(MT6582_DISP_(OVL|RDMA|COLOR|BLS|MUTEX)_IRQ_ID|DSI_IRQ_ID' <árbol-downstream>/
   ```
   Anotar los 6 números, restar 32, guardar para los nodos DT.

2. **(En el dispositivo, defensivo) Dump** de `nvram`/`protect_f`/`protect_s`/`md1img` y **backup del kernel actual** (simpledrm+dispfix = backup #25, ya existe — confirmar que está en los 4 sitios). Red de seguridad antes de tocar el DISP.

3. **DT incremental**: añadir SOLO tres cosas al `mt6582-PI-actual.dts`, **dejando el simplefb y el dispfix activos**:
   - `mmsys: syscon@14000000` (`mediatek,mt6582-mmsys`/`mt2701-mmsys`, `#clock-cells=<1>`).
   - `mutex@1400e000` (`mediatek,mt2701-disp-mutex`) + su fixed-clock stub.
   - Un fixed-clock `mm_dummy_clk` compartido (patrón idéntico a `disp_pwm_clk` que ya funciona, líneas 231-238 del DTS actual).

4. **Kconfig mínimo**: `CONFIG_DRM_MEDIATEK=y`, `CONFIG_MTK_MMSYS=y`, `CONFIG_MTK_MUTEX=y`, `CONFIG_MTK_CMDQ=n`, `CONFIG_MTK_IOMMU=n`. Mantener `clk_ignore_unused`.

5. **Código mínimo**: solo la entrada `mt6582-mmsys` en `of_match_mtk_mmsys[]` con `clk_driver=NULL` + routing table propia (OVL_MOUT_EN=0x030). MUTEX sin tocar.

6. **Bootear con `initcall_debug drm.debug=0x1e`** y **leer dmesg, sin esperar imagen del DRM todavía**. Criterio de éxito de la sesión:
   - `mediatek-drm` aparece y empieza a sondear.
   - `mutex@1400e000` pasa a `bound` (o queda en `-EPROBE_DEFER` esperando a OVL/RDMA/COLOR/DSI — **eso también es éxito**: significa que mmsys+component framework arrancan).
   - **NO** aparece "Failed to get clock" ni cuelga el boot.
   - El simplefb/dispfix sigue pintando (no rompimos nada).

**Por qué es el mejor primer paso**: cero riesgo para la imagen (el hack sigue mandando), valida el eslabón raíz (mmsys → mediatek-drm) y el patrón fixed-clock contra el bloqueante F, y produce un `-EPROBE_DEFER` *limpio* que es la señal verde para añadir OVL/RDMA/COLOR en la siguiente sesión. Si mmsys no lanza `mediatek-drm`, lo sabremos antes de invertir en el panel (lo caro).

---

## 5. Valoración honesta del esfuerzo y viabilidad

### Esfuerzo total
**Medio-alto, pero claramente viable. Estimación: 5-8 sesiones de trabajo iterativo.** El reparto NO es uniforme:

- **El 70% del riesgo y del esfuerzo está concentrado en UN componente: el DSI+PHY+panel (Fase 3).** Todo lo demás (mmsys, OVL, RDMA, COLOR, MUTEX, BLS) es reuso de compatibles mt2701 con, a lo sumo, parches de pocas líneas (`MEM_START_ADDR`, `bls_debug_mask`, routing table). Esos son "bajo/medio" y bien acotados.
- El **driver del panel HX8389 (~250 líneas)** hay que escribirlo desde cero: traducir ~20 comandos init del LK (`init_lcm_registers`) a `mipi_dsi_generic_write` (los `0x39` = generic-long, **no** DCS) / `mipi_dsi_dcs_write` (los `0x11`/`0x29`), `.get_modes` con los porches exactos (vsa3/vbp9/vfp9/hsa8/hbp20/hfp22), reset GPIO112, power VGP2. Es trabajo mecánico pero con muchos detalles que fallan silenciosamente.

### ¿Hay un muro que haga inviable el camino?
**No hay ningún muro arquitectónico.** Las tres cosas que podrían serlo NO lo son:

1. **IOMMU/M4U**: descartado como muro — el código confirma que el OVL es el `dma_dev` y no hay path SG en este flujo; CMA contiguo lo resuelve, y el hack ya prueba que el OVL lee física directa.
2. **Clocks sin CCF**: resuelto con fixed-clocks excepto el "hs" del DSI, que tiene tres salidas (PHY PLL / parche poweron / clk_hw no-op).
3. **Topología**: el MT6582 es **bit-a-bit idéntico al mt2701** en routing, MUTEX MOD bits, offsets de OVL/RDMA/COLOR/MUTEX y generación del DSI — verificado registro a registro en las mapas. mt2701 es la base correcta (no mt8167/mt8173).

**El único "muro blando" es el take-over del panel** (Fase 3, riesgo ALTO): el LK ya dejó el panel auto-refrescando en HS, y el flujo DRM hace poweron completo (reprograma PLL MIPI-TX + reset D-PHY + reenvía init). Esto **glitchea la imagen momentáneamente** — no es un take-over transparente. **Es aceptable** (Phosh repinta), pero si el PLL no re-engancha tras el reset, la pantalla podría quedarse negra hasta el plan B.

### Plan B (si el DSI/panel resulta intratable)
**Camino híbrido pragmático que aún cumple el objetivo real (backlight + vsync):**

- Mantener `simpledrm` para el scanout (ya funciona, Phosh corre sobre él), PERO **levantar SOLO el MUTEX + BLS reales** del DRM stack como drivers independientes, sin el CRTC completo. El MUTEX con SOF=DSI0 (configurado a mano una vez, como hace el dispfix) latcha el BLS por frame.
- O incluso más simple: **driver backlight custom (~80 líneas)** que escriba `PWM_DUTY@0xA0` y re-dispare `MUTEX_EN(id)=1` sincronizado, **sin tocar `BLS_DEBUG`**. Era el plan original del hito #25; desacopla del bug de `pwm-mtk-disp` y da brillo latcheado sin migrar todo el pipeline. **Pierde**: vsync real del KMS y aceleración, pero **gana** el control de brillo sin descuadre con riesgo mínimo.

**Recomendación final**: intentar el camino completo (es viable y da vsync+aceleración), pero **estructurar el trabajo para que el Plan B sea el fallback natural** — empezar por mmsys+mutex (Fase 1, el primer paso de arriba) significa que el MUTEX queda operativo temprano, que es justo lo que el Plan B necesita. Si la Fase 3 (DSI) se atasca, ya se tiene el 80% del Plan B hecho.

**Archivos clave para la próxima sesión** (rutas absolutas):
- DT a editar: `C:\Users\jferr\Desktop\pmos-krillin\mt6582-PI-actual.dts` (patrón fixed-clock en líneas 231-238)
- Hack a desactivar al final: `C:\Users\jferr\Desktop\pmos-krillin\mt6582-dispfix.c`
- Offsets de referencia: `C:\Users\jferr\Desktop\pmos-krillin\ddp_reg.h`
- IRQ enum (símbolos, sin raw): `C:\Users\jferr\Desktop\pmos-krillin\disp-research\ds_ddp_drv.c` (líneas 643/777/862/880/895)
- Código a parchear (RDMA): `ml_mtk_disp_rdma.c` L53/L285 — el `0x0f00` hardcoded
- Código a parchear (DSI hs): `ml_mtk_dsi.c` L695 — el `clk_set_rate` que rompe con fixed-clock
- Confirmación no-IOMMU: `ml_mtk_drm_drv.c` L561 — OVL es el `dma_dev`