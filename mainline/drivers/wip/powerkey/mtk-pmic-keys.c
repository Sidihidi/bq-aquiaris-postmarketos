// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2017 MediaTek, Inc.
 *
 * Author: Chen Zhong <chen.zhong@mediatek.com>
 */

#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mfd/mt6323/registers.h>
#include <linux/mfd/mt6331/registers.h>
#include <linux/mfd/mt6357/registers.h>
#include <linux/mfd/mt6358/registers.h>
#include <linux/mfd/mt6359/registers.h>
#include <linux/mfd/mt6397/core.h>
#include <linux/mfd/mt6397/registers.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#define MTK_PMIC_RST_DU_MASK	GENMASK(9, 8)
#define MTK_PMIC_PWRKEY_RST	BIT(6)
#define MTK_PMIC_HOMEKEY_RST	BIT(5)

#define MTK_PMIC_MT6331_RST_DU_MASK	GENMASK(13, 12)
#define MTK_PMIC_MT6331_PWRKEY_RST	BIT(9)
#define MTK_PMIC_MT6331_HOMEKEY_RST	BIT(8)

#define MTK_PMIC_PWRKEY_INDEX	0
#define MTK_PMIC_HOMEKEY_INDEX	1
#define MTK_PMIC_MAX_KEY_COUNT	2

struct mtk_pmic_keys_regs {
	u32 deb_reg;
	u32 deb_mask;
	u32 intsel_reg;
	u32 intsel_mask;
	u32 rst_en_mask;
};

#define MTK_PMIC_KEYS_REGS(_deb_reg, _deb_mask,		\
	_intsel_reg, _intsel_mask, _rst_mask)		\
{							\
	.deb_reg		= _deb_reg,		\
	.deb_mask		= _deb_mask,		\
	.intsel_reg		= _intsel_reg,		\
	.intsel_mask		= _intsel_mask,		\
	.rst_en_mask		= _rst_mask,		\
}

struct mtk_pmic_regs {
	const struct mtk_pmic_keys_regs keys_regs[MTK_PMIC_MAX_KEY_COUNT];
	u32 pmic_rst_reg;
	u32 rst_lprst_mask; /* Long-press reset timeout bitmask */
	bool key_release_irq;
};

static const struct mtk_pmic_regs mt6397_regs = {
	.keys_regs[MTK_PMIC_PWRKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6397_CHRSTATUS,
		0x8, MT6397_INT_RSV, 0x10, MTK_PMIC_PWRKEY_RST),
	.keys_regs[MTK_PMIC_HOMEKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6397_OCSTATUS2,
		0x10, MT6397_INT_RSV, 0x8, MTK_PMIC_HOMEKEY_RST),
	.pmic_rst_reg = MT6397_TOP_RST_MISC,
	.rst_lprst_mask = MTK_PMIC_RST_DU_MASK,
};

static const struct mtk_pmic_regs mt6323_regs = {
	.keys_regs[MTK_PMIC_PWRKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6323_CHRSTATUS,
		0x2, MT6323_INT_MISC_CON, 0x10, MTK_PMIC_PWRKEY_RST),
	.keys_regs[MTK_PMIC_HOMEKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6323_CHRSTATUS,
		0x4, MT6323_INT_MISC_CON, 0x8, MTK_PMIC_HOMEKEY_RST),
	.pmic_rst_reg = MT6323_TOP_RST_MISC,
	.rst_lprst_mask = MTK_PMIC_RST_DU_MASK,
};

static const struct mtk_pmic_regs mt6331_regs = {
	.keys_regs[MTK_PMIC_PWRKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6331_TOPSTATUS, 0x2,
				   MT6331_INT_MISC_CON, 0x4,
				   MTK_PMIC_MT6331_PWRKEY_RST),
	.keys_regs[MTK_PMIC_HOMEKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6331_TOPSTATUS, 0x4,
				   MT6331_INT_MISC_CON, 0x2,
				   MTK_PMIC_MT6331_HOMEKEY_RST),
	.pmic_rst_reg = MT6331_TOP_RST_MISC,
	.rst_lprst_mask = MTK_PMIC_MT6331_RST_DU_MASK,
};

