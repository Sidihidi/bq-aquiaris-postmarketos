# Mapeo por componente -- Driver DISP DRM MT6582 (krillin)

> Workflow wf_b890ed35. Acompana a DISP-DRM-ROADMAP.md


## mmsys (mtk-mmsys: router de clk/reset del DISPSYS + routing table OVL->RDMA->COLOR->BLS->DSI). Es el nodo RAIZ: su probe registra el controlador de reset, lanza el platform_device del clk-driver y el platform_device "mediatek-drm" (del que cuelgan OVL/RDMA/COLOR/DSI vias component framework). Archivo objetivo: drivers/soc/mediatek/mtk-mmsys.c (+ mtk-mmsys.h).

**Veredicto:** crear-variante-mt6582 | **Esfuerzo:** bajo

**Compatible DT:** "mediatek,mt6582-mmsys", "syscon". Driver code = mtk-mmsys.c SIN cambios; SOLO se anade una entrada nueva en of_match_mtk_mmsys[] -> &mt6582_mmsys_driver_data y una mt6582_mmsys_routing_table propia (NO reusar mt2701 ni mmsys_default_routing_table: ver offsets). El compatible mt2701-mmsys NO sirve tal cual porque su routing table escribe offsets equivocados (0x040 en vez de 0x030) para el MT6582.

**Offsets/diferencias:** TOPOLOGIA = IDENTICA a mt2701, por eso es la base correcta (NO mt8167/mt8173):
- MT6582 main path (ds_ddp_path.c:1359, MUTEX_MOD=(1<<3)|(1<<10)|(1<<7)|(1<<9)=0x688 = OVL,RDMA,COLOR,BLS; OUT_SEL=0->DSI en path.c:1400) == mainline mt2701_mtk_ddp_main (ml_mtk_drm_drv.c:61-67: OVL0,RDMA0,COLOR0,BLS,DSI0). Mismo orden Y comparte el componente BLS (especifico de esta gen ~2014). mt8167/mt8173 ponen COLOR ANTES de RDMA = orden distinto, descartados.

REGISTROS DE ROUTING del MT6582 (ddp_reg.h base MMSYS_CONFIG=0x14000000):
- DISP_REG_CONFIG_DISP_OVL_MOUT_EN = offset 0x030 (ddp_reg.h:143). bit0=OVL->RDMA (path.c:1398), bit1=OVL->WDMA0 (decouple). Para el path normal: escribir 0x1.
- DISP_REG_CONFIG_DISP_OUT_SEL = offset 0x04c (ddp_reg.h:150). 0=salida a DSI (path.c:1400), 1=salida a DPI0. Para DSI: escribir 0x0 (=valor por defecto; en mainline el campo BLS->DSI usa OUT_SEL).
- NO existe RDMA0_SOUT_EN ni DSI_SEL en el MT6582: el tramo RDMA->COLOR->BLS->DSI es cableado fijo; quien esta en el path lo decide MUTEX_MOD, no registros MOUT intermedios.

DIFERENCIAS DE OFFSET vs mainline (mtk_drm_ddp.c viejo / mmsys_default_routing_table, confirmado por WebFetch):
- mainline DISP_OVL0_MOUT_EN = 0x040  -> MT6582 = 0x030  (DISTINTO, -0x10)
- mainline OUT_SEL = 0x04c            -> MT6582 = 0x04c  (IGUAL)
- mainline DSI_SEL = 0x050            -> en MT6582 0x050 = MMSYS_CG_SET0 (NO existe DSI_SEL)
=> por eso hace falta routing table propia aunque el .c sea reutilizable.

Bits de valor (reusables de mainline): OVL_MOUT_EN_RDMA=0x1, OUT_SEL DSI=0x0.

RESET: SW_RST_B = offset 0x138 (ddp_reg.h:163, ds_ddp_drv.c:2468). Igual semantica que mainline MMSYS_SW0_RST_B (deassert=set bit). PERO se deja num_resets=0 como mt2701 (no registrar rcdev): los bloques ya los enciende LK y los drivers OVL/RDMA/COLOR mt2701 no piden reset por el rcdev del mmsys.

DSI: CORRECCION al prompt -> DSI base = 0x1400C000 (ddp_reg.h:16), NO 0x14012000; DPI=0x1400D000.

**Nodo DT:**
```
/* Stub de clk para el clk-driver que el mmsys intenta registrar (sin CCF).
   El probe hace platform_device_register_data(clk_driver); con clk_driver=NULL
   no registra nada -> usamos data->clk_driver = NULL en mt6582_mmsys_driver_data,
   y el DISPSYS lo deja encendido el LK + clk_ignore_unused. NO se necesitan
   clocks en el nodo mmsys (no los consume el driver). */
&{/soc} {
    mmsys: syscon@14000000 {
        compatible = "mediatek,mt6582-mmsys", "syscon";
        reg = <0x14000000 0x1000>;
        #clock-cells = <1>;   /* presente por compat de binding; sin CCF real */
        /* sin power-domains (no hay scpsys), sin clocks (los pone LK) */
        status = "okay";
    };
};

/* El probe del mmsys hace platform_device_register_data("mediatek-drm"),
   asi que NO se declara un nodo "mediatek-drm" en DT: lo crea el mmsys.
   Los componentes (ovl@14007000, rdma@14008000, color@1400b000, dsi@1400c000)
   SI van como nodos hermanos en &/soc con sus compatibles mt2701-disp-* y
   son enganchados por el driver mediatek-drm via component framework.
   El nodo disp_pwm@1400a000 (BLS, mt2701-disp-pwm) ya existe en el DT actual. */
```

**Kconfig:** CONFIG_MTK_MMSYS=y  (drivers/soc/mediatek/Kconfig: "MediaTek MMSYS Support"; selecciona MTK_CMDQ opcionalmente, no requerido). 
Necesario tambien para que el conjunto enlace: CONFIG_DRM_MEDIATEK=y (mediatek-drm), y el clk-driver del mmsys se evita poniendo clk_driver=NULL (no hace falta CONFIG_COMMON_CLK_MT2701_MMSYS ni similar). 
Mantener: clk_ignore_unused en bootargs (ya en uso). CONFIG_RESET_CONTROLLER no es estrictamente necesario si num_resets=0, pero dejarlo =y no estorba.

**Clocks:** El driver mtk-mmsys NO consume clocks del nodo DT directamente; lo unico relacionado es que en probe llama platform_device_register_data(dev, data->clk_driver, ...) para instanciar el clk provider del MM. SOLUCION sin CCF: poner .clk_driver = NULL en mt6582_mmsys_driver_data. El codigo (linea ~429) hace platform_device_register_data con name=NULL -> registra un pdev anonimo inofensivo (o se puede guardar con guard); en la practica con clk_driver=NULL no se crea ningun driver de reloj y el DISPSYS sigue alimentado porque el LK dejo los CG (clock gates) MMSYS_CG_CON0/CON1 abiertos y usamos clk_ignore_unused. Los relojes que SI necesitan los sub-bloques (OVL/RDMA/PWM) se resuelven en SUS nodos con fixed-clock stubs (patron ya validado: disp_pwm_clk/disp_pwm_mm_clk en el DT actual, lineas 231-238). El mmsys en si: cero clocks.

**Dependencias:** - mediatek-drm (DRM_MEDIATEK): lo lanza el propio probe del mmsys; sin el, no hay pipeline. Es el consumidor de la routing table via mtk_mmsys_ddp_connect/disconnect.
- Sub-componentes que deben existir como nodos DT y tener driver: OVL (mt2701-disp-ovl @0x14007000), RDMA (mt2701-disp-rdma @0x14008000), COLOR (mt2701-disp-color @0x1400b000), DSI (mt2701-dsi @0x1400c000) + phy MIPI, MUTEX (mt2701-disp-mutex @0x1400e000), BLS/PWM (mt2701-disp-pwm @0x1400a000, ya en DT).
- MUTEX (mtk-mutex) es clave: el latcheo de frame/SOF que da el backlight sincronizado depende del mutex; el mmsys solo define el routing, el mutex arma el path real. Coordinar con el componente 'mutex'.
- fixed-clock stubs en los sub-nodos (no en mmsys).
- Quitar el hack mt6582-dispfix.c (late_initcall) una vez el path DRM arranque: ambos escriben OVL/MUTEX y entrarian en conflicto.

**Riesgos:** 1) RIESGO PRINCIPAL: el binding moderno espera que el mmsys instancie el clk-driver; con clk_driver=NULL hay que verificar que platform_device_register_data(name=NULL) no devuelve error en esta version del kernel (7.0.12). Si lo hace, parchear el probe para saltar el registro cuando clk_driver==NULL (3 lineas: if(data->clk_driver) {...}).
2) Routing table: si se reusa mmsys_default_routing_table por error, escribira 0x040 (que en MT6582 es DISP_OVL_MOUT_EN... NO: 0x040 en MT6582 = MDP_TDSHP_SEL) -> rompe el path silenciosamente. Hay que usar 0x030. Verificar con devmem que OVL_MOUT_EN@0x14000030=0x1 tras connect.
3) M4U/IOMMU: el OVL leera el FB FISICO 0xBF400000 directo (sin M4U). El mediatek-drm mainline puede exigir iommus/larb en el nodo OVL; hay que stubbearlo o usar el path 'no-iommu' (el OVL del hack ya prueba que funciona sin M4U). Esto afecta al componente OVL, no al mmsys, pero el mmsys no debe declarar iommus.
4) Conflicto con dispfix: si ambos corren, doble programacion de MUTEX_MOD/EN -> parpadeo o cuelgue. Desactivar dispfix al activar DRM.
5) DSI base: si alguien copia 0x14012000 del prompt, el DSI no responde. Es 0x1400C000.
6) reset: mt2701 usa num_resets=0; si se pone >0 sin rcdev de verdad, los drivers que pidan reset fallaran -EPROBE_DEFER. Mantener 0.

**Pasos:**

