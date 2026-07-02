/*! \file   gl_kal_prototypes.h
 *  \brief  KAL shim contract for the linux-7.0.12 (postmarketOS / BQ krillin)
 *          port of the MediaTek MT6582 'mt_wifi' full-MAC driver.
 *
 *  This header is the single contract shared by the six KAL shim translation
 *  units that replace the Android-3.10-era gl_kal.c:
 *
 *      gl_kal_mem.c       - memory / buffer / packet helpers
 *      gl_kal_thread.c    - tx_thread, spinlocks, event, timer tick
 *      gl_kal_ioctl.c     - kalIoctl / OID clearance / command enqueue
 *      gl_kal_timer.c     - kalOsTimerInitialize / kalSetTimer / kalCancelTimer
 *      gl_kal_indicate.c  - cfg80211 status/scan/bss/mgmt/media indications
 *      gl_kal_fw.c        - firmware image mapping + NVRAM/cfg + fw addresses
 *      gl_kal_dev.c       - kalDev* HIF PIO backend glue + card-removal + misc
 *
 *  Only the prototypes that the *stock core* (mgmt/ nic/ common/) actually
 *  references are declared here (see the grep of kal* call sites). Symbols the
 *  core reaches through macros in gl_kal.h (kalMemAlloc/Free/Copy/Set/Zero/Cmp,
 *  kalSendComplete, kalUdelay/Mdelay/Msleep, kalGetTimeTick, kalPrint,
 *  kalCopyFrame, kalDevSetPowerState) are NOT re-declared here - they stay
 *  macros. P2P (kalP2P...) and BOW (kalGetBow, kalSetBow, kalXxxBowDevice,
 *  kalIndicateBOWEvent) symbols are omitted because CFG_ENABLE_WIFI_DIRECT and
 *  CFG_ENABLE_BT_OVER_WIFI are 0 in this STA-only port.
 *
 *  All signatures are the stock 2.3 signatures (IN/OUT are the no-op markers
 *  from gl_typedef.h); the *bodies* are re-implemented for 7.0.12 per the API
 *  deltas (timer_setup / cfg80211_* struct-arg forms / request_firmware / etc.)
 *  but the prototypes seen by the core MUST NOT change, or the stock callers
 *  will not link.
 */

#ifndef _GL_KAL_PROTOTYPES_H
#define _GL_KAL_PROTOTYPES_H

#include "gl_typedef.h"
#include "gl_os.h"
#include "gl_kal.h"

/*==============================================================================
 *  gl_kal_mem.c  -  memory / buffer / packet / assoc-info / classifier
 *============================================================================*/

VOID
kalUpdateMACAddress(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PUINT_8          pucMacAddr
    );

VOID
kalPacketFree(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            pvPacket
    );

PVOID
kalPacketAlloc(
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  UINT_32         u4Size,
    OUT PUINT_8         *ppucData
    );

WLAN_STATUS
kalProcessRxPacket(
    IN P_GLUE_INFO_T        prGlueInfo,
    IN PVOID                pvPacket,
    IN PUINT_8              pucPacketStart,
    IN UINT_32              u4PacketLen,
    IN BOOLEAN              fgIsRetain,
    IN ENUM_CSUM_RESULT_T   aeCSUM[]
    );

WLAN_STATUS
kalRxIndicatePkts(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            apvPkts[],
    IN UINT_8           ucPktNum
    );

VOID
kalSendCompleteAndAwakeQueue(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            pvPacket
    );

#if CFG_TCP_IP_CHKSUM_OFFLOAD
VOID
kalQueryTxChksumOffloadParam(
    IN  PVOID           pvPacket,
    OUT PUINT_8         pucFlag
    );

VOID
kalUpdateRxCSUMOffloadParam(
    IN PVOID                pvPacket,
    IN ENUM_CSUM_RESULT_T   eCSUM[]
    );
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

#if CFG_TX_FRAGMENT
BOOLEAN
kalQueryTxPacketHeader(
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  PVOID           pvPacket,
    OUT PUINT_16        pu2EtherTypeLen,
    OUT PUINT_8         pucEthDestAddr
    );
#endif /* CFG_TX_FRAGMENT */

