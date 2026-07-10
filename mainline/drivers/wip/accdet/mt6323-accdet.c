// SPDX-License-Identifier: GPL-2.0
/*
 * MT6323 PMIC accessory detect (headset jack + inline buttons)
 *
 * Port of the MediaTek downstream accdet driver (accdet/mt6582/accdet.c,
 * kernel 3.4/3.10) to mainline, for the BQ Aquaris E4.5 (krillin).
 *
 * Architecture (krillin: ACCDET_EINT + ACCDET_MULTI_KEY_FEATURE + 2.8V mode):
 *  - Jack insert/remove is detected by an AP-side GPIO/EINT (EINT14 on the
 *    krillin, active low), NOT by the PMIC comparator.
 *  - The MT6323 ACCDET block (comparator + micbias PWM) distinguishes
 *    headset-with-mic vs headphone and detects button press/release
 *    (comparator state AB in ACCDET_STATE_RG[7:6]).
 *  - Inline buttons are told apart by voltage: AUXADC sw-channel 8
 *    (hw output CH5, register AUXADC_ADC7) while the button is pressed:
 *      0 - 90 mV   -> KEY_PLAYPAUSE  (MD)
 *     90 - 240 mV  -> KEY_VOLUMEUP   (UP)
 *    240 - 500 mV  -> KEY_VOLUMEDOWN (DW)
 *    (thresholds from the downstream key_check(); mV = raw * 1800 / 32768)
 *
 * Userspace sees a plain input device: SW_HEADPHONE_INSERT,
 * SW_MICROPHONE_INSERT and the three keys. WirePlumber/UCM or a small
 * daemon can re-route the codec (the analog routing lives in userspace
 * scripts on this platform for now).
 *
 * Register/sequence provenance: downstream accdet.c/reg_accdet.h RE
 * (see the INTEGRATION.md next to this file in the pmos-krillin repo).
 *
 * v2 (0709): fixes from VERIFICACION-DRIVERS-PARALELA-0709 —
 *  - key reads route accdet->AUXADC (RSV=0x5A20) and boost the micbias
 *    PWM to 100% duty for the measurement, restoring both afterwards;
 *  - VBUF enabled around the ADC read (stock does it for ch < 9);
 *  - SWCTRL keeps comparator/vth/micbias on in idle (0x77, multi-key);
 *  - dropped the 0x0400/bit14 write (FSA8049 pin-swap, not fitted);
 *  - plug-out leaves RSV at the low-power value (0x1A10).
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/mfd/mt6397/core.h>

/* ---- MT6323 top-level registers (PMIC offsets, via pwrap regmap) ---- */
#define MT6323_TOP_CKPDN		0x0108
#define MT6323_TOP_CKPDN_SET		0x010a
#define MT6323_TOP_CKPDN_CLR		0x010c
#define  RG_ACCDET_CLK			BIT(9)

#define MT6323_TOP_RST_ACCDET		0x0114
#define MT6323_TOP_RST_ACCDET_SET	0x0116
#define MT6323_TOP_RST_ACCDET_CLR	0x0118
#define  ACCDET_RESET			BIT(4)

/* ---- MT6323 AUXADC (for the multi-key voltage) ---- */
#define MT6323_AUXADC_ADC7		0x0722	/* out of hw CH5 = sw ch8 (ACCDET) */
#define  AUXADC_ADC_RDY			BIT(15)
#define  AUXADC_ADC_OUT_MASK		GENMASK(14, 0)
#define MT6323_AUXADC_CON1		0x0758
#define  AUXADC_VBUF_EN			BIT(4)	/* stock enables it for ch < 9 */
#define MT6323_AUXADC_CON22		0x076e	/* RG_AP_RQST_LIST (9 bits) */
#define  AUXADC_RQST_CH8		BIT(8)

