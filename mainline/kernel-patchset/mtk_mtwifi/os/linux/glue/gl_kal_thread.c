// SPDX-License-Identifier: GPL-2.0
/*
 * os/linux/glue/gl_kal_thread.c
 *
 *  KAL shim (thread / command-stream) for the linux-7.0.12 (postmarketOS /
 *  BQ krillin, armv7) port of the MediaTek MT6582 'mt_wifi' full-MAC driver.
 *
 *  This translation unit owns the SINGLE glue thread ("tx_thread") whose
 *  serialization *is* the command stream of the whole stock core: every OID,
 *  every command packet (wlanSendSetQueryCmd -> kalEnqueueCommand -> rCmdQueue),
 *  every interrupt-service pass and every TX flush is driven from here, one at a
 *  time, so the full-MAC firmware never sees two overlapping requests.
 *
 *  It is a faithful port of the stock Android-3.10 tx_thread() in
 *      mt_wifi/wlan/os/linux/gl_kal.c
 *  with the dispatch order preserved EXACTLY:
 *
 *      (1) INT      -> wlanIST(prAdapter)                (nicProcessIST inside)
 *      (2) OID      -> wlanSetInformation/wlanQueryInformation on OidEntry
 *      (3) TXREQ    -> wlanProcessMboxMessage
 *                      + wlanProcessCommandQueue(rCmdQueue)   (command stream)
 *                      + wlanEnqueueTxPacket loop + wlanTxPendingPackets
 *      (4) TIMEOUT  -> wlanTimerTimeoutCheck
 *      (5) HALT     -> break out, flush, complete(rHaltComp)
 *
 *  Deltas vs stock (see MEMORY / port contract):
 *    - STA-only diet: CFG_ENABLE_WIFI_DIRECT paths (p2p multicast / frame-filter)
 *      are compiled out; only the AIS frame-filter kick is kept.
 *    - KAL_WAKE_LOCK/UNLOCK -> no-op (mainline has no vendor wakelock here).
 *    - CFG_DBG_GPIO_PINS debug toggles dropped.
 *    - kthread_run/kthread_stop lifecycle wrapped in glStartTxThread/
 *      glStopTxThread helpers used by gl_init (Fase 1).
 *    - netdev_priv() on 3.10 stored 'P_GLUE_INFO_T*' (double deref); this port
 *      keeps the stock idiom so the same gl_init that fills netdev_priv links.
 *
 *  The stock function prototypes (kalSetEvent / kalEnqueueCommand /
 *  kalAcquireSpinLock / kalReleaseSpinLock / kalFlushPendingTxPackets /
 *  kalGetTxPending{Frame,Cmd}Count / kalClear{Security,Mgmt}Frames* /
 *  kalSecurityFrameSendComplete / tx_thread) MUST NOT change, or the stock
 *  callers in mgmt/ nic/ common/ will not link. Bodies only are re-implemented.
 */

#include "precomp.h"		/* GLUE_INFO_T (rTxQueue/rCmdQueue/u4Flag/waitq/
				 * OidEntry/rPendComp/rHaltComp/rSpinLock/
				 * i4TxPending*), ADAPTER_T, QUE_T, CMD_INFO_T,
				 * wlanIST/wlanSetInformation/wlanQueryInformation/
				 * wlanProcessMboxMessage/wlanProcessCommandQueue/
				 * wlanEnqueueTxPacket/wlanGetTxPendingFrameCount/
				 * wlanTxPendingPackets/wlanTimerTimeoutCheck/
				 * wlanAcquirePowerControl/wlanReleasePowerControl/
				 * wlanoidTimeoutCheck/wlanReleasePendingOid/
				 * wlanReleaseCommand/cmdBufFreeCmdInfo, all
				 * GLUE_* / QUEUE_* macros, GLUE_FLAG_*_BIT */

#include "gl_kal_prototypes.h"	/* our single KAL shim contract */

#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>

/*
 * Stock global ISR pass counter. Declared 'extern UINT_32 TaskIsrCnt;' inside
 * the stock tx_thread and *defined* by the HIF layer. Provide a weak fallback
 * definition here so the KAL shim links stand-alone in Fase 1 even before the
 * HIF object that owns it is in the link set; the real (strong) definition in
 * the HIF/glue wins at Fase 3-4.
 */
