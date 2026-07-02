// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-thermal — Thermal sensor para BQ Aquaris E4.5 (MT6582)
 *
 * El MT6582 tiene un sensor de temperatura interno accesible via AUXADC del MT6323.
 * El canal AUXADC de temperatura del MT6323 da un valor crudo que se convierte a grados.
 *
 * Este driver expone una zona térmica simple (/sys/class/thermal/thermal_zone0/temp)
 * que lee el AUXADC del MT6323 en modo polling.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>

struct mt6582_thermal {
	struct thermal_zone_device *tzd;
	struct iio_channel *chan;
};

static int mt6582_thermal_get_temp(struct thermal_zone_device *tzd, int *temp)
{
	struct mt6582_thermal *t = thermal_zone_device_priv(tzd);
	int val, ret;

	ret = iio_read_channel_processed(t->chan, &val);
	if (ret)
		return ret;

	/* el valor crudo del AUXADC se convierte a millicelsius.
	 * El MT6323 temperature channel da valores en el rango ~1200-1800 mV
	 * que corresponden a ~-20°C a ~85°C. Factor de conversión aproximado:
	 * temp_mC = (val - 1200) * 100 (calibrar con HW real) */
	*temp = (val - 1200) * 100;
	return 0;
}

static struct thermal_zone_device_ops mt6582_thermal_ops = {
	.get_temp = mt6582_thermal_get_temp,
};

static int mt6582_thermal_probe(struct platform_device *pdev)
{
	struct mt6582_thermal *t;

	t = devm_kzalloc(&pdev->dev, sizeof(*t), GFP_KERNEL);
	if (!t)
		return -ENOMEM;

	t->chan = devm_iio_channel_get(&pdev->dev, "temp");
	if (IS_ERR(t->chan))
		return dev_err_probe(&pdev->dev, PTR_ERR(t->chan),
				    "no se encontró el canal IIO 'temp'\n");

	t->tzd = thermal_tripless_zone_device_register("mt6582-soc",
			t, &mt6582_thermal_ops, NULL);
	if (IS_ERR(t->tzd))
		return PTR_ERR(t->tzd);

	platform_set_drvdata(pdev, t);
	dev_info(&pdev->dev, "mt6582 thermal sensor registrado\n");
	return 0;
}

static void mt6582_thermal_remove(struct platform_device *pdev)
{
	struct mt6582_thermal *t = platform_get_drvdata(pdev);
	thermal_zone_device_unregister(t->tzd);
}

static const struct of_device_id mt6582_thermal_match[] = {
	{ .compatible = "mediatek,mt6582-thermal" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_thermal_match);

static struct platform_driver mt6582_thermal_driver = {
	.probe = mt6582_thermal_probe,
	.remove = mt6582_thermal_remove,
	.driver = {
		.name = "mt6582-thermal",
		.of_match_table = mt6582_thermal_match,
	},
};
module_platform_driver(mt6582_thermal_driver);

MODULE_DESCRIPTION("MT6582 thermal sensor via AUXADC del MT6323");
MODULE_LICENSE("GPL");