1. 1. En drivers/soc/mediatek/mtk-mmsys.c anadir: static const struct mtk_mmsys_routes mt6582_mmsys_routing_table[] = { { DDP_COMPONENT_OVL0, DDP_COMPONENT_RDMA0, 0x030 /*OVL_MOUT_EN*/, 0x1 /*mask*/, 0x1 /*OVL->RDMA*/ }, { DDP_COMPONENT_BLS, DDP_COMPONENT_DSI0, 0x04c /*OUT_SEL*/, 0x1, 0x0 /*->DSI*/ } };  (RDMA->COLOR->BLS son cableados; no llevan entrada). Revisar el enum DDP_COMPONENT_BLS exista en mtk_drm header de 7.0.12 (existe para mt2701).
2. 2. Anadir struct mtk_mmsys_driver_data mt6582_mmsys_driver_data = { .clk_driver = NULL, .routes = mt6582_mmsys_routing_table, .num_routes = ARRAY_SIZE(...), .num_resets = 0 };
3. 3. Anadir a of_match_mtk_mmsys[]: { .compatible = "mediatek,mt6582-mmsys", .data = &mt6582_mmsys_driver_data },
4. 4. (Defensivo) En mtk_mmsys_probe envolver el registro del clk pdev: if (mmsys->data->clk_driver) { clks = platform_device_register_data(... clk_driver ...); ... } else mmsys->clks_pdev = NULL; y en remove chequear NULL. Asi clk_driver=NULL no rompe.
5. 5. Verificar mtk_mmsys_ddp_connect: itera data->routes comparando from_comp/to_comp; nuestra tabla con OVL0->RDMA0 y BLS->DSI0 basta para que el mediatek-drm arme el path mt2701 (que pide OVL0,RDMA0,COLOR0,BLS,DSI0).
6. 6. DT: anadir nodo syscon@14000000 (compatible mediatek,mt6582-mmsys,syscon; reg 0x14000000 0x1000; #clock-cells=<1>). NO anadir nodo mediatek-drm (lo crea el probe). Asegurar nodos hermanos ovl@14007000/rdma@14008000/color@1400b000/dsi@1400c000/mutex@1400e000 con compatibles mt2701-disp-*.
7. 7. Kconfig: CONFIG_MTK_MMSYS=y, CONFIG_DRM_MEDIATEK=y; mantener clk_ignore_unused.
8. 8. Desactivar mt6582-dispfix.c (quitar del Makefile o gate por !DRM) para evitar doble programacion de OVL/MUTEX.
9. 9. Boot + verificar: dmesg 'mediatek-drm' bind OK; devmem 0x14000030 ==0x1 (OVL->RDMA) y 0x1400004c ==0x0 (OUT_SEL DSI) tras modeset; /dev/dri/card0 pasa a ser el mediatek-drm (no simpledrm).
10. 10. Una vez el path corre por DRM, el MUTEX sincroniza SOF -> el backlight (disp_pwm) ya latchea por frame: validar cambio de brillo en caliente sin descuadre de color.

---


## OVL (DISP overlay) — MT6582 @ físico 0x14007000 (virt downstream 0xF4007000). Capa de entrada del pipeline DISP del krillin. Es el bloque que el dispfix.c ya reprograma a RGB565 (L3) y el que pinta hoy.

**Veredicto:** reusar-compatible-existente | **Esfuerzo:** bajo

**Compatible DT:** mediatek,mt2701-disp-ovl

**Offsets/diferencias:** VEREDICTO PROBADO REGISTRO A REGISTRO (ddp_reg.h L229-336 + inc_ddp_ovl.h vs ml_mtk_disp_ovl.c). El mainline parametriza la capa N como CON(n)=0x30+0x20n, SRC_SIZE(n)=0x38+0x20n, OFFSET(n)=0x3c+0x20n, PITCH(n)=0x44+0x20n, ADDR(ovl,n)=data->addr+0x20n=0x40+0x20n, RDMA_CTRL(n)=0xc0+0x20n, RDMA_GMC(n)=0xc8+0x20n. Contra los offsets FIJOS del downstream:
  REGISTROS COMUNES (idénticos): STA 0x00, INTEN 0x04, INTSTA 0x08, EN 0x0C, RST 0x14, ROI_SIZE 0x20, DATAPATH_CON 0x24, ROI_BGCLR 0x28, SRC_CON 0x2C.
  CAPA L0 (idx0): CON ds 0x30 == 0x30+0; SRC_SIZE ds 0x38 == 0x38; OFFSET ds 0x3C == 0x3c; ADDR ds 0x40 == 0x40+0  OK ; PITCH ds 0x44 == 0x44.
  CAPA L3 (idx3, la que usa el FB downstream): CON ds 0x90 == 0x30+0x60; SRC_SIZE ds 0x98 == 0x38+0x60; OFFSET ds 0x9C == 0x3c+0x60; ADDR ds 0xA0 == 0x40+0x60  OK ; PITCH ds 0xA4 == 0x44+0x60; RDMA3_CTRL ds 0x120 == 0xc0+0x60; RDMA3_MEM_GMC_SETTING ds 0x128 == 0xc8+0x60.
  -> La clave es ADDR(n)=0x40+0x20n: COINCIDE EXACTO con DISP_REG_OVL_ADDR_MT2701. Por eso mt2701 es el match.

FORMATO RGB565 (la causa del tinte amarillo) — PRUEBA DURA: el dispfix escribe L3_CON=0x400010ff. Decodificado con los campos REALES del MT6582 (inc_ddp_ovl.h: L0_CON_FLD_CLRFMT=REG_FLD(4,12), ALPHA_EN=REG_FLD(1,8), ALPHA=REG_FLD(8,0), RGB_SWAP=REG_FLD(1,25), SRCKEY_EN=REG_FLD(1,30)): bits[15:12]=0x1=CLRFMT (RGB565 en codificación MT2701), bit8=AEN, bits[7:0]=0xff alpha, bit30=SRCKEY_EN. El driver mt2701 produce para DRM_FORMAT_RGB565 -> OVL_CON_CLRFMT_RGB565(ovl) = OVL_CON_CLRFMT_RGB = (1<<12) (porque fmt_rgb565_is_0=false), MÁS OVL_CON_AEN(BIT8) y alpha 0xff. Es decir, el driver real generará el MISMO patrón de CLRFMT que el hack -> RGB565 correcto SIN tinte, sin parchear nada. (Confirma además que el bit-layout del OVL_CON del MT6582 == OVL_CON del mt2701/mt8173: CLRFMT<<12, AEN bit8, ALPHA 0xff, RGB_SWAP bit25, BYTE_SWAP bit24.)

DIFERENCIAS DE HARDWARE = NULAS para lo que el driver toca: el OVL MT6582 es de la misma generación 2014 que el MT2701. NO tiene AFBC (mt2701 data: supports_afbc=false, no toca DISP_REG_OVL_DATAPATH_CON AFBC bits), NO tiene CLRFMT_EXT/10-bit (supports_clrfmt_ext=false, no toca 0x2d0), NO tiene HDR addr. layer_nr=4 (L0-L3 presentes, correcto). gmc_bits=8 -> mtk_ovl_layer_on escribe RDMA_GMC(n) un valor de threshold de 8-bit; el downstream solo fuerza 0x00f00040 a 1080p, a qHD el valor calculado del driver basta (sin underflow).

DIFERENCIA REAL Y ÚNICA = DIRECCIONAMIENTO DE MEMORIA (IOMMU vs físico): mtk_ovl_layer_config escribe DISP_REG_OVL_ADDR(ovl,idx)=pending->addr y activa el RDMA interno del OVL (RDMA_CTRL=1, GMC, SRC_CON bit-idx). En mt2701/mt8167 ese ADDR es una IOVA del M4U/IOMMU+larb. En el krillin NO hay M4U y el dispfix demuestra que escribiendo la FÍSICA directa (0xBF400000) el OVL lee DRAM sin traducir y pinta. Por tanto el nodo OVL NO debe declarar 'iommus' ni 'mediatek,larb'; sin esas props mtk_drm pasa el dma_addr tal cual y, si el GEM entrega CMA contiguo, el OVL lo lee igual que el hack.

OTRA DIFERENCIA MENOR: el DRM asignará la plane primaria al idx 0 (L0_CON@0x30, SRC_CON bit0), no al L3 que usa el downstream. Es funcionalmente equivalente (las 4 capas son simétricas), solo cambia qué bit de SRC_CON y qué RDMA interno se usan.

**Nodo DT:**
```
/* fixed-clock stub para el OVL — mismo patrón ya validado en este equipo (disp_pwm/msdc/i2c). */
/ {
    mm_disp_ovl_clk: mm-disp-ovl-clk {
        compatible = "fixed-clock";
        #clock-cells = <0>;
        clock-frequency = <156000000>; /* nominal; el OVL NO lee la freq, clk_prepare_enable sobre fixed-clock es no-op */
    };
};

&{/soc} {
    ovl0: ovl@14007000 {
        compatible = "mediatek,mt2701-disp-ovl";
        reg = <0x14007000 0x1000>;
        interrupts = <GIC_SPI 132 IRQ_TYPE_LEVEL_LOW>; /* PLACEHOLDER — VERIFICAR EN EL DISPOSITIVO (ver riesgos): MT6582_DISP_OVL_IRQ_ID; el handler solo limpia INTSTA y dispara vblank; polaridad LEVEL_LOW como el resto del DISP del SoC. mtk_disp_ovl_probe ABORTA si platform_get_irq<0, así que el nº DEBE ser correcto. */
        clocks = <&mm_disp_ovl_clk>;
        /* SIN iommus, SIN mediatek,larb -> lee dirección física directa como el dispfix (0xBF400000). */
        /* SIN power-domains -> no hay scpsys/genpd; el LK deja DISPSYS encendido y pm_runtime queda no-op. */
    };
};

/* IMPRESCINDIBLE: el nodo NO se prueba solo. mtk_disp_ovl_probe hace component_add() y solo bind-ea
 * cuando el master mtk-drm sondea. Hace falta el contenedor: nodo mmsys (mediatek,mt2701-mmsys @0x14000000)
 * con el grafo 'ports' (OVL0->...->DSI0), + mutex + rdma0 + color0 + bls(disp_pwm) + dsi0. Sin ellos el
 * OVL queda en -EPROBE_DEFER y no hace nada. Coordinar con los otros componentes del port. */
```

**Kconfig:** CONFIG_DRM=y
CONFIG_DRM_MEDIATEK=y           (compila mtk_disp_ovl.o + crtc + ddp_comp + drm_drv; arrastra MTK_MMSYS y MTK_MUTEX)
CONFIG_MTK_MMSYS=y              (mtk-mmsys.c: routing OVL_MOUT_EN @MMSYS+0x030, sw-reset; es el master/syscon padre)
CONFIG_MTK_CMDQ=n               (NO hay GCE en MT6582; con CMDQ deshabilitado, el #if IS_REACHABLE(CONFIG_MTK_CMDQ) del probe queda fuera y en runtime cmdq_pkt=NULL -> mtk_ddp_write*/mtk_ddp_write_relaxed caen a writel directo. Es justo el camino que queremos: el OVL se programa por CPU igual que el hack)
CONFIG_MTK_IOMMU=n              (clave: sin M4U; el OVL lee físico)
CONFIG_DRM_FBDEV_EMULATION=y    (para /dev/fb0 y consola; opcional si Phosh va directo a card0)
CONFIG_DRM_MEDIATEK_HDMI=n
# bootargs ya lleva clk_ignore_unused (necesario para que el fixed-clock no se apague aunque se considere unused).

**Clocks:** SIN CCF -> exactamente el patrón ya validado en el equipo (msdc/i2c/disp_pwm): UN nodo compatible="fixed-clock" con #clock-cells=<0>, referenciado en clocks=<&mm_disp_ovl_clk>. mtk_disp_ovl_probe hace priv->clk = devm_clk_get(dev, NULL) (UN solo clk, SIN nombre) y mtk_ovl_clk_enable hace clk_prepare_enable(ovl->clk). Sobre un fixed-clock ambas son no-op y devuelven 0, nunca fallan. El LK deja el OVL/DISPSYS encendido y bootargs lleva clk_ignore_unused, así que aunque el fixed-clock se considere 'unused' no se apaga. La clock-frequency es IRRELEVANTE para el OVL (no se lee; el pixel-clock lo pone el DSI). Se puede compartir un único stub MM entre OVL/RDMA/COLOR/MUTEX, o dar uno por bloque; como devm_clk_get pide índice 0 sin nombre, basta cualquier fixed-clock por nodo. NO referenciar relojes de un clk-mt2701-mm (no existe CCF aquí). Ahorro de energía vía gating real = fuera de alcance (requeriría clk-driver).

**Dependencias:** El OVL NO funciona aislado. Cadena (component framework de mtk_drm; path estilo mt2701 = OVL0->RDMA0->COLOR0->BLS->DSI0):
1. MASTER mmsys (mediatek,mt2701-mmsys @0x14000000): device padre + component-master; define el routing OVL_MOUT_EN (DISP_REG_CONFIG_DISP_OVL_MOUT_EN @MMSYS+0x030, existe en ddp_reg.h L143). Sin él, component_add no bind-ea -> OVL en EPROBE_DEFER. CRÍTICO.
2. mtk-mutex (mediatek,mt2701-disp-mutex @0x1400E000): el OVL entra en el MUTEX con MT2701_MUTEX_MOD_DISP_OVL=3 (coincide con el bit3 del MT6582). El MUTEX sincroniza el commit con el SOF del DSI — ESTE es el latch-por-SOF que el dispfix imita a mano (MOD/EN en MUTEX id3) y que el backlight necesita.
3. RDMA0 (lee la salida del OVL), COLOR0, BLS/DISP_PWM (ya hay nodo disp_pwm 'mediatek,mt2701-disp-pwm' @0x1400a000 en el DT actual, reutilizable como DDP_COMPONENT_BLS), DSI0 (@0x1400C000 según ddp_reg.h, NO 0x14012000; panel hx8389) — todos aguas abajo, necesarios para encender el path.
4. mtk_drm_drv: su of_match/tabla de routing del SoC debe enumerar este OVL como DDP_COMPONENT_OVL0 en el path (reusar el routing mt2701 o crear el del MT6582).
5. Allocator GEM contiguo / reserva CMA (por ausencia de M4U; ver riesgos).
En resumen: el OVL es trivial; el trabajo está en levantar mmsys+mutex+rdma+color+dsi y resolver la memoria sin IOMMU.

**Riesgos:** 1) IRQ del OVL = INCERTIDUMBRE PRINCIPAL Y PROBE-BLOCKER. mtk_disp_ovl_probe hace irq=platform_get_irq(pdev,0); if(irq<0) return irq; -> sin un nº válido el OVL NO arranca. Además mtk_crtc engancha el vblank al IRQ del primer componente (el OVL): nº erróneo => sin frame-done => page-flips colgados (regresión vs el hack, que ni usa IRQ). El nº GIC_SPI NO está en los archivos locales (vive en cust/mt_irq.h del downstream en el dispositivo). El valor del DT (GIC_SPI 132) es PLACEHOLDER. VERIFICAR antes de probar con: en el árbol downstream del equipo, grep -rhnE 'X_DEFINE_IRQ\(MT6582_DISP_OVL_IRQ_ID' -> el 2º argumento es el 'raw' GIC id; GIC_SPI = raw - 32 (patrón confirmado: EINT raw145->SPI113, USB0 raw64->SPI32). Anotar polaridad (LEVEL_LOW como el resto del DISP).
2) MEMORIA SIN IOMMU = RIESGO REAL DE QUE NO PINTE. El hack escribe una física fija contigua (0xBF400000). El driver DRM pide buffers vía GEM: si entrega memoria CONTIGUA física (CMA) el OVL la lee (como el hack); pero mtk_drm_gem en mainline asume IOMMU para scatter-gather. Hay que forzar GEM contiguo (CMA) + dma-ranges/coherent mask. Si entrega páginas no contiguas, el OVL lee basura -> pantalla rota. Mitigación: reservar CMA grande, apuntar el primer FB a la MISMA región reservada que ya usa el dispfix; en el peor caso parchear el GEM a dma_alloc_coherent. Es el mayor trabajo oculto del port (del contenedor mtk_drm, no del OVL per se).
3) COHERENCIA CACHE: el A7 del MT6582 es no-coherente; los frames que escribe CPU/GPU deben volcarse antes de que el OVL los lea. El hack no sufre (simplefb/pixman + panel auto-refresco). Con DRM hay que asegurar buffers WC/coherent o flush -> riesgo de tearing/datos rancios.
4) RST en mtk_ovl_config: el driver hace soft-reset (DISP_REG_OVL_RST=1 luego 0) en cada modeset; el downstream OVLReset ADEMÁS hace poll de DISP_REG_OVL_FLOW_CTRL_DBG (&0x3ff ∈ {1,2}). El path mainline NO espera -> normalmente OK, pero si el OVL queda en estado raro tras el LK podría necesitar ese poll. Bajo-medio.
5) layer index: la plane primaria cae en idx0 (SRC_CON bit0), no en L3 como el downstream; cualquier suposición del MUTEX/MOUT debe cubrir la capa usada. Bajo riesgo (capas simétricas).
6) Coexistencia con el hack: mientras se prueba, el late_initcall dispfix también escribe OVL/RDMA/COLOR/BLS/MUTEX. DESACTIVAR el dispfix (y quitar el simple-framebuffer del chosen) cuando el OVL real tome el control, o pelearán por los mismos registros.

**Pasos:**

1. 1. VERIFICAR el nº de IRQ del DISP_OVL del MT6582 (probe-blocker): en el árbol downstream del equipo ejecutar grep -rhnE 'X_DEFINE_IRQ\(MT6582_DISP_OVL_IRQ_ID' arch/arm/mach-mt6582/ (o cust/mt_irq.h); GIC_SPI = (2º arg raw) - 32. Anotar polaridad. Reemplazar el placeholder GIC_SPI 132 del nodo.
2. 2. Añadir al DT el nodo ovl@14007000 con compatible='mediatek,mt2701-disp-ovl', reg=<0x14007000 0x1000>, el IRQ del paso 1, clocks=<&mm_disp_ovl_clk>, y el fixed-clock stub mm_disp_ovl_clk. NO poner iommus, mediatek,larb ni power-domains.
3. 3. Habilitar Kconfig: DRM=y, DRM_MEDIATEK=y, MTK_MMSYS=y, MTK_CMDQ=n, MTK_IOMMU=n. Confirmar que mtk_disp_ovl.c compila sin cmdq (mtk_ddp_write* -> writel directo, cmdq_pkt=NULL).
4. 4. CRÍTICO (contenedor): portar en paralelo los nodos mmsys (mt2701-mmsys @0x14000000) + mutex (mt2701-disp-mutex @0x1400E000) + rdma0 + color0 + reusar disp_pwm como BLS + dsi0 (@0x1400C000), con el grafo 'ports' del path. Sin ellos el OVL queda en EPROBE_DEFER.
5. 5. Resolver memoria sin M4U: reservar región CMA y asegurar que mtk_drm_gem entrega buffers FÍSICOS contiguos (como la 0xBF400000 del hack). Probar primero apuntando el FB inicial a la MISMA región reservada del dispfix para descartar variables.
6. 6. Arranque incremental: cargar con drm.debug; en dmesg comprobar que ovl@14007000 pasa de EPROBE_DEFER a 'bound', que mtk_crtc se crea y card0 aparece. Si el OVL no bind-ea, el problema está en mmsys/ports (paso 4), NO en el OVL.
7. 7. Validar formato: modeset 540x960 y leer (devmem/pmem) OVL L0_CON@0x14007030; debe tener bit12 (CLRFMT=RGB) puesto -> mismo valor que el hack (0x...10..) -> RGB565 sin tinte amarillo.
8. 8. Validar vblank/backlight: con el MUTEX (mt2701-disp-mutex, OVL=mod bit3) sincronizando al SOF del DSI, el commit del OVL queda latcheado por frame -> el DISP_PWM/BLS ya controla brillo (objetivo del port). Probar page-flip (modetest -v) y un fade de backlight.
9. 9. Una vez el path real pinte y dé vblank, retirar mt6582-dispfix.c (late_initcall) y la entrada simple-framebuffer del chosen para evitar dos dueños sobre el OVL.

---


## rdma (DISP_RDMA — Read-DMA, alimenta el pipeline desde el framebuffer en memoria hacia DSI). Driver mainline objetivo: drivers/gpu/drm/mediatek/mtk_disp_rdma.c (ml_mtk_disp_rdma.c). Downstream de referencia: ds_ddp_rdma.c. Base MT6582: virtual 0xF4008000 -> fisico 0x14008000, ventana 0x1000.

**Veredicto:** crear-variante-mt6582 | **Esfuerzo:** medio