UINT_32 TaskIsrCnt __attribute__((weak)) = 0;

/*==============================================================================
 *  kalSetEvent  -  GLUE_SET_EVENT backend: raise TXREQ + wake the single thread.
 *
 *  Called by wlanSendSetQueryCmd / the cfg80211 ops after kalEnqueueCommand, and
 *  by any core path that has queued work for the thread. Must stay a plain
 *  set_bit + wake (no lock) because it is invoked from process and softirq ctx.
 *============================================================================*/
VOID
kalSetEvent(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	ASSERT(prGlueInfo);

	set_bit(GLUE_FLAG_TXREQ_BIT, &prGlueInfo->u4Flag);
	wake_up_interruptible(&prGlueInfo->waitq);
}

/*==============================================================================
 *  kalEnqueueCommand  -  push one command entry onto rCmdQueue.
 *
 *  This is THE entry point of the command stream: wlanSendSetQueryCmd() of the
 *  ENTIRE core (mgmt scan/connect/key/bss-info, nic power, cnm...) funnels every
 *  CMD_INFO_T through here. The tx_thread later drains rCmdQueue in FIFO order
 *  under GLUE_FLAG_TXREQ, guaranteeing a single in-flight command to the FW.
 *
 *  NOTE: the caller (wlanSendSetQueryCmd) is responsible for the subsequent
 *  kalSetEvent()/GLUE_SET_EVENT() to wake the thread; stock does exactly that,
 *  so we do NOT wake here (keeps enqueue cheap and matches stock semantics).
 *============================================================================*/
VOID
kalEnqueueCommand(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN P_QUE_ENTRY_T	prQueueEntry
	)
{
	P_QUE_T		prCmdQue;
	P_CMD_INFO_T	prCmdInfo;
	P_MSDU_INFO_T	prMsduInfo;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);
	ASSERT(prQueueEntry);

	prCmdQue = &prGlueInfo->rCmdQueue;

	/* stamp management-frame commands with cmd type / CID / inqueue time so the
	 * TX path and the mgmt-tx-status indication can correlate them. */
	prCmdInfo = (P_CMD_INFO_T)prQueueEntry;
	if (prCmdInfo->prPacket &&
	    prCmdInfo->eCmdType == COMMAND_TYPE_MANAGEMENT_FRAME) {
		prMsduInfo = (P_MSDU_INFO_T)(prCmdInfo->prPacket);
		prMsduInfo->eCmdType = prCmdInfo->eCmdType;
		prMsduInfo->ucCID = prCmdInfo->ucCID;
		prMsduInfo->u4InqueTime = kalGetTimeTick();
	}

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

/*==============================================================================
 *  kalAcquireSpinLock / kalReleaseSpinLock  -  core-facing spinlock wrappers.
 *
 *  Same category array (prGlueInfo->rSpinLock[]) the GLUE_ACQUIRE/RELEASE macros
 *  use; the core calls these when it holds the saved-flags itself (e.g. rx/tx
 *  descriptor lists). Honour CFG_USE_SPIN_LOCK_BOTTOM_HALF like stock.
 *============================================================================*/
VOID
kalAcquireSpinLock(
	IN  P_GLUE_INFO_T		prGlueInfo,
	IN  ENUM_SPIN_LOCK_CATEGORY_E	rLockCategory,
	OUT PUINT_32			pu4Flags
	)
{
	UINT_32 u4Flags = 0;

	ASSERT(prGlueInfo);
	ASSERT(pu4Flags);

	if (rLockCategory < SPIN_LOCK_NUM) {
#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
		spin_lock_bh(&prGlueInfo->rSpinLock[rLockCategory]);
#else
		spin_lock_irqsave(&prGlueInfo->rSpinLock[rLockCategory], u4Flags);
#endif
		*pu4Flags = u4Flags;
	}
}

