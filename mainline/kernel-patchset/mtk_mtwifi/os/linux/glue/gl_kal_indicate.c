// SPDX-License-Identifier: GPL-2.0
/*
 * os/linux/glue/gl_kal_indicate.c
 *
 * KAL shim (indicate) del port del MediaTek MT6582 'mt_wifi' full-MAC a
 * linux-7.0.12 (postmarketOS / BQ krillin). Reimplementa las up-calls a
 * cfg80211 del gl_kal.c stock (Android 3.10-era) usando la API MODERNA:
 *
 *   kalIndicateStatusAndComplete
 *       MEDIA_CONNECT / ROAM_OUT_FIND_BEST
 *           -> cfg80211_connect_bss()  (BSS ref retenida via cfg80211_get_bss)
 *           -> cfg80211_roamed(&struct cfg80211_roam_info)
 *       MEDIA_DISCONNECT
 *           -> cfg80211_disconnected(..., locally_generated, gfp)
 *       SCAN_COMPLETE
 *           -> cfg80211_scan_done(req, &struct cfg80211_scan_info)
 *       CONNECT_INDICATION (join fail)
 *           -> cfg80211_connect_result(..., status_code, gfp)
 *       MEDIA_SPECIFIC (MIC failure)
 *           -> cfg80211_michael_mic_failure()
 *   kalIndicateBssInfo   -> cfg80211_inform_bss_frame_data(&struct cfg80211_inform_bss)
 *   kalRxIndicatePkts    -> eth_type_trans() + netif_rx()  (netif_rx_ni eliminado)
 *   kalIndicateRxMgmtFrame -> cfg80211_rx_mgmt(wdev, freq, sig, buf, len, flags)
 *   kalIndicateMgmtTxStatus -> cfg80211_mgmt_tx_status(wdev, ...)
 *   kalGetMediaStateIndicated / kalSetMediaStateIndicated  (estado propio: el
 *       core ya guarda eParamMediaStateIndicated en GLUE_INFO_T; sustituye al
 *       wdev->sme_state eliminado en 7.0.12)
 *   kalUpdateReAssocReqInfo / kalUpdateReAssocRspInfo  (cachean los IE que luego
 *       viajan en cfg80211_connect_bss / cfg80211_roamed)
 *   kalScanDone  (drena las beacon/probe encoladas y dispara SCAN_COMPLETE)
 *
 * Notas de port (DELTAS 3.10 -> 7.0.12):
 *  - wdev->sme_state ELIMINADO: el "estado de conexion" para decidir
 *    connect_bss vs roamed lo trackeamos con prGlueInfo->eParamMediaStateIndicated
 *    (CONNECTED == ya asociado -> roamed; DISCONNECTED/CONNECTING -> connect_bss).
 *  - wext_indicate_wext_event() se elimina (WEXT muerto en este port cfg80211-only):
 *    las llamadas del stock se suprimen; netif_carrier_on/off se conservan.
 *  - IEEE80211_BAND_* -> NL80211_BAND_*.
 *  - cfg80211_inform_bss_frame() (5 args, signal directo) ->
 *    cfg80211_inform_bss_frame_data() (struct cfg80211_inform_bss con .signal).
 *  - cfg80211_rx_mgmt() ahora toma wdev + flags (0).
 *  - netif_rx_ni() eliminado: netif_rx() es seguro en cualquier contexto en 7.0.12.
 *  - Todas las up-calls que en el stock pasaban prDevHandler y ahora exigen wdev
 *    usan prGlueInfo->prDevHandler->ieee80211_ptr.
 */

#include "precomp.h"
#include "gl_kal_prototypes.h"

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>

/*
 * priv_to_wiphy() y prDevHandler->ieee80211_ptr vienen del core (gl_os.h /
 * gl_cfg80211). WLAN_CAPABILITY_ESS / IEEE80211_STYPE_* de <linux/ieee80211.h>.
 */

/* ---- diagnostico de scan (paridad con el stock, sin exportar) ---- */
static UINT_32 g_u4ScanInform;
static UINT_32 g_u4ScanInformFail;

