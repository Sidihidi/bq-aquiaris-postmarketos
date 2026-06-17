// SPDX-License-Identifier: GPL-2.0
/*
 * gpio-mt6582-eint.c — GPIO + EINT (external interrupt) controller del MediaTek
 * MT6582 (BQ Aquaris E4.5 "krillin"), para mainline.
 *
 * El MT6582 no tiene pinctrl en mainline. Este driver mínimo provee:
 *   - un gpio_chip básico (get/set/dir) sobre el bloque GPIO @0x10005000, y
 *   - el controlador de interrupciones externas (EINT) @0x1000B000 usando la
 *     librería mtk-eint (mtk_eint_do_init + mtk_generic_eint_regs).
 *
 * La librería mtk-eint EXIGE un gpio_chip real detrás (gpiochip_lock_as_irq),
 * por eso ambos van juntos en este driver.
 *
 * Layout GPIO (= generación MT8127; DOUT confirmado por los pokes del táctil:
 * GPIO115 → DOUT 0x470/set0x474/clr0x478, bit 3):
 *   reg de banco = (gpio>>4)*0x10 ; bit = gpio&0xf ; SET=+4, CLR=+8
 *   dir=0x000  dout=0x400  din=0x500  (pinmux=0x600, no usado aquí)
 *
 * EINT: registros == mtk_generic_eint_regs, 169 EINTs, IRQ padre GIC_SPI 113.
 * Mapeo EINT↔GPIO: 1:1 (eint_n == gpio_n).  [VERIFICAR para EINT117/táctil]
 *
 * DT esperado:
 *   eint: interrupt-controller@1000b000 {
 *       compatible = "mediatek,mt6582-eint";
 *       reg = <0x1000b000 0x1000>,   // [0] EINT
 *             <0x10005000 0x1000>;   // [1] GPIO
 *       interrupt-controller; #interrupt-cells = <2>;
 *       gpio-controller;       #gpio-cells = <2>;
 *       interrupts = <GIC_SPI 113 IRQ_TYPE_LEVEL_HIGH>;
 *   };
 */
#include <linux/gpio/driver.h>
#include <linux/io.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>

#include "mtk-eint.h"

#define MT6582_NGPIO		169

#define GPIO_DIR_OFF		0x000
#define GPIO_DOUT_OFF		0x400
#define GPIO_DIN_OFF		0x500
#define GPIO_SET		0x4
#define GPIO_CLR		0x8

/* registro de banco + bit dentro del banco */
static inline u32 bank_reg(unsigned int off, u32 base)
{
	return ((off >> 4) << 4) + base;	/* (off/16)*0x10 + base */
}
static inline u32 pin_bit(unsigned int off)
{
	return BIT(off & 0xf);
}

struct mt6582_eint {
	struct device		*dev;
	void __iomem		*gpio;	/* 0x10005000 */
	struct gpio_chip	chip;
	struct mtk_eint		*eint;
};

/* -------- gpio_chip básico -------- */

static int mt6582_gpio_get(struct gpio_chip *chip, unsigned int off)
{
	struct mt6582_eint *g = gpiochip_get_data(chip);

	return !!(readl(g->gpio + bank_reg(off, GPIO_DIN_OFF)) & pin_bit(off));
}

static int mt6582_gpio_set(struct gpio_chip *chip, unsigned int off, int val)
{
	struct mt6582_eint *g = gpiochip_get_data(chip);
	u32 reg = bank_reg(off, GPIO_DOUT_OFF) + (val ? GPIO_SET : GPIO_CLR);

	writel(pin_bit(off), g->gpio + reg);
	return 0;
}

static int mt6582_gpio_direction_input(struct gpio_chip *chip, unsigned int off)
{
	struct mt6582_eint *g = gpiochip_get_data(chip);

	/* dir bit: 1=out, 0=in -> escribir en CLR pone dir=in */
	writel(pin_bit(off), g->gpio + bank_reg(off, GPIO_DIR_OFF) + GPIO_CLR);
	return 0;
}

static int mt6582_gpio_direction_output(struct gpio_chip *chip, unsigned int off,
					int val)
{
	struct mt6582_eint *g = gpiochip_get_data(chip);

	mt6582_gpio_set(chip, off, val);
	writel(pin_bit(off), g->gpio + bank_reg(off, GPIO_DIR_OFF) + GPIO_SET);
	return 0;
}

static int mt6582_gpio_get_direction(struct gpio_chip *chip, unsigned int off)
{
	struct mt6582_eint *g = gpiochip_get_data(chip);

	if (readl(g->gpio + bank_reg(off, GPIO_DIR_OFF)) & pin_bit(off))
		return GPIO_LINE_DIRECTION_OUT;
	return GPIO_LINE_DIRECTION_IN;
}

static int mt6582_gpio_to_irq(struct gpio_chip *chip, unsigned int off)
{
	struct mt6582_eint *g = gpiochip_get_data(chip);

	return mtk_eint_find_irq(g->eint, off);
}

/* -------- callbacks que la librería mtk-eint pide (mtk_eint_xt) -------- */

