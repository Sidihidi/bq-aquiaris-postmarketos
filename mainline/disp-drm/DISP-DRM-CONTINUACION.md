# Driver DISP DRM MT6582 (krillin) — ESTADO y CÓMO RETOMAR

> Última actualización: 2026-06-18. Continúa el roadmap de `DISP-DRM-ROADMAP.md`.
> Objetivo: reemplazar el hack `simplefb`+`mt6582-dispfix.c` por el driver DRM real
> (`mtk_drm`) → flujo de frames por DSI → MUTEX latchea con SOF → **backlight
> controlable** + vsync + base GPU.

## ✅✅✅ ESTADO FINAL (2026-06-18) — DISPLAY + COLOR + PHOSH ACELERADO, TODO OK
**Cadena de display COMPLETA y verificada en caliente.** Tras FASE 6 (quitar dispfix+simplefb):
- **Color correcto**: el tinte rojizo era la **gamma del panel** (E0 SETGAMMA + 4×C1 DGC). Solución:
  envolver esos bloques en `#if 0` en `panel-himax-hx8389.c` → el panel usa su gamma por defecto.
- **lima/Mali-400 MP2 OK**: `Initialized lima 1.1.0 for 13010000.gpu` (bus 312 / core 416 MHz),
  gracias al poke MTCMOS del MFG (`mt6582-mfg-power.c`, `subsys_initcall` antes de lima). `renderD128`.
- **Phosh ACELERADO**: `phoc` abre **`card1`** (mtk_drm, scanout) **+ `renderD128`** (lima, render GL).
  Stack vivo: phoc + phosh + squeekboard. `WLR_RENDERER=gles2` en `launch_phosh.sh`.
- **Autostart robusto**: `zzzz-phosh.start` ahora **espera a `/dev/dri/card1` + seatd** (timeout 90s)
  en vez de `sleep 3` ciego → arregla el "primer boot de kernel nuevo no lanza GUI" (crng lento ~60s
  retrasaba udev → phoc arrancaba sin DRM y moría). Tras este fix no hace falta reiniciar.

**Siguiente driver del roadmap: WiFi (CONSYS MT6582 / conn_soc + WMT).**

## 🏆🏆🏆 HITO (2026-06-18, kernel #31, boot-disp6) — EL DRM PINTA EL PANEL
**El driver `mtk_drm` mainline inicializa y escanea el panel HX8389 del MT6582.** Ramoops del boot:
```
probe of 10010000.dsi-phy returned 0                     # PHY mipi-tx OK
component match: ovl, rdma, color, dsi (los 4)
bound 14007000.ovl / 14008000.rdma / 1400b000.color / 1400c000.dsi
[drm] Initialized mediatek 1.0.0 ... on minor 1          # DRM device creado (card1)
[drm] fb1: mediatekdrmfb frame buffer device
probe of 1400c000.dsi.0 returned 0                        # panel hx8389 enganchado
```
Pantalla **AMARILLA** = el OVL→RDMA→COLOR→DSI escaneando de verdad. El boot **siguió** (montó p7,
switch_root a Alpine) — NO cuelga; fue lento (`crng init` 61s, sin entropía → sshd/red tardan).

**Los 4 fixes que lo lograron** (ver `code/mtk_drm_drv-mt6582.patch` + el OF graph en el DT):
1. **OF graph DSI↔panel** (ports/endpoints) → el panel engancha (`mipi_dsi_attach`).
2. **Quitar BLS** del path mt2701 (`mt2701_mtk_ddp_main`).
3. **mt6582-mmsys en `mtk_drm_of_ids`** (+ data) → el master mediatek-drm enlaza (era el -ENODEV).
4. **mt6582-disp-rdma en la tabla de tipos del DRM** → el RDMA entra al path (era "Failed to find comp in ddp table").

**PENDIENTE — afinado (NO muro):**
- **Color (amarillo = R/B/formato):** probar formato del OVL/RDMA o `bus_format` del panel; y sobre
  todo **RETIRAR `mt6582-dispfix.c` + simplefb** (FASE 6) — siguen activos y compiten por el OVL
  (probable causa del amarillo). Con el DRM funcionando ya no hacen falta.
- **Boot lento:** `crng init` 61s → añadir entropía (haveged / `random.trust_cpu` / rng) para que
  sshd y la red suban rápido. (El boot llega a Alpine, solo tarda.)
- **Phosh autostart** ya configurado (`rootfs/zzzz-phosh.start`, jwm/startx deshabilitado).

Recuperación de pantalla mientras se afina: `boot-simpledrm.img`.

---

## ⚠️⚠️ HALLAZGO CLAVE FASE 3 (2026-06-18 tarde, kernels #29/#30) — (resuelto arriba)
Tras escribir el panel + DSI + mipi-tx + DT (boot-disp3/disp4) y flashear:
- ✅ **El panel YA ENGANCHA**: `probe of 1400c000.dsi.0 returned 0`. El `mipi_dsi_attach -ENODEV`
  se arregló añadiendo el **OF graph DSI↔panel** (ports/endpoints; el panel como hijo simple NO basta
  porque `mtk_dsi_host_attach` usa `devm_drm_of_get_bridge`). DSI proba (returned 0), mipi-tx idem.
- ❌ **PERO `mediatek-drm` sigue devolviendo -ENODEV (19) y NO crea card0.** DOS causas reales
  (la "ENODEV esperado" de FASE 1/2 era ESTO, no "faltan componentes"):
  1. **`mediatek,mt6582-mmsys` NO está en `mtk_drm_of_ids[]`** (drivers/gpu/drm/mediatek/mtk_drm_drv.c).
     `mtk_drm_probe` hace `of_id = of_match_node(mtk_drm_of_ids, mmsys_node)` → NULL → `return -ENODEV`.
     **FIX:** añadir `{ .compatible="mediatek,mt6582-mmsys", .data=&mt2701_mmsys_driver_data }` (o variante).
  2. **Este mtk_drm (7.0.12) construye el pipeline por OF GRAPH**, no iterando /soc por compatible
     (`mtk_drm_of_ddp_path_build_one`→`mtk_drm_of_get_ddp_ep_cid` sigue endpoints desde el mmsys).
     El modelo "hijos de /soc" que asumía esta doc/roadmap es de un mtk_drm MÁS VIEJO. → hay que
     **cablear el grafo completo: mmsys→OVL→RDMA→COLOR→DSI→panel** con ports/endpoints en cada nodo.
- **PRÓXIMO** (la rework de FASE 3): (a) meter mt6582-mmsys en `mtk_drm_of_ids`; (b) rehacer el DT con
  el grafo de puertos de TODO el pipeline (no solo DSI↔panel); (c) build+flash. El `panel-himax-hx8389.c`
  ya está bien (compila, engancha). Quitar BLS del path mt2701 quizá sea irrelevante con el modelo de grafo.
- Recuperación de pantalla: `boot-simpledrm.img` (#25, Phosh).

---

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
- **Pi de build**: `ssh cpcd@192.168.0.123` (antes .38; pass `cpcdupct`; sudo NOPASSWD). Kernel en
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