/*----------------------------------------------------------------------------*/
/*! \brief  Getter del estado de media indicado. Sustituto de wdev->sme_state.   */
/*----------------------------------------------------------------------------*/
ENUM_PARAM_MEDIA_STATE_T
kalGetMediaStateIndicated(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	ASSERT(prGlueInfo);
	return prGlueInfo->eParamMediaStateIndicated;
}

/*----------------------------------------------------------------------------*/
/*! \brief  Setter del estado de media indicado.                                */
/*----------------------------------------------------------------------------*/
VOID
kalSetMediaStateIndicated(
	IN P_GLUE_INFO_T		prGlueInfo,
	IN ENUM_PARAM_MEDIA_STATE_T	eParamMediaStateIndicate
	)
{
	ASSERT(prGlueInfo);
	prGlueInfo->eParamMediaStateIndicated = eParamMediaStateIndicate;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Buscar el ieee80211_channel del BSS AIS actual (2.4/5 GHz).
 *         Devuelve NULL si el canal no esta habilitado en el wiphy.
 */
/*----------------------------------------------------------------------------*/
static struct ieee80211_channel *
kalGetAisChannel(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN UINT_8		ucChannelNum
	)
{
	struct wiphy *wiphy = priv_to_wiphy(prGlueInfo);
	enum nl80211_band eBand =
		(ucChannelNum <= 14) ? NL80211_BAND_2GHZ : NL80211_BAND_5GHZ;

	return ieee80211_get_channel(wiphy,
		ieee80211_channel_to_frequency(ucChannelNum, eBand));
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Indicate un evento de estado a cfg80211 (asoc / disasoc / scan-done /
 *         join-fail / MIC failure). Reemplazo moderno del stock.
 *
 * \param[in] prGlueInfo   GLUE
 * \param[in] eStatus      WLAN_STATUS_*
 * \param[in] pvBuf        buffer opcional (P_PARAM_STATUS_INDICATION_T)
 * \param[in] u4BufLen     longitud del buffer
 */
/*----------------------------------------------------------------------------*/
VOID
kalIndicateStatusAndComplete(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN WLAN_STATUS		eStatus,
	IN PVOID		pvBuf,
	IN UINT_32		u4BufLen
	)
{
	UINT_32 bufLen = 0;
	P_PARAM_STATUS_INDICATION_T pStatus = (P_PARAM_STATUS_INDICATION_T) pvBuf;
	P_PARAM_AUTH_EVENT_T pAuth = (P_PARAM_AUTH_EVENT_T) pStatus;
	PARAM_MAC_ADDRESS arBssid;
	struct cfg80211_scan_request *prScanRequest = NULL;
	struct wiphy *wiphy;
	struct net_device *prDevHandler;
	struct ieee80211_channel *prChannel = NULL;
	struct cfg80211_bss *bss = NULL;
	PARAM_SSID_T ssid;
	UINT_8 ucChannelNum;
	P_BSS_DESC_T prBssDesc = NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	wiphy = priv_to_wiphy(prGlueInfo);
	prDevHandler = prGlueInfo->prDevHandler;
	kalMemZero(arBssid, MAC_ADDR_LEN);

	switch (eStatus) {
	case WLAN_STATUS_ROAM_OUT_FIND_BEST:
	case WLAN_STATUS_MEDIA_CONNECT:
	{
		BOOLEAN fgWasConnected =
			(prGlueInfo->eParamMediaStateIndicated ==
			 PARAM_MEDIA_STATE_CONNECTED);

		/* consultar BSSID y SSID del enlace actual */
		wlanQueryInformation(prGlueInfo->prAdapter,
			wlanoidQueryBssid, &arBssid[0], sizeof(arBssid), &bufLen);

		/* FASE 5 FIX (visto con wpa -dd): wlanoidQueryBssid devuelve 00:00:00:00:00:00 si rCurrBssId
		 * aun no esta poblado en el instante de la indicacion MEDIA_CONNECT -> caer al BSSID del target
		 * BSS desc (el AP real). Sin esto wpa deriva la PTK con A2=00:00:00 y manda el EAPOL-Key M2 al
		 * BSSID nulo -> el AP no lo recibe -> el 4-way nunca completa. */
		if ((arBssid[0] | arBssid[1] | arBssid[2] |
		     arBssid[3] | arBssid[4] | arBssid[5]) == 0) {
			prBssDesc = wlanGetTargetBssDescByNetwork(
				prGlueInfo->prAdapter, NETWORK_TYPE_AIS_INDEX);
			if (prBssDesc != NULL)
				COPY_MAC_ADDR(arBssid, prBssDesc->aucBSSID);
		}

		kalMemZero(&ssid, sizeof(ssid));
		wlanQueryInformation(prGlueInfo->prAdapter,
			wlanoidQuerySsid, &ssid, sizeof(ssid), &bufLen);
		if (ssid.u4SsidLen > PARAM_MAX_LEN_SSID)
			ssid.u4SsidLen = PARAM_MAX_LEN_SSID;

		prGlueInfo->eParamMediaStateIndicated =
			PARAM_MEDIA_STATE_CONNECTED;

		/* encender netif */
		netif_carrier_on(prDevHandler);
		DBGLOG(INIT, INFO,
			("[wifi] %s netif_carrier_on [ssid_len:%u " MACSTR "]\n",
			 prDevHandler->name, (UINT32)ssid.u4SsidLen,
			 MAC2STR(arBssid)));

		if (prGlueInfo->fgIsRegistered != TRUE)
			break;

		/* localizar canal + BSS */
		ucChannelNum = wlanGetChannelNumberByNetwork(
			prGlueInfo->prAdapter, NETWORK_TYPE_AIS_INDEX);
		prChannel = kalGetAisChannel(prGlueInfo, ucChannelNum);

		/* retener una ref al BSS (cfg80211_get_bss -> hay que ponerla) */
		bss = cfg80211_get_bss(wiphy, prChannel, arBssid,
			ssid.aucSsid, ssid.u4SsidLen,
			IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);

		if (bss == NULL) {
			/* crear el BSS al vuelo desde el BSS_DESC del core */
			prBssDesc = wlanGetTargetBssDescByNetwork(
				prGlueInfo->prAdapter, NETWORK_TYPE_AIS_INDEX);

			if (prBssDesc != NULL && prChannel != NULL) {
				struct cfg80211_inform_bss rInformData;

				kalMemZero(&rInformData, sizeof(rInformData));
				rInformData.chan = prChannel;
				/* scan_width eliminado de cfg80211_inform_bss en 7.0.12 (ancho 20 por defecto) */
				rInformData.signal =
					RCPI_TO_dBm(prBssDesc->ucRCPI) * 100;

				bss = cfg80211_inform_bss_data(wiphy,
					&rInformData,
					CFG80211_BSS_FTYPE_PRESP,
					arBssid,
					0,			/* TSF */
					WLAN_CAPABILITY_ESS,
					prBssDesc->u2BeaconInterval,
					prBssDesc->aucIEBuf,
					prBssDesc->u2IELength,
					GFP_KERNEL);
			}
		}

		if (eStatus == WLAN_STATUS_MEDIA_CONNECT) {
			/*
			 * Estado previo != CONNECTED -> es la conexion inicial:
			 * cfg80211_connect_bss consume/pone la ref del BSS.
			 */
			struct cfg80211_bss *prConnBss = bss;

			cfg80211_connect_bss(prDevHandler,
				arBssid,
				prConnBss,
				prGlueInfo->aucReqIe,
				prGlueInfo->u4ReqIeLength,
				prGlueInfo->aucRspIe,
				prGlueInfo->u4RspIeLength,
				WLAN_STATUS_SUCCESS,
				GFP_KERNEL,
				NL80211_TIMEOUT_UNSPECIFIED);
			/* cfg80211_connect_bss se queda la ref; no put aqui */
			bss = NULL;
		} else if (eStatus == WLAN_STATUS_ROAM_OUT_FIND_BEST &&
			   fgWasConnected && bss != NULL) {
			struct cfg80211_roam_info rRoamInfo;

			kalMemZero(&rRoamInfo, sizeof(rRoamInfo));
			/* 7.0.12 MLO: bss/bssid/channel movidos a links[]; valid_links=0 = enlace único legacy */
			rRoamInfo.links[0].bss = bss;
			rRoamInfo.req_ie = prGlueInfo->aucReqIe;
			rRoamInfo.req_ie_len = prGlueInfo->u4ReqIeLength;
			rRoamInfo.resp_ie = prGlueInfo->aucRspIe;
			rRoamInfo.resp_ie_len = prGlueInfo->u4RspIeLength;

			cfg80211_roamed(prDevHandler, &rRoamInfo, GFP_KERNEL);
			/* cfg80211_roamed consume la ref del bss */
			bss = NULL;
		}

		/* si quedo una ref sin consumir, liberarla */
		if (bss != NULL)
			cfg80211_put_bss(wiphy, bss);

		break;
	}

	case WLAN_STATUS_MEDIA_DISCONNECT:
	{
		BOOLEAN fgWasConnected =
			(prGlueInfo->eParamMediaStateIndicated ==
			 PARAM_MEDIA_STATE_CONNECTED);

		DBGLOG(INIT, INFO,
			("[wifi] %s netif_carrier_off\n", prDevHandler->name));

		netif_carrier_off(prDevHandler);

		if (prGlueInfo->fgIsRegistered == TRUE && fgWasConnected) {
			DBGLOG(INIT, INFO,
				("[wifi] %s cfg80211_disconnected\n",
				 prDevHandler->name));
			/*
			 * locally_generated = TRUE: la desconexion la origina el
			 * driver/host (no un deauth explicito del AP recibido por
			 * esta ruta). El core que sepa el reason lo pasa por pvBuf;
			 * aqui usamos 0 (unspecified) igual que el stock.
			 */
			cfg80211_disconnected(prDevHandler,
				0,		/* reason */
				NULL, 0,	/* ie, ie_len */
				TRUE,		/* locally_generated */
				GFP_KERNEL);
		}

		prGlueInfo->eParamMediaStateIndicated =
			PARAM_MEDIA_STATE_DISCONNECTED;
		break;
	}

	case WLAN_STATUS_SCAN_COMPLETE:
	{
		struct cfg80211_scan_info rScanInfo;

		/* 1. arrancar el request pendiente bajo spinlock */
		GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);
		if (prGlueInfo->prScanRequest != NULL) {
			prScanRequest = prGlueInfo->prScanRequest;
			prGlueInfo->prScanRequest = NULL;
		}
		GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);

		DBGLOG(INIT, TRACE,
			("[ais] scan complete req=%p inform=%u fail=%u\n",
			 prScanRequest, g_u4ScanInform, g_u4ScanInformFail));

		/* 2. cfg80211_scan_done con struct cfg80211_scan_info */
		if (prScanRequest != NULL) {
			kalMemZero(&rScanInfo, sizeof(rScanInfo));
			rScanInfo.aborted = FALSE;
			cfg80211_scan_done(prScanRequest, &rScanInfo);
		}
		break;
	}

	case WLAN_STATUS_CONNECT_INDICATION:
		/* join fail: solo si estabamos intentando conectar */
		if (prGlueInfo->eParamMediaStateIndicated !=
		    PARAM_MEDIA_STATE_CONNECTED &&
		    prGlueInfo->prAdapter->rWifiVar.rAisFsmInfo.prTargetBssDesc
			!= NULL) {
			cfg80211_connect_result(prDevHandler,
				prGlueInfo->prAdapter->rWifiVar.rAisFsmInfo
					.prTargetBssDesc->aucBSSID,
				prGlueInfo->aucReqIe,
				prGlueInfo->u4ReqIeLength,
				prGlueInfo->aucRspIe,
				prGlueInfo->u4RspIeLength,
				WLAN_STATUS_UNSPECIFIED_FAILURE,
				GFP_KERNEL);
		}
		break;

	case WLAN_STATUS_MEDIA_SPECIFIC_INDICATION:
		if (pStatus &&
		    pStatus->eStatusType == ENUM_STATUS_TYPE_AUTHENTICATION) {
			/* solo MIC ERROR (UC/GC) llega a cfg80211 */
			if (pAuth->arRequest[0].u4Flags ==
				PARAM_AUTH_REQUEST_PAIRWISE_ERROR ||
			    pAuth->arRequest[0].u4Flags ==
				PARAM_AUTH_REQUEST_GROUP_ERROR) {
				cfg80211_michael_mic_failure(prDevHandler,
					NULL,
					(pAuth->arRequest[0].u4Flags ==
					 PARAM_AUTH_REQUEST_PAIRWISE_ERROR) ?
						NL80211_KEYTYPE_PAIRWISE :
						NL80211_KEYTYPE_GROUP,
					0, NULL, GFP_KERNEL);
			}
		}
		break;

	default:
		DBGLOG(INIT, TRACE,
			("kalIndicateStatusAndComplete: unknown status 0x%x\n",
			 (unsigned int)eStatus));
		break;
	}
} /* kalIndicateStatusAndComplete */

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Cachear los IE de la (Re)Assoc REQUEST para el connect/roam.
 */
