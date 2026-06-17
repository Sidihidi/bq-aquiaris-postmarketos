# Plan EINT (interrupciones externas) MT6582 — INVESTIGACIÓN COMPLETA (2026-06-17)

Objetivo: controlador de interrupciones externas → **táctil-por-IRQ** (en vez de polling),
**botones HW** (gpio-keys), y base para el driver de kernel `edt-ft5x06`.

## Hallazgos clave (todo verificado contra mainline + downstream)
- **`mtk-eint.c` (mainline) es una librería** (no driver standalone). La usa el pinctrl.
- **El layout de registros del MT6582 == `mtk_generic_eint_regs`** EXACTO (stat=0x0, ack=0x40,
  mask=0x80/set 0xc0/clr 0x100, sens=0x140, soft=0x200, pol=0x300, dom_en=0x400, dbnc=0x500).
  → pasar `eint->regs = NULL` y la librería usa el genérico. ✅
- **EINT base física = `0x1000B000`** (virt 0xF000B000 en downstream).
- **169 EINTs** (`EINT_AP_MAXNUMBER/EINT_MAX_CHANNEL = 169`).
- **IRQ padre = GIC_SPI 113**, level-high (`X_DEFINE_IRQ(MT_EINT_IRQ_ID, 145,...)`; 145-32=113).
- **Táctil = EINT117, flanco de BAJADA** (`CUST_EINT_TOUCH_PANEL_NUM=117`, TRIGGER_FALLING).
- `eint_hw` para MT6582: `{ .port_mask=7, .ports=6, .ap_num=169, .db_cnt=16, .db_time=debounce_time_mt2701 }`.
- GPIO base = `0x10005000`. Layout (de los pokes del táctil): DIR @0x000+bank*0x10,
  DOUT val/set/clr @0x400+bank*0x10 (+0/+4/+8), DIN @0x500?, MODE (mux) @0x700 (3 bits/pin).
  GPIO115 → DIR 0x070, DOUT 0x470/474/478, MODE 0x770 (bank=n/16, bit=n%16).

## ⚠️ Restricción que define el tamaño
`mtk_eint_irq_request_resources()` llama a **`gpiochip_lock_as_irq(gpio_c, gpio_n)`** — la
librería EXIGE un `gpio_chip` real detrás (no vale stub con gpio_c=NULL → crash). Por tanto
hay que registrar un **gpio_chip mínimo del MT6582** (ngpio=169) además del eint.

## Plan de implementación (~250-300 líneas)
1. `drivers/pinctrl/mediatek/gpio-mt6582-eint.c` (driver nuevo):
   - gpio_chip mínimo: `get/set/direction_input/output/get_direction` leyendo DIR/DOUT/DIN.
   - `set_gpio_as_eint`: poner MODE del pin a función EINT + DIR=in (vía registro MODE).
   - struct mtk_eint: nbase=1, base[0]=ioremap(0x1000B000), irq=irq_of_parse_and_map,
     hw=&mt6582_eint_hw, regs=NULL, gpio_xlate=&xt (get_gpio_n devuelve nuestro chip,
     get_gpio_state=gpio_get, set_gpio_as_eint), pctl=drvdata.
   - `mtk_eint_do_init(eint, NULL)` (NULL → construye los 169 pines solo).
2. Kconfig: `config GPIO_MT6582_EINT ... select EINT_MTK select GPIOLIB select IRQ_DOMAIN`.
   Makefile: `obj-$(CONFIG_GPIO_MT6582_EINT) += gpio-mt6582-eint.o`. Habilitar en config.
3. DT: nodo `eint: interrupt-controller@1000b000 { compatible="mediatek,mt6582-eint";
   reg=<0x1000b000 0x1000>; interrupt-controller; #interrupt-cells=<2>; gpio-controller;
   #gpio-cells=<2>; interrupts=<GIC_SPI 113 IRQ_TYPE_LEVEL_HIGH>; };`
4. Win inmediato: `gpio-keys` (botones power/volumen) con `interrupts-extended=<&eint N ...>`.
5. Luego: táctil por `edt-ft5x06` (`interrupts-extended=<&eint 117 IRQ_TYPE_EDGE_FALLING>` +
   reset-gpios=<&eint 115 ...> + vcc-supply=<&mt6323_vgp1_reg>) → retira el daemon userspace.

## Plantilla de montaje del eint (de pinctrl-mtk-common.c:1019-1047)
eint=devm_kzalloc; nbase=1; base=kzalloc(1 ptr); base[0]=devm_platform_ioremap_resource;
irq=irq_of_parse_and_map(np,0); dev=&pdev->dev; regs=NULL; hw=&mt6582_eint_hw;
pctl=drvdata; gpio_xlate=&xt; mtk_eint_do_init(eint, NULL).

## Riesgo
Sin riesgo de pantalla. Iteraciones de reflasheo (BROM `wo 0x2900000` o fastboot) para depurar.
Si un EINT no dispara: comprobar que el MODE del pad está en función EINT (poke si hace falta).