**Compatible DT:** USAR "mediatek,mt2701-disp-rdma" como base de comportamiento (fifo_size=SZ_4K coincide EXACTO), PERO crear y declarar tambien "mediatek,mt6582-disp-rdma" porque hay UNA diferencia HW real de offset (MEM_START_ADDR, ver abajo) que NO se puede expresar en DT. Plan recomendado: dual-compatible en el nodo -> compatible = "mediatek,mt6582-disp-rdma", "mediatek,mt2701-disp-rdma"; y anadir en mtk_disp_rdma.c una entry mt6582 (con su propio mtk_disp_rdma_data) + registrar el compatible en ml_mtk_drm_drv.c junto a los otros *-disp-rdma (linea ~797). Si se opta por NO tocar offsets (riesgo, ver riesgos), el mt2701 puro arrancaria pero escribiria la direccion del framebuffer al registro equivocado.

**Offsets/diferencias:** LAYOUT MAYORMENTE IDENTICO entre MT6582 (ddp_reg.h) y mt2701/mt8173 (ml_mtk_disp_rdma.c) — verificado registro a registro:
- INT_ENABLE      MT6582 base+0x00  == mainline DISP_REG_RDMA_INT_ENABLE 0x0000  OK
- INT_STATUS      base+0x04         == 0x0004  OK
- GLOBAL_CON      base+0x10         == DISP_REG_RDMA_GLOBAL_CON 0x0010  OK (bit0=ENGINE_EN, bit1=MODE_MEMORY; downstream usa bits 0x700 para soft-reset/estado)
- SIZE_CON_0      base+0x14         == 0x0014  OK (width 0xfff, bit17 MATRIX_ENABLE, bits23:20 MTX_SEL)
- SIZE_CON_1      base+0x18         == 0x0018  OK (height 0xfffff)
- TARGET_LINE     base+0x1C         == 0x001c  OK
- MEM_CON         base+0x24         == DISP_RDMA_MEM_CON 0x0024  OK (formato bits>>4; RGB565=0x000<<4, SWAP bit8)
- MEM_SRC_PITCH   base+0x2C         == DISP_RDMA_MEM_SRC_PITCH 0x002c  OK
- MEM_GMC_SETTING_0 base+0x30       == DISP_RDMA_MEM_GMC_SETTING_0 0x0030  OK
- FIFO_CON        base+0x40         == DISP_REG_RDMA_FIFO_CON 0x0040  OK (bit31 UNDERFLOW_EN, pseudo_size<<16, threshold)
*** UNICA DIFERENCIA CRITICA ***
- MEM_START_ADDR: MT6582 = base+0x28 (ddp_reg.h L398 DISP_REG_RDMA_MEM_START_ADDR; confirmado por downstream RDMASetAddress/RDMAGetAddress y RDMAConfig L360). Mainline define DISP_RDMA_MEM_START_ADDR = 0x0f00 (ml_mtk_disp_rdma.c L53). En el HW viejo (MT6582/MT2701-era real) la direccion va a 0x28; el 0x0f00 es de SoCs posteriores. => si se usa el codigo mt2701 SIN cambiar la constante, mtk_rdma_layer_config escribe la fisica del FB a 0xf00 y el RDMA nunca lee el buffer. NB: el dispfix actual NI siquiera programa MEM_START_ADDR (el OVL es quien lee 0xBF400000 en este hack OVL->RDMA0_CON), pero con el driver DRM real en modo memory-input el RDMA SI necesita la direccion en 0x28.
OTRAS DIFERENCIAS MENORES (no bloquean, son matiz de implementacion):
- FIFO/threshold: downstream calcula threshold = (width+120)*bpp/16 (acotado a 256) y programa ultra/pre-ultra en MEM_GMC_SETTING_0; mainline ignora ultra y pone threshold = 70% de fifo_size con UNDERFLOW_EN. Para 540x960 RGB565 el calculo mainline es seguro (DSI video-mode auto-refresca, los stalls de ultra no son criticos en un panel command-less). fifo_pseudo_length downstream=256 -> 256*16=4096B = SZ_4K = mt2701_rdma_driver_data.fifo_size EXACTO (confirmacion fuerte del match mt2701).
- GMC: mainline escribe RDMA_MEM_GMC=0x40402020 en GMC_SETTING_0; downstream calcula valor ultra ahi. Conflicto benigno (mismo registro, distinto valor) — el 0x40402020 mainline funciona como GMC generico.
- formats: mt8173_formats (compartido por mt2701) incluye RGB565/XRGB8888/etc; cubre el RGB565 del panel krillin. OK.
- soft-reset: downstream RDMAReset hace handshake con bits 0x700 de GLOBAL_CON; mainline NO resetea RDMA en probe (solo limpia INT). No bloquea pero conviene un reset una vez (ver pasos).
- DISP_INDEX_OFFSET=0 en MT6582 (solo 1 RDMA) — irrelevante, mainline trata cada RDMA como device propio.

**Nodo DT:**
```
rdma0: rdma@14008000 {
	compatible = "mediatek,mt6582-disp-rdma", "mediatek,mt2701-disp-rdma";
	reg = <0x14008000 0x1000>;
	interrupts = <GIC_SPI 192 IRQ_TYPE_LEVEL_LOW>; /* VERIFICAR nº SPI del RDMA en MT6582; si el handshake de vblank no llega, el driver igual hace flip por commit, pero enable_vblank usa RDMA_FRAME_END_INT */
	clocks = <&disp_dummy_clk>;          /* fixed-clock stub, ver clocks */
	power-domains = <&scpsys ...>;       /* OMITIR: no hay scpsys CCF en este port; el LK deja DISPSYS encendido. NO poner power-domains para evitar -EPROBE_DEFER */
	mediatek,gce-client-reg = <...>;     /* OMITIR: no hay CMDQ/GCE en este port; el driver cae a escritura directa (cmdq_pkt=NULL) cuando CONFIG_MTK_CMDQ no esta */
	mediatek,rdma-fifo-size = <4096>;    /* OPCIONAL pero recomendado fijarlo explicito = SZ_4K; si se omite, el .data del compatible mt2701 ya da SZ_4K */
	iommus = <...>;                      /* OMITIR: no hay M4U/IOMMU; el RDMA usa direcciones FISICAS directas (igual que el OVL del hack que lee 0xBF400000). El FB debe estar en memoria reservada contigua y fisicamente direccionable */
};
NOTA: el nodo debe ser hijo/asociado del mmsys (mediatek,mt2701-mmsys @0x14000000) y del mutex (mediatek,mt2701-disp-mutex @0x1400E000) para que se forme el main_path mt2701 (OVL0->COLOR0->...->RDMA0->DSI0). El RDMA es DDP_COMPONENT_RDMA0.
```

**Kconfig:** CONFIG_DRM_MEDIATEK=y (arrastra mtk_disp_rdma.o via el Makefile del dir mediatek; el driver RDMA no tiene Kconfig propio, se compila siempre con DRM_MEDIATEK). 
NO necesita: CONFIG_MTK_CMDQ (dejar SIN -> el driver usa el path #if IS_REACHABLE(CONFIG_MTK_CMDQ) y opera con cmdq_pkt=NULL = escritura directa con writel, justo lo que queremos sin GCE).
NO necesita: CONFIG_MTK_IOMMU / CONFIG_MTK_SMI a nivel RDMA (el RDMA lee fisico; el resto del DRM puede requerir SMI — eso es dependencia del nucleo mtk_drm, no del RDMA en si).
Dependencia transversal: CONFIG_COMMON_CLK + el fixed-clock (CONFIG_COMMON_CLK_... no hace falta vendor; "fixed-clock" esta en drivers/clk/clk-fixed-rate, CONFIG_COMMON_CLK basta) y clk_ignore_unused en bootargs.

**Clocks:** El driver hace devm_clk_get(dev, NULL) (1 solo clk, sin nombre) y clk_prepare_enable en mtk_rdma_clk_enable. SIN CCF real -> declarar un fixed-clock stub y referenciarlo:
disp_dummy_clk: disp-dummy-clk {
	compatible = "fixed-clock";
	#clock-cells = <0>;
	clock-frequency = <200000000>; /* valor nominal; el RDMA no deriva timing de el, el panel DSI video-mode marca el ritmo. Cualquier freq>0 sirve */
};
Y en el nodo rdma: clocks = <&disp_dummy_clk>;
clk_prepare_enable sobre un fixed-clock es no-op exitoso (devuelve 0), asi que mtk_rdma_clk_enable nunca falla. El bloque ya esta encendido por LK, asi que aunque el gate real no se toque, el RDMA funciona. Anadir clk_ignore_unused en cmdline por si algun otro consumidor lo apagara. NOTA: se puede COMPARTIR un unico disp_dummy_clk entre ovl/rdma/color/mutex/dsi (todos piden 1 clk sin nombre) — mas simple que un stub por bloque.

**Dependencias:** 1) mtk-mmsys (mediatek,mt2701-mmsys): genera el main_path mt2701 que INCLUYE DDP_COMPONENT_RDMA0 (ml_mtk-mmsys.c L61-66) y hace el routing OVL->...->RDMA via mmsys regs. SIN el, el RDMA no se ata al CRTC.
2) mtk-mutex (mediatek,mt2701-disp-mutex): el RDMA0 ocupa MOD bit 10 (MT2701_MUTEX_MOD_DISP_RDMA0=10, ml_mtk-mutex.c L295). El mutex sincroniza el latch de registros con el SOF del DSI — ESTO es lo que el hack dispfix NO tiene y por lo que el brillo no se latcha; el RDMA real + mutex lo arreglan.
3) DSI (mediatek,mt2701-dsi) + panel Himax HX8389: el RDMA alimenta al DSI; el SOF del DSI dispara el mutex. Es el consumidor downstream del RDMA en el path.
4) mtk_drm_drv (nucleo): registra mtk_disp_rdma_driver (ml_mtk_drm_drv.c L1289) y trata MTK_DISP_RDMA como componente con .data (L797-806, L1182). El RDMA es un component que se bind-ea al CRTC.
5) OVL/COLOR upstream en el path (componentes previos). El RDMA es el penultimo antes del DSI.
6) clk fixed-stub (arriba). NINGUNA dependencia de IOMMU/CMDQ/scpsys en el RDMA mismo.

**Riesgos:** ALTO — Offset MEM_START_ADDR: si se reutiliza mt2701 puro, la constante 0x0f00 (hardcoded en ml_mtk_disp_rdma.c L53, NO parametrizada por .data) hace que la direccion fisica del FB se escriba al registro equivocado y el RDMA lea basura/negro. Mitigacion: variante mt6582 que use 0x28. Requiere parametrizar DISP_RDMA_MEM_START_ADDR en struct mtk_disp_rdma_data (el upstream actual NO lo tiene -> hay que anadir un campo, p.ej. .mem_start_addr_reg). Es el cambio de codigo principal de este componente.
MEDIO — IRQ/vblank: el numero SPI del RDMA en MT6582 hay que verificarlo (no esta en los .c de referencia). Si es incorrecto, devm_request_irq falla o no llegan FRAME_END; mtk_rdma_enable_vblank usa RDMA_FRAME_END_INT(bit2). Sin IRQ correcta no hay vsync real (Phosh igual pinta pero sin throttling). El probe usa IRQF_TRIGGER_NONE; en MT6582 puede requerir TRIGGER_LOW.
MEDIO — Direcciones fisicas sin IOMMU: el RDMA leera la fisica que le pase el plane (pending->addr). El framebuffer DRM debe estar en memoria contigua reservada (CMA/reserved-memory) y fisicamente direccionable por el RDMA, igual que el hack usa 0xBF400000. Si el GEM da paginas no contiguas, el RDMA solo ve la primera pagina -> corrupcion. Mitigacion: forzar scanout contiguo (mtk_drm ya usa CMA cuando no hay IOMMU; verificar).
BAJO — FIFO underflow: el calculo de threshold mainline (70%) ignora el tuning ultra del downstream; en DSI video-mode auto-refresh a 540x960 el ancho de banda es bajo, riesgo de underflow minimo. Si aparece flicker, portar el calculo de ultra/pre-ultra del downstream a MEM_GMC_SETTING_0.
BAJO — GMC value: mainline 0x40402020 vs valor ultra downstream en el mismo registro; benigno.
BAJO — SOF/mutex timing: si el mutex no se configura con el SOF del DSI correcto, el latch puede no ocurrir (mismo sintoma que el hack). Depende del componente mutex, no del RDMA.

**Pasos:**

1. 1. Confirmar base RDMA = 0x14008000 (ddp_reg.h L12: DDP_REG_BASE_DISP_RDMA=0xf4008000 virtual -> 0x14008000 fisico). Ventana 0x1000. YA verificado.
2. 2. En drivers/gpu/drm/mediatek/mtk_disp_rdma.c: anadir campo a struct mtk_disp_rdma_data para el offset de MEM_START_ADDR (ej: unsigned int mem_start_addr_reg;). Por defecto 0x0f00 para los SoCs existentes; usar 0x28 para mt6582. Reemplazar el #define DISP_RDMA_MEM_START_ADDR fijo por priv->data->mem_start_addr_reg en mtk_rdma_layer_config (L284). NB: mantener compat — los datos existentes (mt2701/mt8173/8183/8195) deben inicializar el campo a 0x0f00.
3. 3. Anadir struct mt6582_rdma_driver_data = { .fifo_size = SZ_4K, .formats = mt8173_formats, .num_formats = ARRAY_SIZE(mt8173_formats), .mem_start_addr_reg = 0x28, } y entry { .compatible = "mediatek,mt6582-disp-rdma", .data = &mt6582_rdma_driver_data } en mtk_disp_rdma_driver_dt_match.
4. 4. En mtk_drm_drv.c (ml_mtk_drm_drv.c L797): anadir { .compatible = "mediatek,mt6582-disp-rdma", .data = (void *)MTK_DISP_RDMA } a mtk_ddp_matches[] para que el nucleo lo reconozca como componente del path.
5. 5. DeviceTree: anadir el nodo rdma@14008000 con dual-compatible (mt6582 primero, mt2701 fallback), reg, clocks=<&disp_dummy_clk>, interrupts (SPI a verificar), mediatek,rdma-fifo-size=<4096>. NO poner power-domains/iommus/gce-client-reg.
6. 6. DeviceTree: declarar el fixed-clock disp_dummy_clk (compartible con ovl/color/mutex/dsi). Anadir clk_ignore_unused a bootargs.
7. 7. Asegurar que mmsys (mt2701-mmsys @0x14000000) y mutex (mt2701-disp-mutex @0x1400E000) esten en el DT para que se forme el main_path mt2701 con RDMA0; el RDMA depende de ellos para el routing y el latch SOF.
8. 8. (Opcional, robustez) Portar el soft-reset del downstream RDMAReset (handshake bits 0x700 de GLOBAL_CON @0x10) a una rutina de reset llamada una vez en el primer enable, por si LK dejo el RDMA en estado sucio. El mainline no resetea RDMA — si arranca limpio, omitir.
9. 9. Quitar/condicionar el bloque RDMA del hack mt6582-dispfix.c (escribe SIZE0=0x14=540, SIZE1=0x18=960, INT_ENABLE=0x00=0x3f). El driver DRM real reprograma esos mismos registros via mtk_rdma_config (SIZE_CON_0/1) + INT, asi que el hack debe desactivarse para no competir. NB: el hack NO programa MEM_START_ADDR ni GLOBAL_CON.ENGINE_EN del RDMA en modo memoria — el driver real SI (mtk_rdma_start pone ENGINE_EN bit0 @0x10, mtk_rdma_layer_config pone MODE_MEMORY bit1 y MEM_START_ADDR).
10. 10. Validar: dmesg debe mostrar el probe del rdma (mediatek-disp-rdma) sin -EPROBE_DEFER; el path mt2701 debe enlazar RDMA0; comprobar con devmem que GLOBAL_CON@0x14008010 tenga bit0=1 (engine en) y bit1=1 (memory mode), SIZE_CON_0@..14=540, SIZE_CON_1@..18=960, MEM_START_ADDR@..28=fisica del FB DRM. Confirmar que el frame se ve correcto (no negro=addr mal, no tint=formato MEM_CON RGB565). Con esto el latch via mutex/SOF queda activo -> habilita el control de brillo BLS sincronizado que el hack no tenia.