BOOL
kalQoSFrameClassifierAndPacketInfo(
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  P_NATIVE_PACKET prPacket,
    OUT PUINT_8         pucPriorityParam,
    OUT PUINT_32        pu4PacketLen,
    OUT PUINT_8         pucEthDestAddr,
    OUT PBOOLEAN        pfgIs1X,
    OUT PBOOLEAN        pfgIsPAL,
    OUT PUINT_8         pucNetworkType
    );

VOID
kalHandleAssocInfo(
    IN P_GLUE_INFO_T        prGlueInfo,
    IN P_EVENT_ASSOC_INFO   prAssocInfo
    );

VOID
kalUpdateReAssocReqInfo(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PUINT_8          pucFrameBody,
    IN UINT_32          u4FrameBodyLen,
    IN BOOLEAN          fgReassocRequest
    );

VOID
kalUpdateReAssocRspInfo(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PUINT_8          pucFrameBody,
    IN UINT_32          u4FrameBodyLen
    );

BOOLEAN
kalRetrieveNetworkAddress(
    IN     P_GLUE_INFO_T        prGlueInfo,
    IN OUT PARAM_MAC_ADDRESS    *prMacAddr
    );

UINT_32
kalRandomNumber(
    VOID
    );

/* I/O buffer pre-allocation (physically-continuous coalescing buffer). */
BOOLEAN kalInitIOBuffer(VOID);
VOID    kalUninitIOBuffer(VOID);
PVOID   kalAllocateIOBuffer(IN UINT_32 u4AllocSize);
VOID    kalReleaseIOBuffer(IN PVOID pvAddr, IN UINT_32 u4Size);

/*==============================================================================
 *  gl_kal_thread.c  -  tx_thread, spinlocks, event kick, flush, counters
 *============================================================================*/

int tx_thread(void *data);

