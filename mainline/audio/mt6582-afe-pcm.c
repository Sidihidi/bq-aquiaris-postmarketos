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

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/mfd/mt6397/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "mt6582-afe-regs.h"

#define MT6582_AFE_BUFFER_BYTES_MAX	(64 * 1024)
#define MT6582_AFE_PERIOD_BYTES_MIN	512

/* GPIO del SoC MT6582 (0x10005000): DIR base 0x000, DOUT base 0x400 (SET +4, RST +8),
 * stride 0x10 por registro de 16 GPIOs. GPIO118 (amp de altavoz externo) = reg7, bit6. */
#define MT6582_GPIO_PHYS	0x10005000
#define MT6582_GPIO_SIZE	0x1000
#define GPIO118_DIR_SET		0x074	/* DIR reg7 SET  -> dir out */
#define GPIO118_DOUT_SET	0x474	/* DOUT reg7 SET -> HIGH */
#define GPIO118_DOUT_RST	0x478	/* DOUT reg7 RST -> LOW */
#define GPIO118_BIT		0x40	/* 118 % 16 = 6 */

struct mt6582_afe {
	void __iomem *base;			/* AFE del SoC @0x11220000 */
	void __iomem *gpio;			/* GPIO @0x10005000 (amp altavoz) */
	struct regmap *pmic;			/* MT6323 (codec analogico) via pwrap */
	struct device *dev;
	spinlock_t lock;			/* RMW de registros */
	struct snd_pcm_substream *dl1_substream;
	bool fm_route;				/* radio FM ruteada al DAC (kcontrol) */
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

/* Amplificador EXTERNO del altavoz por GPIO118 (yusu_android_speaker.c:Sound_Speaker_Turnon).
 * Tren de pulsos 1-0-1-0-1 (udelay 2) + 40 ms warm-up, queda HIGH = selecciona modo/ganancia
 * del chip amp por conteo de flancos. El altavoz del krillin NO usa el class-D interno del
 * MT6323 (SPK_CON queda a 0x200) sino este amp externo (confirmado por RE del HAL). */
static void mt6582_spk_amp(struct mt6582_afe *afe, bool on)
{
	if (!afe->gpio)
		return;
	writel(GPIO118_BIT, afe->gpio + GPIO118_DIR_SET);	/* dir out */
	if (on) {
		writel(GPIO118_BIT, afe->gpio + GPIO118_DOUT_SET); udelay(2);
		writel(GPIO118_BIT, afe->gpio + GPIO118_DOUT_RST); udelay(2);
		writel(GPIO118_BIT, afe->gpio + GPIO118_DOUT_SET); udelay(2);
		writel(GPIO118_BIT, afe->gpio + GPIO118_DOUT_RST); udelay(2);
		writel(GPIO118_BIT, afe->gpio + GPIO118_DOUT_SET);	/* queda HIGH */
		msleep(40);						/* SPK_WARM_UP_TIME */
	} else {
		writel(GPIO118_BIT, afe->gpio + GPIO118_DOUT_RST);	/* LOW */
	}
}

/* Enciende el codec analogico del MT6323 (via pwrap regmap). Secuencia EXACTA reconstruida
 * por RE del HAL de Android (audio.primary.mt6582.so, AudioAnalogControl::AnalogOpen dev 2/3).
 * LA CLAVE = el ORDEN + el usleep(10ms) de asentamiento del bias entre AUDTOP_CON4=0x7c y
 * AUDTOP_CON6=0xf5ba; un write plano del estado final NO suena. Registros del PMIC:
 * banco ABB_AFE @0x4000 (digital) + AUDTOP_CON @0x0700 (analogico) + reloj @0x010c. */
static void mt6582_codec_dl_on(struct mt6582_afe *afe)
{
	struct regmap *r = afe->pmic;

	/* --- §2 AFE/downlink digital del PMIC --- */
	regmap_update_bits(r, 0x010c, 0x0100, 0x0100);	/* reloj del bloque AFE del PMIC */
	regmap_write(r, 0x4024, 0x7330);		/* ABB_AFE_PMIC_NEWIF_CFG0 (freq ADDA=7) */
	regmap_update_bits(r, 0x4002, 0x000f, 0x0009);	/* ABB_AFE_CON1 (sample-rate) */
	regmap_update_bits(r, 0x4000, 0x0001, 0x0001);	/* ABB_AFE_CON0 = AFE_ON */
	regmap_write(r, 0x4006, 0x0253);		/* CON3 */
	regmap_write(r, 0x4008, 0x0274);		/* CON4 */
	regmap_write(r, 0x4014, 0x0001);		/* CON10 */
	regmap_write(r, 0x4016, 0x0303);		/* CON11 */

	/* --- §3 secuencia ANALOGICA de auriculares (ORDEN LITERAL + delay de bias) --- */
	regmap_write(r, 0x070c, 0xf7f2);		/* AUDTOP_CON6: bias/LDO/refgen HP (pre-depop) */
	regmap_update_bits(r, 0x0700, 0xf000, 0x7000);	/* AUDTOP_CON0 bits[15:12] */
	regmap_write(r, 0x070a, 0x0014);		/* AUDTOP_CON5 */
	regmap_write(r, 0x0708, 0x007c);		/* AUDTOP_CON4 = bias + L/R DAC */
	usleep_range(10000, 12000);			/* 10 ms ASENTAMIENTO DEL BIAS (LO CLAVE) */
	regmap_write(r, 0x070c, 0xf5ba);		/* AUDTOP_CON6: enciende drivers HP L/R */
	regmap_write(r, 0x070a, 0x2214);		/* AUDTOP_CON5 */
	regmap_update_bits(r, 0x070a, 0x7000, 0x4000);	/* ganancia HP-L */
	regmap_update_bits(r, 0x070a, 0x0700, 0x0400);	/* ganancia HP-R */
}

static void mt6582_codec_dl_off(struct mt6582_afe *afe)
{
	struct regmap *r = afe->pmic;

	regmap_write(r, 0x0708, 0x0000);		/* AUDTOP_CON4: disable bias + L-DAC */
	regmap_update_bits(r, 0x4000, 0x0001, 0x0000);	/* ABB_AFE_CON0 off */
}

/* Ruteo del audio de la RADIO FM al DAC. El chip FM (CONNSYS on-die, MT6627-IP)
 * emite I2S a 32 kHz en modo master — su salida I2S NO se enciende en el powerup
 * (viene comentada en el stock): la enciende userspace con FM_IOCTL_I2S_SETTING
 * {ON, MASTER, 32K}. Ese I2S entra al AFE por el 2º I2S IN (pad INTERNO del
 * CONNSYS, sin pinmux: AFE_I2S_CON=0x8000000d = esclavo + fmt I2S + phase-fix,
 * el mismo valor del ground truth que .prepare ya escribe) -> ASRC 32k->44.1k
 * -> HW GAIN1 (rampa) -> interconexion -> I2S-DAC -> codec HP (los auriculares
 * son la antena). Secuencia del downstream mt_soc_pcm_fm_i2s.c; conexiones de
 * las tablas de mt_soc_afe_connection.c. */
static void mt6582_afe_fm_route(struct mt6582_afe *afe, bool on)
{
	if (on) {
		/* clocks + lado DAC a 44.1k (mismo camino que .prepare) */
		afe_wr(afe, AUDIOAFE_TOP_CON0, 0x60004000);
		afe_wr(afe, AFE_PREDIS_CON0, 0);
		afe_wr(afe, AFE_PREDIS_CON1, 0);
		afe_wr(afe, AFE_ADDA_DL_SRC2_CON0,
		       (7u << 28) | (0x03 << 24) | (0x03 << 11) | BIT(1));
		afe_wr(afe, AFE_ADDA_DL_SRC2_CON1, 0xF74F0000);
		afe_wr(afe, AFE_I2S_CON1, (9u << 8) | BIT(3));
		afe_rmw(afe, AFE_DAC_CON1, 0xf << 8, 9u << 8);
		afe_rmw(afe, AFE_ADDA_DL_SRC2_CON0, BIT(0), BIT(0));
		afe_rmw(afe, AFE_I2S_CON1, BIT(0), BIT(0));
		afe_rmw(afe, AFE_ADDA_UL_DL_CON0, BIT(0), BIT(0));
		afe_wr(afe, AFE_ADDA_NEWIF_CFG0, 0x03f87200);
		afe_wr(afe, AFE_ADDA_NEWIF_CFG1, 0x03117180);
		afe_rmw(afe, AFE_DAC_CON1, 0x9000, 0x9000);

		/* 2º I2S IN desde CONNSYS (esclavo) + enable */
		afe_wr(afe, AFE_I2S_CON, 0x8000000d);

		/* ASRC 32k (chip FM master) -> 44.1k (dominio del DAC) */
		afe_rmw(afe, AFE_CONN4, BIT(30), 0);		/* 0 = pasar por ASRC */
		afe_rmw(afe, AFE_ASRC_CON13, BIT(16), 0);	/* stereo */
		afe_wr(afe, AFE_ASRC_CON14, 0xDC8000);
		afe_wr(afe, AFE_ASRC_CON15, 0xA00000);
		afe_wr(afe, AFE_ASRC_CON17, 0x1FBD);
		afe_wr(afe, AFE_ASRC_CON16, 0x00075987);
		afe_wr(afe, AFE_ASRC_CON20, 0x00001b00);
		afe_rmw(afe, AFE_ASRC_CON0, BIT(6) | BIT(0), BIT(6) | BIT(0));

		/* HW GAIN1 a 0 dB (con rampa desde 0) + interconexiones del path */
		afe_wr(afe, AFE_GAIN1_CUR, 0);
		afe_rmw(afe, AFE_GAIN1_CON0, 0xfff0, (0x80 << 8) | (9 << 4));
		afe_rmw(afe, AFE_GAIN1_CON0, BIT(0), BIT(0));
		afe_wr(afe, AFE_GAIN1_CON1, 0x10000);
		afe_rmw(afe, AFE_CONN_GAIN1_IN, BIT(2) | BIT(16), BIT(2) | BIT(16));
		afe_rmw(afe, AFE_CONN_GAIN1_OUT, BIT(8) | BIT(10), BIT(8) | BIT(10));

		afe_rmw(afe, AFE_DAC_CON0, BIT(0), BIT(0));	/* AFE_ON */

		/* codec HP on (si no lo tiene ya encendido un playback activo) */
		if (afe->pmic && !afe->dl1_substream)
			mt6582_codec_dl_on(afe);
	} else {
		afe_rmw(afe, AFE_CONN_GAIN1_IN, BIT(2) | BIT(16), 0);
		afe_rmw(afe, AFE_CONN_GAIN1_OUT, BIT(8) | BIT(10), 0);
		afe_rmw(afe, AFE_ASRC_CON0, BIT(6) | BIT(0), 0);
		afe_rmw(afe, AFE_CONN4, BIT(30), BIT(30));	/* bypass ASRC */
		afe_rmw(afe, AFE_GAIN1_CON0, BIT(0), 0);
		/* si no hay playback activo, apagar tambien codec y lado DAC */
		if (!afe->dl1_substream) {
			if (afe->pmic)
				mt6582_codec_dl_off(afe);
			afe_rmw(afe, AFE_ADDA_DL_SRC2_CON0, BIT(0), 0);
			afe_rmw(afe, AFE_I2S_CON1, BIT(0), 0);
			afe_rmw(afe, AFE_ADDA_UL_DL_CON0, BIT(0), 0);
			afe_rmw(afe, AFE_DAC_CON0, BIT(0), 0);
		}
	}
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

	/* apagar el amp del altavoz + el codec analogico (antes que el AFE digital) */
	mt6582_spk_amp(afe, false);

	/* con la radio FM ruteada, el codec y el lado DAC deben SEGUIR vivos
	 * (los apagara el kcontrol de FM al desactivarse) */
	if (afe->fm_route)
		return 0;

	if (afe->pmic)
		mt6582_codec_dl_off(afe);

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

	/* --- deltas del SoC AFE (ground truth de LineageOS sonando): el enlace serie
	 * NEWIF al PMIC + los bits que faltaban de DAC_CON0/1 y CONN2 --- */
	afe_wr(afe, AFE_I2S_CON, 0x8000000d);
	afe_wr(afe, AFE_ADDA_NEWIF_CFG0, 0x03f87200);
	afe_wr(afe, AFE_ADDA_NEWIF_CFG1, 0x03117180);
	afe_rmw(afe, AFE_DAC_CON0, 0x13040, 0x13040);	/* bits 6,12,16 (el trigger anade 0,1) */
	afe_rmw(afe, AFE_DAC_CON1, 0x9000, 0x9000);
	afe_rmw(afe, AFE_CONN2, 0x410000, 0x410000);	/* bits 16,22 (el trigger anade el 6) */

	/* --- encender el codec analogico del MT6323 (secuencia con delay de bias) + el
	 * amplificador externo del altavoz. Con esto aplay/Phosh suenan solos. --- */
	if (afe->pmic)
		mt6582_codec_dl_on(afe);
	mt6582_spk_amp(afe, true);

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

/* kcontrol de card: "FM Radio Route" — activa/desactiva el path FM->DAC.
 * Uso: amixer -c 0 cset name='FM Radio Route' 1 (con el FM encendido y su
 * I2S activado por ioctl). snd_kcontrol_chip = snd_soc_card (los controles
 * de card llevan la card como private data). */
static int mt6582_fm_route_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
	struct mt6582_afe *afe = snd_soc_card_get_drvdata(card);

	ucontrol->value.integer.value[0] = afe->fm_route;
	return 0;
}

static int mt6582_fm_route_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
	struct mt6582_afe *afe = snd_soc_card_get_drvdata(card);
	bool on = !!ucontrol->value.integer.value[0];