---


## COLOR (DISP color-correction engine) @ MT6582 phys 0x1400B000

**Veredicto:** reusar-compatible-existente | **Esfuerzo:** bajo

**Compatible DT:** mediatek,mt2701-disp-color

**Offsets/diferencias:** CONCLUSION: el COLOR del MT6582 es REGISTRO-POR-REGISTRO igual al MT2701; reusar el driver mainline mtk_disp_color.c sin tocar codigo. Pruebas:

BASE: ddp_reg.h linea 15 DDP_REG_BASE_DISP_COLOR=0xf400B000 (virt) -> fisico 0x1400B000. Coincide con dispfix (#define MT6582_COLOR 0x14007000-base map -> 0x1400B000) y con el prompt.

OFFSETS (verbatim mainline mtk_disp_color.c vs downstream ds_ddp_color.c / ddp_reg.h):
- CFG_MAIN: mainline DISP_COLOR_CFG_MAIN=0x0400. Downstream DpEngine_COLORonInit escribe (DISPSYS_COLOR_BASE+0x400)... espera: usa macro CFG_MAIN; dispfix escribe col+0x400=0x20000000; dispsys_bypass_color escribe (BASE+0x400)=0x2000323c. => offset 0x400 IDENTICO en los tres.
- color_offset (registro START block): mainline mt2701 .color_offset=DISP_COLOR_START_MT2701=0x0F00. Downstream COLOR start=(BASE+0xF00)=0x1 (ds_ddp_color.c L822 y ds_ddp_path.c L88). IDENTICO. (mt8167=0x0400 y mt8173=0x0C00 NO sirven; el MT6582 usa 0x0F00 = la variante mt2701).
- WIDTH/HEIGHT: mainline DISP_COLOR_WIDTH=color_offset+0x50=0xF50, HEIGHT=+0x54=0xF54. Downstream escribe wrapper width/height en (BASE+0xF50)/(BASE+0xF54) (ds_ddp_color.c L847-848). IDENTICO.

PIPELINE: mainline mt2701_mtk_ddp_main[] = OVL0->RDMA0->COLOR0->BLS->DSI0 (ml_mtk_drm_drv.c L61-67). Downstream MT6582 = identico: MUTEX MOD = (1<<3)|(1<<10)|(1<<7)|(1<<9) = OVL bit3 + RDMA bit10 + COLOR bit7 + BLS bit9 (ds_ddp_path.c L1359), y OVL_MOUT->RDMA, DISP_OUT_SEL=0->DSI (L1398-1400). COLOR es bit7 del MUTEX MOD = MT2701_MUTEX_MOD_DISP_COLOR=7 (ml_mtk-mutex.c L293). Coincidencia EXACTA incluyendo el orden inusual RDMA-antes-de-COLOR y el uso de BLS (no PWM generico).

DIFERENCIA FUNCIONAL UNICA (importante, no de offset): el VALOR escrito a CFG_MAIN difiere.
- mainline mtk_color_start(): writel(COLOR_BYPASS_ALL|COLOR_SEQ_SEL) = BIT7|BIT13 = 0x00002080, luego writel(0x1) a START(0xF00).
- dispfix actual: col+0x400 = 0x20000000 (bit29, 'color enable' del downstream COLORonInit).
- downstream bypass real: 0x2000323c (bit29 + bits relay 0x323c).
Los tres son modos de PASO-LIMPIO (bypass/relay, sin correccion). El mainline 0x2080 NO toca saturacion/hue/gamma (mtk_color_config solo escribe WIDTH/HEIGHT; mtk_color_start solo CFG_MAIN+START). RIESGO: que 0x2080 (que NO setea bit29) deje el COLOR en un bypass que altere el formato/orden de canales y reintroduzca el tinte M3. Hay que verificar en HW que 0x2080 da passthrough identico a 0x20000000; si tintara, es la unica linea a parchear (un quirk de un valor, no un driver nuevo). El driver mainline NO toca G_PIC_ADJ/PARTIAL_SAT/LOCAL_HUE ni el 10->8 rounding (0xF0C=0x333) que si hace el downstream: irrelevante para passthrough RGB565.

**Nodo DT:**
```
/* fixed-clock stub (1 sola reloj: el driver hace devm_clk_get(dev,NULL)) */
/ {
    disp_color_clk: disp-color-clk {
        compatible = "fixed-clock";
        #clock-cells = <0>;
        clock-frequency = <156000000>;  /* valor nominal; el LK ya dejo el CG on */
    };
};

&{/soc} {
    color0: color@1400b000 {
        compatible = "mediatek,mt2701-disp-color";
        reg = <0x1400b000 0x1000>;
        interrupt-parent = <&sysirq>;     /* opcional: COLOR IRQ; omitible */
        clocks = <&disp_color_clk>;
        clock-names = "color";            /* mtk_disp_color usa idx 0 / NULL; el name no se valida */
        power-domains = <&scpsys MT6582_POWER_DOMAIN_DISP>; /* OMITIR: no hay scpsys -> ver clocks */
        status = "okay";
    };
};

/* NOTA: el COLOR NO se referencia por phandle desde el CRTC; el binding mtk-drm
 * descubre los componentes por compatible al construir el path mt2701_mtk_ddp_main[].
 * Asegurar que existan tambien los nodos hermanos ovl@/rdma@/bls(disp_pwm)@/dsi@
 * con sus compatibles mt2701-* y un nodo mmsys (mediatek,mt2701-mmsys) padre que
 * los enumere, o el COLOR sola NO produce frames. */
```

**Kconfig:** CONFIG_DRM_MEDIATEK=y (selecciona mtk_disp_color.o automaticamente; el COLOR no tiene Kconfig propio, se compila siempre dentro de mediatek-drm.ko junto a ovl/rdma/dsi). Implica: CONFIG_DRM=y, CONFIG_DRM_KMS_HELPER=y, CONFIG_DRM_MIPI_DSI=y, CONFIG_DRM_PANEL=y, CONFIG_MTK_MMSYS=y (driver mtk-mmsys que provee el routing mt2701 y los resets), CONFIG_MTK_DEVAPC opcional. CONFIG_PHY_MTK_MIPI_DSI (lo necesita el DSI, no el COLOR). CONFIG_COMMON_CLK_MT2701_MM NO (no hay CCF -> fixed-clock). CONFIG_MTK_CMDQ NO (CMDQ es opcional; cmdq_dev_get_client_reg falla limpio sin nodo mediatek,gce y el driver cae a writel directo, que es justo lo que queremos sin M4U/GCE). clk_ignore_unused en bootargs.

**Clocks:** El driver mainline pide UNA sola reloj: devm_clk_get(dev, NULL) -> clk_prepare_enable en mtk_color_clk_enable. Sin CCF: declarar disp_color_clk como compatible='fixed-clock' #clock-cells=<0> clock-frequency=<156000000> (valor nominal irrelevante; clk_prepare_enable sobre fixed-clock es no-op exitoso). El gate REAL del COLOR vive en MMSYS_CG_CON0 (bit DISP_COLOR) que el fixed-clock NO togglea; el LK lo deja encendido (asi lo confirma la memoria del proyecto y el patron del resto de bloques DISPSYS), por lo que basta el stub + clk_ignore_unused en bootargs. Si al boot el bit estuviera gateado, des-gatearlo una vez via CG_CLR0 @0x14000108 (el hack mmsys o el dispfix). clock-names puede ser cualquier cosa ('color'): el driver usa el indice 0 / NULL y no valida el nombre.

**Dependencias:** Depende de que el resto del path DRM mt2701 este levantado por los sub-agentes hermanos: (a) mtk-mmsys (mediatek,mt2701-mmsys) que provee el routing OVL0->RDMA0->COLOR0->BLS->DSI0 y los resets; (b) OVL (mt2701-disp-ovl) que mete el frame; (c) RDMA0 (mt2701-disp-rdma) justo aguas-arriba del COLOR; (d) BLS/DISP_PWM (mt2701-disp-pwm, YA en DT) aguas-abajo; (e) DSI0 (mt2701-dsi) + panel HX8389; (f) el CRTC de mtk_drm que enumera el array mt2701_mtk_ddp_main[]. El COLOR en si solo depende de: 1 fixed-clock stub y su ventana de registros 0x1400B000. NO depende de M4U/IOMMU (no toca framebuffer), NO de CMDQ (opcional), NO de scpsys/power-domain (omitible con el LK dejando el CG on).

**Riesgos:** 1) CFG_MAIN value mismatch (PRINCIPAL): mtk_color_start escribe 0x2080 (BYPASS_ALL|SEQ_SEL) en vez del 0x20000000 que el dispfix probo bueno. Riesgo de reintroducir el tinte M3 si 0x2080 cambia orden de canal. Mitigacion: medir en HW; si tinta, parche de 1 linea (quirk de valor). Probabilidad media-baja (ambos son bypass documentados).
2) Sin clk CCF: el driver hace clk_prepare_enable de un fixed-clock (no-op) -> OK, pero si el COLOR dependiera de un gate real del MMSYS_CG_CON0 (bit DISP_COLOR), el fixed-clock NO lo togglea. El LK lo deja on (memoria del proyecto) -> OK, pero verificar MMSYS_CG_CON0 al boot que el bit COLOR este des-gateado; si no, anadir su des-gate al hack mmsys o dejar el dispfix solo para el CG.
3) CMDQ: si por error se compila CONFIG_MTK_CMDQ y falta el nodo gce, cmdq_dev_get_client_reg podria -EPROBE_DEFER y colgar el probe del CRTC. Mitigacion: NO habilitar MTK_CMDQ.
4) El COLOR no arranca solo: depende de que el CRTC mtk-drm enumere TODO el path mt2701 (ovl+rdma+color+bls+dsi+mmsys). Si falta cualquiera, no hay frames y el COLOR queda inerte. Es el riesgo de integracion, no del COLOR en si.
5) IRQ del COLOR (frame-done, ds_ddp_color.c L825 escribe INTEN=0x7): el mainline NO usa la IRQ del color (vsync viene del DSI/RDMA). Omitir interrupts en el nodo es seguro.
6) Coexistencia con dispfix: mientras se valida, el late_initcall dispfix y el driver DRM compiten por los mismos regs. Quitar la parte COLOR del dispfix (col+0x400) al activar el driver para evitar doble escritura/carrera.

**Pasos:**

1. 1. Confirmar en HW (devmem2 o el propio dispfix) el estado bueno: leer 0x1400B400 (CFG_MAIN), 0x1400BF00 (START), 0x1400BF50/F54 (W/H) bajo el downstream 3.10 que pinta bien; anotar el valor real de CFG_MAIN (esperado bit29 set).
2. 2. Verificar el gate de reloj: leer MMSYS_CG_CON0 @0x14000100 al boot mainline y comprobar que el bit del DISP_COLOR este des-gateado (=0). Si esta gateado, des-gatearlo (escribir a CG_CLR0 @0x14000108) en el hack mmsys o mantener esa unica linea en dispfix.
3. 3. Anadir al DT el nodo color@1400b000 (compatible mediatek,mt2701-disp-color) + el fixed-clock disp_color_clk, exactamente con el patron que ya usa disp_pwm@1400a000 en mt6582-PI-actual.dts (mismo estilo fixed-clock).
4. 4. Asegurar/crear los nodos hermanos del path mt2701 (ovl, rdma, color, bls=disp_pwm/bls, dsi) y el nodo padre mmsys (mediatek,mt2701-mmsys con clk-mt2701-mm reemplazado por stub) — esto lo coordinan los otros sub-agentes; el COLOR no funciona aislado.
5. 5. Kconfig: CONFIG_DRM_MEDIATEK=y, MTK_MMSYS=y, MIPI_DSI=y; DEJAR FUERA MTK_CMDQ y COMMON_CLK_MT2701_MM. Recompilar mediatek-drm.ko (incluye mtk_disp_color.o).
6. 6. Quitar del mt6582-dispfix.c la escritura del COLOR (writel(0x20000000, col+0x400)) para no competir con el driver una vez activo; dejar el resto del hack hasta que OVL/RDMA/DSI del DRM tomen el relevo.
7. 7. Bootear con clk_ignore_unused. Si el COLOR probetea OK pero hay tinte: parchear mtk_color_start() para escribir el valor bueno (0x20000000 o 0x2000323c) en vez de 0x2080 -> quirk de 1 linea o nueva mtk_disp_color_data.color_cfg_main. Si pinta limpio con 0x2080, no tocar nada.
8. 8. Validar passthrough RGB565: patron de color conocido por el framebuffer 0xBF400000 y comprobar canales correctos (sin swap R/B) -> mismo criterio que cerro el M3.

---


## MM_MUTEX (DISP MUTEX / "disp-mutex") del MT6582 @0x1400E000 — sincronizador de commits con SOF para el path OVL->RDMA0->COLOR->BLS->DSI0

**Veredicto:** reusar-compatible-existente | **Esfuerzo:** bajo

**Compatible DT:** mediatek,mt2701-disp-mutex  (driver mainline ml_mtk-mutex.c -> mt2701_mutex_driver_data). NO crear variante: la tabla mt2701 ya calza bit-a-bit con el MT6582.