/* ---- MT6323 ACCDET block ---- */
#define MT6323_ACCDET_RSV		0x077a
#define  ACCDET_1V9_MODE_OFF		0x1a10
#define  ACCDET_2V8_MODE_OFF		0x5a10
#define  ACCDET_1V9_MODE_ON		0x1e10
#define  ACCDET_2V8_MODE_ON		0x5a20
#define MT6323_ACCDET_CTRL		0x077c
#define  ACCDET_CTRL_EN			BIT(0)
#define MT6323_ACCDET_STATE_SWCTRL	0x077e
#define  ACCDET_SWCTRL_EN		0x07	/* cmp + vth + micbias PWM */
#define  ACCDET_SWCTRL_IDLE_EN		(0x07 << 4) /* keep them on in idle (multi-key) */
#define MT6323_ACCDET_PWM_WIDTH		0x0780
#define MT6323_ACCDET_PWM_THRESH	0x0782
#define MT6323_ACCDET_EN_DELAY_NUM	0x0784
#define MT6323_ACCDET_DEBOUNCE0		0x0786
#define MT6323_ACCDET_DEBOUNCE1		0x0788
#define MT6323_ACCDET_DEBOUNCE3		0x078c
#define MT6323_ACCDET_IRQ_STS		0x0790
#define  ACCDET_IRQ_STATUS		BIT(0)
#define  ACCDET_IRQ_CLR			BIT(8)
#define MT6323_ACCDET_STATE_RG		0x0794
#define  ACCDET_STATE_AB(v)		(((v) & 0xc0) >> 6)

/* comparator states (AB) */
#define AB_HOOK_OR_NOMIC		0
#define AB_MIC_PLUGGED			1
#define AB_PLUG_OUT			3

/*
 * krillin cust_headset_settings (ACCDET_MULTI_KEY_FEATURE):
 * { pwm_width 0x900, pwm_thresh 0x400, fall 1, rise 0x3f0,
 *   debounce0 0x800, debounce1 0x800, debounce3 0x20 }
 */
#define KRILLIN_PWM_WIDTH		0x900
#define KRILLIN_PWM_THRESH		0x400
#define KRILLIN_FALL_DELAY		1
#define KRILLIN_RISE_DELAY		0x3f0
#define KRILLIN_DEBOUNCE0		0x800
#define KRILLIN_DEBOUNCE1		0x800
#define KRILLIN_DEBOUNCE3		0x20
/* downstream: shorter hook debounce while a headset is plugged */
#define KRILLIN_BTN_PRESS_DEBOUNCE	0x400

/* multi-key thresholds, mV (downstream key_check) */
#define KEY_MD_MAX_MV			90
#define KEY_UP_MAX_MV			240
#define KEY_DW_MAX_MV			500

#define JACK_DEBOUNCE_MS		80
#define MIC_SETTLE_MS			250

struct mt6323_accdet {
	struct device *dev;
	struct regmap *regmap;
	struct input_dev *input;
	struct gpio_desc *jack_gpio;
	int jack_irq;
	int accdet_irq;
	struct delayed_work plug_work;
	struct mutex lock;	/* serialize plug/button flows */
	bool plugged;
	bool has_mic;
	unsigned int cur_key;
	bool micbias_2v8;
};

static unsigned int accdet_read(struct mt6323_accdet *acc, unsigned int reg)
{
	unsigned int val = 0;

	regmap_read(acc->regmap, reg, &val);
	return val;
}

static void accdet_write(struct mt6323_accdet *acc, unsigned int reg,
			 unsigned int val)
{
	regmap_write(acc->regmap, reg, val);
}

/*
 * Read the button voltage: AUXADC sw ch8 (hw CH5), result in mV.
 *
 * Per the downstream flow (accdet_auxadc_switch + multi_key_detection):
 *  - ACCDET_RSV bit5 = 0x5A20 routes the accdet pin to the AUXADC
 *    (without it every button reads < 90 mV = PLAYPAUSE);
 *  - during the measurement the micbias PWM runs at 100% duty
 *    (PWM_THRESH = PWM_WIDTH), otherwise the voltage is chopped;
 *  - VBUF enabled for channels < 9.
 * Everything is restored afterwards.
 */