static const struct mtk_pmic_regs mt6357_regs = {
	.keys_regs[MTK_PMIC_PWRKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6357_TOPSTATUS,
				   0x2, MT6357_PSC_TOP_INT_CON0, 0x5,
				   MTK_PMIC_PWRKEY_RST),
	.keys_regs[MTK_PMIC_HOMEKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6357_TOPSTATUS,
				   0x8, MT6357_PSC_TOP_INT_CON0, 0xa,
				   MTK_PMIC_HOMEKEY_INDEX),
	.pmic_rst_reg = MT6357_TOP_RST_MISC,
	.rst_lprst_mask = MTK_PMIC_RST_DU_MASK,
};

static const struct mtk_pmic_regs mt6358_regs = {
	.keys_regs[MTK_PMIC_PWRKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6358_TOPSTATUS,
				   0x2, MT6358_PSC_TOP_INT_CON0, 0x5,
				   MTK_PMIC_PWRKEY_RST),
	.keys_regs[MTK_PMIC_HOMEKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6358_TOPSTATUS,
				   0x8, MT6358_PSC_TOP_INT_CON0, 0xa,
				   MTK_PMIC_HOMEKEY_RST),
	.pmic_rst_reg = MT6358_TOP_RST_MISC,
	.rst_lprst_mask = MTK_PMIC_RST_DU_MASK,
	.key_release_irq = true,
};

static const struct mtk_pmic_regs mt6359_regs = {
	.keys_regs[MTK_PMIC_PWRKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6359_TOPSTATUS,
				   0x2, MT6359_PSC_TOP_INT_CON0, 0x5,
				   MTK_PMIC_PWRKEY_RST),
	.keys_regs[MTK_PMIC_HOMEKEY_INDEX] =
		MTK_PMIC_KEYS_REGS(MT6359_TOPSTATUS,
				   0x8, MT6359_PSC_TOP_INT_CON0, 0xa,
				   MTK_PMIC_HOMEKEY_RST),
	.pmic_rst_reg = MT6359_TOP_RST_MISC,
	.rst_lprst_mask = MTK_PMIC_RST_DU_MASK,
	.key_release_irq = true,
};

struct mtk_pmic_keys_info {
	struct mtk_pmic_keys *keys;
	const struct mtk_pmic_keys_regs *regs;
	unsigned int keycode;
	int irq;
	int irq_r; /* optional: release irq if different */
	bool wakeup:1;
	bool last_pressed:1; /* ultimo estado reportado (ruta IRQ + polling) */
};

struct mtk_pmic_keys {
	struct input_dev *input_dev;
	struct device *dev;
	struct regmap *regmap;
	struct mtk_pmic_keys_info keys[MTK_PMIC_MAX_KEY_COUNT];
	/* Polling de respaldo (workaround MT6323): el IRQ del MFD no se genera en
	 * el primer toque tras idle aunque CHRSTATUS si cambie. Kthread que lee el
	 * debounce register cada ~100ms y reporta el input si cambio. */
	struct task_struct *poll_task;
};

enum mtk_pmic_keys_lp_mode {
	LP_DISABLE,
	LP_ONEKEY,
	LP_TWOKEY,
};

static void mtk_pmic_keys_lp_reset_setup(struct mtk_pmic_keys *keys,
					 const struct mtk_pmic_regs *regs)
{
	const struct mtk_pmic_keys_regs *kregs_home, *kregs_pwr;
	u32 long_press_mode, long_press_debounce;
	u32 value, mask;
	int error;

	kregs_home = &regs->keys_regs[MTK_PMIC_HOMEKEY_INDEX];
	kregs_pwr = &regs->keys_regs[MTK_PMIC_PWRKEY_INDEX];

	error = of_property_read_u32(keys->dev->of_node, "power-off-time-sec",
				     &long_press_debounce);
	if (error)
		long_press_debounce = 0;

	mask = regs->rst_lprst_mask;
	value = long_press_debounce << (ffs(regs->rst_lprst_mask) - 1);

	error  = of_property_read_u32(keys->dev->of_node,
				      "mediatek,long-press-mode",
				      &long_press_mode);
	if (error)
		long_press_mode = LP_DISABLE;

	switch (long_press_mode) {
	case LP_TWOKEY:
		value |= kregs_home->rst_en_mask;
		fallthrough;

	case LP_ONEKEY:
		value |= kregs_pwr->rst_en_mask;
		fallthrough;

	case LP_DISABLE:
		mask |= kregs_home->rst_en_mask;
		mask |= kregs_pwr->rst_en_mask;
		break;

	default:
		break;
	}

	regmap_update_bits(keys->regmap, regs->pmic_rst_reg, mask, value);
}