static int mt6582_xt_get_gpio_n(void *data, unsigned long eint_n,
				unsigned int *gpio_n, struct gpio_chip **gchip)
{
	struct mt6582_eint *g = data;

	if (eint_n >= MT6582_NGPIO)
		return -EINVAL;
	*gchip = &g->chip;
	*gpio_n = eint_n;		/* mapeo 1:1 EINT<->GPIO */
	return 0;
}

static int mt6582_xt_get_gpio_state(void *data, unsigned long eint_n)
{
	struct mt6582_eint *g = data;

	return mt6582_gpio_get(&g->chip, eint_n);
}

static int mt6582_xt_set_gpio_as_eint(void *data, unsigned long eint_n)
{
	struct mt6582_eint *g = data;

	/*
	 * El pad del táctil (EINT117) ya queda en modo EINT por el bootloader.
	 * Aquí solo garantizamos dirección de entrada. Si algún EINT no dispara,
	 * habrá que poner además el mux (pinmux_offset 0x600) a la función EINT.
	 */
	mt6582_gpio_direction_input(&g->chip, eint_n);
	return 0;
}

static const struct mtk_eint_xt mt6582_eint_xt = {
	.get_gpio_n	= mt6582_xt_get_gpio_n,
	.get_gpio_state	= mt6582_xt_get_gpio_state,
	.set_gpio_as_eint = mt6582_xt_set_gpio_as_eint,
};

/* tiempos de debounce soportados por el HW (us), estilo familia v1 */
static const unsigned int mt6582_eint_debounce[] = {
	500, 1000, 16000, 32000, 64000, 128000, 256000,
};

static const struct mtk_eint_hw mt6582_eint_hw = {
	.port_mask	= 7,
	.ports		= 6,
	.ap_num		= MT6582_NGPIO,	/* 169 */
	.db_cnt		= 16,
	.db_time	= mt6582_eint_debounce,
};

static int mt6582_eint_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_eint *g;
	struct mtk_eint *eint;
	int ret, irq;

	g = devm_kzalloc(dev, sizeof(*g), GFP_KERNEL);
	if (!g)
		return -ENOMEM;
	g->dev = dev;

	eint = devm_kzalloc(dev, sizeof(*eint), GFP_KERNEL);
	if (!eint)
		return -ENOMEM;
	g->eint = eint;

	/* mtk-eint espera un array de bases */
	eint->base = devm_kcalloc(dev, 1, sizeof(*eint->base), GFP_KERNEL);
	if (!eint->base)
		return -ENOMEM;

	/* reg[0] = EINT (0x1000b000), reg[1] = GPIO (0x10005000) */
	eint->base[0] = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(eint->base[0]))
		return PTR_ERR(eint->base[0]);

	g->gpio = devm_platform_ioremap_resource(pdev, 1);
	if (IS_ERR(g->gpio))
		return PTR_ERR(g->gpio);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	/* gpio_chip primero: la librería lo necesita registrado */
	g->chip.label		= "mt6582-gpio";
	g->chip.parent		= dev;
	g->chip.owner		= THIS_MODULE;
	g->chip.fwnode		= dev_fwnode(dev);
	g->chip.base		= -1;
	g->chip.ngpio		= MT6582_NGPIO;
	g->chip.get		= mt6582_gpio_get;
	g->chip.set		= mt6582_gpio_set;
	g->chip.direction_input	= mt6582_gpio_direction_input;
	g->chip.direction_output = mt6582_gpio_direction_output;
	g->chip.get_direction	= mt6582_gpio_get_direction;
	g->chip.to_irq		= mt6582_gpio_to_irq;

	ret = devm_gpiochip_add_data(dev, &g->chip, g);
	if (ret)
		return dev_err_probe(dev, ret, "no se pudo registrar gpio_chip\n");

	/* EINT */
	eint->dev		= dev;
	eint->nbase		= 1;
	eint->irq		= irq;
	eint->regs		= NULL;		/* -> mtk_generic_eint_regs */
	eint->hw		= &mt6582_eint_hw;
	eint->num_db_time	= ARRAY_SIZE(mt6582_eint_debounce);
	eint->pctl		= g;
	eint->gpio_xlate	= &mt6582_eint_xt;

	ret = mtk_eint_do_init(eint, NULL);	/* NULL -> autoconstruye 169 pines */
	if (ret)
		return dev_err_probe(dev, ret, "mtk_eint_do_init falló\n");

	platform_set_drvdata(pdev, g);
	dev_info(dev, "MT6582 GPIO+EINT listo (%d pines, irq %d)\n",
		 MT6582_NGPIO, irq);
	return 0;
}

static const struct of_device_id mt6582_eint_of_match[] = {
	{ .compatible = "mediatek,mt6582-eint" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_eint_of_match);

static struct platform_driver mt6582_eint_driver = {
	.driver = {
		.name		= "mt6582-eint",
		.of_match_table	= mt6582_eint_of_match,
	},
	.probe = mt6582_eint_probe,
};
builtin_platform_driver(mt6582_eint_driver);

MODULE_DESCRIPTION("MediaTek MT6582 GPIO + EINT controller");
MODULE_LICENSE("GPL");