**Offsets/diferencias:** CERO diferencias de layout entre ddp_reg.h (MT6582) y el mt2701 mainline. Registros por instancia n: EN=base+0x20+0x20*n (DISP_REG_MUTEX_EN(n)=0x20+0x20*n), MUTEX/COMMIT=base+0x24+0x20*n, RST=base+0x28+0x20*n, MOD=base+0x2C+0x20*n, SOF=base+0x30+0x20*n. El mainline define MT2701_MUTEX0_MOD0=0x2c, MT2701_MUTEX0_SOF0=0x30, MT2701_MUTEX0_MOD1=0x34 y la macro DISP_REG_MUTEX_MOD/SOF(reg,n)=reg+0x20*n -> IDENTICO a ddp_reg.h.
BITS MOD (mt2701_mutex_mod[]): OVL0=3, COLOR0=7, BLS=9, RDMA0=10, RDMA1=12, WDMA=6. PRUEBA cruzada con downstream MT6582 ds_ddp_path.c:1359 "ovl,rdma1,color,bls" = (1<<3)|(1<<10)|(1<<7)|(1<<9) = 0x688 y :1365 path-desde-RDMA "rdma,color,bls"=(1<<10)|(1<<7)|(1<<9)=0x680 -> coinciden exactamente con OVL=3/COLOR=7/BLS=9/RDMA0=10 de mt2701. (Nota: el "rdma1" del comentario downstream usa bit10=RDMA0 porque el 82 solo tiene 1 RDMA; mainline mapea OUR path como OVL0+RDMA0+COLOR0 todos presentes en mt2701_mutex_mod.)
SOF (mt2712_mutex_sof, usado por mt2701): SINGLE_MODE=0, DSI0=1, DPI0=3. El downstream pone mutexMode=1 para DISP_MODULE_DSI_VDO (ds_ddp_path.c:1331) = SOF DSI0; coincide con MUTEX_SOF_DSI0=1. El hack escribe mtx+0x30=1 (SOF=DSI0) y mtx+0x2c=0x680 (COLOR+BLS+RDMA0) -> confirma 1:1.
DIFERENCIA OPERATIVA (no de layout): mtk_drm normalmente hace mtk_mutex_add_comp(OVL0)+(RDMA0)+(COLOR0)+(DSI0). add_comp(DSI0) NO toca MOD: escribe SOF=mutex_sof[DSI0]=1 en SOF(reg,id). add_comp(OVLn/RDMAn/COLOR0) hace read-modify-write OR BIT(mod%32) en MOD(reg,id). El BLS (bit9) en mainline solo se agrega si DDP_COMPONENT_BLS esta en el path; en el krillin el BLS lo maneja el driver pwm-mtk-disp (mt2701-disp-pwm ya en el DT), NO el mtk_drm -> por eso el bit9 lo setea el path-config, no add_comp. Ver RIESGOS.
mtk_mutex_enable() escribe 1 en EN(id); acquire/release usan MUTEX(id) (COMMIT) con poll de INT_MUTEX=BIT(1). El downstream ademas toca INTEN=0x0101 e INTSTA=(1<<id) (ddp_reg.h DISP_REG_CONFIG_MUTEX_INTEN=base+0x0, INTSTA=base+0x4) — el mainline NO los usa (vsync vendra por IRQ del RDMA/DSI, no del mutex), es seguro omitirlos.
INSTANCIA: hay 6 mutex (MUTEX_RESOURCE_NUM=6, ddp_reg.h). El hack usa id efectivo via MOD0/SOF0 (id 0 -> offsets 0x2c/0x30). El downstream comenta que el BLS usa mutex id 3, pero en el path REAL de display primario el OVL+RDMA+COLOR+BLS van TODOS en el MISMO mutex (mutexId del CRTC), no en mutex separado; mtk_drm pide 1 mutex por CRTC via mtk_mutex_get() (primer libre). El id concreto lo elige el driver, no el DT.

**Nodo DT:**
```
&{/soc} {
	disp_mutex: mutex@1400e000 {
		compatible = "mediatek,mt2701-disp-mutex";
		reg = <0x1400e000 0x1000>;
		/* interrupts: OPCIONAL — el driver de mutex no registra handler.
		 * Omitible. Si se pone, verificar el SPI real del DISP_MUTEX en el GIC del MT6582. */
		clocks = <&disp_mutex_clk>;
		status = "okay";
	};
};

/ {
	disp_mutex_clk: disp-mutex-clk {
		compatible = "fixed-clock";
		#clock-cells = <0>;
		clock-frequency = <26000000>;
	};
};
```

**Kconfig:** CONFIG_MTK_MUTEX=y (o =m) — provee drivers/soc/mediatek/mtk-mutex.o (símbolos mtk_mutex_get/prepare/add_comp/enable...). Suele venir como dependencia de CONFIG_DRM_MEDIATEK. Asegurar tambien CONFIG_MTK_MMSYS=y (mtk-mmsys, el mutex/drm lo necesita para routing). NO requiere CONFIG_MTK_CMDQ (cmdq es opcional: el probe hace cmdq_dev_get_client_reg y solo logea dev_dbg si falta; sin nodo mediatek,gce-client-reg el mutex funciona en modo CPU-write, que es justo lo que queremos sin GCE/CMDQ).

**Clocks:** El driver mtk-mutex con data mt2701 (no_clk=false) EXIGE un clock: mtx->clk = devm_clk_get(dev, NULL). Sin CCF -> proveer 1 fixed-clock stub (disp_mutex_clk, compatible 'fixed-clock', #clock-cells=0, freq cualquiera p.ej 26MHz) referenciado por clocks=<&disp_mutex_clk> en el nodo mutex. clk_prepare_enable sobre el stub es no-op pero satisface el probe y mtk_mutex_prepare(). El bloque físico ya quedó encendido por el LK (gate MM abierto; el downstream ademas hace CG_CLR0/1=0xFFFFFFFF en MMSYS), por lo que el stub NO necesita encender nada real. Mantener clk_ignore_unused en bootargs para que el core no apague gates 'huérfanos'. ALTERNATIVA si se quisiera evitar el stub: clonar mt2701_mutex_driver_data a una variante mt6582 con .no_clk=true (como mt8167/mt8365) -> el probe NO pide clk; pero eso obliga a tocar el .c (crear-variante), mientras que el stub fixed-clock NO toca el driver. Recomendado: stub fixed-clock (veredicto reusar).

**Dependencias:** - mtk-mmsys (CONFIG_MTK_MMSYS): el mtk_drm enruta los componentes via mmsys; el mutex se obtiene por el CRTC del mtk_drm. El nodo mmsys@0x14000000 debe existir y exponer el routing OVL->RDMA->COLOR->DSI.
- mtk_drm_drv (CONFIG_DRM_MEDIATEK): es quien llama mtk_mutex_get()/add_comp()/enable(); el mutex por sí solo no hace nada — necesita el CRTC del mtk_drm para armar el path.
- Componentes del path que entran al MOD: OVL0 (bit3), RDMA0 (bit10), COLOR0 (bit7) y, para backlight sincronizado, BLS (bit9). Cada uno debe estar registrado como mtk_ddp_comp para que add_comp los sume.
- DSI0: provee el SOF (mutex_sof[DSI0]=1, video mode). El IRQ de vsync/vblank lo da el RDMA o el DSI, NO el mutex.
- Clock stub fixed-clock (disp_mutex_clk) + clk_ignore_unused (sin CCF).
- Pre-requisito de estado: LK deja DISPSYS encendido y MMSYS_CG_CLR limpios (gate MM abierto); de lo contrario mtk_mutex_acquire colgaría 10ms.
- Conflicto a resolver: el hack mt6582-dispfix (late_initcall) debe dejar de programar MOD/SOF/EN del mutex antes de que el driver real tome el control.

**Riesgos:** 1) CLOCK: mtk_mutex_probe hace devm_clk_get(dev,NULL) y FALLA el probe si data->no_clk==false (mt2701 NO es no_clk). -> OBLIGATORIO el fixed-clock stub + clk_ignore_unused en cmdline (ya en uso en el equipo). Si no, -EPROBE_DEFER/-ENOENT y el CRTC nunca arma.
2) mtk_mutex_prepare() hace clk_prepare_enable del stub (no-op real); el LK ya dejó el gate MM abierto, asi que aunque el stub no encienda nada físico, el HW responde. OK validado por el patrón pwm.
3) BLS/bit9 y BACKLIGHT: el objetivo (latch de brillo sincronizado a SOF) NO sale "gratis" del mtk-mutex. El brillo del BLS se latchea cuando el MUTEX con SOF=DSI0 dispara el SOF; PERO mainline mete el BLS en el mutex SOLO si DDP_COMPONENT_BLS está en el ddp_path del mtk_drm. En mt2701 el path mainline es OVL->RDMA->BLS->DSI (BLS=color+backlight juntos en el 2701). En el krillin el brillo lo da pwm-mtk-disp (nodo separado) -> hay que decidir: (a) dejar que mtk_drm incluya BLS en el path (entonces el componente BLS debe existir como mtk_ddp_comp y su add_comp setea bit9), o (b) que el bit9 lo ponga el COLOR/OVL config. RIESGO de que el brillo NO se latchee si el bit9 nunca entra al MOD del mutex del CRTC. MITIGACIÓN: replicar el MOD del hack (bits 7,9,10 + OVL 3) asegurando que add_comp se llame para OVL0,RDMA0,COLOR0 y, para BLS, o bien añadir DDP_COMPONENT_BLS al ddp_path del mt6582 en mtk_drm_drv, o forzar el bit9 en el setup del COLOR. Este es EL punto delicado del objetivo backlight.
4) SOF/IRQ vsync: el mutex con SOF=DSI0 genera STREAM_DONE; pero el vblank/vsync que necesita el KMS lo entrega el IRQ del RDMA (frame-done) o del DSI, NO el mutex. Verificar que el nodo RDMA/DSI tenga su interrupts correcto. El mutex NO da vsync por sí solo.
5) IRQ number del mutex en DT: lo puse como placeholder (207). El driver de mutex mainline NO registra handler de irq, asi que el campo interrupts es cosmético/ignorado -> se puede OMITIR sin riesgo. No inventar un nº que colisione con otro periférico.
6) mtk_mutex_acquire() hace readl_poll_timeout 10ms esperando INT_MUTEX=BIT(1) en MUTEX(id); si el bloque no estuviera clockeado colgaría 10ms y "could not acquire mutex". Como el LK lo deja on, debería pasar; si aparece ese pr_err -> revisar gate MM (MMSYS_CG_CON0/1) — el hack/mmsys debe limpiarlos (CG_CLR0/1=0xFFFFFFFF, ds_ddp_path.c:1324).
7) Coexistencia con el hack: mientras se prueba, el late_initcall mt6582-dispfix sigue tocando mtx+0x2c/0x30/0x00 -> conflicto con mtk_drm. Al activar el driver real HAY QUE quitar el hack (o al menos su bloque mtx) para que no pelee por el MOD/SOF/EN.

**Pasos:**

1. 1. Añadir el nodo mutex@1400e000 (compatible mediatek,mt2701-disp-mutex, reg 0x1400e000/0x1000) y el fixed-clock disp_mutex_clk al DT, replicando el patrón ya usado por disp_pwm. Mantener clk_ignore_unused en cmdline.
2. 2. Habilitar CONFIG_MTK_MUTEX y CONFIG_MTK_MMSYS (ambos =y). Confirmar que el .config los trae al activar DRM_MEDIATEK; NO activar CMDQ.
3. 3. Compilar ml_mtk-mutex.c TAL CUAL (no requiere parche para el MT6582: mt2701_mutex_driver_data ya es correcto). Verificar que el símbolo mtk_mutex_get/add_comp/enable se exportan para el mtk_drm.
4. 4. En el binding del mtk_drm para el MT6582 (driver-data del mtk_drm_drv, componente 'crtc'), declarar el ddp_path primario = { OVL0, RDMA0, COLOR0, DSI0 } y referenciar el mutex por phandle (igual que mt2701 hace con su mmsys/mutex). Esto hace que el CRTC llame mtk_mutex_get()+add_comp(OVL0/RDMA0/COLOR0)+add_comp(DSI0->SOF=1).
5. 5. Decidir el manejo del BLS (bit9): para que el brillo se latchee a SOF, o (a) incluir DDP_COMPONENT_BLS en el ddp_path del mt6582 (preferible, replica mt2701) y dejar que su add_comp ponga bit9, o (b) si el BLS se queda como pwm-mtk-disp separado, parchear el setup del COLOR0/OVL para OR bit9 en el MOD del mutex del CRTC. Validar contra el MOD esperado 0x688 (OVL3+COLOR7+BLS9+RDMA10).
6. 6. RETIRAR el bloque 'if (mtx)' del mt6582-dispfix.c (lineas 68-73) — y a término todo el late_initcall — para que el driver real sea dueño único del MUTEX (MOD/SOF/EN). Probar primero con el resto del hack vivo pero el bloque mtx fuera.
7. 7. Arrancar y comprobar en dmesg que NO aparece 'could not acquire mutex' ni 'Failed to get clock'. Leer por debugfs/devmem MOD(0)=base+0x2c (esperar 0x688 o 0x680+0x8) y SOF(0)=base+0x30 (esperar 1=DSI0) y EN(0)=base+0x20 (1).
8. 8. Validar el objetivo: con el mutex armado y SOF=DSI0, comprobar que cambiar PWM_DUTY (pwm-mtk-disp) ahora se latchea sincronizado (sin parpadeo) y que el KMS recibe vblank (de RDMA/DSI). Si no hay vblank, es problema del RDMA/DSI irq, NO del mutex.
9. 9. Una vez estable, eliminar definitivamente el hack mt6582-dispfix y dejar el flujo de frames manejado por mtk_drm+mtk-mutex.

---


## dsi-panel (controlador MIPI-DSI mtk_dsi + PHY MIPI-TX + panel Himax HX8389 qHD 540x960 video-mode "truly")

**Veredicto:** crear-variante-mt6582 | **Esfuerzo:** alto

**Compatible DT:** DSI: "mediatek,mt2701-dsi" (reusar tal cual; driver_data mt2701 = reg_cmdq_off 0x180, reg_vm_cmd_off 0x130, reg_shadow_dbg_off 0x190, sin has_shadow_ctl/has_size_ctl -> es EXACTAMENTE la generacion del MT6582). PHY MIPI-TX: "mediatek,mt2701-mipi-tx" (de drivers/phy/mediatek/phy-mtk-mipi-dsi.c + phy-mtk-mipi-dsi-mt8173.c; mt2701_mipitx_data mppll_preserve=3, ref 26MHz). PANEL: driver nuevo en drivers/gpu/drm/panel/panel-himax-hx8389.c (NO panel-simple: requiere ~20 comandos init DSI generic-long-write + DCS; usa drm_panel + mipi_dsi_device). Alternativa rapida: bridge panel generico via "himax,hx8389" propio.

**Offsets/diferencias:** VERIFICADO contra mt_reg_base.h MT6582 (android-4.4.4_r3) y ddp_reg.h local. El prompt ERRABA el base DSI: NO es 0x14012000. Convencion confirmada por DT (GPIO 0xF0005000->0x10005000, EINT 0xF000B000->0x1000B000, KP 0xF0011000->0x10011000): 0xF4xx->0x14xx y 0xF0xx->0x10xx.
- DSI_BASE: 0xF400C000 -> FISICO 0x1400C000, size 0x1000 (DPI esta en 0xF400D000, asi que la ventana DSI es 0xC000..0xD000). El bloque DSI INCLUYE los regs de PHY-timing/lane (TIMECON0-3 @0x110-0x11c, PHY_LCCON @0x104, PHY_LD0CON @0x108) igual que mt2701 -> el mapa de offsets de ml_mtk_dsi.c calza 1:1.
- MIPITX/D-PHY analog (PLL): MIPI_CONFIG_BASE 0xF0010000 -> FISICO 0x10010000. mt2701-mipi-tx accede hasta offset 0x84 (MIPITX_DSI_SW_CTRL_CON0) -> declarar reg size 0x1000. Offsets PLL que toca: CON 0x00, CLOCK_LANE 0x04, DATA_LANE0-3 0x08/0x0c/0x10/0x14, TOP_CON 0x40, BG_CON 0x44, PLL_CON0-2 0x50/0x54/0x58, PLL_TOP 0x64, PLL_PWR 0x68. (MIPI_RX_ANA 0xF0010800=0x10010800 es el RX, NO usado por TX.)
- Diferencia que OBLIGA variante: el MT6582 NO tiene CCF; el flujo mainline mtk_dsi_poweron() hace clk_set_rate(hs_clk, data_rate) sobre el reloj PLL que EXPORTA el mt2701-mipi-tx. Hay que tener ese PHY+su clk-provider para que la cadena cierre. Tambien M4U/IOMMU ausente (no afecta al DSI; afecta al OVL).
- Panel params (de panel_hx8389.c): 540x960, 2 lanes (LCM_TWO_LANE), RGB888 (LCM_DSI_FORMAT_RGB888, PS=PACKED_PS_24BIT_RGB888), SYNC_PULSE_VDO_MODE. Porches: vsa=3 vbp=9 vfp=9, hsa=8 hbp=20 hfp=22 (rama "#if 1" activa). PLL_CLOCK=250 -> 500 Mbps/lane. data_rate mainline = pixelclock*24/2. Reset = GPIO112 (1->0 50ms->1 20ms). Power = PMIC VGP2 2.8V (no VGP1).

