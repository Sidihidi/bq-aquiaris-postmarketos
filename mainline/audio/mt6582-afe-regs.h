/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Registros del AFE MT6582 (offsets sobre 0x11220000).
 * Fuente: downstream AudDrv_Afe.h (sound/mt6582), verificados los load-bearing.
 */
#ifndef _MT6582_AFE_REGS_H_
#define _MT6582_AFE_REGS_H_

#define AUDIOAFE_TOP_CON0	0x0000	/* bit2 = AFE_ON, bit6 = I2S clk */
#define AFE_DAC_CON0		0x0010	/* enables DL1/VUL/... */
#define AFE_DAC_CON1		0x0014	/* sample rates por memif */
#define AFE_I2S_CON		0x0018
#define AFE_CONN0		0x0020	/* mixer: interconexiones I05/I06->O03/O04 */
#define AFE_CONN1		0x0024
#define AFE_CONN2		0x0028
#define AFE_CONN3		0x002C
#define AFE_CONN4		0x0030
#define AFE_I2S_CON1		0x0034
#define AFE_I2S_CON2		0x0038

#define AFE_DL1_BASE		0x0040	/* buffer DMA playback (phys) */
#define AFE_DL1_CUR		0x0044	/* puntero HW (RO) */
#define AFE_DL1_END		0x0048
#define AFE_DL2_BASE		0x0050
#define AFE_DL2_CUR		0x0054
#define AFE_DL2_END		0x0058
#define AFE_AWB_BASE		0x0070
#define AFE_AWB_END		0x0078
#define AFE_AWB_CUR		0x007C
#define AFE_VUL_BASE		0x0080	/* buffer DMA captura */
#define AFE_VUL_END		0x0088
#define AFE_VUL_CUR		0x008C

#define AFE_ADDA_DL_SRC2_CON0	0x0108
#define AFE_ADDA_DL_SRC2_CON1	0x010C
#define AFE_ADDA_UL_SRC_CON0	0x0114
#define AFE_ADDA_UL_SRC_CON1	0x0118
#define AFE_ADDA_TOP_CON0	0x0120
#define AFE_ADDA_UL_DL_CON0	0x0124
#define AFE_ADDA_NEWIF_CFG0	0x0138
#define AFE_ADDA_NEWIF_CFG1	0x013C

#define AFE_IRQ_MCU_CON		0x03A0	/* enable/modo IRQ1/IRQ2 */
#define AFE_IRQ_MCU_STATUS	0x03A4
#define AFE_IRQ_MCU_CLR		0x03A8
#define AFE_IRQ_MCU_CNT1	0x03AC	/* samples entre IRQs (DL) */
#define AFE_IRQ_MCU_CNT2	0x03B0
#define AFE_IRQ_MCU_EN		0x03B4
#define AFE_MEMIF_PBUF_SIZE	0x03D8

#endif /* _MT6582_AFE_REGS_H_ */
