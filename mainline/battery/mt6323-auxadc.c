// SPDX-License-Identifier: GPL-2.0
/*
 * mt6323-auxadc — VBAT (BATSNS, canal 7 del AUXADC del PMIC MT6323) como hwmon.
 *
 * Sustituye la lectura userspace por pwrap_poke (MMIO crudo al pwrap
 * @0x1000d000 SIN lock contra el driver pwrap del kernel; sospechosa del
 * wedge del bloque de interrupciones del PMIC observado el 2026-07-08):
 * aqui todo va por el regmap del mt6397, serializado con el resto de
 * usuarios del pwrap (keys, rtc, regulator, codec).
 *
 * Receta AUXADC validada en HW (identica a /usr/local/bin/battery):
 *   CON11 (0x0758) bit4  = VBUF enable
 *   CON22 (0x076E) bit7  = request canal 7 (BATSNS)
 *   ADC0  (0x0714) bit15 = ready, bits 14:0 = raw
 *   VBAT_mV = raw * 4 * 1800 / 32768   (divisor externo x4, ref 1800 mV, 15 bit)
 * Validado: raw=18609 -> 4089 mV, coincide con la medida real cargando.
 *
 * El regmap del MT6323 se obtiene como en mt6582-afe-pcm.c: phandle DT
 * mediatek,pmic -> platform device del mt6323 (hijo del MFD mt6397) ->
 * drvdata = mt6397_chip -> chip->regmap.
 *
 * Sysfs: /sys/class/hwmon/hwmonN/{name=mt6323_auxadc, in0_input(mV), in0_label=vbat}
 */

#include <linux/bits.h>
#include <linux/delay.h>
#include <linux/hwmon.h>
#include <linux/math64.h>
#include <linux/mfd/mt6397/core.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#define MT6323_AUXADC_ADC0		0x0714
#define MT6323_AUXADC_CON11		0x0758
#define MT6323_AUXADC_CON22		0x076e

#define MT6323_AUXADC_RDY		BIT(15)		/* ADC0 */
#define MT6323_AUXADC_DATA		GENMASK(14, 0)	/* ADC0 */
#define MT6323_AUXADC_VBUF_EN		BIT(4)		/* CON11 */
#define MT6323_AUXADC_CH7_RQST		BIT(7)		/* CON22: BATSNS */

#define MT6323_AUXADC_SAMPLES		8

struct mt6323_auxadc {
	struct regmap *regmap;	/* regmap del mt6397 (pwrap, con lock) */
	struct mutex lock;	/* serializa lectores de sysfs entre si */
};

static int mt6323_auxadc_read_vbat(struct mt6323_auxadc *adc, long *val_mv)
{
	unsigned int v;
	u64 sum = 0;
	int ret, i, n = 0;

	mutex_lock(&adc->lock);

	/* VBUF + request del canal 7: idempotente, se re-asegura en cada
	 * lectura por si el PMIC perdio estado (p.ej. tras s2idle). */
	ret = regmap_update_bits(adc->regmap, MT6323_AUXADC_CON11,
				 MT6323_AUXADC_VBUF_EN, MT6323_AUXADC_VBUF_EN);
	if (ret)
		goto out;
	ret = regmap_update_bits(adc->regmap, MT6323_AUXADC_CON22,
				 MT6323_AUXADC_CH7_RQST, MT6323_AUXADC_CH7_RQST);
	if (ret)
		goto out;

	ret = regmap_read_poll_timeout(adc->regmap, MT6323_AUXADC_ADC0, v,
				       v & MT6323_AUXADC_RDY, 2000, 200000);
	if (ret)
		goto out;

	for (i = 0; i < MT6323_AUXADC_SAMPLES; i++) {
		ret = regmap_read(adc->regmap, MT6323_AUXADC_ADC0, &v);
		if (ret)
			goto out;
		if (v & MT6323_AUXADC_RDY) {
			sum += v & MT6323_AUXADC_DATA;
			n++;
		}
		usleep_range(1500, 2500);
	}
	if (!n) {
		ret = -EIO;
		goto out;
	}

	*val_mv = div_u64(sum * 4 * 1800, (u64)n * 32768);
out:
	mutex_unlock(&adc->lock);
	return ret;
}