**Nodo DT:**
```
/* ---- relojes stub sin CCF (igual patron que disp_pwm_clk del DT actual) ---- */
/ {
    /* ref 26MHz para el PLL del MIPI-TX (PCW se calcula con 26e6) */
    mipi_tx_ref_clk: mipi-tx-ref-clk {
        compatible = "fixed-clock"; #clock-cells = <0>;
        clock-frequency = <26000000>;
    };
    /* relojes de engine/digital del DSI: el LK deja DISPSYS encendido,
     * clk_ignore_unused evita gate; valores nominales, no se reprograman */
    dsi_engine_clk: dsi-engine-clk {
        compatible = "fixed-clock"; #clock-cells = <0>;
        clock-frequency = <156000000>;
    };
    dsi_digital_clk: dsi-digital-clk {
        compatible = "fixed-clock"; #clock-cells = <0>;
        clock-frequency = <26000000>;
    };
};

&{/soc} {
    /* PHY MIPI-TX: registra el clk-provider (PLL) que el DSI usa como "hs" */
    mipi_tx0: mipi-dphy@10010000 {
        compatible = "mediatek,mt2701-mipi-tx";
        reg = <0x10010000 0x1000>;
        clocks = <&mipi_tx_ref_clk>;
        clock-output-names = "mipi_tx0_pll";
        #clock-cells = <0>;
        #phy-cells = <0>;
        status = "okay";
    };

    dsi0: dsi@1400c000 {
        compatible = "mediatek,mt2701-dsi";
        reg = <0x1400c000 0x1000>;
        interrupts = <GIC_SPI 39 IRQ_TYPE_LEVEL_LOW>; /* VERIFICAR vector DISP/DSI MT6582; placeholder, irq es opcional para arrancar en vdo-mode */
        clocks = <&dsi_engine_clk>, <&dsi_digital_clk>, <&mipi_tx0>;
        clock-names = "engine", "digital", "hs";
        phys = <&mipi_tx0>;
        phy-names = "dphy";
        status = "okay";

        port {
            dsi0_out: endpoint {
                remote-endpoint = <&panel_in>;
            };
        };

        panel@0 {
            compatible = "himax,hx8389-truly-qhd"; /* driver nuevo */
            reg = <0>;
            reset-gpios = <&eint 112 GPIO_ACTIVE_HIGH>; /* GPIO112 via gpio-mt6582-eint */
            /* VGP2 del MT6323 alimenta VCI del panel; anadir ldo_vgp2 al nodo pmic */
            vci-supply = <&mt6323_vgp2_reg>;
            backlight = <&backlight>;
            port {
                panel_in: endpoint {
                    remote-endpoint = <&dsi0_out>;
                };
            };
        };
    };
};

/* anadir al &pmic/mt6323regulator (el DT ya tiene vgp1; falta vgp2): */
/* mt6323_vgp2_reg: ldo_vgp2 { regulator-name="vgp2";
      regulator-min-microvolt=<2800000>; regulator-max-microvolt=<2800000>; }; */
```

**Kconfig:** CONFIG_DRM=y ; CONFIG_DRM_MEDIATEK=y (arrastra mtk_dsi/ovl/rdma/color/mutex/mmsys) ; CONFIG_DRM_MEDIATEK_DSI? -> el DSI va dentro de DRM_MEDIATEK (no hay symbol aparte) ; CONFIG_PHY_MTK_MIPI_DSI=y (drivers/phy/mediatek, da el mt2701-mipi-tx) ; CONFIG_DRM_PANEL=y ; CONFIG_DRM_PANEL_HIMAX_HX8389=m/y (NUEVO Kconfig que hay que anadir en drivers/gpu/drm/panel/Kconfig+Makefile) ; CONFIG_BACKLIGHT_PWM=y + CONFIG_PWM_MEDIATEK_DISP/CONFIG_PWM=y (ya en uso por quick-win backlight) ; CONFIG_GENERIC_PHY=y ; CONFIG_COMMON_CLK=y (necesario para que el mipi-tx registre su clk_hw aunque no haya clk-SoC driver). Bootargs: mantener clk_ignore_unused.

**Clocks:** SIN CCF -> 3 relojes que pide el driver DSI (devm_clk_get "engine","digital","hs") se cubren asi: "engine" y "digital" = fixed-clock stubs (el LK ya dejo encendido el DISPSYS y MMSYS_CG; clk_prepare_enable sobre fixed-clock es no-op, no apaga nada). "hs" = NO puede ser fixed: el driver hace clk_set_rate(hs_clk, data_rate) en cada poweron -> tiene que ser el reloj PLL que EXPORTA el PHY mt2701-mipi-tx (clock-output-names="mipi_tx0_pll", referenciado como <&mipi_tx0>). Ese PHY SI implementa .set_rate/.round_rate (mtk_mipi_tx_pll_ops) calculando PCW desde su ref 26MHz (mipi_tx_ref_clk fixed-clock). Resumen de la cadena: mipi_tx_ref_clk(26M fixed) -> mt2701-mipi-tx PLL(programable, =hs_clk del DSI) ; engine/digital = fixed stubs. Asi NO se necesita ningun clk-mt6582 driver. Riesgo: clk_set_rate reprograma el PLL que el LK ya configuro (ver riesgos).

**Dependencias:** 1) PHY mt2701-mipi-tx OBLIGATORIO: el DSI hace devm_phy_get(dev,"dphy") y phy_power_on() en poweron; sin el, -EPROBE_DEFER eterno. 2) COMMON_CLK on para que el mipi-tx registre el clk_hw del PLL. 3) Componente del bus DRM: el DSI es un component que el mtk_drm_drv ensambla; necesita que existan en el mismo grafo OVL0/RDMA0/COLOR0/BLS/DSI0 (path mt2701_mtk_ddp_main[] = OVL0->RDMA0->COLOR0->BLS->DSI0, IDENTICO al pipeline MT6582). Es decir, este componente depende del trabajo de los componentes ovl/rdma/color/mmsys/mutex/bls (hermanos del port). 4) gpio-mt6582-eint (ya en DT) para reset-gpios=GPIO112. 5) MT6323 VGP2 regulator (anadir; el DT solo tiene vgp1). 6) backlight pwm (quick-win ya hecho). 7) El panel depende de mtk_dsi (host MIPI-DSI) registrado.

**Riesgos:** ALTO - Reinit del panel ya encendido por LK: el flujo DRM real hace poweron completo (mtk_dsi_poweron->clk_set_rate(PLL)->phy_power_on->reset_engine->reconfig timings) y el panel driver hara reset GPIO112 + reenvio de init_lcm_registers en .prepare/.enable. Como el LK YA dejo el panel auto-refrescando en HS, reprogramar el PLL MIPI-TX y resetear el D-PHY GLITCHEA la imagen momentaneamente (no es take-over transparente; hay un parpadeo/reinit). Es aceptable (Phosh repinta) pero hay que reenviar TODA la secuencia init del panel, no asumir estado.
MEDIO - mtk_mipi_tx PCW/txdiv: a 500Mbps txdiv=1; si el PLL no engancha por falta del reloj de banda-base real del SoC (que el LK preparo), el HS no sale. Mitigacion: probar primero dejando el PLL como esta (no llamar set_rate) o calibrar data_rate para que coincida con lo que dejo el LK.
MEDIO - IRQ DSI: el vector exacto del DISP/DSI en MT6582 no esta confirmado en los fuentes locales (mt_devs.c no lo expone). En vdo-mode el DSI arranca sin depender criticamente del IRQ para frames continuos (solo lo usa para cmd-done/vm-done en transfer); se puede arrancar y luego afinar. Si el IRQ es erroneo, los mipi_dsi_host_transfer (lectura de ID/ESD) timeoutearan pero el video sigue.
MEDIO - Comandos init del panel: traducir los data_array LK (Data ID 0x39 generic-long, 0x15 generic-short, 0x05/0x29 DCS) a mipi_dsi_generic_write/mipi_dsi_dcs_write. El mtk_dsi_cmdq mainline soporta long/short y BTA; OK. Cuidado: el LK usa 0x39 (GENERIC long) no 0x29 (DCS long) para la mayoria -> usar mipi_dsi_generic_write, no dcs.
BAJO - format: confirmado RGB888 24bpp (no RGB565 como el resto del hack); el OVL/RDMA upstream pueden ir en 565 pero el DSI emite 888 (PS_SEL=3). Coherente con downstream (DPI tmp buffer 565 -> DSI 888 con dithering). Verificar que el COLOR/dither no haga falta o aceptar banding.
BAJO - falta &mt6323_vgp2_reg en el DT actual (solo hay vgp1).

**Pasos:**

