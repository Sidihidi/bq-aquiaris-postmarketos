// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-afe-pcm — AFE (Audio Front End) del MT6582, Fase A.2: PCM DL1 + IRQ.
 *
 * El AFE es el bloque digital de audio del SoC (memif DL1/VUL + ADDA + I2S-DAC
 * interno + mixer CONN0-4) @ 0x11220000. El codec analogico (ANA) vive en el
 * PMIC MT6323 via pwrap y va en un driver aparte (Fase B) — con solo este
 * driver el PCM corre (punteros/IRQ/timing) pero NO suena.
 *
 * Secuencias extraidas del downstream (extraccion 2026-07-06, ver
 * HANDOFF-AUDIO-PORT-0702.md):
 *  - AudDrv_Kernel.c/AudDrv_Afe.c (driver chardev del krillin, MT6582)
 *  - mt_soc_pcm_dl1.c/mt_soc_afe_control.c (mismo AFE, referencia in-kernel)
 * GOTCHAS clave respetados aqui:
 *  - DOS codificaciones de fs distintas: memif/I2S/IRQ (44k1=9) vs ADDA (44k1=7).
 *  - AFE_DL1_END es INCLUSIVO (base+len-1); AFE_DL1_CUR devuelve fisica absoluta
 *    y puede leer 0/fuera de rango (sanear).
 *  - IRQ level-low: limpiar SIEMPRE (clear-all bit6 en espurias) o hay tormenta.
 *  - Orden: CONN antes de IRQ-on/DL1-on al arrancar; IRQ-off ANTES de DL1-off
 *    al parar.
 *  - Buffer en DRAM (dma_alloc_coherent via ALSA managed): MEMIF_MAXLEN bit0=1.
 *    (El deep-idle del downstream no existe en mainline; no aplica su workaround.)
 *
 * Clocks: mt6582 no tiene CCF en mainline; el LK deja INFRA_AUDIO encendido
 * (validado en HW 0705: INFRA_PDN_STA bit5=0, AFE lee/escribe OK) y
 * clk_ignore_unused lo preserva. En prepare escribimos el valor de clk-on del
 * stock en TOP_CON0 (0x60004000) igual que hace AudDrv_Clk_On.
 */

#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "mt6582-afe-regs.h"

#define MT6582_AFE_BUFFER_BYTES_MAX	(64 * 1024)
#define MT6582_AFE_PERIOD_BYTES_MIN	512

struct mt6582_afe {
	void __iomem *base;
	struct device *dev;
	spinlock_t lock;			/* RMW de registros */
	struct snd_pcm_substream *dl1_substream;
};

static u32 afe_rd(struct mt6582_afe *afe, u32 reg)
{
	return readl(afe->base + reg);
}

static void afe_wr(struct mt6582_afe *afe, u32 reg, u32 val)
{
	writel(val, afe->base + reg);
}

static void afe_rmw(struct mt6582_afe *afe, u32 reg, u32 mask, u32 val)
{
	unsigned long flags;
	u32 v;

	spin_lock_irqsave(&afe->lock, flags);
	v = afe_rd(afe, reg);
	v = (v & ~mask) | (val & mask);
	afe_wr(afe, reg, v);
	spin_unlock_irqrestore(&afe->lock, flags);
}

/* Recupera nuestro 'afe' desde el drvdata de la CARD (que fijamos en probe).
 * NO usar snd_soc_component_get_drvdata: component y card comparten el mismo
 * device, y register_card pisa el drvdata del device con el puntero de la card
 * -> get_drvdata devolveria la card y afe->base seria basura (oops en .open,
 * visto 0706). El drvdata de la card es campo propio, inmune a ese pisado. */
static struct mt6582_afe *afe_from_substream(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	return snd_soc_card_get_drvdata(rtd->card);
}

/* fs para memif/I2S-out/IRQ (AFE_DAC_CON1, AFE_I2S_CON1, AFE_IRQ_MCU_CON).
 * Fuente: AudDrv_SampleRateIndexConvert (44100=9, 48000=10). */
static int mt6582_fs_memif(unsigned int rate)
{
	switch (rate) {
	case 8000:	return 0;
	case 11025:	return 1;
	case 12000:	return 2;
	case 16000:	return 4;
	case 22050:	return 5;
	case 24000:	return 6;
	case 32000:	return 8;
	case 44100:	return 9;
	case 48000:	return 10;
	default:	return -EINVAL;
	}
}

/* fs para el ADDA DL SRC2 ([31:28] de AFE_ADDA_DL_SRC2_CON0) — codificacion
 * PROPIA del ADDA (44100=7, 48000=8). NO mezclar con la de memif. */