/*----------------------------------------------------------------------------*/
VOID
kalUpdateReAssocReqInfo(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PUINT_8		pucFrameBody,
	IN UINT_32		u4FrameBodyLen,
	IN BOOLEAN		fgReassocRequest
	)
{
	PUINT_8 cp;

	ASSERT(prGlueInfo);

	prGlueInfo->u4ReqIeLength = 0;

	if (fgReassocRequest) {
		if (u4FrameBodyLen < 15)
			return;
	} else {
		if (u4FrameBodyLen < 9)
			return;
	}

	cp = pucFrameBody;

	if (fgReassocRequest) {
		/* cap(2) + listen(2) + current AP(6) */
		cp += 10;
		u4FrameBodyLen -= 10;
	} else {
		/* cap(2) + listen(2) */
		cp += 4;
		u4FrameBodyLen -= 4;
	}

	if (u4FrameBodyLen <= CFG_CFG80211_IE_BUF_LEN) {
		prGlueInfo->u4ReqIeLength = u4FrameBodyLen;
		kalMemCopy(prGlueInfo->aucReqIe, cp, u4FrameBodyLen);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Cachear los IE de la (Re)Assoc RESPONSE para el connect/roam.
 */
/*----------------------------------------------------------------------------*/
VOID
kalUpdateReAssocRspInfo(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PUINT_8		pucFrameBody,
	IN UINT_32		u4FrameBodyLen
	)
{
	UINT_32 u4IEOffset = 6;	/* cap_info + status_code + assoc_id */
	UINT_32 u4IELength;

	ASSERT(prGlueInfo);

	prGlueInfo->u4RspIeLength = 0;

	if (u4FrameBodyLen < u4IEOffset)
		return;

	u4IELength = u4FrameBodyLen - u4IEOffset;

	if (u4IELength <= CFG_CFG80211_IE_BUF_LEN) {
		prGlueInfo->u4RspIeLength = u4IELength;
		kalMemCopy(prGlueInfo->aucRspIe,
			pucFrameBody + u4IEOffset, u4IELength);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Indicate un beacon/probe-resp a la capa cfg80211 (scan results).
 *         cfg80211_inform_bss_frame_data() con struct cfg80211_inform_bss.
 */
/*----------------------------------------------------------------------------*/
VOID
kalIndicateBssInfo(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PUINT_8		pucFrameBuf,
	IN UINT_32		u4BufLen,
	IN UINT_8		ucChannelNum,
	IN INT_32		i4SignalStrength
	)
{
	struct wiphy *wiphy;
	struct ieee80211_channel *prChannel;
	struct ieee80211_mgmt *prMgmtFrame;

	ASSERT(prGlueInfo);
	wiphy = priv_to_wiphy(prGlueInfo);

	prChannel = kalGetAisChannel(prGlueInfo, ucChannelNum);

	if (prChannel != NULL && prGlueInfo->prScanRequest != NULL) {
		struct cfg80211_bss *bss;
		struct cfg80211_inform_bss rInformData;

		prMgmtFrame = (struct ieee80211_mgmt *)pucFrameBuf;
		prMgmtFrame->u.beacon.timestamp = kalGetBootTime();

		g_u4ScanInform++;

		kalMemZero(&rInformData, sizeof(rInformData));
		rInformData.chan = prChannel;
		/* scan_width eliminado de cfg80211_inform_bss en 7.0.12 (ancho 20 por defecto) */
		rInformData.signal = i4SignalStrength * 100;	/* mBm */

		bss = cfg80211_inform_bss_frame_data(wiphy,
			&rInformData,
			(struct ieee80211_mgmt *)pucFrameBuf,
			u4BufLen,
			GFP_KERNEL);

		if (!bss) {
			g_u4ScanInformFail++;
			DBGLOG(SCN, TRACE,
				("cfg80211_inform_bss_frame_data() -> NULL\n"));
		} else {
			cfg80211_put_bss(wiphy, bss);
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Drena las beacon/probe encoladas y dispara el SCAN_COMPLETE.
 */
/*----------------------------------------------------------------------------*/
VOID
kalScanDone(
	IN P_GLUE_INFO_T			prGlueInfo,
	IN ENUM_KAL_NETWORK_TYPE_INDEX_T	eNetTypeIdx,
	IN WLAN_STATUS				status
	)
{
	P_AIS_FSM_INFO_T prAisFsmInfo;

	ASSERT(prGlueInfo);

	prAisFsmInfo = &(prGlueInfo->prAdapter->rWifiVar.rAisFsmInfo);

	/* volcar todas las beacon/probe encoladas a cfg80211 */
	scanReportBss2Cfg80211(prGlueInfo->prAdapter,
		BSS_TYPE_INFRASTRUCTURE, NULL);

	cnmTimerStopTimer(prGlueInfo->prAdapter, &prAisFsmInfo->rScanDoneTimer);

	wlanCheckSystemConfiguration(prGlueInfo->prAdapter);

	kalIndicateStatusAndComplete(prGlueInfo,
		WLAN_STATUS_SCAN_COMPLETE, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Entregar los skb de datos RX a la pila de red.
 *         netif_rx_ni() esta eliminado en 7.0.12: netif_rx() vale en cualquier
 *         contexto. eth_type_trans() fija skb->protocol/dev.
 */
/*----------------------------------------------------------------------------*/
WLAN_STATUS
kalRxIndicatePkts(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PVOID		apvPkts[],
	IN UINT_8		ucPktNum
	)
{
	UINT_8 ucIdx;
	struct net_device *prNetDev;
	struct sk_buff *prSkb;

	ASSERT(prGlueInfo);
	ASSERT(apvPkts);

	for (ucIdx = 0; ucIdx < ucPktNum; ucIdx++) {
		prSkb = (struct sk_buff *)apvPkts[ucIdx];
		if (prSkb == NULL)
			continue;

		/* STA-only: siempre el netdev AIS */
		prNetDev = prGlueInfo->prDevHandler;
		prGlueInfo->rNetDevStats.rx_bytes += prSkb->len;
		prGlueInfo->rNetDevStats.rx_packets++;

		/*
		 * filtrar unicast originado por nosotros (SA == DA): el HW ya
		 * filtra bcast/mcast propios, pero el unicast reflejado llega.
		 */
		if (kalMemCmp(prSkb->data, prSkb->data + 6, 6) == 0) {
			DBGLOG(RX, EVENT,
				("kalRxIndicatePkts: from-us, drop ("
				 MACSTR " len %u)\n",
				 MAC2STR(prSkb->data), (UINT32)prSkb->len));
			wlanReturnPacket(prGlueInfo->prAdapter, prSkb);
			continue;
		}

		prSkb->dev = prNetDev;
		prSkb->protocol = eth_type_trans(prSkb, prNetDev);

		DBGLOG(RX, EVENT,
			("kalRxIndicatePkts len = %u\n", (UINT32)prSkb->len));

		/* netif_rx_ni eliminado: netif_rx es seguro en todo contexto */
		netif_rx(prSkb);

		/* devolver el SW_RFB al pool (el skb ya lo posee la pila) */
		wlanReturnPacket(prGlueInfo->prAdapter, NULL);
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Estado TX de un mgmt frame (P2P/action). wdev + cfg80211_mgmt_tx_status.
 */
/*----------------------------------------------------------------------------*/
VOID
kalIndicateMgmtTxStatus(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN UINT_64		u8Cookie,
	IN BOOLEAN		fgIsAck,
	IN PUINT_8		pucFrameBuf,
	IN UINT_32		u4FrameLen
	)
{
	if (prGlueInfo == NULL || pucFrameBuf == NULL || u4FrameLen == 0) {
		DBGLOG(AIS, TRACE,
			("kalIndicateMgmtTxStatus: bad param %p %p %u\n",
			 prGlueInfo, pucFrameBuf, (UINT32)u4FrameLen));
		return;
	}

	if (prGlueInfo->prDevHandler == NULL ||
	    prGlueInfo->prDevHandler->ieee80211_ptr == NULL)
		return;		/* mismo patron que el OOPS de kalIndicateRxMgmtFrame */

	cfg80211_mgmt_tx_status(prGlueInfo->prDevHandler->ieee80211_ptr,
		u8Cookie,
		pucFrameBuf,
		u4FrameLen,
		fgIsAck,
		GFP_KERNEL);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Indicate un mgmt frame RX (probe/action) a cfg80211.
 *         cfg80211_rx_mgmt(wdev, freq, sig_dbm, buf, len, flags).
 */
/*----------------------------------------------------------------------------*/
VOID
kalIndicateRxMgmtFrame(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN P_SW_RFB_T		prSwRfb
	)
{
	INT_32 i4Freq;
	UINT_8 ucChnlNum;

	struct net_device *prDev;
	struct wireless_dev *prWdev;

	/* Capturar el frame en locales antes de pasarlo a cfg80211: evita que el
	 * SW_RFB se recicle (pool RX devolviendolo al FW) entre nuestros checks y
	 * la llamada, dejando pvHeader colgando. */
	const u8 *pucFrame;
	u32 u4FrameLen;

	if (prGlueInfo == NULL || prSwRfb == NULL) {
		ASSERT(FALSE);
		return;
	}

	/* OOPS recurrente en HW (0705 kernel #235, 0707 kernel #238):
	 * NULL deref en cfg80211_rx_mgmt_ext+0x48 (ldrh [r4,#10] = mgmt->sa)
	 * al indicar un action frame al stack. Causas identificadas:
	 *  - frame demasiado corto (<24B header mgmt) -> cfg80211 lee fuera del buf
	 *  - pvHeader reciclado por el SW_RFB en carrera del pool RX
	 *  - wdev con iftype fuera de rango -> mgmt_stypes[iftype] OOB en cfg80211
	 * Guards adicionales: longitud minima + iftype STA + snapshot en locales. */
	prDev = prGlueInfo->prDevHandler;
	if (prDev == NULL || prSwRfb->pvHeader == NULL || prSwRfb->u2PacketLen == 0)
		return;
	prWdev = prDev->ieee80211_ptr;
	if (prWdev == NULL || prWdev->wiphy == NULL)
		return;
	/* Solo indicar mgmt en modo STA (el unico iftype que soportamos). En otro
	 * iftype (o sin iftype valido/OOB), mgmt_stypes[wdev->iftype] en cfg80211
	 * seria un acceso fuera de rango -> NULL deref como el visto. */
	if (prWdev->iftype != NL80211_IFTYPE_STATION)
		return;
	if (prSwRfb->prHifRxHdr == NULL)
		return;
	/* Longitud minima de un frame mgmt 802.11: 24 bytes de header. Si el FW nos
	 * pasa algo mas corto (fragmento, basura, o probe shorter de 4B), cfg80211
	 * lo dereferenciara fuera del buffer al leer mgmt->sa (offset 10). */
	if (prSwRfb->u2PacketLen < 24)
		return;

	/* Snapshot en locales: a partir de aqui NO tocamos prSwRfb (que el pool RX
	 * puede reciclar) -- solo nuestros locales, que son los que pasamos a cfg80211. */
	pucFrame = (const u8 *)prSwRfb->pvHeader;
	u4FrameLen = (u32)prSwRfb->u2PacketLen;
	/* Sanity final de los punteros capturados (no-NULL y alineado). */
	if (pucFrame == NULL || ((unsigned long)pucFrame & 1))
		return;

	ucChnlNum = prSwRfb->prHifRxHdr->ucHwChannelNum;
	i4Freq = nicChannelNum2Freq(ucChnlNum) / 1000;
	if (i4Freq <= 0)
		return;		/* canal invalido: sin freq no hay a que atribuir la trama */

	cfg80211_rx_mgmt(prWdev,
		i4Freq,
		RCPI_TO_dBm(prSwRfb->prHifRxHdr->ucRcpi),
		pucFrame,
		u4FrameLen,
		0);	/* flags: 7.0.12 exige el arg extra (0 = ninguno) */
}