1. 1. DT: anadir nodo mipi_tx0@10010000 (mt2701-mipi-tx, reg 0x10010000/0x1000, ref=mipi_tx_ref_clk 26MHz fixed, clock-output-names mipi_tx0_pll, #phy-cells=0, #clock-cells=0).
2. 2. DT: anadir dsi0@1400c000 (mt2701-dsi, reg 0x1400c000/0x1000, clocks engine/digital=fixed stubs + hs=<&mipi_tx0>, phys=<&mipi_tx0> phy-names dphy), con subnodo port->endpoint y panel@0.
3. 3. DT: anadir ldo_vgp2 (2.8V) al &pmic/mt6323regulator y referenciarlo como vci-supply del panel; reset-gpios=<&eint 112>.
4. 4. Kconfig: habilitar CONFIG_PHY_MTK_MIPI_DSI, CONFIG_DRM_MEDIATEK (incluye el DSI), CONFIG_DRM_PANEL, CONFIG_GENERIC_PHY, CONFIG_COMMON_CLK; mantener clk_ignore_unused en bootargs.
5. 5. Escribir drivers/gpu/drm/panel/panel-himax-hx8389.c: struct con drm_panel + mipi_dsi_device; .prepare = VGP2 on -> reset GPIO112 (1,5ms;0,50ms;1,20ms) -> enviar la secuencia de init_lcm_registers traducida (mipi_dsi_generic_write para los 0x39, mipi_dsi_dcs_write para 0x11/0x29) -> sleep-out 200ms -> display-on 10ms; .get_modes = 540x960 con porches vsa3/vbp9/vfp9/hsa8/hbp20/hfp22 y clock derivado (pixelclock t.q. data_rate=500Mbps con 24bpp/2lanes); .unprepare = sleep-in + VGP2 off. mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE; format = MIPI_DSI_FMT_RGB888; lanes = 2.
6. 6. Anadir entry en panel Kconfig+Makefile (CONFIG_DRM_PANEL_HIMAX_HX8389) y compatible himax,hx8389-truly-qhd.
7. 7. Build modulos (make modules) e instalar SIN reflashear (insmod phy-mtk-mipi-dsi.ko, mtk-drm con sus comp, panel). Verificar probe: dmesg 'mtk-dsi' y 'mipi-tx' sin EPROBE_DEFER colgado, y aparicion del conector DSI en /sys/class/drm.
8. 8. Estrategia de take-over: PRIMERA prueba dejando que el panel driver reenvie init completo (esperar parpadeo). Si el PLL no engancha, plan B: parchear mtk_dsi_poweron para NO hacer clk_set_rate la primera vez (heredar el estado del LK) y solo arrancar el engine.
9. 9. Una vez hay flujo de frames del DSI real: quitar simplefb/simpledrm y el hack mt6582-dispfix.c del arranque; confirmar que el MUTEX sincroniza el latch del BLS con SOF (eso da el backlight controlable que era el objetivo).
10. 10. Afinar: corregir el vector de IRQ DSI real para que mipi_dsi_host_transfer (compare_id/ESD) funcione; validar colores RGB888 vs banding; medir vsync.

---


## pwm-backlight (DISP_PWM / BLS @0x1400A000) — backlight dentro del flujo DRM mtk_drm. /sys/class/backlight via pwm-backlight -> pwm_chip del driver pwm-mtk-disp (mt2701) que ADEMAS es el DDP_COMPONENT_BLS del pipeline del CRTC.

**Veredicto:** crear-variante-mt6582 | **Esfuerzo:** medio

**Compatible DT:** DT-compatible = "mediatek,mt2701-disp-pwm" (NO crear compatible DT nuevo; los offsets calzan). PERO el comportamiento del SoC obliga a una variante a NIVEL DE CODIGO en pwm-mtk-disp.c (mt6582_pwm_data) por el bug del BLS_DEBUG. Doble papel del mismo nodo @0x1400a000: (1) en ml_mtk_drm_drv.c el match "mediatek,mt2701-disp-pwm" -> .data=MTK_DISP_BLS => el CRTC lo mete como DDP_COMPONENT_BLS en el path mt2701 (OVL0->RDMA0->COLOR0->BLS->DSI0, ml_mtk_drm_drv.c L65) y lo anade al MUTEX con MT2701_MUTEX_MOD_DISP_BLS=9 (ml_mtk-mutex.c L294,372) => latch por SOF del DSI; (2) el driver de PWM pwm-mtk-disp.c bindea el MISMO compatible y expone el pwm_chip que consume pwm-backlight. Ambos drivers conviven sobre el mismo reg. ALTERNATIVA evaluada y DESCARTADA: que MTK_DISP_BLS en el CRTC ya controle el brillo solo (no lo hace; el comp BLS en mtk_drm solo entra al MUTEX, el duty lo escribe el pwm_chip).

**Offsets/diferencias:** TODOS los offsets BLS verificados en ddp_reg.h (DISPSYS_BLS_BASE=0x1400A000) y coinciden con lo que toca pwm-mtk-disp.c mt2701:
- BLS_EN/PWM_EN: 0x00 (bit0=BLS_EN, bit16=PWM_EN). El driver mt2701 usa enable_mask=BIT(16). El hack ya deja 0x10001. VALIDADO en HW: PWM_DUTY mueve brillo, BLS_EN bit16 = PWM_EN.
- PWM_DUTY: 0xA0 (0..1023). VALIDADO por barrido /dev/mem. El downstream ds_ddp_path.c L174 escribe 0x800003FF (bit31 set + duty=0x3ff) -> bit31 NO es duty, observar (probable "duty valid/latch"); ddp_bls.c disp_bls_set_backlight escribe solo el nivel sin bit31 y funciona via MUTEX. mt2701 pwm-mtk-disp escribe duty crudo en con1.
- PWM_CON: 0xA8 (PWM_DUTY... en realidad CON0). gPWMDiv<<16 (ddp_bls.c L312: BLS_PWM_CON = gPWMDiv<<16). PWM_MANUAL/CON1: 0xAC.
- BLS_DEBUG: 0xB0  <-- DIFERENCIA CRITICA. El driver mt2701 (pwm-mtk-disp.c, mt2701 NO tiene has_commit) en CADA mtk_disp_pwm_apply hace OR de bls_debug_mask=0x3 en 0xB0 = DESACTIVA EL DOBLE-BUFFER del BLS. ddp_bls.c confirma la semantica: disp_bls_set_backlight (rama !USE_DISP_BLS_MUTEX) hace DISP_REG_SET(BLS_DEBUG,0x3) antes y 0x0 despues; disp_bls_contrl_directly(false) pone 0x3. Es decir 0xB0=0x3 fuerza modo manual/bypass-doblebuffer. SIN el pipeline esto descuadra colores (HITO #26 confirmo: "colores raros"). CON el pipeline real igual es danino: rompe el latch sincronizado que el MUTEX/SOF provee.
- DITHER block: 0xE00 + 4*X (ddp_reg.h L387). Para RGB565 (nuestro caso, stride1088 r5g6b5) hay que escribir (ddp_bls.c disp_bls_config_full, rama dither_bpp==16): DITHER(15)@0xE3C=0x50500001, DITHER(16)@0xE40=0x50504040, DITHER(0)@0xE00=0x1. OJO: el downstream LK (ds_ddp_path.c L168-170) deja 888 (0xE3C=0x20200001/0xE40=0x20202020/0xE00=0) porque su panel sale 888; en mainline RGB565 hay que poner los valores de 565. El driver pwm-mtk-disp NO toca DITHER -> ESTE es el config que falta integrar.
- Luminance LUT: 0x300 + 4*X .. 0x384 (ddp_reg.h L384-385). Valores en ddp_bls.c L425-459 / ds_ddp_path.c L130-163. Curva PWM->luminancia; el driver pwm-mtk-disp NO la programa.
- GAMMA LUT/SETTING: 0x30/0x34/0x400 (DDP_GAMMA_SUPPORT). El config_full hace BLS_SETTING@0x10=0x00100007 (gamma on).
- BLS_SETTING: 0x10. SRC_SIZE: 0x18 = (h<<16)|w.
DIFERENCIA DE GENERACION: el BLS del MT6582 == BLS del MT2701 (misma gen 2014, mismo bloque "BLS" con AAL/dither/gamma/PWM integrados). MT8167/MT8173 NO tienen BLS (usan DISP_PWM puro=MTK_DISP_PWM) -> por eso SOLO sirve el compatible mt2701. NO hay registro has_commit estilo mt8167/mt8173 (esos SI tienen commit@0x08 + manual-double-buffer-off distinto).

**Nodo DT:**
```
/* Reusar el nodo ya presente en mt6582-PI-actual.dts (Quick Win #1) tal cual los offsets,
 * pero ahora bajo el contenedor mmsys/CRTC. Sin CCF -> 2 fixed-clock stubs. */
/ {
    disp_pwm_clk: disp-pwm-clk {        /* "main" = fuente PWM (26MHz osc, el LK ya lo dejo en CLK_CFG_1) */
        compatible = "fixed-clock"; #clock-cells = <0>;
        clock-frequency = <26000000>;
    };
    disp_pwm_mm_clk: disp-pwm-mm-clk {  /* "mm" = gate MM del bloque BLS (MT_CG_DISP0_DISP_BLS) */
        compatible = "fixed-clock"; #clock-cells = <0>;
        clock-frequency = <26000000>;
    };

    backlight: backlight {
        compatible = "pwm-backlight";
        pwms = <&disp_pwm 0 50000>;     /* periodo 50us; el divisor real lo pone PWM_CON<<16 */
        brightness-levels = <0 16 32 64 96 128 160 192 224 255>;
        default-brightness-level = <7>;
        /* power-supply OPCIONAL: el panel ya esta encendido por el LK; si se pone, usar el
         * fixed reg_vmmc always-on que ya existe, NO un GPIO real (no hay pinctrl). */
    };
};

&{/soc} {
    /* DISP_PWM/BLS MT6582 @0x1400A000. Es a la vez DDP_COMPONENT_BLS (lo enruta el CRTC
     * por el path mt2701 y lo mete al MUTEX bit9) y proveedor del pwm_chip de pwm-backlight. */
    disp_pwm: pwm@1400a000 {
        compatible = "mediatek,mt2701-disp-pwm";
        reg = <0x1400a000 0x1000>;
        #pwm-cells = <2>;
        clocks = <&disp_pwm_clk>, <&disp_pwm_mm_clk>;
        clock-names = "main", "mm";
        /* SIN interrupts: el BLS no aporta vblank (lo da el RDMA irq). */
        /* SIN power-domains, SIN mediatek,gce (no GCE/CMDQ), SIN iommus. */
        status = "okay";
    };
};

/* IMPORTANTE: el nodo disp_pwm DEBE ser hijo-hermano de los demas bloques DISP bajo el
 * mismo padre que enumera mtk_drm_drv (for_each_child_of_node(phandle->parent,...) L1132),
 * para que mtk_ddp_comp_get_id le asigne DDP_COMPONENT_BLS y el CRTC lo incluya. */
```

**Kconfig:** CONFIG_DRM_MEDIATEK=y        (compila el lado BLS=DDP_COMPONENT_BLS dentro de mtk_drm; el match mt2701-disp-pwm->MTK_DISP_BLS vive en ml_mtk_drm_drv.c L791-792)
CONFIG_MTK_MMSYS=y           (routing + master del CRTC)
CONFIG_MTK_CMDQ=n            (no hay GCE: pwm-mtk-disp y el CRTC usan writel directo; has_commit/cmdq fuera)
CONFIG_PWM=y
CONFIG_PWM_MEDIATEK_DISP=y   (symbol que compila drivers/pwm/pwm-mtk-disp.c -> el pwm_chip de "mediatek,mt2701-disp-pwm". En algunos arboles el symbol es CONFIG_PWM_MTK_DISP; el .config del equipo ya usaba PWM_MTK_DISP=y)
CONFIG_BACKLIGHT_PWM=y       (pwm_bl.c -> /sys/class/backlight)
CONFIG_BACKLIGHT_CLASS_DEVICE=y
clk_ignore_unused YA en bootargs (los 2 fixed-clock no se apagan; el LK dejo MT_CG_DISP0_DISP_BLS / MDP_BLS_26M encendidos)

**Clocks:** SIN CCF -> 2 fixed-clock stubs (patron ya validado en msdc/i2c). pwm-mtk-disp.c hace clk_get por NOMBRE: "main" (fuente del PWM) y "mm" (gate del bloque MM/BLS) y clk_prepare_enable de ambos. Sobre fixed-clock ambas son no-op y devuelven 0, nunca fallan. El LK ya dejo encendidos los gates reales (MT_CG_DISP0_MDP_BLS_26M y MT_CG_DISP0_DISP_BLS, ver ddp_bls.c L308-309/L355-356) y clk_ignore_unused esta en bootargs, asi que aunque los fixed-clock se consideren unused no se apagan. La clock-frequency (26MHz) es formal: el divisor PWM real se programa por PWM_CON@0xA8 (gPWMDiv<<16), no por el rate del clk. NO referenciar relojes de un clk-mt2701-mm driver (no existe CCF aqui). Dos nodos separados (main/mm) porque el driver pide 2 names; pueden ser dos fixed-clock triviales (o el mismo si el binding aceptara el mismo phandle, pero mejor 2 por claridad). pm_runtime: si el driver lo usa, sin power-domain real es no-op.

**Dependencias:** El backlight NO funciona aislado; depende de TODO el contenedor DRM para que el latch sincronizado (objetivo del proyecto) exista:
1. mmsys (mediatek,mt2701-mmsys @0x14000000): master/CRTC; sin el, el comp BLS no entra al pipeline y el pwm-mtk-disp solo daria PWM crudo (= el caso roto del HITO). CRITICO.
2. mtk-mutex (mediatek,mt2701-disp-mutex @0x1400E000): el comp BLS se anade con MT2701_MUTEX_MOD_DISP_BLS=9; el MUTEX_EN(id)=1 (mtk_mutex_enable, ml_mtk-mutex.c L977-985) latchea el reg-update sincronizado con el SOF del DSI. ESTE es el mecanismo que da brillo-latcheado+vsync que al hack le falta. DEPENDENCIA DURA.
3. DSI0 (mt2701-dsi; base REAL 0x1400C000, panel hx8389 video-mode auto-refresh): fuente del SOF que el MUTEX usa para latchear. Sin SOF, mtk_mutex_acquire cuelga 10ms y el duty no latchea. DURA.
4. RDMA0 (ml_mtk_disp_rdma.c): aporta el IRQ de frame-end = vblank del CRTC (mtk_disp_rdma_irq_handler L91-103). Sin vblank, el atomic-commit que arrastra el brillo no completa. Indirecta pero necesaria.
5. OVL0 + COLOR0: aguas arriba en el path; deben estar para que el CRTC arme el pipeline completo (OVL0->RDMA0->COLOR0->BLS->DSI0).
6. mtk_drm_drv.c: el match "mediatek,mt2701-disp-pwm"->MTK_DISP_BLS (L791-792) y el path mt2701 que lista DDP_COMPONENT_BLS (L65). Si se crea variante mt6582-disp-pwm, anadir el match analogo aqui Y en pwm-mtk-disp.c.
7. pwm-backlight (pwm_bl.c) -> consume el pwm_chip del disp_pwm; ultimo eslabon para /sys/class/backlight.
8. config_full RGB565 del BLS (dither/LUT/gamma): no es un driver, pero es prerequisito para que el brillo no venga con colores rotos (ver pasos 3 y riesgos 2).
NO depende de: CMDQ/GCE, IOMMU/M4U, pinctrl, scpsys/genpd.

**Riesgos:** 1) RIESGO PRINCIPAL = BLS_DEBUG=0x3 EN CADA APPLY (rompe colores). pwm-mtk-disp.c mt2701 (sin has_commit) hace OR 0x3 en 0xB0 cada vez que cambia el brillo -> desactiva doble-buffer -> descuadra el pipeline de color (CONFIRMADO en kernel #26: "texto legible, colores raros"; mismo sintoma que tocar 0xB0 a mano, ver ddp_bls.c disp_bls_contrl_directly). DENTRO del flujo DRM completo NO se cura solo: el MUTEX/SOF latchea el reg-update del pipeline, pero BLS_DEBUG=0x3 fuerza al propio BLS a modo manual ignorando ese doble-buffer. MITIGACION (elegir una):
   (a) VARIANTE mt6582_disp_pwm_data en pwm-mtk-disp.c que NO escriba bls_debug (poner bls_debug_mask=0 / saltar el OR) y en su lugar confie en el latch del MUTEX (el comp BLS ya esta en el mutex del CRTC). Es el cambio minimo y correcto.
   (b) Driver backlight custom (~80 lineas, estilo mt6582-dispfix) que solo escriba PWM_DUTY@0xA0 y reaplique el commit del MUTEX (MUTEX_EN(id)=1), SIN tocar 0xB0. Era el plan del HITO; sigue siendo valido y desacopla del bug de pwm-mtk-disp. Pero pierde la integracion limpia con pwm-backlight/atomic.
   RECOMENDADO: (a) — minimo parche, integra con DRM/pwm-backlight estandar.
2) COLOR RGB565 EN EL BLS: el BLS hace dithering de salida. Si arranca con el dither 888 que dejo el LK (ds_ddp_path.c L168) y el pipeline mainline es 565, puede haber bandeo/tinte. Hay que asegurar config_full con dither 565 (DITHER(15)@0xE3C=0x50500001, (16)@0xE40=0x50504040, (0)@0xE00=0x1) + luminance LUT@0x300. Quien lo escribe: si el comp BLS del CRTC no lo hace (mtk_drm trata BLS como passthrough en el mutex, NO programa dither/LUT), hay que hacerlo una vez (en el probe de la variante mt6582, o reusar el bloque BLS de mt6582-dispfix hasta validar). RIESGO MEDIO.
3) DOS DUENOS DEL REG 0xA000: mientras exista mt6582-dispfix (late_initcall) que escribe BLS_EN=0x10001/BLS_SETTING, y a la vez pwm-mtk-disp + CRTC, pelean. Hay que retirar el bloque BLS del dispfix cuando el path real tome control (igual que para OVL/RDMA/COLOR).
4) bit31 de PWM_DUTY: el downstream escribe 0x800003ff (L174). Si el driver mt2701 escribe el duty sin bit31 y el brillo no "latchea", anadir bit31 (probable flag de validez) en la variante. RIESGO BAJO (ddp_bls.c set_backlight no lo usa y funciona via mutex).
5) clock-names "main"/"mm": pwm-mtk-disp pide clk por nombre. Con fixed-clock stubs clk_prepare_enable es no-op; si el driver hace clk_get(dev,"main") y falta el name, falla probe. Asegurar los 2 names. BAJO.
6) ORDEN DE PROBE: el pwm_chip (pwm-mtk-disp) debe estar listo antes que pwm-backlight (defer ok). Y el comp BLS del CRTC requiere que el master mmsys pruebe. Cadenas de EPROBE_DEFER normales; vigilar que no quede colgado si el DSI/SOF no arranca.
7) DSI base: el mapeo cruzado indica DSI real MT6582 = 0x1400C000 (ddp_reg.h DDP_REG_BASE_DSI), NO 0x14012000. El SOF que latchea el BLS viene de ese DSI; si el DSI no da SOF (panel mal traido por LK) el mutex_acquire cuelga 10ms y el brillo no latchea. Dependencia dura del DSI.

**Pasos:**

