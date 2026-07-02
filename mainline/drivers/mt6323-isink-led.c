// SPDX-License-Identifier: GPL-2.0
/*
 * mt6323-isink-leds — RGB LED + button backlight driver for BQ Aquaris E4.5
 *
 * Los LEDs del krillin son 4 current sinks (ISINK0-3) del PMIC MT6323:
 *   ISINK0 = verde, ISINK1 = rojo, ISINK2 = azul, ISINK3 = botón
 * Se controlan por registros PMIC via pwrap regmap:
 *   - ISINKx_CON0 (0x330+ch*8): mode[2:3], enable[15]
 *   - ISINKx_CON1 (0x332+ch*8): dim duty[8:12], dim fsel[0:15]
 *   - ISINKx_CON2 (0x334+ch*8): step[12:14] (0=4mA ... 3=16mA)
 *   - ISINKx_CON3 (0x336+ch*8): breath timing
 *   - Clock enable: OCGRES (bit ch) + ISINK_EN_CTRL
 *
 * Expone cada ISINK como un LED class device (brightness 0=off, 1-255=on con duty).
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/mfd/mt6323/registers.h>

/* ISINK register offsets from PMIC base */
#define ISINK_CON0(ch)	(0x0330 + (ch) * 8)
#define ISINK_CON1(ch)	(0x0332 + (ch) * 8)
#define ISINK_CON2(ch)	(0x0334 + (ch) * 8)
#define ISINK_CON3(ch)	(0x0336 + (ch) * 8)
#define ISINK_EN_CTRL	0x0356
#define OCGRES0		0x0102  /* clock gate register (ISINK clock bits 0-3) */

/* ISINK_CON0 bits */
#define CH_EN_SHIFT	15
#define CH_MODE_SHIFT	2
#define CH_MODE_PWM	1

/* ISINK_CON2 bits */
#define CH_STEP_SHIFT	12
#define CH_STEP_16MA	3

/* ISINK_CON1 bits */
#define DIM_DUTY_SHIFT	8
#define DIM_DUTY_MAX	31

struct mt6323_isink_led {
	struct led_classdev cdev;
	struct regmap *regmap;
	u32 channel;	/* 0-3 */
};

static void mt6323_isink_set(struct led_classdev *cdev, enum led_brightness brightness)
{
	struct mt6323_isink_led *led = container_of(cdev, struct mt6323_isink_led, cdev);
	struct regmap *r = led->regmap;
	int ch = led->channel;
	u16 con0, con1, con2;

	if (brightness) {
		u32 duty = (brightness * DIM_DUTY_MAX) / 255;

		/* habilitar clock del ISINK */
		regmap_set_bits(r, OCGRES0, BIT(ch));
		/* CON2: step = 16mA */
		regmap_write(r, ISINK_CON2(ch), CH_STEP_16MA << CH_STEP_SHIFT);
		/* CON1: duty cycle */
		regmap_write(r, ISINK_CON1(ch), (duty << DIM_DUTY_SHIFT));
		/* CON0: mode=PWM, enable */
		con0 = (CH_MODE_PWM << CH_MODE_SHIFT) | BIT(CH_EN_SHIFT);
		regmap_write(r, ISINK_CON0(ch), con0);
	} else {
		/* disable */
		regmap_clear_bits(r, ISINK_CON0(ch), BIT(CH_EN_SHIFT));
		regmap_clear_bits(r, OCGRES0, BIT(ch));
	}
}

static int mt6323_isink_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node, *child;
	struct regmap *regmap;
	int ret, count = 0;

	regmap = dev_get_regmap(dev->parent, NULL);
	if (!regmap)
		return -ENODEV;

	for_each_available_child_of_node(np, child) {
		struct mt6323_isink_led *led;
		u32 ch;
		const char *name;

		if (of_property_read_u32(child, "reg", &ch) || ch > 3)
			continue;

		of_property_read_string(child, "label", &name);
		if (!name) {
			const char *col[] = {"green", "red", "blue", "button"};
			name = col[ch];
		}

		led = devm_kzalloc(dev, sizeof(*led), GFP_KERNEL);
		if (!led)
			return -ENOMEM;
		led->regmap = regmap;
		led->channel = ch;
		led->cdev.name = devm_kasprintf(dev, GFP_KERNEL,
				"mt6323:%s:%d", name, ch);
		led->cdev.brightness_set = mt6323_isink_set;
		led->cdev.max_brightness = 255;

		ret = devm_led_classdev_register(dev, &led->cdev);
		if (ret) {
			of_node_put(child);
			return ret;
		}
		count++;
	}

	dev_info(dev, "mt6323 ISINK LEDs: %d registrados\n", count);
	return 0;
}

static const struct of_device_id mt6323_isink_match[] = {
	{ .compatible = "mediatek,mt6323-isink-led" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6323_isink_match);

static struct platform_driver mt6323_isink_driver = {
	.probe = mt6323_isink_probe,
	.driver = {
		.name = "mt6323-isink-led",
		.of_match_table = mt6323_isink_match,
	},
};
module_platform_driver(mt6323_isink_driver);

MODULE_DESCRIPTION("MT6323 ISINK LED driver (RGB + button backlight)");
MODULE_LICENSE("GPL");
