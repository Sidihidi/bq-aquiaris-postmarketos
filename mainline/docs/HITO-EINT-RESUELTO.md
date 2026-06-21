# HITO — EINT (interrupciones externas) MT6582 RESUELTO (2026-06-17)

Driver GPIO + EINT del MT6582 **funcionando en mainline**. El táctil dispara su
línea de interrupción (EINT117) y el kernel recibe y entrega las IRQ. Esto valida
la infraestructura para: **táctil-por-IRQ**, **botones HW** (gpio-keys) y el driver
de kernel **edt-ft5x06**.

## Resultado (verificado en hardware)
```
dmesg:  mt6582-eint 1000b000.interrupt-controller: MT6582 GPIO+EINT listo (169 pines, irq 25)
/proc/interrupts:  143:  mt-eint 117 Edge  touch-int-test
evtest (gpio-keys de prueba sobre EINT117):  cada toque en la pantalla -> KEY_F1 press/release
```
El flood de KEY_F1 al tocar confirma que el INT del FT5336 es muy activo (pulsa por
muestra, ~60-100 Hz). Para botones reales será un evento por pulsación.

## Driver: `drivers/pinctrl/mediatek/gpio-mt6582-eint.c`
- gpio_chip mínimo (get/set/direction/get_direction/to_irq) sobre el bloque GPIO
  `0x10005000`, + EINT (librería `mtk-eint`) sobre `0x1000b000`.
- `mtk_eint_do_init(eint, NULL)` → autoconstruye los 169 pines; `regs=NULL` → usa
  `mtk_generic_eint_regs`.
- `mtk_eint_xt`: `get_gpio_n` 1:1 (eint_n == gpio_n), `get_gpio_state` = gpio_get,
  `set_gpio_as_eint` = poner DIR=input (el pad del táctil ya está en modo EINT por el LK).
- Layout GPIO (= generación MT8127, confirmado por los pokes del táctil):
  `dir=0x000 dout=0x400 din=0x500`; banco = `(gpio>>4)*0x10`, bit = `gpio&0xf`, SET=+4, CLR=+8.
- `mt6582_eint_hw`: `port_mask=7, ports=6, ap_num=169, db_cnt=16`. IRQ padre `GIC_SPI 113`.
- Detalles de versión (kernel 7.0.12): `gpio_chip` usa **`fwnode`** (no `of_node`);
  `.set` devuelve **`int`** (no void).

## DT (`arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts`)
```dts
&{/soc} {
    eint: interrupt-controller@1000b000 {
        compatible = "mediatek,mt6582-eint";
        reg = <0x1000b000 0x1000>,   /* [0] EINT */
              <0x10005000 0x1000>;   /* [1] GPIO */
        interrupt-controller; #interrupt-cells = <2>;
        gpio-controller;       #gpio-cells = <2>;
        interrupts = <GIC_SPI 113 IRQ_TYPE_LEVEL_HIGH>;
    };
};
/* test (a sustituir por botones reales): INT del táctil como KEY_F1 */
/ {
    eint_test_keys: eint-test-keys {
        compatible = "gpio-keys";
        key-touchint {
            label = "touch-int-test";
            linux,code = <59>;  /* KEY_F1 */
            interrupts-extended = <&eint 117 IRQ_TYPE_EDGE_FALLING>;
        };
    };
};
```

## Kconfig / Makefile (`drivers/pinctrl/mediatek/`)
```
config GPIO_MT6582_EINT
	bool "MediaTek MT6582 GPIO + EINT controller"
	depends on ARCH_MEDIATEK || COMPILE_TEST
	select EINT_MTK
	select GPIOLIB
	select IRQ_DOMAIN
```
`obj-$(CONFIG_GPIO_MT6582_EINT) += gpio-mt6582-eint.o`  (+ `=y` en build-krillin/.config)

## Cómo se construyó el boot.img (#23)
zImage+dtb nuevo, manteniendo el initrd del #22 (todos los servicios):
`abootimg -x boot-pmic.img` (extrae initrd+cfg), `abootimg --create boot-eint.img -f cfg2
(sin bootsize) -k zimage-dtb-mtk -r initrd.img`. (El `abootimg -u` no vale: el kernel
nuevo es ~4 KB mayor y no cabe en el hueco fijo.)

## Siguiente
1. **Botones HW reales**: sustituir el gpio-keys de prueba por power/volumen
   (investigar sus EINT en el árbol downstream `cust_eint.h`). Win tangible para la GUI.
2. **Táctil por IRQ** (`edt-ft5x06`, `interrupts-extended=<&eint 117 ...>`): requiere
   antes arreglar el apdma del i2c-mt65xx para lecturas >8 bytes; entonces se retira
   el daemon userspace.
3. Roadmap: WiFi, Audio, ruta Phosh.