static int mt6323_accdet_read_key_mv(struct mt6323_accdet *acc)
{
	unsigned int val;
	int tries = 40;
	int mv = -ETIMEDOUT;

	/* accdet -> AUXADC switch on + PWM at 100% duty + VBUF on */
	accdet_write(acc, MT6323_ACCDET_RSV, ACCDET_2V8_MODE_ON);
	accdet_write(acc, MT6323_ACCDET_PWM_THRESH, KRILLIN_PWM_WIDTH);
	accdet_write(acc, MT6323_AUXADC_CON1,
		     accdet_read(acc, MT6323_AUXADC_CON1) | AUXADC_VBUF_EN);

	/* request: toggle bit 8 of RG_AP_RQST_LIST (clear, then set) */
	val = accdet_read(acc, MT6323_AUXADC_CON22);
	accdet_write(acc, MT6323_AUXADC_CON22, val & ~AUXADC_RQST_CH8);
	val = accdet_read(acc, MT6323_AUXADC_CON22);
	accdet_write(acc, MT6323_AUXADC_CON22, val | AUXADC_RQST_CH8);

	/* downstream: channel 8 needs no settling delay (HW quirk) */
	while (tries-- > 0) {
		val = accdet_read(acc, MT6323_AUXADC_ADC7);
		if (val & AUXADC_ADC_RDY) {
			mv = (int)((val & AUXADC_ADC_OUT_MASK) * 1800 / 32768);
			break;
		}
		usleep_range(500, 1000);
	}
	if (mv < 0)
		dev_warn(acc->dev, "key ADC not ready\n");

	/* restore: VBUF off, PWM duty back to cust, ADC switch off */
	accdet_write(acc, MT6323_AUXADC_CON1,
		     accdet_read(acc, MT6323_AUXADC_CON1) & ~AUXADC_VBUF_EN);
	accdet_write(acc, MT6323_ACCDET_PWM_THRESH, KRILLIN_PWM_THRESH);
	accdet_write(acc, MT6323_ACCDET_RSV, ACCDET_2V8_MODE_OFF);

	return mv;
}

static unsigned int mt6323_accdet_key_from_mv(int mv)
{
	if (mv < 0)
		return KEY_PLAYPAUSE;	/* safest default */
	if (mv < KEY_MD_MAX_MV)
		return KEY_PLAYPAUSE;
	if (mv < KEY_UP_MAX_MV)
		return KEY_VOLUMEUP;
	if (mv < KEY_DW_MAX_MV)
		return KEY_VOLUMEDOWN;
	return 0;
}

static void mt6323_accdet_clear_irq(struct mt6323_accdet *acc)
{
	unsigned int val;
	int tries = 10;

	accdet_write(acc, MT6323_ACCDET_IRQ_STS, ACCDET_IRQ_CLR);
	while (tries-- > 0 &&
	       (accdet_read(acc, MT6323_ACCDET_IRQ_STS) & ACCDET_IRQ_STATUS))
		usleep_range(200, 400);
	val = accdet_read(acc, MT6323_ACCDET_IRQ_STS);
	accdet_write(acc, MT6323_ACCDET_IRQ_STS, val & ~ACCDET_IRQ_CLR);
}

static void mt6323_accdet_power_on(struct mt6323_accdet *acc)
{
	/* clock on + reset pulse */
	accdet_write(acc, MT6323_TOP_CKPDN_CLR, RG_ACCDET_CLK);
	accdet_write(acc, MT6323_TOP_RST_ACCDET_SET, ACCDET_RESET);
	accdet_write(acc, MT6323_TOP_RST_ACCDET_CLR, ACCDET_RESET);

	/*
	 * Micbias mode: 2.8V on the krillin (RSV = 0x5A10; the ADC-switch
	 * bit 5 stays OFF outside key reads). The downstream 0x0400/bit14
	 * write is FSA8049 pin-swap only (not fitted on the krillin).
	 */
	accdet_write(acc, MT6323_ACCDET_RSV,
		     acc->micbias_2v8 ? ACCDET_2V8_MODE_OFF : ACCDET_1V9_MODE_ON);

	/* PWM + delays + debounce (krillin cust values) */
	accdet_write(acc, MT6323_ACCDET_PWM_WIDTH, KRILLIN_PWM_WIDTH);
	accdet_write(acc, MT6323_ACCDET_PWM_THRESH, KRILLIN_PWM_THRESH);
	accdet_write(acc, MT6323_ACCDET_EN_DELAY_NUM,
		     (KRILLIN_FALL_DELAY << 15) | KRILLIN_RISE_DELAY);
	accdet_write(acc, MT6323_ACCDET_DEBOUNCE0, KRILLIN_DEBOUNCE0);
	accdet_write(acc, MT6323_ACCDET_DEBOUNCE1, KRILLIN_DEBOUNCE1);
	accdet_write(acc, MT6323_ACCDET_DEBOUNCE3, KRILLIN_DEBOUNCE3);

	/*
	 * Enable comparator/vth/micbias PWM + keep them on in idle
	 * (IDLE_EN, downstream multi-key: SWCTRL = 0x77) + the unit.
	 */
	accdet_write(acc, MT6323_ACCDET_STATE_SWCTRL,
		     accdet_read(acc, MT6323_ACCDET_STATE_SWCTRL) |
		     ACCDET_SWCTRL_EN | ACCDET_SWCTRL_IDLE_EN);
	accdet_write(acc, MT6323_ACCDET_CTRL, ACCDET_CTRL_EN);
}