	if (on == afe->fm_route)
		return 0;
	afe->fm_route = on;
	mt6582_afe_fm_route(afe, on);
	dev_info(afe->dev, "FM route %s\n", on ? "ON" : "OFF");
	return 1;
}

static const struct snd_kcontrol_new mt6582_card_controls[] = {
	SOC_SINGLE_BOOL_EXT("FM Radio Route", 0,
			    mt6582_fm_route_get, mt6582_fm_route_put),
};

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
	.controls = mt6582_card_controls,
	.num_controls = ARRAY_SIZE(mt6582_card_controls),
};

/* ------------------------------ probe ------------------------------ */

static int mt6582_afe_probe(struct platform_device *pdev)
{
	struct device_node *pmic_np;
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

	/* codec analogico: regmap del MT6323 via el phandle mediatek,pmic. El mt6323 es
	 * hijo del MFD mt6397; su drvdata es el mt6397_chip, que expone el regmap del pwrap
	 * (max_register 0xffff -> alcanza AUDTOP @0x700 y ABB_AFE @0x4000). */
	pmic_np = of_parse_phandle(pdev->dev.of_node, "mediatek,pmic", 0);
	if (pmic_np) {
		struct platform_device *pmic_pdev = of_find_device_by_node(pmic_np);
		struct mt6397_chip *chip;

		of_node_put(pmic_np);
		if (pmic_pdev) {
			chip = dev_get_drvdata(&pmic_pdev->dev);
			if (chip)
				afe->pmic = chip->regmap;
			put_device(&pmic_pdev->dev);
		}
		if (!afe->pmic)
			return dev_err_probe(&pdev->dev, -EPROBE_DEFER,
					     "MT6323 aun no listo\n");
		/* GPIO del amplificador externo del altavoz (GPIO118) */
		afe->gpio = devm_ioremap(&pdev->dev, MT6582_GPIO_PHYS, MT6582_GPIO_SIZE);
		if (!afe->gpio)
			dev_warn(&pdev->dev, "sin ioremap GPIO: altavoz no funcionara\n");
	} else {
		dev_warn(&pdev->dev, "sin mediatek,pmic: solo PCM digital (sin sonido)\n");
	}

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