1. 1. Confirmar el symbol Kconfig del driver de PWM en el arbol 7.0.12 (CONFIG_PWM_MEDIATEK_DISP vs CONFIG_PWM_MTK_DISP) y que pwm-mtk-disp.c trae el match mt2701 -> usa enable_mask=BIT(16), con0=0xa8, con1=0xac, bls_debug=0xb0, bls_debug_mask=0x3, has_commit=false (esa es la combinacion que rompe colores).
2. 2. Decidir estrategia anti-BLS_DEBUG: implementar mt6582_disp_pwm_data en pwm-mtk-disp.c IGUAL a mt2701 pero con bls_debug_mask=0 (o un flag .skip_debug) para que mtk_disp_pwm_apply NO escriba 0x3 en 0xB0; el latch lo da el MUTEX del CRTC. Anadir el match { .compatible="mediatek,mt6582-disp-pwm", .data=&mt6582_disp_pwm_data } AL DRIVER DE PWM, y en ml_mtk_drm_drv.c anadir la misma cadena con .data=MTK_DISP_BLS (para que el CRTC lo reconozca como DDP_COMPONENT_BLS). Si se prefiere DT estable, dejar compatible mt2701-disp-pwm y parchear la driver-data mt2701 a skip_debug (mas invasivo a otros SoC, peor).
3. 3. Integrar config_full RGB565: que el BLS quede en dither 565 + luminance LUT + gamma. Opciones: (a) en el probe de la variante mt6582 del pwm-mtk-disp escribir una vez el bloque de ddp_bls.c::disp_bls_config_full (rama dither_bpp==16: DITHER(15)@0xE3C=0x50500001,(16)@0xE40=0x50504040,(0)@0xE00=0x1; LUT@0x300..0x384; GAMMA; BLS_SETTING@0x10=0x00100007; SRC_SIZE@0x18=(960<<16)|540); (b) provisionalmente reusar el bloque BLS de mt6582-dispfix hasta validar colores. NO depender de que mtk_drm lo haga (trata BLS solo como entrada del MUTEX).
4. 4. DT: dejar el nodo disp_pwm@1400a000 (compatible elegido en paso 2) con clocks main/mm (2 fixed-clock) + nodo pwm-backlight. Asegurar que disp_pwm es hermano de ovl0/rdma0/color0/dsi0 bajo el padre que enumera mtk_drm_drv (for_each_child_of_node parent).
5. 5. Kconfig: DRM_MEDIATEK=y, MTK_MMSYS=y, MTK_CMDQ=n, PWM=y, PWM_MEDIATEK_DISP=y, BACKLIGHT_PWM=y, BACKLIGHT_CLASS_DEVICE=y. Recompilar (olddefconfig antes de zImage/dtbs).
6. 6. Verificar bind: dmesg -> el pwm_chip 1400a000.pwm proba; backlight pwm-backlight crea /sys/class/backlight/*; y el CRTC reporta DDP_COMPONENT_BLS en el path (drm.debug). Confirmar que el MUTEX del CRTC incluye bit9 (BLS): leer MUTEX_MOD(id)@0x1400E000+0x2c+0x20*id y ver bit9 set junto a 3/7/10.
7. 7. Probar brillo SIN romper color: echo a /sys/class/backlight/*/brightness en varios niveles; confirmar (a) el brillo cambia (PWM_DUTY@0xA0 se mueve) y (b) los colores NO se descuadran (leer 0xB0: debe seguir != 0x3). Si 0xB0 quedo 0x3 -> la variante skip_debug no se aplico; revisar.
8. 8. Validar latch por SOF: como el comp BLS esta en el mutex y el CRTC hace mtk_mutex_enable (MUTEX_EN=1) sincronizado con SOF del DSI, el duty debe latchear por frame (sin parpadeo ni tearing del brillo). Comparar contra el hack (que no latcheaba).
9. 9. Retirar del mt6582-dispfix.c las escrituras a BLS (bls+0x00/+0x10) y al MUTEX cuando el path real este estable, para que no haya dos duenos del 0xA000/0xE000.
10. 10. (Opcional robustez) Si el brillo no latchea, probar PWM_DUTY con bit31 (0x80000000|duty) en la variante; y si el panel sale con bandeo, ajustar la curva luminance LUT.
11. 11. Cross-check final con downstream: dump BLS regs (0x00,0x10,0x18,0xA0,0xA8,0xB0,0xE00,0xE3C,0xE40) y comparar con el estado bueno 3.10 (ds_ddp_path.c dispsys_bypass_bls) — el unico que DEBE diferir es DITHER (565 vs 888) y BLS_DEBUG (0 vs lo que sea).

---


## clocks-iommu-crtc (capa transversal: relojes sin CCF + IOMMU/M4U + mtk_drm_drv/mtk_crtc que une OVL→RDMA→COLOR→BLS→DSI en un CRTC)

**Veredicto:** crear-variante-mt6582 | **Esfuerzo:** muy-alto

**Compatible DT:** mmsys padre: "mediatek,mt2701-mmsys" (0x14000000). OVL: "mediatek,mt2701-disp-ovl" (0x14007000, fmt_rgb565_is_0=false → coincide con CLRFMT=1 del MT6582). RDMA: "mediatek,mt2701-disp-rdma" (0x14008000, requiere interrupts). COLOR: "mediatek,mt2701-disp-color" (0x1400B000). BLS: "mediatek,mt2701-disp-pwm" (0x1400A000, DDP_COMPONENT_BLS en el path). DSI: "mediatek,mt2701-dsi" (0x1400C000, NO 0x14012000). MUTEX: crear variante NUEVA "mediatek,mt6582-disp-mutex" (tabla mutex_mod de mt2701 + .no_clk=true al estilo mt8167); el offset MOD0=0x2c/SOF0=0x30 es el mismo (MT2701_MUTEX0_*).

**Offsets/diferencias:** Bases MT6582 confirmadas en ddp_reg.h (virt 0xF4→fis 0x14): MMSYS=0x14000000, OVL=0x14007000, RDMA=0x14008000, WDMA=0x14009000, BLS/DISP_PWM=0x1400A000, COLOR=0x1400B000, DSI=0x1400C000 (CORRIGE el prompt que decía 0x14012000), DPI=0x1400D000, MUTEX=0x1400E000, CMDQ=0x1400F000. Routing OVL→RDMA: DISP_OVL_MOUT_EN=MMSYS+0x030, valor 1<<0 (ds_ddp_path.c L921/L1398) — exactamente lo que escribe la tabla de routing mt2701 de mainline. Mutex: MOD0=0x2c, SOF0=0x30, EN(n)=0x20+0x20*n, MUTEX(n)=0x24+0x20*n (ml_mtk-mutex.c L18-27; el BLS usa SOF=DSI0). Diferencias que obligan a variante: (a) mtk-mmsys.c L429 registra clk provider CCF mt2701 inexistente (bloqueante, se evita con fixed-clocks por nodo); (b) DSI clk_set_rate sobre fixed-clock = -EINVAL (L695); (c) RDMA platform_get_irq obligatorio (L323); (d) mt8167/mt2701 mutex_mod no cubren a la vez .no_clk + BLS bit → variante mt6582; (e) OVL fmt: mt2701 (rgb565_is_0=false) sí, mt8173 (=true) NO. CMDQ/M4U/SMI-larb/SCPSYS ausentes en MT6582 mainline: omitir en los nodos (todos opcionales en los probes).

**Nodo DT:**
```
/* STUBS DE RELOJ (sin CCF) */
/ {
  mm_dummy_clk: mm-dummy-clk { compatible="fixed-clock"; #clock-cells=<0>; clock-frequency=<156000000>; };
  dsi_hs_clk:  dsi-hs-clk  { compatible="fixed-clock"; #clock-cells=<0>; clock-frequency=<360000000>; }; /* set_rate fallará: ver riesgos */
};
&{/soc} {
  mmsys: syscon@14000000 { compatible="mediatek,mt2701-mmsys","syscon"; reg=<0x14000000 0x1000>; #clock-cells=<1>; };
  ovl0: ovl@14007000  { compatible="mediatek,mt2701-disp-ovl";  reg=<0x14007000 0x1000>; interrupts=<GIC_SPI OVL_IRQ  IRQ_TYPE_LEVEL_LOW>; clocks=<&mm_dummy_clk>; };
  rdma0: rdma@14008000{ compatible="mediatek,mt2701-disp-rdma"; reg=<0x14008000 0x1000>; interrupts=<GIC_SPI RDMA_IRQ IRQ_TYPE_LEVEL_LOW>; clocks=<&mm_dummy_clk>; mediatek,rdma-fifo-size=<8192>; };
  color0: color@1400b000{compatible="mediatek,mt2701-disp-color";reg=<0x1400b000 0x1000>;interrupts=<GIC_SPI COLOR_IRQ IRQ_TYPE_LEVEL_LOW>;clocks=<&mm_dummy_clk>; };
  bls: pwm@1400a000   { compatible="mediatek,mt2701-disp-pwm";   reg=<0x1400a000 0x1000>; interrupts=<GIC_SPI BLS_IRQ IRQ_TYPE_LEVEL_LOW>; clocks=<&mm_dummy_clk>,<&mm_dummy_clk>; clock-names="main","mm"; #pwm-cells=<2>; };
  mutex: mutex@1400e000{compatible="mediatek,mt6582-disp-mutex"; reg=<0x1400e000 0x1000>; interrupts=<GIC_SPI MUTEX_IRQ IRQ_TYPE_LEVEL_LOW>; }; /* data.no_clk=true */
  dsi0: dsi@1400c000  { compatible="mediatek,mt2701-dsi"; reg=<0x1400c000 0x1000>; interrupts=<GIC_SPI DSI_IRQ IRQ_TYPE_LEVEL_LOW>;
    clocks=<&mm_dummy_clk>,<&mm_dummy_clk>,<&dsi_hs_clk>; clock-names="engine","digital","hs";
    phys=<&mipi_tx0>; phy-names="dphy";
    port{ dsi0_out:endpoint{ remote-endpoint=<&panel_in>; };};
    panel@0{ compatible="truly,hx8389"; reg=<0>; port{ panel_in:endpoint{ remote-endpoint=<&dsi0_out>; };};};
  };
};
/* NOTA: resolver OVL_IRQ/RDMA_IRQ/COLOR_IRQ/BLS_IRQ/MUTEX_IRQ/DSI_IRQ desde la tabla GIC SPI del MT6582 (símbolos MT6582_DISP_*_IRQ_ID). Reservar CMA (cma=8M o nodo linux,cma) en lugar de M4U. NO poner 'iommus'. */
```

**Kconfig:** CONFIG_DRM=y; CONFIG_DRM_MEDIATEK=y (incluye mtk_drm_drv+mtk_crtc+OVL/RDMA/COLOR); CONFIG_DRM_MEDIATEK_DSI=y; CONFIG_MTK_MMSYS=y (lanza mediatek-drm; cuidado clk_driver); CONFIG_PHY_MTK_MIPI_DSI=y (para el DSI, tarea aparte); CONFIG_DRM_FBDEV_EMULATION=y; CONFIG_DMA_CMA=y + reserva CMA. OFF: CONFIG_MTK_CMDQ=n (CMDQ opcional en todos los probes), CONFIG_MTK_IOMMU=n (no hay M4U; OVL escanea físico/CMA). NO seleccionar COMMON_CLK_MT2701/MT2701_MM. Cmdline: conservar clk_ignore_unused.

**Clocks:** SIN CCF se resuelve con fixed-clock stubs apuntados desde CADA nodo de componente (no desde un provider mmsys). Inventario exacto de lo que pide cada driver: OVL devm_clk_get(NULL) [ml_mtk_disp_ovl.c L625]; RDMA devm_clk_get(NULL) [L327]; COLOR devm_clk_get(NULL) (init inline en mtk_drm_drv); MUTEX devm_clk_get(NULL) SOLO si data.no_clk==false [ml_mtk-mutex.c L1109] → usar variante con .no_clk=true (como mt8167, L763) para NO pedir clock; DSI pide TRES por nombre: 'engine','digital','hs' [ml_mtk_dsi.c L1210/1216/1221] y hace clk_set_rate('hs',data_rate) [L695] + clk_prepare_enable(engine/digital) [L703/709]. Receta: un único 'mm_dummy_clk' compatible='fixed-clock' clock-frequency≈156MHz para engine/digital/ovl/rdma/color/bls (clk_prepare_enable sobre fixed-clock = no-op, válido porque el LK deja DISPSYS+MMSYS_CG encendidos y el cmdline ya trae clk_ignore_unused). Para 'hs' del DSI hace falta un clk que acepte set_rate: usar un clk_hw a medida con .set_rate no-op (devolviendo 0) o assigned-clock-rates, porque un fixed-clock devuelve -EINVAL en clk_set_rate y rompería dsi_enable. El provider de clocks del mmsys (mtk-mmsys.c L429 registra 'clk-mt2701-mm') se evita NO referenciando <&mmsys N> en ningún componente; si aun así molesta, parche de 1 línea para saltar ese register. NINGÚN clock AP real se toca; CMDQ/GCE off (opcional en todos los probes).

**Dependencias:** DSI+PHY mipi-tx mt2701 (sin ellos mtk_crtc rechaza el path: exige DSI0/DPI0 como último componente, ml_mtk_drm_drv.c L982-996). Panel HX8389 portado a drm_panel/mipi_dsi. Variante mt6582-disp-mutex (.no_clk). BLS ya operativo como backlight pwm-mt2701 (se reintegra como DDP_COMPONENT_BLS). CMA/DMA contiguo (sustituye M4U). clk_ignore_unused en cmdline (ya presente). Desactivar dispfix.c (excluyente).

**Riesgos:** Bloqueante #1: mtk-mmsys.c L429 registra clk provider CCF mt2701 inexistente → mitigar con fixed-clocks por nodo (no usar <&mmsys> como clock) o parche 1 línea. Bloqueante #2: DSI clk_set_rate(hs) sobre fixed-clock devuelve -EINVAL (L695) → dsi_enable falla → CRTC sin frames → clk set_rate no-op. Bloqueante #3: vblank del CRTC depende de IRQ del OVL (L185) + STREAM_DONE del mutex SOF=DSI0; IRQ mal mapeada = commit colgado (panel video-mode, SOF del DSI). Riesgo #4 IOMMU: sin M4U el OVL (dma_dev del DRM, L558-575) escanea físico/CMA → OK con DMA_CMA y sin iommus; MTK_IOMMU=n. Riesgo #5: MMSYS_CG cerrado = registros COLOR/RDMA en basura → clk_ignore_unused. Riesgo #6: dispfix vs driver real colisionan en OVL/MUTEX → desactivar hack, conservar backup simpledrm.

**Pasos:**

1. 1. Reservar CMA contiguo (cma=8M o nodo linux,cma) y CONFIG_DMA_CMA=y; NO declarar iommus en el OVL (el OVL es el dma_dev del DRM y escanea físico).
2. 2. Resolver números GIC SPI reales de MT6582_DISP_{OVL,RDMA,COLOR,BLS,MUTEX,DSI}_IRQ_ID (grep tabla irq downstream) y rellenarlos: RDMA falla el probe sin IRQ, y el vblank del CRTC depende de la IRQ del OVL.
3. 3. Añadir variante 'mediatek,mt6582-disp-mutex' en ml_mtk-mutex.c: mtk_mutex_data con mutex_mod=mt2701_mutex_mod (define BLS/COLOR/OVL/RDMA), mutex_mod_reg=MT2701_MUTEX0_MOD0, mutex_sof_reg=MT2701_MUTEX0_SOF0, .no_clk=true; registrar en mutex_driver_dt_match.
4. 4. Evitar el provider CCF del mmsys: poner clocks=<&mm_dummy_clk> en cada componente (sin referenciar <&mmsys>). Verificar ausencia de EPROBE_DEFER por clock en boot; si surge, parchear mtk_mmsys_probe para no registrar clk_driver.
5. 5. Arreglar el 'hs' del DSI (clk_set_rate): clk_hw con set_rate no-op o assigned-clock-rates; coordinar con la tarea DSI (phy mipi-tx + panel HX8389).
6. 6. Desactivar mt6582-dispfix.c (quitar del Makefile/initcall) antes de probar el driver real.
7. 7. Compilar con MTK_CMDQ=n, MTK_IOMMU=n; arrancar con initcall_debug y validar la cadena mmsys→mediatek-drm→bind(mutex)→component_bind_all→mtk_crtc_create.
8. 8. Confirmar /dev/dri/card0 creado por mtk_drm (no simpledrm) y que mtk_crtc_dma_dev_get devuelve el OVL (si no, el OVL no bindó: revisar clock/IRQ).
9. 9. Probar modeset; si drm_atomic_commit cuelga, revisar IRQ del OVL y reproducir la config de mutex que ya valida el hack (MOD=0x680/SOF=1/EN=0x303) ahora vía mtk_mutex_add_comp(DSI0).
10. 10. Verificar que con el path real el BLS queda latcheado por el SOF (objetivo: backlight sincronizado/vsync) y que el color sigue correcto (mt2701 ovl fmt_rgb565_is_0=false).
11. 11. Mantener kernel de respaldo simpledrm+dispfix (backup #25) e iterar IRQ/mutex/DSI hasta commit estable.

---