static void mt6323_accdet_power_off(struct mt6323_accdet *acc)
{
	accdet_write(acc, MT6323_ACCDET_CTRL, 0);
	accdet_write(acc, MT6323_ACCDET_STATE_SWCTRL, 0);
	/* lowest-power RSV value while unplugged (downstream plug-out) */
	accdet_write(acc, MT6323_ACCDET_RSV, ACCDET_1V9_MODE_OFF);
	mt6323_accdet_clear_irq(acc);
	accdet_write(acc, MT6323_TOP_CKPDN_SET, RG_ACCDET_CLK);
}

static void mt6323_accdet_release_key(struct mt6323_accdet *acc)
{
	if (acc->cur_key) {
		input_report_key(acc->input, acc->cur_key, 0);
		input_sync(acc->input);
		acc->cur_key = 0;
	}
}

static void mt6323_accdet_plug_work(struct work_struct *work)
{
	struct mt6323_accdet *acc =
		container_of(to_delayed_work(work), struct mt6323_accdet,
			     plug_work);
	int inserted = gpiod_get_value_cansleep(acc->jack_gpio);
	unsigned int ab;

	mutex_lock(&acc->lock);

	if (inserted && !acc->plugged) {
		mt6323_accdet_power_on(acc);
		/* let the micbias/comparator settle, then look at AB */
		msleep(MIC_SETTLE_MS);
		ab = ACCDET_STATE_AB(accdet_read(acc, MT6323_ACCDET_STATE_RG));

		acc->plugged = true;
		acc->has_mic = (ab == AB_MIC_PLUGGED);

		input_report_switch(acc->input, SW_HEADPHONE_INSERT, 1);
		if (acc->has_mic) {
			input_report_switch(acc->input, SW_MICROPHONE_INSERT, 1);
			/* downstream: shorter hook debounce while plugged */
			accdet_write(acc, MT6323_ACCDET_DEBOUNCE0,
				     KRILLIN_BTN_PRESS_DEBOUNCE);
		}
		input_sync(acc->input);
		dev_info(acc->dev, "jack: %s (AB=%u)\n",
			 acc->has_mic ? "headset (mic)" : "headphone", ab);
	} else if (!inserted && acc->plugged) {
		acc->plugged = false;
		acc->has_mic = false;
		mt6323_accdet_release_key(acc);
		input_report_switch(acc->input, SW_HEADPHONE_INSERT, 0);
		input_report_switch(acc->input, SW_MICROPHONE_INSERT, 0);
		input_sync(acc->input);
		mt6323_accdet_power_off(acc);
		dev_info(acc->dev, "jack: removed\n");
	}

	mutex_unlock(&acc->lock);
}

static irqreturn_t mt6323_accdet_jack_isr(int irq, void *data)
{
	struct mt6323_accdet *acc = data;

	mod_delayed_work(system_wq, &acc->plug_work,
			 msecs_to_jiffies(JACK_DEBOUNCE_MS));
	return IRQ_HANDLED;
}

/* PMIC comparator interrupt: button press/release while a headset is in */
static irqreturn_t mt6323_accdet_pmic_isr(int irq, void *data)
{
	struct mt6323_accdet *acc = data;
	unsigned int ab, key;
	int mv;

	mutex_lock(&acc->lock);

	ab = ACCDET_STATE_AB(accdet_read(acc, MT6323_ACCDET_STATE_RG));
	dev_dbg(acc->dev, "accdet irq, AB=%u\n", ab);

	if (!acc->plugged || !acc->has_mic)
		goto out;

	if (ab == AB_HOOK_OR_NOMIC && !acc->cur_key) {
		/* button pressed: identify it by ADC voltage */
		mv = mt6323_accdet_read_key_mv(acc);
		key = mt6323_accdet_key_from_mv(mv);
		dev_dbg(acc->dev, "key adc = %d mV -> key %u\n", mv, key);
		if (key) {
			acc->cur_key = key;
			input_report_key(acc->input, key, 1);
			input_sync(acc->input);
		}
	} else if (ab == AB_MIC_PLUGGED) {
		/* back to mic-bias level: button released */
		mt6323_accdet_release_key(acc);
	}

out:
	mt6323_accdet_clear_irq(acc);
	mutex_unlock(&acc->lock);
	return IRQ_HANDLED;
}

