/* SPDX-License-Identifier: GPL-2.0 */
/*
 * os/linux/glue/hif.h — CONTRATO del backend HIF que el core stock del mt_wifi consume.
 *
 * Este es el "abajo" de la arquitectura HIBRIDA del port a linux-7.0.12: el core stock
 * (nic/hal.h, nic_tx/nic_rx) accede al chip EXCLUSIVAMENTE por las macros HAL_* de
 * <nic/hal.h> (HAL_MCR_RD/WR, HAL_PORT_RD/WR, HAL_READ_INTR_STATUS, HAL_INTR_*...), y esas
 * macros llaman a kalDevRegRead/kalDevRegWrite/kalDevPortRead/kalDevPortWrite.
 *
 * Aqui declaramos:
 *   (a) el struct de estado del backend (GL_HIF_INFO_T) EMBEBIDO en GLUE_INFO_T como rHifInfo,
 *       que guarda los ioremaps del bloque MCR (0x180F0000) + MCU (0x18070000) + PDMA, el irq y
 *       el estado del guard driver-own/settle.
 *   (b) los prototipos de los cuatro kalDev* (firmas EXACTAS del stock gl_kal.h:1888-1918).
 *   (c) las primitivas glBusSetIrq/glBusFreeIrq (firmas de os/linux/hif/ahb/include/hif.h).
 *
 * La IMPLEMENTACION vive en os/linux/hif/mt6582-hif.c (extraida del driver PIO probado
 * mt6582-wifi.c). Los defines de registro (MCR_*, WHLPCR_*, WHISR_*, HSTCR_*, HIF_TARGET_*)
 * los provee <mt6582-wifi-reg.h>, que este header incluye para que las macros HAL_* del core
 * (que usan MCR_WRDR0/MCR_WTDR0/MCR_WHISR/...) resuelvan.
 *
 * OJO gl_typedef.h redefine BIT()/BITS() y colisiona con <linux/bits.h> (que usa
 * mt6582-wifi-reg.h). El orden de include del port hace #undef BIT/#undef BITS antes de
 * gl_typedef.h; aqui asumimos que BIT()/GENMASK() son los de <linux/bits.h>.
 */
#ifndef _GLUE_HIF_H
#define _GLUE_HIF_H

#include <linux/types.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include "mt6582-wifi-reg.h"	/* MCR_*, WHLPCR_*, WHISR_*, WHIER_*, HSTCR_*, HIF_TARGET_*, WIFI_HIF_PHYS... */

/* Tipos del core (gl_typedef.h). Se declaran forward para no arrastrar todo el core en
 * este header; el .c real incluye precomp.h que trae GLUE_INFO_T completo. */
struct _GLUE_INFO_T;
typedef struct _GLUE_INFO_T	GLUE_INFO_T;
typedef struct _GLUE_INFO_T	*P_GLUE_INFO_T;

#ifndef _HIF_H	/* no chocar con el hif.h stock si alguien lo arrastra */

/*
 * Estado del backend HIF, embebido como GLUE_INFO_T.rHifInfo (igual nombre que el stock
 * GL_HIF_INFO_T -> el core que hace prGlueInfo->rHifInfo.fgIntReadClear compila sin tocar).
 * Guarda las bases ioremapeadas + el IRQ + el guard driver-own/settle de nuestro PIO probado.
 */
typedef struct _GL_HIF_INFO_T {
	struct device		*Dev;		/* &pdev->dev (dma / dev helpers) */
	struct platform_device	*PDev;		/* fuente del platform_get_irq() */

	u32			ChipID;		/* leido de WCIR (== 0x6582) */

	/* flags de control que el core stock consulta (hal.h HAL_SET_INTR_STATUS_*) */
	bool			fgIntReadClear;
	bool			fgMbxReadClear;
	bool			fgDmaEnable;	/* siempre false: backend PIO */

	/* bases ioremapeadas */
	void __iomem		*HifRegBaseAddr;	/* bloque MCR @ WIFI_HIF_PHYS (0x180F0000) */
	void __iomem		*McuRegBaseAddr;	/* bloque MCU @ WIFI_MCU_PHYS (0x18070000): chip-id/CPUPCR */
	void __iomem		*DmaRegBaseAddr;	/* canal PDMA @ WIFI_PDMA_PHYS (Fase 3; PIO no lo usa) */

	spinlock_t		DdmaLock;	/* reservado (PDMA) */

	/* ---- guard anti-cuelgue (extraido de wifi_hif_alive) ---- */
	bool			fgHifDead;	/* driver-own no recuperado -> abortar PIO en vez de colgar el AHB */
	unsigned long		DiscSettleUntil;	/* jiffies: ventana post-disconnect sin tocar el data-port */

	/* ---- IRQ real del HIF (GIC_SPI 184) ---- */
	int			Irq;		/* <=0 => sin IRQ (el core cae a polling del tx_thread) */
	bool			fgIrqOk;	/* request_irq tuvo exito */
	bool			fgIrqDisabled;	/* enmascarado en GIC durante teardown/settle del disconnect */
} GL_HIF_INFO_T, *P_GL_HIF_INFO_T;

#endif /* _HIF_H */

/*******************************************************************************
 *  CONTRATO kalDev* — firmas EXACTAS del stock os/linux/include/gl_kal.h:1888-1918.
 *  El core las llama SIEMPRE a traves de las macros HAL_* de <nic/hal.h>.
 *  Devuelven BOOL (int): TRUE=ok, FALSE=fallo de bus (el core setea ADAPTER_FLAG_HW_ERR).
 *******************************************************************************/
