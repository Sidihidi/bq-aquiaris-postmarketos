# Display DRM (MT6582 / panel HX8389)

Driver de display **DRM real** (`mtk_drm`) para el krillin: reemplaza el hack
`simplefb` + `mt6582-dispfix.c` por el flujo de frames vía MIPI-DSI al panel
**Himax HX8389** (qHD 540×960, 2 lanes), con el MUTEX latcheando por SOF. Esto
habilita backlight controlable, vsync y la base para la GPU (lima/Mali-400).

**Estado: ✅ completo y verificado en caliente.** Color correcto (la gamma del
panel iba envuelta en `#if 0`), lima/Mali-400 MP2 OK, **Phosh acelerado**
(`phoc` abre `card1` mtk_drm + `renderD128` lima) y autostart robusto que espera
a `/dev/dri/card1`. Pendiente fino: brillo por slider de Phosh.

Esta carpeta es **material de portado** (código de referencia + parches + la
bitácora del hito). Los drivers que acabaron en el árbol del kernel viven en
`../drivers/`; aquí está el "cómo se hizo" y los componentes mainline tocados.

## Documentación detallada

- **`DISP-DRM-CONTINUACION.md`** — estado final + cómo retomar (empieza por aquí).
- **`DISP-DRM-ROADMAP.md`** — fases del portado del DRM y hechos confirmados.
- **`DISP-DRM-componentes.md`** — mapeo por componente (OVL/RDMA/COLOR/MUTEX/DSI…).
- **`DISP-DRM-mapeo-completo.json`** — mapeo completo de registros/componentes
  (datos crudos para consulta).

## `code/` — código de referencia y parches

- **`panel-himax-hx8389.c`** — panel mainline del HX8389 (init traducido del LCM
  downstream; gamma del panel desactivada con `#if 0`). El driver de panel real.
- **`panel_hx8389.c`** — LCM driver **downstream** original (referencia de la
  secuencia de init).
- **`mtk-mmsys.c`**, **`mtk_disp_rdma.c`** — componentes mainline MediaTek de
  referencia para el MT6582.
- **`mtk_drm_drv-mt6582.patch`**, **`mtk_drm_drv-mt6582-no-bls.patch`** — los
  cambios sobre `mtk_drm_drv.c` que hicieron arrancar el DRM (quitar
  `DDP_COMPONENT_BLS` del path, etc.).
- **`mt6582-bq-krillin.dts`** — DTS con el pipeline DSI/DRM del krillin.
- **`mt6582-mfg-power.c`** — poke MTCMOS que enciende el power-domain del GPU
  antes de que lima probe (copia; el activo está en `../drivers/`).
