# Driver DISP DRM MT6582 (krillin) — ESTADO y CÓMO RETOMAR

> Última actualización: 2026-06-18. Continúa el roadmap de `DISP-DRM-ROADMAP.md`.
> Objetivo: reemplazar el hack `simplefb`+`mt6582-dispfix.c` por el driver DRM real
> (`mtk_drm`) → flujo de frames por DSI → MUTEX latchea con SOF → **backlight
> controlable** + vsync + base GPU.

## ESTADO ACTUAL
- ✅✅ **FASE 2 VERIFICADA EN HW (2026-06-18, kernel #28, `boot-disp2.img` flasheada):**
  `probe of 14007000.ovl/14008000.rdma/1400b000.color returned 0` los tres. `mediatek-drm`
  sigue en 19 (ENODEV, esperado sin DSI). card0+fb0 intactos, no rompió nada.
- ✅ **Driver del panel mainline ESCRITO y COMPILA** — `drivers/gpu/drm/panel/panel-himax-hx8389.c`
  (repo `mainline/disp-drm/code/panel-himax-hx8389.c`, ~260 líneas; traduce las 16 cmds init del
  LCM downstream, modo 540x960, 2 lanes RGB888 video-sync-pulse, reset GPIO112, power VGP2).
  `CONFIG_DRM_PANEL_HIMAX_HX8389=y` + `CONFIG_PHY_MTK_MIPI_DSI=y` ya en el .config. Kconfig+Makefile hechos.
  Compatible DT: **`truly,hx8389-qhd`**. **FALTA: nodos DT (VGP2 + mipi-tx + DSI + panel) → build → flash.**
- (histórico) FASE 1 (kernel #27): mmsys+mutex bindean, mediatek-drm sondea.

---
- ✅✅ **FASE 1 COMPLETADA y VERIFICADA EN HW** (kernel #27, `boot-disp1.img`):
  `mmsys` (mt6582-mmsys) + `mutex` bindean, `mediatek-drm` sondea. dmesg:
  `probe of 14000000.syscon returned 0`, `1400e000.mutex returned 0`,
  `mediatek-drm.0.auto returned 19` (=ENODEV, ESPERADO sin componentes).
  NO rompió la imagen: dispfix + simplefb + `/dev/dri/card0` + Phosh siguen OK.
- 🔨 **FASE 2 COMPILADA, lista para flashear** (`boot-disp2.img`, ya en `~/mainline/pkg/`):
  añadidos OVL + RDMA + COLOR. **NO flasheada aún** (el teléfono no entró en
  fastboot al final de la sesión). → **PRIMER PASO al retomar: flashear y ver dmesg.**

## CAMBIOS DE CÓDIGO HECHOS (subidos a este repo en `code/`, y en la Pi)
### 1. `drivers/soc/mediatek/mtk-mmsys.c` (FASE 1) — backup Pi `.bak-pre-mt6582`
- `mt6582_mmsys_driver_data { .clk_driver = NULL, .routes = mmsys_default_routing_table, .num_routes = ... }`
- Parche en el probe: `if (mmsys->data->clk_driver) { ...register clk pdev... }` (salta sin CCF)
- of_match: `{ "mediatek,mt6582-mmsys", &mt6582_mmsys_driver_data }`
### 2. `drivers/gpu/drm/mediatek/mtk_disp_rdma.c` (FASE 2) — backup `.bak-pre-mt6582`
- Campo `unsigned int mem_start_addr_reg;` en `struct mtk_disp_rdma_data` (0 → default 0x0f00)
- L285: `rdma->data->mem_start_addr_reg ?: DISP_RDMA_MEM_START_ADDR`
- `mt6582_rdma_driver_data { .fifo_size = SZ_4K, .formats = mt8173_formats, ..., .mem_start_addr_reg = 0x28 }`
- of_match: `{ "mediatek,mt6582-disp-rdma", &mt6582_rdma_driver_data }`
### 3. DT `arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts` (ver `code/mt6582-bq-krillin.dts`)
- FASE 1: `mm_dummy_clk` (fixed-clock 156MHz) + `mmsys: syscon@14000000` (mt6582-mmsys, #clock-cells=1) + `mutex@1400e000` (mt2701-disp-mutex)
- FASE 2: `ovl@14007000` (mt2701-disp-ovl, IRQ **153**) + `rdma@14008000` (mt6582-disp-rdma, IRQ **152**) + `color@1400b000` (mt2701-disp-color, IRQ **156**). Todos LEVEL_LOW, clocks=`<&mm_dummy_clk>`.
- (DT base = el limpio del #25, SIN el backlight/sensores del #26 que rompían colores.)

## CONFIG (`build-krillin/.config`)
`DRM_MEDIATEK=y`, `MTK_MMSYS=y` (compila también mtk-mutex), `MTK_CMDQ=n`, `MTK_IOMMU=n`, `DMA_CMA=y`, `PWM_MTK_DISP=n`.

## IRQ del DISP (confirmados: X_DEFINE_IRQ downstream, **SPI = raw − 32**, LEVEL_LOW)
OVL=153 (raw185), RDMA=152 (184), COLOR=156 (188), BLS=155 (187), DSI=157 (189), MUTEX=161 (193).
(Un 3er conjunto en irqs.h con +160/163 era INCONSISTENTE — descartado.)

## CÓMO el mediatek-drm enumera componentes (clave del DT)
Itera **los hijos de `/soc`** por compatible (`for_each_child_of_node(phandle->parent, ...)` +
`mtk_drm_of_get_ddp_comp_type`). **NO usa grafo de puertos**. → poner OVL/RDMA/COLOR/DSI como
hermanos del mmsys basta. El MUTEX lo busca por `of_alias_get_id(node,"mutex")` (id<0 → lo usa,
no hace falta alias). main_path mt2701 = OVL0→RDMA0→COLOR0→BLS→DSI0.

## ACCESO / ENTORNO (para continuar desde otro PC)
- **Pi de build**: `ssh cpcd@192.168.0.38` (pass `cpcdupct`; sudo NOPASSWD). Kernel en
  `~/mainline/linux-7.0.12`, build dir `build-krillin`. Downstream 3.10 de referencia en
  `~/mainline/downstream/`. boot.img y backups en `~/mainline/pkg/`.
- **Teléfono** (desde la Pi): `ssh root@172.16.42.1` (la Pi tiene 172.16.42.2/24 en usb0;
  re-poner con `sudo ip addr replace 172.16.42.2/24 dev usb0`).
- **Compilar**: `cd ~/mainline/linux-7.0.12 && make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- O=build-krillin -j4 zImage dtbs`. (Cambiar .config: `./scripts/config --file build-krillin/.config --enable X` + `make ... olddefconfig` ANTES.)
- **Empaquetar boot.img**: `cd ~/mainline/pkg; cat <build>/arch/arm/boot/zImage <build>/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dtb > z; python3 mtk_hdr.py KERNEL z z-mtk; abootimg --create boot-X.img -f /tmp/asm/cfg2 -k z-mtk -r /tmp/asm/initrd.img`.
- **Flashear**: teléfono en fastboot (apagar con Power ~10s → Power+Vol↑, **pantalla negra** normal) → `sudo fastboot flash boot boot-X.img` → `sudo fastboot reboot`. **El LK NO soporta `fastboot boot`** (temporal) — usar siempre `flash boot`. Antes: `sudo systemctl stop ModemManager`.
- **Relanzar Phosh tras boot** (no hay autostart): en el teléfono, `nohup su - sxmo -c 'sh /usr/local/bin/launch_phosh.sh' &` (servicios udevd+seatd+runtime dir antes; PIN lockscreen=1234).
- **RECUPERAR** si un kernel rompe el boot: flashear `~/mainline/pkg/boot-simpledrm.img` (=#25 bueno: simplefb+dispfix, sin DRM).

## ⚠️ REGLAS DE SEGURIDAD (no romper)
- En softbrick: reinstalar KitKat 1.5.2 con SP Flash Tool **"Download Only"**, NUNCA "Format All" (borra IMEI). NUNCA restaurar el LK de Lollipop (verifica firma → bootloop); el bueno es KitKat (`~/stock-1.5.2/lk.bin`).
- `mtkclient`: usar `wo <offset> <len> <img>` **NUNCA `wf`** (wf machaca el sector 0/MBR).
- Cargar módulos con `insmod` sin reflashear cuando se pueda (el musb no aguanta transferir el zImage 13MB y reinicia).

## CÓMO RETOMAR — PASOS INMEDIATOS
1. **Flashear `boot-disp2.img`** (FASE 2) y leer dmesg: ¿`ovl@14007000`/`rdma@14008000`/`color@1400b000` "probe returned 0"? ¿el `mediatek-drm` avanza? (criterio: los 3 componentes sondean; el mediatek-drm aún SIN card0 propio hasta tener el DSI — eso es OK).
2. **FASE 3 (la más grande, ~70% del trabajo)**: DSI @**0x1400C000** (`mt2701-dsi`) + PHY mipi-tx @**0x10010000** (`mt2701-mipi-tx`, da el clk "hs") + **driver de panel HX8389 nuevo ~250 líneas** (clonar comandos init de `code/panel_hx8389.c`; el `0x39`=generic-long, `0x11`/`0x29`=DCS; porches vsa3/vbp9/vfp9/hsa8/hbp20/hfp22; reset **GPIO112**, power **VGP2 2.8V**; formato **RGB888** PS_SEL=3). Bloqueante: `clk_set_rate(hs)` en `ml_mtk_dsi.c` L695 → resuelto por el PHY mipi-tx PLL.
3. **FASE 4**: CRTC se crea con el path completo → `card0` del mediatek-drm; reservar CMA (sin M4U). **FASE 5**: reintegrar BLS (backlight latcheado). **FASE 6**: retirar `mt6582-dispfix.c` + simplefb del chosen.

Roadmap detallado: `DISP-DRM-ROADMAP.md` + `DISP-DRM-componentes.md` (este directorio).