/* kalDev* los declara gl_kal.h (tipos stock BOOL/UINT_32/...) */

/* HifIsFwOwn(P_ADAPTER_T) lo define el CORE STOCK (nic/nic.c: return prAdapter->fgIsFwOwn).
 * Nuestro backend hace su guard interno (hif_alive) dentro de kalDevPortRead/Write, asi que
 * NO redeclaramos aqui un HifIsFwOwn(P_GLUE_INFO_T): colisionaria con la firma del stock. */

/* wrappers de propiedad (los usan glSetPowerState / el bring-up del core) */
int  kalDevSetDriverOwn(P_GLUE_INFO_T prGlueInfo);	/* 0 = driver-own conseguido */
void kalDevSetFwOwn(P_GLUE_INFO_T prGlueInfo);

/*******************************************************************************
 *  IRQ — firmas del stock os/linux/hif/ahb/include/hif.h.
 *  pvData   = struct net_device* (el core saca GlueInfo de netdev_priv).
 *  pfnIsr   = ISR del core (IGNORADO: usamos nuestro glHifIsr, que setea GLUE_FLAG_INT).
 *  pvCookie = GLUE_INFO_T* (dev_id de request_irq -> nuestro ISR lo usa directo).
 *******************************************************************************/
int  glBusSetIrq(void *pvData, void *pfnIsr, void *pvCookie);
void glBusFreeIrq(void *pvData, void *pvCookie);

/*******************************************************************************
 *                                 M A C R O S   H A L _ *
 *  Redefinicion del contrato de <nic/hal.h> para el backend AHB-PIO de 7.0.12.
 *  El stock hal.h ya trae la rama non-SDIO (kalDev*) — la reproducimos aqui SOLO
 *  para las macros que el core consume directamente y que el enunciado pide explicitar,
 *  con la firma real (5 args en PORT_RD/WR: prAdapter, port, len, buf, validbufsize).
 *  El core que incluye <nic/hal.h> ya las tiene; estas son idempotentes (mismo cuerpo).
 *******************************************************************************/
#ifndef _HAL_H	/* si el core ya incluyo hal.h, NO redefinir (evita colision) */

#define HAL_MCR_RD(_prAdapter, _u4Offset, _pu4Value) \
	kalDevRegRead((_prAdapter)->prGlueInfo, (_u4Offset), (_pu4Value))

#define HAL_MCR_WR(_prAdapter, _u4Offset, _u4Value) \
	kalDevRegWrite((_prAdapter)->prGlueInfo, (_u4Offset), (_u4Value))

#define HAL_PORT_RD(_prAdapter, _u4Port, _u4Len, _pucBuf, _u4ValidBufSize) \
	kalDevPortRead((_prAdapter)->prGlueInfo, (_u4Port), (_u4Len), \
		       (_pucBuf), (_u4ValidBufSize))

#define HAL_PORT_WR(_prAdapter, _u4Port, _u4Len, _pucBuf, _u4ValidBufSize) \
	kalDevPortWrite((_prAdapter)->prGlueInfo, (_u4Port), (_u4Len), \
			(_pucBuf), (_u4ValidBufSize))

#define HAL_READ_RX_PORT(_prAdapter, _u4PortId, _u4Len, _pvBuf, _u4ValidBufSize) \
	HAL_PORT_RD((_prAdapter), \
		    ((_u4PortId) == 0) ? MCR_WRDR0 : MCR_WRDR1, \
		    (_u4Len), (_pvBuf), (_u4ValidBufSize))

#define HAL_WRITE_TX_PORT(_prAdapter, _ucTxPortIdx, _u4Len, _pucBuf, _u4ValidBufSize) \
	HAL_PORT_WR((_prAdapter), \
		    ((_ucTxPortIdx) == 0) ? MCR_WTDR0 : MCR_WTDR1, \
		    (_u4Len), (_pucBuf), (_u4ValidBufSize))

#define HAL_READ_TX_RELEASED_COUNT(_prAdapter, _aucTxReleaseCount) \
	{ \
		u32 *__p = (u32 *)(_aucTxReleaseCount); \
		HAL_MCR_RD((_prAdapter), MCR_WTSR0, &__p[0]); \
		HAL_MCR_RD((_prAdapter), MCR_WTSR1, &__p[1]); \
	}

#define HAL_READ_RX_LENGTH(_prAdapter, _pu2Rx0Len, _pu2Rx1Len) \
	{ \
		u32 __v = 0; \
		HAL_MCR_RD((_prAdapter), MCR_WRPLR, &__v); \
		*(_pu2Rx0Len) = (u16)(__v); \
		*(_pu2Rx1Len) = (u16)((__v) >> 16); \
	}

/* HAL_READ_INTR_STATUS: leer 'length' bytes desde el puerto WHISR. length==88 dispara la
 * lectura del bloque enhance (read-clear que re-arma RX0_DONE); length==4 lee solo el whisr. */
#define HAL_READ_INTR_STATUS(_prAdapter, _length, _pvBuf) \
	HAL_PORT_RD((_prAdapter), MCR_WHISR, (_length), (_pvBuf), (_length))

#define HAL_INTR_DISABLE(_prAdapter) \
	HAL_MCR_WR((_prAdapter), MCR_WHLPCR, WHLPCR_INT_EN_CLR)

#define HAL_INTR_ENABLE(_prAdapter) \
	HAL_MCR_WR((_prAdapter), MCR_WHLPCR, WHLPCR_INT_EN_SET)

#endif /* _HAL_H */

#endif /* _GLUE_HIF_H */