/* Reporta el estado del botón al input subsystem y (MT6323) re-arma INT_SEL.
 * Llamada tanto desde el IRQ thread como desde el kthread de polling. Usa el
 * estado compartido last_pressed para evitar reportar duplicados y para que
 * ambas rutas no pisen entre ellas. */
static void mtk_pmic_keys_report(struct mtk_pmic_keys_info *info, bool pressed)
{
	if (pressed == info->last_pressed)
		return; /* sin cambio: ignorar (evita dobles reportes IRQ+poll) */

	input_report_key(info->keys->input_dev, info->keycode, pressed);
	input_sync(info->keys->input_dev);
	info->last_pressed = pressed;

	dev_dbg(info->keys->dev, "(%s) key =%d using PMIC\n",
		 pressed ? "pressed" : "released", info->keycode);

	/* MT6323 workaround: el bit INT_SEL del registro intsel_con selecciona que
	 * flanco (press/release) dispara el IRQ del PMIC. El driver original MTK
	 * (pmic_mt6323.c pwrkey_int_handler) lo TOGGLEA en cada evento para re-armar
	 * la deteccion del flanco opuesto. Sin este toggle, el PMIC MT6323 deja de
	 * generar IRQs tras el primer press+release (se queda esperando el flanco
	 * equivocado). Toggle: press detectado (deb==0) -> INT_SEL=1 (para el
	 * release siguiente); release detectado (deb==1) -> INT_SEL=0 (para el
	 * press siguiente). Solo aplica donde intsel_mask != 0 (MT6323/MT6397). */
	if (info->regs->intsel_mask) {
		u32 intsel_val = pressed ? info->regs->intsel_mask : 0;
		regmap_update_bits(info->keys->regmap, info->regs->intsel_reg,
				   info->regs->intsel_mask, intsel_val);
	}
}

static irqreturn_t mtk_pmic_keys_irq_handler_thread(int irq, void *data)
{
	struct mtk_pmic_keys_info *info = data;
	u32 key_deb;

	regmap_read(info->keys->regmap, info->regs->deb_reg, &key_deb);
	key_deb &= info->regs->deb_mask;

	mtk_pmic_keys_report(info, !key_deb);

	return IRQ_HANDLED;
}

/* Kthread de polling de respaldo (workaround MT6323, krillin 0707):
 * El IRQ del MFD NO se genera en el primer toque del powerkey tras idle,
 * aunque el registro CHRSTATUS (deb_reg) SÍ cambia con cada press/release
 * (confirmado en HW: 61 eventos en CHRSTATUS mientras el IRQ del MFD solo
 * disparaba ~10). Sin este polling, el daemon del boton no recibe el primer
 * toque tras idle -> el movil parece "no responder" al desbloquear y hace
 * falta pulsar 2+ veces. El kthread lee el debounce register de cada key
 * cada ~100ms y reporta cambios via mtk_pmic_keys_report (que deduplica con
 * last_pressed, asi no hay dobles reportes cuando IRQ y polling coinciden). */