VOID
kalReleaseSpinLock(
	IN P_GLUE_INFO_T		prGlueInfo,
	IN ENUM_SPIN_LOCK_CATEGORY_E	rLockCategory,
	IN UINT_32			u4Flags
	)
{
	ASSERT(prGlueInfo);

	if (rLockCategory < SPIN_LOCK_NUM) {
#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
		spin_unlock_bh(&prGlueInfo->rSpinLock[rLockCategory]);
#else
		spin_unlock_irqrestore(&prGlueInfo->rSpinLock[rLockCategory],
				       u4Flags);
#endif
	}
}

/*==============================================================================
 *  Pending-count helpers consumed by the core / teardown.
 *============================================================================*/
UINT_32
kalGetTxPendingFrameCount(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	ASSERT(prGlueInfo);

	return (UINT_32)(prGlueInfo->i4TxPendingFrameNum);
}

UINT_32
kalGetTxPendingCmdCount(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	P_QUE_T prCmdQue;

	ASSERT(prGlueInfo);
	prCmdQue = &prGlueInfo->rCmdQueue;

	return prCmdQue->u4NumElem;
}

/*==============================================================================
 *  kalFlushPendingTxPackets  -  drain rTxQueue on halt, completing each skb.
 *============================================================================*/
VOID
kalFlushPendingTxPackets(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	P_QUE_T		prTxQue;
	P_QUE_ENTRY_T	prQueueEntry;
	PVOID		prPacket;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prTxQue = &prGlueInfo->rTxQueue;

	if (prGlueInfo->i4TxPendingFrameNum) {
		while (TRUE) {
			GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);
			QUEUE_REMOVE_HEAD(prTxQue, prQueueEntry, P_QUE_ENTRY_T);
			GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);

			if (prQueueEntry == NULL)
				break;

			prPacket = GLUE_GET_PKT_DESCRIPTOR(prQueueEntry);

			kalSendComplete(prGlueInfo, prPacket,
					WLAN_STATUS_NOT_ACCEPTED);
		}
	}
}

/*==============================================================================
 *  Security-frame lifecycle (802.1X EAPOL queued into rCmdQueue).
 *============================================================================*/
