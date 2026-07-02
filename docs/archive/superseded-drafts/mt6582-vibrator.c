// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-vibrator — Vibrator driver for BQ Aquaris E4.5 (krillin / MT6582)
 *
 * El vibrador es un motor DC controlado por el LDO VIBR del PMIC MT6323.
 * Este driver expone el vibrador como un dispositivo input con FF_RUMBLE,
 * para que Phosh y las apps puedan usarlo (vibración al tocar, notificaciones).
 *
 * El LDO VIBR del MT6323 ya está en mainline (mt6323-regulator.c: "ldo_vibr").
 * Solo necesitamos toggle ese regulador.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

struct mt6582_vib {
	struct input_dev *input;
	struct regulator *supply;
	struct work_struct work;
	struct timer_list timer;
	bool enabled;
	u16 duration_ms;
};

static void mt6582_vib_enable(struct mt6582_vib *vib, bool on)
{
	int ret;

	if (vib->enabled == on)
		return;

	if (on) {
		ret = regulator_enable(vib->supply);
		if (ret)
			dev_err(&vib->input->dev, "regulator_enable: %d\n", ret);
		else
			vib->enabled = true;
	} else {
		regulator_disable(vib->supply);
		vib->enabled = false;
	}
}

static void mt6582_vib_work(struct work_struct *work)
{
	struct mt6582_vib *vib = container_of(work, struct mt6582_vib, work);

	mt6582_vib_enable(vib, vib->duration_ms > 0);
	if (vib->duration_ms > 0)
		mod_timer(&vib->timer, jiffies + msecs_to_jiffies(vib->duration_ms));
}

static void mt6582_vib_timer(struct timer_list *t)
{
	struct mt6582_vib *vib = container_of(t, struct mt6582_vib, timer);

	vib->duration_ms = 0;
	schedule_work(&vib->work);
}

static int mt6582_vib_play(struct input_dev *input, void *data,
			   struct ff_effect *effect)
{
	struct mt6582_vib *vib = input_get_drvdata(input);

	if (effect->type != FF_RUMBLE)
		return -EINVAL;

	if (effect->u.rumble.strong_magnitude ||
	    effect->u.rumble.weak_magnitude) {
		vib->duration_ms = effect->replay.length;
		if (!vib->duration_ms)
			vib->duration_ms = 200;  /* default 200ms */
	} else {
		vib->duration_ms = 0;
	}

	schedule_work(&vib->work);
	return 0;
}

static int mt6582_vib_probe(struct platform_device *pdev)
{
	struct mt6582_vib *vib;
	struct input_dev *input;
	int ret;

	vib = devm_kzalloc(&pdev->dev, sizeof(*vib), GFP_KERNEL);
	if (!vib)
		return -ENOMEM;

	vib->supply = devm_regulator_get(&pdev->dev, "vibr");
	if (IS_ERR(vib->supply))
		return dev_err_probe(&pdev->dev, PTR_ERR(vib->supply),
				     "no se encontró el regulador 'vibr'\n");

	input = devm_input_allocate_device(&pdev->dev);
	if (!input)
		return -ENOMEM;

	input->name = "mt6582-vibrator";
	input->id.bustype = BUS_HOST;
	input_set_capability(input, EV_FF, FF_RUMBLE);

	ret = input_ff_create_memless(input, NULL, mt6582_vib_play);
	if (ret)
		return ret;

	vib->input = input;
	INIT_WORK(&vib->work, mt6582_vib_work);
	timer_setup(&vib->timer, mt6582_vib_timer, 0);
	input_set_drvdata(input, vib);
	platform_set_drvdata(pdev, vib);

	ret = input_register_device(input);
	if (ret)
		return ret;

	/* set voltage to 2.8V (the krillin vibrator voltage) */
	regulator_set_voltage(vib->supply, 2800000, 2800000);

	dev_info(&pdev->dev, "mt6582 vibrator registrado (LDO VIBR del MT6323)\n");
	return 0;
}

static void mt6582_vib_remove(struct platform_device *pdev)
{
	struct mt6582_vib *vib = platform_get_drvdata(pdev);

	timer_delete_sync(&vib->timer);
	cancel_work_sync(&vib->work);
	if (vib->enabled)
		regulator_disable(vib->supply);
}

static const struct of_device_id mt6582_vib_match[] = {
	{ .compatible = "mediatek,mt6582-vibrator" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_vib_match);

static struct platform_driver mt6582_vib_driver = {
	.probe = mt6582_vib_probe,
	.remove = mt6582_vib_remove,
	.driver = {
		.name = "mt6582-vibrator",
		.of_match_table = mt6582_vib_match,
	},
};
module_platform_driver(mt6582_vib_driver);

MODULE_DESCRIPTION("MT6582 vibrator driver (LDO VIBR del MT6323)");
MODULE_LICENSE("GPL");
