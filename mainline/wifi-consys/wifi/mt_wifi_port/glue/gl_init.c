// SPDX-License-Identifier: GPL-2.0
/*
 * gl_init.c — MINIMO (Fase 1.5): lo justo para que el modulo ENLACE en un .ko.
 *
 * Contiene:
 *   - globales owner del ciclo probe/remove: g_halt_sem, g_u4HaltFlag, fgIsResetting,
 *     aucDebugModule (declarados extern en gl_kal.h / debug.h).
 *   - stubs de los ~12 shims KAL que el core stock referencia y que los 6 gl_kal_*.c
 *     del workflow no llegaron a implementar (mem/config/csum/random/RoC/RSSI/WSC).
 *   - stubs aaa_fsm (AP-mode): no se ejercitan en STA pero los enlazan las tablas de
 *     hem_mbox; devuelven fallo/no-op.
 *   - module_init/exit minimo + MODULE_LICENSE.
 *
 * Fase 3 SUSTITUIRA este fichero por el probe real: platform_driver sobre el consys
 * (EPROBE_DEFER en mt6582_consys_ready, func_on(WIFI), VCN33 always-on) + wlanAdapterStart
 * stock + registro de wiphy/netdev. De momento el modulo carga INERTE (sin probe).
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/semaphore.h>

#include "precomp.h"

/* ===================== globales del ciclo probe/remove ===================== */
struct semaphore g_halt_sem;
int              g_u4HaltFlag = 1;	/* 1 = halted hasta que Fase 3 haga el probe */
BOOLEAN          fgIsResetting = FALSE;	/* CFG_CHIP_RESET_SUPPORT=0 en el port STA-only */
UINT_8           aucDebugModule[DBG_MODULE_NUM];

/* ===================== shims KAL que faltaban (12) ========================= */
PVOID kalAllocateIOBuffer(IN UINT_32 u4AllocSize)
{
	return kmalloc(u4AllocSize, GFP_KERNEL);	/* PIO: no requiere contiguidad DMA */
}

VOID kalReleaseIOBuffer(IN PVOID pvAddr, IN UINT_32 u4Size)
{
	(void)u4Size;
	kfree(pvAddr);
}

P_REG_INFO_T kalGetConfiguration(IN P_GLUE_INFO_T prGlueInfo)
{
	return &prGlueInfo->rRegInfo;	/* NVRAM stub -> defaults (fgNvramAvailable=FALSE) */
}

BOOLEAN kalIsConfigurationExist(IN P_GLUE_INFO_T prGlueInfo)
{
	(void)prGlueInfo;
	return FALSE;			/* sin NVRAM en el port */
}

VOID kalHandleAssocInfo(IN P_GLUE_INFO_T prGlueInfo, IN P_EVENT_ASSOC_INFO prAssocInfo)
{
	(void)prGlueInfo;
	(void)prAssocInfo;		/* los IEs de assoc se pasan a cfg80211 en connect_bss */
}

VOID kalQueryTxChksumOffloadParam(IN PVOID pvPacket, OUT PUINT_8 pucFlag)
{
	(void)pvPacket;
	if (pucFlag)
		*pucFlag = 0;		/* sin HW checksum offload */
}

VOID kalUpdateRxCSUMOffloadParam(IN PVOID pvPacket, IN ENUM_CSUM_RESULT_T eCSUM[])
{
	(void)pvPacket;
	(void)eCSUM;
}

UINT_32 kalRandomNumber(VOID)
{
	return get_random_u32();
}

VOID kalReadyOnChannel(IN P_GLUE_INFO_T prGlueInfo, IN UINT_64 u8Cookie,
		       IN ENUM_BAND_T eBand, IN ENUM_CHNL_EXT_T eSco,
		       IN UINT_8 ucChannelNum, IN UINT_32 u4DurationMs)
{
	(void)prGlueInfo; (void)u8Cookie; (void)eBand;
	(void)eSco; (void)ucChannelNum; (void)u4DurationMs;	/* remain-on-channel: Fase 4 */
}

VOID kalRemainOnChannelExpired(IN P_GLUE_INFO_T prGlueInfo, IN UINT_64 u8Cookie,
			       IN ENUM_BAND_T eBand, IN ENUM_CHNL_EXT_T eSco,
			       IN UINT_8 ucChannelNum)
{
	(void)prGlueInfo; (void)u8Cookie; (void)eBand; (void)eSco; (void)ucChannelNum;
}

VOID kalUpdateRSSI(IN P_GLUE_INFO_T prGlueInfo, IN ENUM_KAL_NETWORK_TYPE_INDEX_T eNetTypeIdx,
		   IN INT_8 cRssi, IN INT_8 cLinkQuality)
{
	(void)prGlueInfo; (void)eNetTypeIdx; (void)cRssi; (void)cLinkQuality;
}

BOOLEAN kalWSCGetActiveState(IN P_GLUE_INFO_T prGlueInfo)
{
	(void)prGlueInfo;
	return FALSE;			/* WSC/WPS no activo */
}

/* ===================== stubs aaa_fsm (AP-mode, no STA) ===================== */
VOID aaaFsmRunEventRxAuth(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb)
{
	(void)prAdapter; (void)prSwRfb;
}

WLAN_STATUS aaaFsmRunEventRxAssoc(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb)
{
	(void)prAdapter; (void)prSwRfb;
	return WLAN_STATUS_FAILURE;
}

WLAN_STATUS aaaFsmRunEventTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
				 IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	(void)prAdapter; (void)prMsduInfo; (void)rTxDoneStatus;
	return WLAN_STATUS_SUCCESS;
}

/* ===================== module init/exit minimo ============================ */
static int __init mtk_mtwifi_init(void)
{
	sema_init(&g_halt_sem, 1);
	pr_info("mtk_mtwifi: cargado (Fase 1.5 — ENLAZA; probe real pendiente de Fase 3)\n");
	return 0;
}

static void __exit mtk_mtwifi_exit(void)
{
	pr_info("mtk_mtwifi: descargado\n");
}

module_init(mtk_mtwifi_init);
module_exit(mtk_mtwifi_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek MT6582 full-MAC WiFi (port stock mt_wifi -> 7.0.12, STA-only)");
MODULE_AUTHOR("pmOS krillin port");