static int mt6323_accdet_probe(struct platform_device *pdev)
{
	struct mt6397_chip *mt6397 = dev_get_drvdata(pdev->dev.parent);
	struct mt6323_accdet *acc;
	int ret;

	if (!mt6397 || !mt6397->regmap)
		return -EPROBE_DEFER;

	acc = devm_kzalloc(&pdev->dev, sizeof(*acc), GFP_KERNEL);
	if (!acc)
		return -ENOMEM;

	acc->dev = &pdev->dev;
	acc->regmap = mt6397->regmap;
	mutex_init(&acc->lock);
	INIT_DELAYED_WORK(&acc->plug_work, mt6323_accdet_plug_work);
	platform_set_drvdata(pdev, acc);

	acc->micbias_2v8 = of_property_read_bool(pdev->dev.of_node,
						 "mediatek,micbias-2v8");

	/* jack detect GPIO (EINT14 on the krillin, active low) */
	acc->jack_gpio = devm_gpiod_get(&pdev->dev, "jack-detect", GPIOD_IN);
	if (IS_ERR(acc->jack_gpio))
		return dev_err_probe(&pdev->dev, PTR_ERR(acc->jack_gpio),
				     "no jack-detect gpio\n");

	/* input device */
	acc->input = devm_input_allocate_device(&pdev->dev);
	if (!acc->input)
		return -ENOMEM;
	acc->input->name = "mt6323-accdet";
	acc->input->phys = "mt6323-accdet/input0";
	input_set_capability(acc->input, EV_SW, SW_HEADPHONE_INSERT);
	input_set_capability(acc->input, EV_SW, SW_MICROPHONE_INSERT);
	input_set_capability(acc->input, EV_KEY, KEY_PLAYPAUSE);
	input_set_capability(acc->input, EV_KEY, KEY_VOLUMEUP);
	input_set_capability(acc->input, EV_KEY, KEY_VOLUMEDOWN);
	ret = input_register_device(acc->input);
	if (ret)
		return ret;

	/* leave the block off until something is plugged */
	mt6323_accdet_power_off(acc);

	/* PMIC comparator IRQ (MFD cell resource "accdet") */
	acc->accdet_irq = platform_get_irq(pdev, 0);
	if (acc->accdet_irq < 0)
		return dev_err_probe(&pdev->dev, acc->accdet_irq,
				     "no accdet irq (mt6397-core cell?)\n");
	ret = devm_request_threaded_irq(&pdev->dev, acc->accdet_irq, NULL,
					mt6323_accdet_pmic_isr,
					IRQF_ONESHOT, "mt6323-accdet", acc);
	if (ret)
		return ret;

	/* jack GPIO IRQ, both edges */
	acc->jack_irq = gpiod_to_irq(acc->jack_gpio);
	if (acc->jack_irq < 0)
		return dev_err_probe(&pdev->dev, acc->jack_irq,
				     "jack gpio has no irq\n");
	ret = devm_request_threaded_irq(&pdev->dev, acc->jack_irq, NULL,
					mt6323_accdet_jack_isr,
					IRQF_TRIGGER_RISING |
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					"mt6323-accdet-jack", acc);
	if (ret)
		return ret;

	/* pick up an already-inserted jack at boot */
	mod_delayed_work(system_wq, &acc->plug_work,
			 msecs_to_jiffies(JACK_DEBOUNCE_MS));

	dev_info(&pdev->dev, "MT6323 accdet ready (micbias %s)\n",
		 acc->micbias_2v8 ? "2.8V" : "1.9V");
	return 0;
}

static void mt6323_accdet_remove(struct platform_device *pdev)
{
	struct mt6323_accdet *acc = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&acc->plug_work);
	mt6323_accdet_power_off(acc);
}

static const struct of_device_id mt6323_accdet_of_match[] = {
	{ .compatible = "mediatek,mt6323-accdet", },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6323_accdet_of_match);

static struct platform_driver mt6323_accdet_driver = {
	.driver = {
		.name = "mt6323-accdet",
		.of_match_table = mt6323_accdet_of_match,
	},
	.probe	= mt6323_accdet_probe,
	.remove	= mt6323_accdet_remove,
};
module_platform_driver(mt6323_accdet_driver);

MODULE_AUTHOR("pmos-krillin project");
MODULE_DESCRIPTION("MT6323 PMIC accessory (headset) detect");
MODULE_LICENSE("GPL");