static int mt6582_fs_adda(unsigned int rate)
{
	switch (rate) {
	case 8000:	return 0;
	case 11025:	return 1;
	case 12000:	return 2;
	case 16000:	return 3;
	case 22050:	return 4;
	case 24000:	return 5;
	case 32000:	return 6;
	case 44100:	return 7;
	case 48000:	return 8;
	default:	return -EINVAL;
	}
}

static const struct snd_pcm_hardware mt6582_afe_hardware = {
	.info = SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |
		 SNDRV_PCM_RATE_12000 | SNDRV_PCM_RATE_16000 |
		 SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_24000 |
		 SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
		 SNDRV_PCM_RATE_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = MT6582_AFE_BUFFER_BYTES_MAX,
	.period_bytes_min = MT6582_AFE_PERIOD_BYTES_MIN,
	.period_bytes_max = MT6582_AFE_BUFFER_BYTES_MAX / 2,
	.periods_min = 2,
	.periods_max = 64,
};

static irqreturn_t mt6582_afe_irq(int irq, void *dev_id)
{
	struct mt6582_afe *afe = dev_id;
	u32 status, con;

	status = afe_rd(afe, AFE_IRQ_MCU_STATUS) & 0xf;
	con = afe_rd(afe, AFE_IRQ_MCU_CON);

	/* espuria (level-low): limpiar TODO o tormenta de IRQs
	 * (patron AudDrv_Clk_On_ClrISRStatus: bit6 clear-all + bits 0..5) */
	if (!status || !(con & 0x3)) {
		afe_wr(afe, AFE_IRQ_MCU_CLR, BIT(6));
		afe_wr(afe, AFE_IRQ_MCU_CLR, 0x3f);
		return IRQ_HANDLED;
	}

	if ((status & BIT(0)) && afe->dl1_substream)
		snd_pcm_period_elapsed(afe->dl1_substream);

	afe_wr(afe, AFE_IRQ_MCU_CLR, status);
	return IRQ_HANDLED;
}

/* ------------------------- ops PCM (component) ------------------------- */

static int mt6582_afe_open(struct snd_soc_component *component,
			   struct snd_pcm_substream *substream)
{
	struct mt6582_afe *afe = afe_from_substream(substream);

	if (substream->stream != SNDRV_PCM_STREAM_PLAYBACK)
		return -ENODEV;	/* captura (VUL) = Fase D */

	snd_soc_set_runtime_hwparams(substream, &mt6582_afe_hardware);

	/* clk-on del stock (AudDrv_Clk_On): idempotente, y re-arma el bloque
	 * si algo lo dejo gateado */
	afe_wr(afe, AUDIOAFE_TOP_CON0, 0x60004000);

	afe->dl1_substream = substream;
	return 0;
}

static int mt6582_afe_close(struct snd_soc_component *component,
			    struct snd_pcm_substream *substream)
{
	struct mt6582_afe *afe = afe_from_substream(substream);

	afe->dl1_substream = NULL;

	/* cierre (mt_soc_pcm_dl1 close): apagar SRC2/I2S-DAC/ADDA y el AFE.
	 * Sin captura implementada podemos apagar ADDA sin mirar el ADC. */
	afe_rmw(afe, AFE_ADDA_DL_SRC2_CON0, BIT(0), 0);
	afe_rmw(afe, AFE_I2S_CON1, BIT(0), 0);
	afe_rmw(afe, AFE_ADDA_UL_DL_CON0, BIT(0), 0);
	afe_rmw(afe, AFE_DAC_CON0, BIT(0), 0);	/* AFE_ON off */
	return 0;
}