VOID
kalClearSecurityFrames(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	P_QUE_T		prCmdQue;
	QUE_T		rTempCmdQue;
	P_QUE_T		prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T	prQueueEntry = (P_QUE_ENTRY_T)NULL;
	P_CMD_INFO_T	prCmdInfo = (P_CMD_INFO_T)NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T)prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_SECURITY_FRAME) {
			prCmdInfo->pfCmdTimeoutHandler(prGlueInfo->prAdapter,
						       prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

VOID
kalClearSecurityFramesByNetType(
	IN P_GLUE_INFO_T		prGlueInfo,
	IN ENUM_NETWORK_TYPE_INDEX_T	eNetworkTypeIdx
	)
{
	P_QUE_T		prCmdQue;
	QUE_T		rTempCmdQue;
	P_QUE_T		prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T	prQueueEntry = (P_QUE_ENTRY_T)NULL;
	P_CMD_INFO_T	prCmdInfo = (P_CMD_INFO_T)NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T)prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_SECURITY_FRAME &&
		    prCmdInfo->eNetworkType == eNetworkTypeIdx) {
			prCmdInfo->pfCmdTimeoutHandler(prGlueInfo->prAdapter,
						       prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}


/*==============================================================================
 *  Management-frame lifecycle (mgmt-tx queued into rCmdQueue).
 *============================================================================*/
VOID
kalClearMgmtFrames(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	P_QUE_T		prCmdQue;
	QUE_T		rTempCmdQue;
	P_QUE_T		prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T	prQueueEntry = (P_QUE_ENTRY_T)NULL;
	P_CMD_INFO_T	prCmdInfo = (P_CMD_INFO_T)NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T)prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_MANAGEMENT_FRAME) {
			wlanReleaseCommand(prGlueInfo->prAdapter, prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

VOID
kalClearMgmtFramesByNetType(
	IN P_GLUE_INFO_T		prGlueInfo,
	IN ENUM_NETWORK_TYPE_INDEX_T	eNetworkTypeIdx
	)
{
	P_QUE_T		prCmdQue;
	QUE_T		rTempCmdQue;
	P_QUE_T		prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T	prQueueEntry = (P_QUE_ENTRY_T)NULL;
	P_CMD_INFO_T	prCmdInfo = (P_CMD_INFO_T)NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T)prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_MANAGEMENT_FRAME &&
		    prCmdInfo->eNetworkType == eNetworkTypeIdx) {
			wlanReleaseCommand(prGlueInfo->prAdapter, prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

/*==============================================================================
 *  Misc helpers referenced from the HIF/ISR path.
 *============================================================================*/

/* Vendor wakelock timeout: no-op on mainline (no MTK STP wakelock here). Kept
 * as a real symbol because the HIF ISR/glue path references it. */
VOID
kalHifAhbKalWakeLockTimeout(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	(void)prGlueInfo;
}

/* Monotonic boot time in ns; used by a couple of debug counters. */
UINT_64 kalGetBootTime(void)
{
	return (UINT_64)ktime_get_boottime_ns();
}

/*==============================================================================
 *                              tx_thread
 *
 *  The single glue kthread. Its while-loop IS the serialization point of the
 *  whole driver: it sleeps on prGlueInfo->waitq until any GLUE_FLAG bit is set,
 *  then services them in the FIXED order INT > OID > TXREQ > TIMEOUT, checking
 *  HALT at each safe point. wlanAcquirePowerControl()/wlanReleasePowerControl()
 *  bracket every pass that touches HW so the FW-own/driver-own handshake (done
 *  in our HIF backend) is entered exactly once per wake.
 *
 *  'data' is prGlueInfo->prDevHandler (the net_device*), matching the stock
 *  kthread_run(tx_thread, prDevHandler, ...) call; netdev_priv stores a
 *  'P_GLUE_INFO_T*' (stock idiom -> double deref).
 *============================================================================*/
int tx_thread(void *data)
{
	struct net_device	*dev = data;
	P_GLUE_INFO_T		prGlueInfo = *((P_GLUE_INFO_T *)netdev_priv(dev));

	P_QUE_ENTRY_T		prQueueEntry = NULL;
	P_QUE_T			prTxQueue = NULL;
	P_QUE_T			prCmdQue = NULL;

	int			ret = 0;
	BOOLEAN			fgNeedHwAccess = FALSE;
	struct sk_buff		*prSkb = NULL;

	/* for GLUE_ACQUIRE/RELEASE_SPIN_LOCK saved-flags */
	GLUE_SPIN_LOCK_DECLARATION();

	prTxQueue = &prGlueInfo->rTxQueue;
	prCmdQue  = &prGlueInfo->rCmdQueue;

	/* never let the freezer touch us: the FW-own handshake must not be
	 * interrupted mid-transfer. */
	current->flags |= PF_NOFREEZE;

	DBGLOG(INIT, INFO, ("tx_thread starts running...\n"));

	while (TRUE) {

		/* AIS mgmt frame-filter kick (STA-only; P2P variants dropped). */
		if (test_and_clear_bit(GLUE_FLAG_FRAME_FILTER_AIS_BIT,
				       &prGlueInfo->u4Flag)) {
			P_AIS_FSM_INFO_T prAisFsmInfo =
				&prGlueInfo->prAdapter->rWifiVar.rAisFsmInfo;
			prAisFsmInfo->u4AisPacketFilter =
				prGlueInfo->u4OsMgmtFrameFilter;
		}

		if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
			DBGLOG(INIT, INFO, ("tx_thread should stop now...\n"));
			break;
		}

		/*
		 * Sleep until an event occurs. u4Flag != 0 means one of
		 * GLUE_FLAG_INT / _OID / _TXREQ / _TIMEOUT / _HALT is pending.
		 * wait_event_interruptible can also return on a signal (ret<0);
		 * we simply re-loop and re-evaluate u4Flag, so a stray wake is
		 * harmless.
		 */
		ret = wait_event_interruptible(prGlueInfo->waitq,
					       (prGlueInfo->u4Flag != 0));
		if (ret < 0) {
			/* interrupted by signal: fall through and re-check flags
			 * (HALT is handled at the top / below). */
		}

		/* re-check the AIS filter after wake (stock does it twice). */
		if (test_and_clear_bit(GLUE_FLAG_FRAME_FILTER_AIS_BIT,
				       &prGlueInfo->u4Flag)) {
			P_AIS_FSM_INFO_T prAisFsmInfo =
				&prGlueInfo->prAdapter->rWifiVar.rAisFsmInfo;
			prAisFsmInfo->u4AisPacketFilter =
				prGlueInfo->u4OsMgmtFrameFilter;
		}

		if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
			DBGLOG(INIT, INFO, ("<1>tx_thread should stop now...\n"));
			break;
		}

		fgNeedHwAccess = FALSE;

		/* -------- (1) INT: run the interrupt service tasklet -------- */
		if (test_and_clear_bit(GLUE_FLAG_INT_BIT, &prGlueInfo->u4Flag)) {
			if (fgNeedHwAccess == FALSE) {
				fgNeedHwAccess = TRUE;
				wlanAcquirePowerControl(prGlueInfo->prAdapter);
			}

			/* the HIF INT is already masked in our ISR; the core
			 * re-enables it inside wlanIST when it re-arms WHLPCR. */
			prGlueInfo->prAdapter->fgIsIntEnable = FALSE;

			if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
				DBGLOG(INIT, INFO, ("ignore pending interrupt\n"));
			} else {
				TaskIsrCnt++;
				wlanIST(prGlueInfo->prAdapter);
			}
		}

		/* -------- (2) OID: service one pending set/query OID -------- */
		do {
			if (test_and_clear_bit(GLUE_FLAG_OID_BIT,
					       &prGlueInfo->u4Flag)) {
				P_GL_IO_REQ_T prIoReq;

				prGlueInfo->u4OidCompleteFlag = 0;
				prIoReq = &prGlueInfo->OidEntry;

				if (FALSE == prIoReq->fgRead) {
					prIoReq->rStatus = wlanSetInformation(
						prIoReq->prAdapter,
						prIoReq->pfnOidHandler,
						prIoReq->pvInfoBuf,
						prIoReq->u4InfoBufLen,
						prIoReq->pu4QryInfoLen);
				} else {
					prIoReq->rStatus = wlanQueryInformation(
						prIoReq->prAdapter,
						prIoReq->pfnOidHandler,
						prIoReq->pvInfoBuf,
						prIoReq->u4InfoBufLen,
						prIoReq->pu4QryInfoLen);
				}

				if (prIoReq->rStatus != WLAN_STATUS_PENDING) {
					/* completes the kalIoctl waiter
					 * (wait_for_completion_timeout). */
					complete(&prGlueInfo->rPendComp);
				} else {
					wlanoidTimeoutCheck(prGlueInfo->prAdapter,
							    prIoReq->pfnOidHandler);
				}
			}
		} while (FALSE);

		/* -------- (3) TXREQ: mailbox + command stream + packet TX ---- */
		if (test_and_clear_bit(GLUE_FLAG_TXREQ_BIT,
				       &prGlueInfo->u4Flag)) {
			/* drain inter-module mailbox messages first. */
			wlanProcessMboxMessage(prGlueInfo->prAdapter);

			/* THE command stream: drain rCmdQueue in FIFO order.
			 * kalEnqueueCommand feeds this; a single command is in
			 * flight to the FW at a time. */
			if (prCmdQue->u4NumElem > 0) {
				if (fgNeedHwAccess == FALSE) {
					fgNeedHwAccess = TRUE;
					wlanAcquirePowerControl(prGlueInfo->prAdapter);
				}
				wlanProcessCommandQueue(prGlueInfo->prAdapter,
							prCmdQue);
			}

			/* hand queued data skbs to the core's MSDU pipeline. */
			while (QUEUE_IS_NOT_EMPTY(prTxQueue)) {
				GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);
				QUEUE_REMOVE_HEAD(prTxQueue, prQueueEntry,
						  P_QUE_ENTRY_T);
				GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);

				ASSERT(prQueueEntry);
				if (NULL == prQueueEntry)
					break;

				prSkb = (struct sk_buff *)
					GLUE_GET_PKT_DESCRIPTOR(prQueueEntry);
				ASSERT(prSkb);
				if (NULL == prSkb) {
					DBGLOG(INIT, WARN, ("prSkb == NULL in tx\n"));
					continue;
				}

				if (wlanEnqueueTxPacket(prGlueInfo->prAdapter,
							(P_NATIVE_PACKET)prSkb)
				    == WLAN_STATUS_RESOURCES) {
					/* no free MSDU_INFO: put it back at the
					 * head and stop for this pass. */
					GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);
					QUEUE_INSERT_HEAD(prTxQueue, prQueueEntry);
					GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);
					break;
				}
			}

			if (wlanGetTxPendingFrameCount(prGlueInfo->prAdapter) > 0) {
				/* push the accumulated MSDUs down to the HIF. */
				wlanTxPendingPackets(prGlueInfo->prAdapter,
						     &fgNeedHwAccess);
			}
		}

		/* release power control once per pass if we grabbed it. */
		if (fgNeedHwAccess == TRUE)
			wlanReleasePowerControl(prGlueInfo->prAdapter);

		/* -------- (4) TIMEOUT: cnmTimer expiry check ---------------- */
		if (test_and_clear_bit(GLUE_FLAG_TIMEOUT_BIT,
				       &prGlueInfo->u4Flag)) {
			wlanTimerTimeoutCheck(prGlueInfo->prAdapter);
		}

		/* loop back; HALT is re-checked at the top of the loop. */
	}

	/* -------- (5) HALT teardown: flush everything, then signal ------- */

	/* flush the pending data-plane TX. */
	if (prGlueInfo->i4TxPendingFrameNum > 0)
		kalFlushPendingTxPackets(prGlueInfo);

	/* flush pending security frames (802.1X). */
	if (prGlueInfo->i4TxPendingSecurityFrameNum > 0)
		kalClearSecurityFrames(prGlueInfo);

	/* fail back any OID still pending in the core. */
	wlanReleasePendingOid(prGlueInfo->prAdapter, 0);

	DBGLOG(INIT, INFO, ("tx_thread stops\n"));

	/* wake glStopTxThread()/gl_init's wait_for_completion(rHaltComp). */
	complete(&prGlueInfo->rHaltComp);

	return 0;
}

