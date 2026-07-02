// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-afe-pcm — AFE (Audio Front End) del MT6582, Fase A: esqueleto.
 *
 * El AFE es el bloque digital de audio del SoC (DMA DL1/VUL + I2S + mixer
 * CONN0-4) @ 0x11220000 (AudDrv_Afe.h:408 del downstream). El codec analógico
 * (ANA) vive en el PMIC MT6323 vía pwrap y va en un driver aparte (Fase B).
 *
 * FASE A (este fichero): probe + ioremap + snd_soc_component vacío. Sin nodo
 * DT no probea, así que puede ir compilado en el kernel compartido sin riesgo.
 * El PCM real (DL1 + IRQ por sample-count, SIN dmaengine: el HW no lo usa)
 * se añade en la fase A.2 tras validar en HW que el bloque tiene reloj
 * (devmem 0x11220000 — lección AUXADC/WiFi: nunca ioremap+readl a ciegas).
 *
 * Clocks: mt6582 no tiene CCF en mainline; dependemos de los relojes que deja
 * el bootloader + clk_ignore_unused (mismo patrón que display/keypad).
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <sound/soc.h>

#include "mt6582-afe-regs.h"

struct mt6582_afe {
	void __iomem *base;
	struct device *dev;
};

static const struct snd_soc_component_driver mt6582_afe_component = {
	.name = "mt6582-afe",
	/* Fase A.2: pcm_construct/pointer/trigger + DAI DL1 */
};

static int mt6582_afe_probe(struct platform_device *pdev)
{
	struct mt6582_afe *afe;
	u32 top;

	afe = devm_kzalloc(&pdev->dev, sizeof(*afe), GFP_KERNEL);
	if (!afe)
		return -ENOMEM;
	afe->dev = &pdev->dev;

	afe->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(afe->base))
		return PTR_ERR(afe->base);

	/* sanity: leer TOP_CON0 — si el reloj del AFE está cortado esto
	 * devolvería basura o colgaría el bus; loguear para el bring-up */
	top = readl(afe->base + AUDIOAFE_TOP_CON0);
	dev_info(afe->dev, "AFE @%pa TOP_CON0=0x%08x\n",
		 &pdev->resource[0].start, top);

	platform_set_drvdata(pdev, afe);
	return devm_snd_soc_register_component(&pdev->dev,
					       &mt6582_afe_component,
					       NULL, 0);
}

static const struct of_device_id mt6582_afe_of_match[] = {
	{ .compatible = "mediatek,mt6582-afe" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_afe_of_match);

static struct platform_driver mt6582_afe_driver = {
	.driver = {
		.name = "mt6582-afe",
		.of_match_table = mt6582_afe_of_match,
	},
	.probe = mt6582_afe_probe,
};
module_platform_driver(mt6582_afe_driver);

MODULE_DESCRIPTION("MediaTek MT6582 AFE (Fase A: esqueleto)");
MODULE_LICENSE("GPL");