static int mtk_pmic_keys_poll_thread(void *data)
{
	struct mtk_pmic_keys *keys = data;
	int index;

	while (!kthread_should_stop()) {
		for (index = 0; index < MTK_PMIC_MAX_KEY_COUNT; index++) {
			struct mtk_pmic_keys_info *info = &keys->keys[index];
			u32 key_deb, pressed;

			if (info->keycode == 0)
				continue; /* key no configurada */

			regmap_read(keys->regmap, info->regs->deb_reg, &key_deb);
			key_deb &= info->regs->deb_mask;
			pressed = !key_deb;

			mtk_pmic_keys_report(info, pressed);
		}
		msleep(100);
	}

	return 0;
}

static int mtk_pmic_key_setup(struct mtk_pmic_keys *keys,
		struct mtk_pmic_keys_info *info)
{
	int ret;

	info->keys = keys;

	ret = regmap_update_bits(keys->regmap, info->regs->intsel_reg,
				 info->regs->intsel_mask,
				 info->regs->intsel_mask);
	if (ret < 0)
		return ret;

	ret = devm_request_threaded_irq(keys->dev, info->irq, NULL,
					mtk_pmic_keys_irq_handler_thread,
					IRQF_ONESHOT | IRQF_TRIGGER_HIGH,
					"mtk-pmic-keys", info);
	if (ret) {
		dev_err(keys->dev, "Failed to request IRQ: %d: %d\n",
			info->irq, ret);
		return ret;
	}

	if (info->irq_r > 0) {
		ret = devm_request_threaded_irq(keys->dev, info->irq_r, NULL,
						mtk_pmic_keys_irq_handler_thread,
						IRQF_ONESHOT | IRQF_TRIGGER_HIGH,
						"mtk-pmic-keys", info);
		if (ret) {
			dev_err(keys->dev, "Failed to request IRQ_r: %d: %d\n",
				info->irq, ret);
			return ret;
		}
	}

	input_set_capability(keys->input_dev, EV_KEY, info->keycode);

	return 0;
}

static int mtk_pmic_keys_suspend(struct device *dev)
{
	struct mtk_pmic_keys *keys = dev_get_drvdata(dev);
	int index;

	for (index = 0; index < MTK_PMIC_MAX_KEY_COUNT; index++) {
		if (keys->keys[index].wakeup) {
			enable_irq_wake(keys->keys[index].irq);
			if (keys->keys[index].irq_r > 0)
				enable_irq_wake(keys->keys[index].irq_r);
		}
	}

	return 0;
}

static int mtk_pmic_keys_resume(struct device *dev)
{
	struct mtk_pmic_keys *keys = dev_get_drvdata(dev);
	int index;

	for (index = 0; index < MTK_PMIC_MAX_KEY_COUNT; index++) {
		if (keys->keys[index].wakeup) {
			disable_irq_wake(keys->keys[index].irq);
			if (keys->keys[index].irq_r > 0)
				disable_irq_wake(keys->keys[index].irq_r);
		}
	}

	return 0;
}

static DEFINE_SIMPLE_DEV_PM_OPS(mtk_pmic_keys_pm_ops, mtk_pmic_keys_suspend,
				mtk_pmic_keys_resume);

static const struct of_device_id of_mtk_pmic_keys_match_tbl[] = {
	{
		.compatible = "mediatek,mt6397-keys",
		.data = &mt6397_regs,
	}, {
		.compatible = "mediatek,mt6323-keys",
		.data = &mt6323_regs,
	}, {
		.compatible = "mediatek,mt6331-keys",
		.data = &mt6331_regs,
	}, {
		.compatible = "mediatek,mt6357-keys",
		.data = &mt6357_regs,
	}, {
		.compatible = "mediatek,mt6358-keys",
		.data = &mt6358_regs,
	}, {
		.compatible = "mediatek,mt6359-keys",
		.data = &mt6359_regs,
	}, {
		/* sentinel */
	}
};
MODULE_DEVICE_TABLE(of, of_mtk_pmic_keys_match_tbl);