VOID
kalSetEvent(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalAcquireSpinLock(
    IN  P_GLUE_INFO_T               prGlueInfo,
    IN  ENUM_SPIN_LOCK_CATEGORY_E   rLockCategory,
    OUT PUINT_32                    pu4Flags
    );

VOID
kalReleaseSpinLock(
    IN P_GLUE_INFO_T                prGlueInfo,
    IN ENUM_SPIN_LOCK_CATEGORY_E    rLockCategory,
    IN UINT_32                      u4Flags
    );

VOID
kalFlushPendingTxPackets(
    IN P_GLUE_INFO_T    prGlueInfo
    );

UINT_32
kalGetTxPendingFrameCount(
    IN P_GLUE_INFO_T    prGlueInfo
    );

UINT_32
kalGetTxPendingCmdCount(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalClearSecurityFrames(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalClearSecurityFramesByNetType(
    IN P_GLUE_INFO_T                prGlueInfo,
    IN ENUM_NETWORK_TYPE_INDEX_T    eNetworkTypeIdx
    );

VOID
kalSecurityFrameSendComplete(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            pvPacket,
    IN WLAN_STATUS      rStatus
    );

VOID
kalClearMgmtFrames(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalClearMgmtFramesByNetType(
    IN P_GLUE_INFO_T                prGlueInfo,
    IN ENUM_NETWORK_TYPE_INDEX_T    eNetworkTypeIdx
    );

/* Wake-lock timeout helper referenced by the HIF ISR path (no-op on mainline). */
VOID
kalHifAhbKalWakeLockTimeout(
    IN P_GLUE_INFO_T    prGlueInfo
    );

UINT_64 kalGetBootTime(void);

/*==============================================================================
 *  gl_kal_timer.c  -  the single glue tick timer (timer_setup based)
 *============================================================================*/

VOID
kalOsTimerInitialize(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            prTimerHandler
    );

/* NOTE: matches the (prGlueInfo, u4Interval) form declared twice in gl_kal.h;
 * returns BOOLEAN. rInterval is in ms (MSEC_TO_SYSTIME). */
BOOLEAN
kalSetTimer(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_32          u4Interval
    );

BOOLEAN
kalCancelTimer(
    IN P_GLUE_INFO_T    prGlueInfo
    );

/* Legacy linux-timer callback trampoline. On 7.0.12 the timer core passes a
 * struct timer_list*, so the .c casts via timer_container_of(); the prototype
 * stays 'unsigned long arg' to keep stock callers/decls consistent. */
VOID
kalTimeoutHandler(unsigned long arg);

/*==============================================================================
 *  gl_kal_ioctl.c  -  OID pump (kalIoctl) with wiphy-lock-safe timeout
 *============================================================================*/

WLAN_STATUS
kalIoctl(
    IN  P_GLUE_INFO_T           prGlueInfo,
    IN  PFN_OID_HANDLER_FUNC    pfnOidHandler,
    IN  PVOID                   pvInfoBuf,
    IN  UINT_32                 u4InfoBufLen,
    IN  BOOL                    fgRead,
    IN  BOOL                    fgWaitResp,
    IN  BOOL                    fgCmd,
    IN  BOOL                    fgIsP2pOid,
    OUT PUINT_32                pu4QryInfoLen
    );

VOID
kalOidComplete(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN BOOLEAN          fgSetQuery,
    IN UINT_32          u4SetQueryInfoLen,
    IN WLAN_STATUS      rOidStatus
    );

VOID
kalOidCmdClearance(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalOidClearance(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalEnqueueCommand(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN P_QUE_ENTRY_T    prQueueEntry
    );

/*==============================================================================
 *  gl_kal_indicate.c  -  cfg80211 up-calls (media / scan / bss / mgmt / rssi)
 *============================================================================*/

VOID
kalIndicateStatusAndComplete(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN WLAN_STATUS      eStatus,
    IN PVOID            pvBuf,
    IN UINT_32          u4BufLen
    );

ENUM_PARAM_MEDIA_STATE_T
kalGetMediaStateIndicated(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalSetMediaStateIndicated(
    IN P_GLUE_INFO_T            prGlueInfo,
    IN ENUM_PARAM_MEDIA_STATE_T eParamMediaStateIndicate
    );

VOID
kalScanDone(
    IN P_GLUE_INFO_T                    prGlueInfo,
    IN ENUM_KAL_NETWORK_TYPE_INDEX_T    eNetTypeIdx,
    IN WLAN_STATUS                      status
    );

VOID
kalIndicateBssInfo(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PUINT_8          pucFrameBuf,
    IN UINT_32          u4BufLen,
    IN UINT_8           ucChannelNum,
    IN INT_32           i4SignalStrength
    );

VOID
kalIndicateMgmtTxStatus(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_64          u8Cookie,
    IN BOOLEAN          fgIsAck,
    IN PUINT_8          pucFrameBuf,
    IN UINT_32          u4FrameLen
    );

VOID
kalIndicateRxMgmtFrame(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN P_SW_RFB_T       prSwRfb
    );

VOID
kalReadyOnChannel(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_64          u8Cookie,
    IN ENUM_BAND_T      eBand,
    IN ENUM_CHNL_EXT_T  eSco,
    IN UINT_8           ucChannelNum,
    IN UINT_32          u4DurationMs
    );

VOID
kalRemainOnChannelExpired(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_64          u8Cookie,
    IN ENUM_BAND_T      eBand,
    IN ENUM_CHNL_EXT_T  eSco,
    IN UINT_8           ucChannelNum
    );

VOID
kalUpdateRSSI(
    IN P_GLUE_INFO_T                    prGlueInfo,
    IN ENUM_KAL_NETWORK_TYPE_INDEX_T    eNetTypeIdx,
    IN INT_8                            cRssi,
    IN INT_8                            cLinkQuality
    );

VOID
kalGetChannelList(
    IN P_GLUE_INFO_T        prGlueInfo,
    IN ENUM_BAND_T          eSpecificBand,
    IN UINT_8               ucMaxChannelNum,
    IN PUINT_8              pucNumOfChannel,
    IN P_RF_CHANNEL_INFO_T  paucChannelList
    );

BOOL
kalIsAPmode(
    IN P_GLUE_INFO_T    prGlueInfo
    );

BOOLEAN
kalWSCGetActiveState(
    IN P_GLUE_INFO_T    prGlueInfo
    );

#if CFG_SUPPORT_802_11W
UINT_32
kalGetMfpSetting(
    IN P_GLUE_INFO_T    prGlueInfo
    );
#endif

/*==============================================================================
 *  gl_kal_fw.c  -  firmware image (request_firmware) + NVRAM/cfg + fw addrs
 *============================================================================*/

#if CFG_ENABLE_FW_DOWNLOAD
PVOID
kalFirmwareImageMapping(
    IN  P_GLUE_INFO_T   prGlueInfo,
    OUT PPVOID          ppvMapFileBuf,
    OUT PUINT_32        pu4FileLength
    );

VOID
kalFirmwareImageUnmapping(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            prFwHandle,
    IN PVOID            pvMapFileBuf
    );
#endif /* CFG_ENABLE_FW_DOWNLOAD */

UINT_32
kalGetFwStartAddress(
    IN P_GLUE_INFO_T    prGlueInfo
    );

UINT_32
kalGetFwLoadAddress(
    IN P_GLUE_INFO_T    prGlueInfo
    );

BOOLEAN
kalIsConfigurationExist(
    IN P_GLUE_INFO_T    prGlueInfo
    );

P_REG_INFO_T
kalGetConfiguration(
    IN P_GLUE_INFO_T    prGlueInfo
    );

VOID
kalGetConfigurationVersion(
    IN  P_GLUE_INFO_T   prGlueInfo,
    OUT PUINT_16        pu2Part1CfgOwnVersion,
    OUT PUINT_16        pu2Part1CfgPeerVersion,
    OUT PUINT_16        pu2Part2CfgOwnVersion,
    OUT PUINT_16        pu2Part2CfgPeerVersion
    );

BOOLEAN
kalCfgDataRead16(
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  UINT_32         u4Offset,
    OUT PUINT_16        pu2Data
    );

BOOLEAN
kalCfgDataWrite16(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_32          u4Offset,
    IN UINT_16          u2Data
    );

/* kernel_read()-based file writer used by a couple of debug OIDs. */
UINT_32
kalWriteToFile(
    IN const PUINT_8    pucPath,
    IN BOOLEAN          fgDoAppend,
    IN PUINT_8          pucData,
    IN UINT_32          u4Size
    );

/*==============================================================================
 *  gl_kal_dev.c  -  HIF PIO backend (our functional mt6582 HIF) + misc dev
 *============================================================================*/

BOOL
kalDevRegRead(
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  UINT_32         u4Register,
    OUT PUINT_32        pu4Value
    );

BOOL
kalDevRegWrite(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_32          u4Register,
    IN UINT_32          u4Value
    );

BOOL
kalDevPortRead(
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  UINT_16         u2Port,
    IN  UINT_16         u2Len,
    OUT PUINT_8         pucBuf,
    IN  UINT_16         u2ValidOutBufSize
    );

BOOL
kalDevPortWrite(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_16          u2Port,
    IN UINT_16          u2Len,
    IN PUINT_8          pucBuf,
    IN UINT_16          u2ValidInBufSize
    );

BOOL
kalDevWriteWithSdioCmd52(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_32          u4Addr,
    IN UINT_8           ucData
    );

VOID
kalDevLoopbkAuto(
    IN GLUE_INFO_T      *GlueInfo
    );

/* HIF TX-loopback auto self-test RX handler. Only referenced by nic_rx.c when
 * CONF_HIF_LOOPBACK_AUTO==1 (debug); nic_rx.c also self-declares it inline. */
#if defined(CONF_HIF_LOOPBACK_AUTO) && (CONF_HIF_LOOPBACK_AUTO == 1)
void
kalDevLoopbkRxHandle(
    IN     P_ADAPTER_T  prAdapter,
    IN OUT P_SW_RFB_T   prSwRfb
    );
#endif

BOOLEAN
kalIsCardRemoved(
    IN P_GLUE_INFO_T    prGlueInfo
    );

UINT_32
kalIOPhyAddrGet(
    IN UINT_32          VirtAddr
    );

VOID
kalDmaBufGet(
    OUT VOID            **VirtAddr,
    OUT VOID            **PhyAddr
    );

#if CFG_SUPPORT_EXT_CONFIG
UINT_32
kalReadExtCfg(
    IN P_GLUE_INFO_T    prGlueInfo
    );
#endif

#endif /* _GL_KAL_PROTOTYPES_H */