static int mt6323_auxadc_hwmon_read(struct device *dev,
				    enum hwmon_sensor_types type, u32 attr,
				    int channel, long *val)
{
	struct mt6323_auxadc *adc = dev_get_drvdata(dev);

	if (type != hwmon_in || attr != hwmon_in_input || channel != 0)
		return -EOPNOTSUPP;

	return mt6323_auxadc_read_vbat(adc, val);
}

static int mt6323_auxadc_hwmon_read_string(struct device *dev,
					   enum hwmon_sensor_types type,
					   u32 attr, int channel,
					   const char **str)
{
	if (type != hwmon_in || attr != hwmon_in_label || channel != 0)
		return -EOPNOTSUPP;

	*str = "vbat";
	return 0;
}

static umode_t mt6323_auxadc_hwmon_is_visible(const void *data,
					      enum hwmon_sensor_types type,
					      u32 attr, int channel)
{
	return 0444;
}

static const struct hwmon_ops mt6323_auxadc_hwmon_ops = {
	.is_visible = mt6323_auxadc_hwmon_is_visible,
	.read = mt6323_auxadc_hwmon_read,
	.read_string = mt6323_auxadc_hwmon_read_string,
};

static const struct hwmon_channel_info * const mt6323_auxadc_hwmon_info[] = {
	HWMON_CHANNEL_INFO(in, HWMON_I_INPUT | HWMON_I_LABEL),
	NULL
};

static const struct hwmon_chip_info mt6323_auxadc_chip_info = {
	.ops = &mt6323_auxadc_hwmon_ops,
	.info = mt6323_auxadc_hwmon_info,
};

static int mt6323_auxadc_probe(struct platform_device *pdev)
{
	struct device_node *pmic_np;
	struct mt6323_auxadc *adc;
	struct device *hwmon;
	long mv = 0;

	adc = devm_kzalloc(&pdev->dev, sizeof(*adc), GFP_KERNEL);
	if (!adc)
		return -ENOMEM;
	mutex_init(&adc->lock);

	/* regmap del MT6323 via phandle, mismo patron que mt6582-afe-pcm.c */
	pmic_np = of_parse_phandle(pdev->dev.of_node, "mediatek,pmic", 0);
	if (pmic_np) {
		struct platform_device *pmic_pdev = of_find_device_by_node(pmic_np);
		struct mt6397_chip *chip;

		of_node_put(pmic_np);
		if (pmic_pdev) {
			chip = dev_get_drvdata(&pmic_pdev->dev);
			if (chip)
				adc->regmap = chip->regmap;
			put_device(&pmic_pdev->dev);
		}
	}
	if (!adc->regmap)
		return dev_err_probe(&pdev->dev, -EPROBE_DEFER,
				     "MT6323 aun no listo\n");

	hwmon = devm_hwmon_device_register_with_info(&pdev->dev,
						     "mt6323_auxadc", adc,
						     &mt6323_auxadc_chip_info,
						     NULL);
	if (IS_ERR(hwmon))
		return PTR_ERR(hwmon);

	if (!mt6323_auxadc_read_vbat(adc, &mv))
		dev_info(&pdev->dev, "VBAT %ld mV\n", mv);

	return 0;
}

static const struct of_device_id mt6323_auxadc_of_match[] = {
	{ .compatible = "mediatek,mt6323-auxadc" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6323_auxadc_of_match);

static struct platform_driver mt6323_auxadc_driver = {
	.probe = mt6323_auxadc_probe,
	.driver = {
		.name = "mt6323-auxadc",
		.of_match_table = mt6323_auxadc_of_match,
	},
};
module_platform_driver(mt6323_auxadc_driver);

MODULE_DESCRIPTION("MT6323 PMIC AUXADC VBAT (hwmon)");
MODULE_LICENSE("GPL");