static int mtk_pmic_keys_probe(struct platform_device *pdev)
{
	int error, index = 0;
	unsigned int keycount;
	struct mt6397_chip *pmic_chip = dev_get_drvdata(pdev->dev.parent);
	struct device_node *node = pdev->dev.of_node;
	static const char *const irqnames[] = { "powerkey", "homekey" };
	static const char *const irqnames_r[] = { "powerkey_r", "homekey_r" };
	struct mtk_pmic_keys *keys;
	const struct mtk_pmic_regs *mtk_pmic_regs;
	struct input_dev *input_dev;
	const struct of_device_id *of_id =
		of_match_device(of_mtk_pmic_keys_match_tbl, &pdev->dev);

	keys = devm_kzalloc(&pdev->dev, sizeof(*keys), GFP_KERNEL);
	if (!keys)
		return -ENOMEM;

	keys->dev = &pdev->dev;
	keys->regmap = pmic_chip->regmap;
	mtk_pmic_regs = of_id->data;

	keys->input_dev = input_dev = devm_input_allocate_device(keys->dev);
	if (!input_dev) {
		dev_err(keys->dev, "input allocate device fail.\n");
		return -ENOMEM;
	}

	input_dev->name = "mtk-pmic-keys";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0001;

	keycount = of_get_available_child_count(node);
	if (keycount > MTK_PMIC_MAX_KEY_COUNT ||
	    keycount > ARRAY_SIZE(irqnames)) {
		dev_err(keys->dev, "too many keys defined (%d)\n", keycount);
		return -EINVAL;
	}

	for_each_child_of_node_scoped(node, child) {
		keys->keys[index].regs = &mtk_pmic_regs->keys_regs[index];

		keys->keys[index].irq =
			platform_get_irq_byname(pdev, irqnames[index]);
		if (keys->keys[index].irq < 0)
			return keys->keys[index].irq;

		if (mtk_pmic_regs->key_release_irq) {
			keys->keys[index].irq_r = platform_get_irq_byname(pdev,
									  irqnames_r[index]);

			if (keys->keys[index].irq_r < 0)
				return keys->keys[index].irq_r;
		}

		error = of_property_read_u32(child,
			"linux,keycodes", &keys->keys[index].keycode);
		if (error) {
			dev_err(keys->dev,
				"failed to read key:%d linux,keycode property: %d\n",
				index, error);
			return error;
		}

		if (of_property_read_bool(child, "wakeup-source"))
			keys->keys[index].wakeup = true;

		error = mtk_pmic_key_setup(keys, &keys->keys[index]);
		if (error)
			return error;

		index++;
	}

	error = input_register_device(input_dev);
	if (error) {
		dev_err(&pdev->dev,
			"register input device failed (%d)\n", error);
		return error;
	}

	mtk_pmic_keys_lp_reset_setup(keys, mtk_pmic_regs);

	platform_set_drvdata(pdev, keys);

	/* Lanzar el kthread de polling de respaldo (workaround MT6323). Solo lo
	 * usamos en MT6323/MT6397 (donde intsel_mask != 0); en otras PMICs el IRQ
	 * del MFD es fiable y el polling seria innecesario. */
	if (mtk_pmic_regs->keys_regs[0].intsel_mask) {
		keys->poll_task = kthread_run(mtk_pmic_keys_poll_thread, keys,
					      "mtk-pmic-keys-poll");
		if (IS_ERR(keys->poll_task)) {
			dev_warn(&pdev->dev, "failed to start poll thread\n");
			keys->poll_task = NULL;
		}
	}

	return 0;
}

static void mtk_pmic_keys_remove(struct platform_device *pdev)
{
	struct mtk_pmic_keys *keys = platform_get_drvdata(pdev);

	if (keys && keys->poll_task)
		kthread_stop(keys->poll_task);
}

static struct platform_driver pmic_keys_pdrv = {
	.probe = mtk_pmic_keys_probe,
	.remove = mtk_pmic_keys_remove,
	.driver = {
		   .name = "mtk-pmic-keys",
		   .of_match_table = of_mtk_pmic_keys_match_tbl,
		   .pm = pm_sleep_ptr(&mtk_pmic_keys_pm_ops),
	},
};

module_platform_driver(pmic_keys_pdrv);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Chen Zhong <chen.zhong@mediatek.com>");
MODULE_DESCRIPTION("MTK pmic-keys driver v0.1");