static int mt6582_afe_hw_params(struct snd_soc_component *component,
				struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct mt6582_afe *afe = afe_from_substream(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	size_t bytes = params_buffer_bytes(params);
	dma_addr_t phys = runtime->dma_addr;

	if (phys & 0x1f) {
		dev_err(afe->dev, "buffer DL1 no alineado a 32B: %pad\n", &phys);
		return -EINVAL;
	}

	afe_wr(afe, AFE_DL1_BASE, lower_32_bits(phys));
	afe_wr(afe, AFE_DL1_END, lower_32_bits(phys) + bytes - 1);	/* inclusivo */
	afe_rmw(afe, AFE_MEMIF_MAXLEN, 0xf, 0x1);	/* buffer en DRAM */

	dev_dbg(afe->dev, "DL1 buf: phys=%pad bytes=%zu\n", &phys, bytes);
	return 0;
}

static int mt6582_afe_prepare(struct snd_soc_component *component,
			      struct snd_pcm_substream *substream)
{
	struct mt6582_afe *afe = afe_from_substream(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int fs = mt6582_fs_memif(runtime->rate);
	int fsa = mt6582_fs_adda(runtime->rate);
	u32 src2;

	if (fs < 0 || fsa < 0)
		return -EINVAL;

	/* clk-on del stock por si close() lo apago */
	afe_wr(afe, AUDIOAFE_TOP_CON0, 0x60004000);

	/* --- SetI2SDacOut(rate) del stock, en su orden --- */
	afe_wr(afe, AFE_PREDIS_CON0, 0);
	afe_wr(afe, AFE_PREDIS_CON1, 0);

	/* ADDA DL SRC2: fs ADDA en [31:28], magicos del stock, bit1=-0.3dB,
	 * bit0 (EN) aun a 0 (44k1 -> 0x73001802) */
	src2 = ((u32)fsa << 28) | (0x03 << 24) | (0x03 << 11) | BIT(1);
	afe_wr(afe, AFE_ADDA_DL_SRC2_CON0, src2);
	afe_wr(afe, AFE_ADDA_DL_SRC2_CON1, 0xF74F0000);

	/* I2S-DAC out: fmt I2S (bit3), 16 bit, fs memif en [11:8], EN aun a 0 */
	afe_wr(afe, AFE_I2S_CON1, ((u32)fs << 8) | BIT(3));

	/* fs del path I2S-out en DAC_CON1[11:8] */
	afe_rmw(afe, AFE_DAC_CON1, 0xf << 8, (u32)fs << 8);

	/* enables (orden SetI2SDacEnable): SRC2 -> I2S-DAC -> interfaz ADDA */
	afe_rmw(afe, AFE_ADDA_DL_SRC2_CON0, BIT(0), BIT(0));
	afe_rmw(afe, AFE_I2S_CON1, BIT(0), BIT(0));
	afe_rmw(afe, AFE_ADDA_UL_DL_CON0, BIT(0), BIT(0));

	/* IRQ1: periodo en FRAMES + fs en IRQ_MCU_CON[7:4] (enable en trigger) */
	afe_wr(afe, AFE_IRQ_MCU_CNT1, runtime->period_size);
	afe_rmw(afe, AFE_IRQ_MCU_CON, 0xf << 4, (u32)fs << 4);

	return 0;
}

static int mt6582_afe_trigger(struct snd_soc_component *component,
			      struct snd_pcm_substream *substream, int cmd)
{
	struct mt6582_afe *afe = afe_from_substream(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int fs = mt6582_fs_memif(runtime->rate);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		/* orden del stock (mt_soc_pcm_dl1 trigger start):
		 * CONN -> IRQ on -> rate/mono DL1 -> DL1_ON -> AFE_ON */
		afe_rmw(afe, AFE_CONN1, BIT(21), BIT(21));	/* I05->O03 */
		afe_rmw(afe, AFE_CONN2, BIT(6), BIT(6));	/* I06->O04 */
		afe_rmw(afe, AFE_IRQ_MCU_CON, BIT(0), BIT(0));
		afe_rmw(afe, AFE_DAC_CON1, 0xf, (u32)fs);
		afe_rmw(afe, AFE_DAC_CON1, BIT(21),
			runtime->channels == 1 ? BIT(21) : 0);
		afe_rmw(afe, AFE_DAC_CON0, BIT(1), BIT(1));	/* DL1_ON */
		afe_rmw(afe, AFE_DAC_CON0, BIT(0), BIT(0));	/* AFE_ON */
		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		/* orden del stock: IRQ off PRIMERO -> DL1 off -> desconectar */
		afe_rmw(afe, AFE_IRQ_MCU_CON, BIT(0), 0);
		afe_rmw(afe, AFE_DAC_CON0, BIT(1), 0);
		afe_rmw(afe, AFE_CONN1, BIT(21), 0);
		afe_rmw(afe, AFE_CONN2, BIT(6), 0);
		/* limpiar IRQ pendiente (Auddrv_Check_Irq) */
		afe_wr(afe, AFE_IRQ_MCU_CLR, BIT(0));
		return 0;
	default:
		return -EINVAL;
	}
}

static snd_pcm_uframes_t
mt6582_afe_pointer(struct snd_soc_component *component,
		   struct snd_pcm_substream *substream)
{
	struct mt6582_afe *afe = afe_from_substream(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	size_t bytes = snd_pcm_lib_buffer_bytes(substream);
	u32 cur = afe_rd(afe, AFE_DL1_CUR);
	u32 base = lower_32_bits(runtime->dma_addr);
	u32 offset;

	/* CUR es fisica absoluta; puede leer 0 o fuera de rango (sanear,
	 * gotcha del downstream :805-810) */
	if (cur < base || cur >= base + bytes)
		return 0;
	offset = cur - base;
	return bytes_to_frames(runtime, offset);
}

static int mt6582_afe_pcm_construct(struct snd_soc_component *component,
				    struct snd_soc_pcm_runtime *rtd)
{
	snd_pcm_set_managed_buffer_all(rtd->pcm, SNDRV_DMA_TYPE_DEV,
				       component->dev,
				       MT6582_AFE_BUFFER_BYTES_MAX,
				       MT6582_AFE_BUFFER_BYTES_MAX);
	return 0;
}

static const struct snd_soc_component_driver mt6582_afe_component = {
	.name		= "mt6582-afe",
	.open		= mt6582_afe_open,
	.close		= mt6582_afe_close,
	.hw_params	= mt6582_afe_hw_params,
	.prepare	= mt6582_afe_prepare,
	.trigger	= mt6582_afe_trigger,
	.pointer	= mt6582_afe_pointer,
	.pcm_construct	= mt6582_afe_pcm_construct,
};

/* ------------------------------ DAI ------------------------------ */

static struct snd_soc_dai_driver mt6582_afe_dais[] = {
	{
		.name = "mt6582-afe-pcm-dai",
		.playback = {
			.stream_name = "DL1 Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
};

/* --------------- tarjeta minima (Fase A: codec dummy) ---------------
 * En Fase C esto se sustituye por un machine driver de verdad con el codec
 * ANA del MT6323. Con el dummy: /proc/asound/cards + aplay corren (timing
 * real de DMA/IRQ) pero no suena nada. */

SND_SOC_DAILINK_DEFS(dl1,
	DAILINK_COMP_ARRAY(COMP_CPU("mt6582-afe-pcm-dai")),
	DAILINK_COMP_ARRAY(COMP_DUMMY()),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

static struct snd_soc_dai_link mt6582_afe_dai_links[] = {
	{
		.name = "DL1",
		.stream_name = "DL1 PCM",
		SND_SOC_DAILINK_REG(dl1),
	},
};

static struct snd_soc_card mt6582_afe_card = {
	.name = "mt6582-audio",
	.owner = THIS_MODULE,
	.dai_link = mt6582_afe_dai_links,
	.num_links = ARRAY_SIZE(mt6582_afe_dai_links),
};

/* ------------------------------ probe ------------------------------ */

static int mt6582_afe_probe(struct platform_device *pdev)
{
	struct mt6582_afe *afe;
	int irq, ret;
	u32 top;

	afe = devm_kzalloc(&pdev->dev, sizeof(*afe), GFP_KERNEL);
	if (!afe)
		return -ENOMEM;
	afe->dev = &pdev->dev;
	spin_lock_init(&afe->lock);

	afe->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(afe->base))
		return PTR_ERR(afe->base);

	ret = dma_coerce_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;
	ret = devm_request_irq(&pdev->dev, irq, mt6582_afe_irq, 0,
			       "mt6582-afe", afe);
	if (ret) {
		dev_err(&pdev->dev, "request_irq %d: %d\n", irq, ret);
		return ret;
	}

	/* sanity: el LK deja el bloque con reloj (validado HW 0705) */
	top = readl(afe->base + AUDIOAFE_TOP_CON0);
	dev_info(afe->dev, "AFE TOP_CON0=0x%08x irq=%d\n", top, irq);

	platform_set_drvdata(pdev, afe);
	dev_set_drvdata(&pdev->dev, afe);

	ret = devm_snd_soc_register_component(&pdev->dev,
					      &mt6582_afe_component,
					      mt6582_afe_dais,
					      ARRAY_SIZE(mt6582_afe_dais));
	if (ret)
		return ret;

	/* tarjeta minima con codec dummy (ver nota arriba). El drvdata de la card
	 * es como los ops recuperan 'afe' (afe_from_substream) — imprescindible
	 * fijarlo, porque register_card pisa el drvdata del device. */
	snd_soc_card_set_drvdata(&mt6582_afe_card, afe);
	mt6582_afe_dai_links[0].cpus->of_node = pdev->dev.of_node;
	mt6582_afe_dai_links[0].platforms->of_node = pdev->dev.of_node;
	mt6582_afe_card.dev = &pdev->dev;
	ret = devm_snd_soc_register_card(&pdev->dev, &mt6582_afe_card);
	if (ret)
		dev_err(&pdev->dev, "register card: %d\n", ret);
	return ret;
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

MODULE_DESCRIPTION("MediaTek MT6582 AFE PCM (Fase A.2: DL1 + IRQ)");
MODULE_LICENSE("GPL");
