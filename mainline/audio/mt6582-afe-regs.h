/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Registros del AFE MT6582 (offsets sobre 0x11220000).
 * Fuente: downstream AudDrv_Afe.h (sound/mt6582) + mt_soc_audio_v1 (mismo AFE),
 * verificados los load-bearing (extracción 2026-07-06).
 */
#ifndef _MT6582_AFE_REGS_H_
#define _MT6582_AFE_REGS_H_

#define AUDIOAFE_TOP_CON0	0x0000	/* bit2 = PDN_AFE, bit6 = PDN_I2S; clk-on stock = 0x60004000 */
#define AFE_DAC_CON0		0x0010	/* bit0 AFE_ON, bit1 DL1_ON, bit3 VUL_ON */
#define AFE_DAC_CON1		0x0014	/* [3:0] fs DL1, [11:8] fs I2S-out, bit21 DL1 mono */
#define AFE_I2S_CON		0x0018
#define AFE_CONN0		0x0020	/* mixer/interconexiones */
#define AFE_CONN1		0x0024	/* b21 = I05(DL1-L)->O03(DAC-L); b16 = I00(FM-L)->O03 */
#define AFE_CONN2		0x0028	/* b6 = I06(DL1-R)->O04(DAC-R); b1 = I01(FM-R)->O04;
					 * b16/b22 = I00/I01 -> O05/O06 (AWB, lo que usa el stock) */
#define AFE_CONN3		0x002C
#define AFE_CONN4		0x0030
#define AFE_I2S_CON1		0x0034	/* I2S-DAC out: bit0 EN, bit3 fmt I2S, [11:8] fs */
#define AFE_I2S_CON2		0x0038

#define AFE_DL1_BASE		0x0040	/* fisica, alineada a 32B */
#define AFE_DL1_CUR		0x0044	/* puntero HW (RO) — DIRECCION FISICA ABSOLUTA */
#define AFE_DL1_END		0x0048	/* INCLUSIVE: base + len - 1 */
#define AFE_DL2_BASE		0x0050
#define AFE_DL2_CUR		0x0054
#define AFE_DL2_END		0x0058
#define AFE_AWB_BASE		0x0070
#define AFE_AWB_END		0x0078
#define AFE_AWB_CUR		0x007C
#define AFE_VUL_BASE		0x0080
#define AFE_VUL_END		0x0088
#define AFE_VUL_CUR		0x008C

#define AFE_ADDA_DL_SRC2_CON0	0x0108	/* [31:28] fs ADDA, bit1 -0.3dB, bit0 EN */
#define AFE_ADDA_DL_SRC2_CON1	0x010C	/* ganancia DL: 0xF74F0000 */
#define AFE_ADDA_UL_SRC_CON0	0x0114
#define AFE_ADDA_UL_SRC_CON1	0x0118
#define AFE_ADDA_TOP_CON0	0x0120
#define AFE_ADDA_UL_DL_CON0	0x0124	/* bit0 = ADDA interface ON */
#define AFE_ADDA_NEWIF_CFG0	0x0138
#define AFE_ADDA_NEWIF_CFG1	0x013C

#define AFE_PREDIS_CON0		0x0260	/* pre-distorsion: limpiar a 0 antes del DAC */
#define AFE_PREDIS_CON1		0x0264

/* --- path de audio FM: 2º I2S IN (CONNSYS on-die) -> ASRC -> conexion directa
 * I00/I01 -> O03/O04 (DAC), via AFE_CONN1/2 de arriba. GOTCHA: los regs de
 * interconexion del GAIN1 (0x448...) NO existen en mt6582 (la tabla de
 * mt_soc_afe_connection.c v3 es de otro chip); GAIN1 queda sin usar.
 * Constantes del ASRC = identicas al dump stock (groundtruth-0708). --- */
#define AFE_GAIN1_CON0		0x0410	/* [15:8] samples/step, [7:4] fs, bit0 EN (sin usar) */
#define AFE_GAIN1_CON1		0x0414	/* ganancia objetivo (0x10000 = 0 dB) (sin usar) */
#define AFE_GAIN1_CUR		0x0424	/* ganancia actual (sin usar) */
#define AFE_ASRC_CON0		0x0500	/* bit0 = EN del ASRC del I2S in (bit6 NO latchea) */
#define AFE_ASRC_CON13		0x0550	/* config estatica; stock = 0x11 */
#define AFE_ASRC_CON14		0x0554	/* 32k->44.1k: 0xDC8000 */
#define AFE_ASRC_CON15		0x0558	/* target 0xA00000 (RO efectivo: tracking vivo ~0x9ffxxx) */
#define AFE_ASRC_CON16		0x055C	/* calibracion: 0x00075987 */
#define AFE_ASRC_CON17		0x0560	/* 32k->44.1k: 0x1FBD */
#define AFE_ASRC_CON18		0x0564	/* monitor de tracking (RO) */
#define AFE_ASRC_CON19		0x0568	/* monitor de tracking (RO) */
#define AFE_ASRC_CON20		0x056C	/* calibracion: 0x00001b00 */
#define AFE_ASRC_CON21		0x0570	/* config estatica; stock = 0x1800 */

#define AFE_IRQ_MCU_CON		0x03A0	/* bit0 IRQ1(DL) en, bit1 IRQ2(UL) en, [7:4] fs IRQ1 */
#define AFE_IRQ_MCU_STATUS	0x03A4	/* & 0xF: bit0=IRQ1, bit1=IRQ2 */
#define AFE_IRQ_MCU_CLR		0x03A8	/* write-1-clear; bit6 = clear-all */
#define AFE_IRQ_MCU_CNT1	0x03AC	/* frames por IRQ1 (periodo) */
#define AFE_IRQ_MCU_CNT2	0x03B0
#define AFE_IRQ_MCU_EN		0x03B4
#define AFE_MEMIF_MAXLEN	0x03D4	/* bit0: 0=buffer en SRAM interna, 1=DRAM */
#define AFE_MEMIF_PBUF_SIZE	0x03D8	/* (nunca escrito por el stock) */

/* SRAM interna de audio (opcion de buffer sin DMA coherente) */
#define MT6582_AFE_SRAM_PHYS	0x11221000
#define MT6582_AFE_SRAM_SIZE	0x4000

#endif /* _MT6582_AFE_REGS_H_ */