/*==============================================================================
 *  Thread lifecycle helpers (used by gl_init at wlanProbe / wlanRemove).
 *
 *  Stock gl_init did kthread_run(tx_thread, prDevHandler, "tx_thread") inline
 *  and, on remove, set GLUE_FLAG_HALT_BIT + wake + wait_for_completion(rHaltComp).
 *  We wrap both so gl_init just calls glStartTxThread/glStopTxThread.
 *============================================================================*/
BOOLEAN glStartTxThread(P_GLUE_INFO_T prGlueInfo)
{
	struct task_struct *task;

	ASSERT(prGlueInfo);

	/* rHaltComp must already be init_completion()'d by gl_init. */
	task = kthread_run(tx_thread, prGlueInfo->prDevHandler, "tx_thread");
	if (IS_ERR(task)) {
		DBGLOG(INIT, ERROR, ("kthread_run(tx_thread) failed: %ld\n",
				     PTR_ERR(task)));
		prGlueInfo->main_thread = NULL;
		return FALSE;
	}

	prGlueInfo->main_thread = task;
	return TRUE;
}

VOID glStopTxThread(P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	if (!prGlueInfo->main_thread)
		return;

	/* ask the thread to leave its loop and wake it. */
	set_bit(GLUE_FLAG_HALT_BIT, &prGlueInfo->u4Flag);
	wake_up_interruptible(&prGlueInfo->waitq);

	DBGLOG(INIT, INFO, ("waiting for tx_thread stop...\n"));

	/* the thread signals rHaltComp right before returning; block for it so
	 * the caller can safely tear down rCmdQueue / rTxQueue afterwards. */
	wait_for_completion(&prGlueInfo->rHaltComp);

	prGlueInfo->main_thread = NULL;
}
