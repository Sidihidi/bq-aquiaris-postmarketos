// SPDX-License-Identifier: GPL-2.0
/*
 * os/linux/glue/gl_cfg80211.c
 *
 * FASE 4/5 del port del MediaTek MT6582 'mt_wifi' full-MAC a linux-7.0.12
 * (postmarketOS / BQ krillin, armv7, cfg80211-only, STA-only).
 *
 * Handlers cfg80211 portados VERBATIM del stock (Android 3.10-era)
 *   .../wlan/os/linux/gl_cfg80211.c
 * con las firmas de 7.0.12 y solo los deltas de API estrictamente necesarios.
 * Los cuerpos (el "command stream" hacia el FW via kalIoctl->wlanoid*) NO se
 * tocan: son el mismo stream que el driver stock usa para scan/connect/keys.
 *
 * Al final define `struct cfg80211_ops mtk_cfg80211_ops` NO-weak (mismo nombre
 * que el stub __weak de gl_init.c): el enlazador se queda con este y descarta
 * el stub. Con esto NO hace falta tocar gl_init.c... salvo que el __weak no se
 * resuelva a favor del no-weak en algun toolchain (ver notas al final del
 * fichero y el handoff): en tal caso, borrar el bloque stub de gl_init.c.
 *
 * -------------------- DELTAS 3.10 -> 7.0.12 aplicados --------------------
 *  - .scan: firma (wiphy, cfg80211_scan_request*): el net_device ya no va como
 *    arg; se deriva de request->wdev->netdev cuando se necesita (aqui no hace
 *    falta, el glue trabaja sobre prGlueInfo).
 *  - .add_key/.del_key/.get_key/.set_default_key: nuevo arg `int link_id`
 *    (5.19/6.0+ MLO). Se acepta y se IGNORA (no MLO en este HW).
 *  - .get_station: `mac` es const u8*; `sinfo->filled` es un bitmap
 *    BIT_ULL(NL80211_STA_INFO_*) (no STATION_INFO_*). Los campos que el stock
 *    rellenaba con STATION_INFO_* se mapean a sus NL80211_STA_INFO_*.
 *  - .change_virtual_intf (change_iface): firma (wiphy, dev, type, vif_params*)
 *    SIN el `u32 *flags` de 3.10.
 *  - Las up-calls a cfg80211 (scan_done / inform_bss / connect_bss / roamed /
 *    disconnected) NO se llaman desde aqui: viven en el glue gl_kal_indicate.c
 *    (kalIndicateBssInfo / kalScanDone / kalIndicateStatusAndComplete). Este
 *    fichero SOLO lanza los comandos hacia el FW.
 *  - kalIoctl(): misma firma que el stock (la implementa gl_kal_ioctl.c).
 *
 * -------------------- DROPPED (NO cableados en el ops) --------------------
 *  Handlers del stock que dependen de P2P / TDLS / IBSS / mgmt-tx /
 *  remain-on-channel / testmode / WAPI: no se portan (STA-only puro). Solo se
 *  exponen los 12 handlers STA listados en el objetivo de la Fase 4/5.
 */

#include "precomp.h"			/* arrastra todo el core: GLUE_INFO_T, PARAM_*, kalIoctl, wlanoid*, DBGLOG, COPY_* */

#include <net/cfg80211.h>

/* Fase 4: helper de busqueda de IE portado VERBATIM de gl_wext.c (wext dropeado del port).
 * Escaner de IE puro (sin deps de kernel/wext) que el .connect usa para el IE de WPS (EID 0xDD,
 * OUI 00:50:f2:04). STA-only: WAPI queda fuera (CFG_SUPPORT_WAPI=0). */
static BOOLEAN wextSrchDesiredWPSIE(IN PUINT_8 pucIEStart, IN INT_32 i4TotalIeLen,
				    IN UINT_8 ucDesiredElemId, OUT PUINT_8 *ppucDesiredIE)
{
	INT_32 i4InfoElemLen;

	while (i4TotalIeLen >= 2) {
		i4InfoElemLen = (INT_32) pucIEStart[1] + 2;
		if (pucIEStart[0] == ucDesiredElemId && i4InfoElemLen <= i4TotalIeLen) {
			if (ucDesiredElemId != 0xDD) {
				*ppucDesiredIE = &pucIEStart[0];
				return TRUE;
			} else if (pucIEStart[1] >= 4 &&
				   memcmp(&pucIEStart[2], "\x00\x50\xf2\x04", 4) == 0) {
				*ppucDesiredIE = &pucIEStart[0];
				return TRUE;
			}
		}
		i4TotalIeLen -= i4InfoElemLen;
		pucIEStart += i4InfoElemLen;
	}
	return FALSE;
}

/*******************************************************************************
*                              P R I V A T E   D A T A
********************************************************************************/
/*
 * Workaround del stock: si el supplicant borra mas de N veces una clave sin que
 * el FW libere el recurso TC, bloqueamos el segundo del_key para no colgar >10s
 * el supplicant. Se conserva verbatim del stock (gucKeyIndex).
 */
static UINT_8 gucKeyIndex = 255;

/* buffer compartido del scan (idiom stock: estatico para no reventar la pila) */
static PARAM_SCAN_REQUEST_EXT_T rScanRequest;

/* buffer de la WEP key para el connect (idiom stock) */
static UINT_8 wepBuf[48];

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************/

/*----------------------------------------------------------------------------*/
/*!
 * @brief .change_virtual_intf — cambia el tipo de STA (Infra / IBSS).
 *        DELTA 7.0.12: sin el arg `u32 *flags`.
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_change_iface(
	struct wiphy *wiphy,
	struct net_device *ndev,
	enum nl80211_iftype type,
	struct vif_params *params
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	ENUM_PARAM_OP_MODE_T eOpMode;
	UINT_32 u4BufLen;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	if (type == NL80211_IFTYPE_STATION) {
		eOpMode = NET_TYPE_INFRA;
	} else if (type == NL80211_IFTYPE_ADHOC) {
		eOpMode = NET_TYPE_IBSS;
	} else {
		return -EINVAL;
	}

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetInfrastructureMode,
			&eOpMode,
			sizeof(eOpMode),
			FALSE,
			FALSE,
			TRUE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, ("set infrastructure mode error:%x\n", (unsigned int)rStatus));
	}

	/* reset wpa info */
	prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
	prGlueInfo->rWpaInfo.u4KeyMgmt = 0;
	prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_NONE;
	prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_NONE;
	prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
#if CFG_SUPPORT_802_11W
	prGlueInfo->rWpaInfo.u4Mfp = IW_AUTH_MFP_DISABLED;
#endif

	(void) ndev;
	(void) params;
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .add_key — instala una clave (PTK/GTK/WEP) en el FW.
 *        DELTA 7.0.12: nuevo arg `int link_id` (ignorado, no MLO).
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_add_key(
	struct wiphy *wiphy,
	struct net_device *ndev,
	int link_id,
	u8 key_index,
	bool pairwise,
	const u8 *mac_addr,
	struct key_params *params
	)
{
	PARAM_KEY_T rKey;
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	INT_32 i4Rslt = -EINVAL;
	UINT_32 u4BufLen = 0;
	UINT_8 tmp1[8];
	UINT_8 tmp2[8];

	(void) link_id;
	(void) pairwise;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	kalMemZero(&rKey, sizeof(PARAM_KEY_T));

	rKey.u4KeyIndex = key_index;

	if (mac_addr) {
		COPY_MAC_ADDR(rKey.arBSSID, mac_addr);
		if ((rKey.arBSSID[0] == 0x00) && (rKey.arBSSID[1] == 0x00) && (rKey.arBSSID[2] == 0x00) &&
		    (rKey.arBSSID[3] == 0x00) && (rKey.arBSSID[4] == 0x00) && (rKey.arBSSID[5] == 0x00)) {
			rKey.arBSSID[0] = 0xff;
			rKey.arBSSID[1] = 0xff;
			rKey.arBSSID[2] = 0xff;
			rKey.arBSSID[3] = 0xff;
			rKey.arBSSID[4] = 0xff;
			rKey.arBSSID[5] = 0xff;
		}
		if (rKey.arBSSID[0] != 0xFF) {
			rKey.u4KeyIndex |= BIT(31);
			if ((rKey.arBSSID[0] != 0x00) || (rKey.arBSSID[1] != 0x00) || (rKey.arBSSID[2] != 0x00) ||
			    (rKey.arBSSID[3] != 0x00) || (rKey.arBSSID[4] != 0x00) || (rKey.arBSSID[5] != 0x00))
				rKey.u4KeyIndex |= BIT(30);
		}
	} else {
		rKey.arBSSID[0] = 0xff;
		rKey.arBSSID[1] = 0xff;
		rKey.arBSSID[2] = 0xff;
		rKey.arBSSID[3] = 0xff;
		rKey.arBSSID[4] = 0xff;
		rKey.arBSSID[5] = 0xff;
		/* rKey.u4KeyIndex |= BIT(31); // BIT31 haria TX con la bc key id; queremos pairwise id 0 */
	}

	if (params->key) {
		kalMemCopy(rKey.aucKeyMaterial, params->key, params->key_len);
		if (params->key_len == 32) {
			kalMemCopy(tmp1, &params->key[16], 8);
			kalMemCopy(tmp2, &params->key[24], 8);
			kalMemCopy(&rKey.aucKeyMaterial[16], tmp2, 8);
			kalMemCopy(&rKey.aucKeyMaterial[24], tmp1, 8);
		}
	}

	rKey.u4KeyLength = params->key_len;
	rKey.u4Length = ((UINT_32) & (((P_PARAM_KEY_T)0)->aucKeyMaterial)) + rKey.u4KeyLength;

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetAddKey,
			&rKey,
			rKey.u4Length,
			FALSE,
			FALSE,
			TRUE,
			FALSE,
			&u4BufLen);

	if (rStatus == WLAN_STATUS_SUCCESS)
		i4Rslt = 0;

	(void) ndev;
	return i4Rslt;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .get_key — no implementado en el stock (devuelve error). Conservado.
 *        DELTA 7.0.12: nuevo arg `int link_id` (ignorado).
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_get_key(
	struct wiphy *wiphy,
	struct net_device *ndev,
	int link_id,
	u8 key_index,
	bool pairwise,
	const u8 *mac_addr,
	void *cookie,
	void (*callback)(void *cookie, struct key_params *)
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;

	(void) link_id;
	(void) ndev;
	(void) key_index;
	(void) pairwise;
	(void) mac_addr;
	(void) cookie;
	(void) callback;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	/* not implemented (igual que el stock) */
	return -EINVAL;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .del_key — elimina una clave del FW.
 *        DELTA 7.0.12: nuevo arg `int link_id` (ignorado).
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_del_key(
	struct wiphy *wiphy,
	struct net_device *ndev,
	int link_id,
	u8 key_index,
	bool pairwise,
	const u8 *mac_addr
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	PARAM_REMOVE_KEY_T rRemoveKey;
	UINT_32 u4BufLen = 0;
	INT_32 i4Rslt = -EINVAL;

	(void) link_id;
	(void) pairwise;
	(void) ndev;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	kalMemZero(&rRemoveKey, sizeof(PARAM_REMOVE_KEY_T));
	if (mac_addr)
		COPY_MAC_ADDR(rRemoveKey.arBSSID, mac_addr);
	else if (key_index > gucKeyIndex) {	/* bypass del siguiente del_key (workaround ANR) */
		gucKeyIndex = key_index;
		return -EBUSY;
	} else				/* nueva operacion, reset gucKeyIndex */
		gucKeyIndex = 255;
	rRemoveKey.u4KeyIndex = key_index;
	rRemoveKey.u4Length = sizeof(PARAM_REMOVE_KEY_T);

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetRemoveKey,
			&rRemoveKey,
			rRemoveKey.u4Length,
			FALSE,
			FALSE,
			TRUE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, ("remove key error:%x\n", (unsigned int)rStatus));
		if (rStatus == WLAN_STATUS_FAILURE && mac_addr) {
			i4Rslt = -EBUSY;
			gucKeyIndex = key_index;
		}
	} else {
		gucKeyIndex = 255;
		i4Rslt = 0;
	}

	return i4Rslt;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .set_default_key — no-op en el stock (el FW gestiona el default TX id).
 *        DELTA 7.0.12: nuevo arg `int link_id` (ignorado).
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_set_default_key(
	struct wiphy *wiphy,
	struct net_device *ndev,
	int link_id,
	u8 key_index,
	bool unicast,
	bool multicast
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;

	(void) link_id;
	(void) ndev;
	(void) key_index;
	(void) unicast;
	(void) multicast;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	/* not implemented (igual que el stock: el FW usa el pairwise id 0) */
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .get_station — rellena RSSI / bitrate / paquetes de la STA conectada.
 *        DELTA 7.0.12: `mac` es const; sinfo->filled es BIT_ULL(NL80211_STA_INFO_*).
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_get_station(
	struct wiphy *wiphy,
	struct net_device *ndev,
	const u8 *mac,
	struct station_info *sinfo
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus;
	PARAM_MAC_ADDRESS arBssid;
	UINT_32 u4BufLen, u4Rate;
	INT_32 i4Rssi;

	(void) ndev;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	kalMemZero(arBssid, MAC_ADDR_LEN);
	wlanQueryInformation(prGlueInfo->prAdapter,
			wlanoidQueryBssid,
			&arBssid[0],
			sizeof(arBssid),
			&u4BufLen);

	/* 1. check BSSID */
	if (UNEQUAL_MAC_ADDR(arBssid, mac)) {
		/* wrong MAC address */
		DBGLOG(REQ, WARN, ("incorrect BSSID: [" MACSTR "] currently connected BSSID[" MACSTR "]\n",
				MAC2STR(mac), MAC2STR(arBssid)));
		return -ENOENT;
	}

	/* 2. fill TX rate */
	rStatus = kalIoctl(prGlueInfo,
			wlanoidQueryLinkSpeed,
			&u4Rate,
			sizeof(u4Rate),
			TRUE,
			FALSE,
			FALSE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, ("unable to retrieve link speed\n"));
	} else {
		sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_BITRATE);
		sinfo->txrate.legacy = u4Rate / 1000;	/* de 100bps a 100kbps */
	}

	if (prGlueInfo->eParamMediaStateIndicated != PARAM_MEDIA_STATE_CONNECTED) {
		/* not connected */
		DBGLOG(REQ, WARN, ("not yet connected\n"));
	} else {
		/* 3. fill RSSI */
		rStatus = kalIoctl(prGlueInfo,
				wlanoidQueryRssi,
				&i4Rssi,
				sizeof(i4Rssi),
				TRUE,
				FALSE,
				FALSE,
				FALSE,
				&u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(REQ, WARN, ("unable to retrieve link speed\n"));
		} else {
			sinfo->filled |= BIT_ULL(NL80211_STA_INFO_SIGNAL);
			/* en cfg80211 signal es signed char */
			if (i4Rssi < -128)
				sinfo->signal = -128;
			else
				sinfo->signal = i4Rssi;	/* dBm */
		}
	}

	sinfo->rx_packets = prGlueInfo->rNetDevStats.rx_packets;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_RX_PACKETS);
	sinfo->tx_packets = prGlueInfo->rNetDevStats.tx_packets;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_PACKETS);

	{
		UINT_32 u4XmitError = 0;

		rStatus = kalIoctl(prGlueInfo,
				wlanoidQueryXmitError,
				&u4XmitError,
				sizeof(UINT_32),
				TRUE,
				TRUE,
				TRUE,
				FALSE,
				&u4BufLen);

		prGlueInfo->rNetDevStats.tx_errors = u4XmitError;
	}

	sinfo->tx_failed = prGlueInfo->rNetDevStats.tx_errors;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_FAILED);

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .scan — lanza un scan hacia el FW. El resultado (beacon/probe -> BSS y
 *        el SCAN_COMPLETE) lo indica gl_kal_indicate.c (kalIndicateBssInfo /
 *        kalScanDone). Aqui SOLO se prepara PARAM_SCAN_REQUEST_EXT_T y se manda
 *        wlanoidSetBssidListScanExt, y se guarda prGlueInfo->prScanRequest.
 *        DELTA 7.0.12: la firma no lleva net_device (va en request->wdev->netdev).
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_scan(
	struct wiphy *wiphy,
	struct cfg80211_scan_request *request
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus;
	UINT_32 u4BufLen;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	DBGLOG(REQ, INFO, ("mtk_cfg80211_scan\n"));
	kalMemZero(&rScanRequest, sizeof(PARAM_SCAN_REQUEST_EXT_T));

	/* check if there is any pending scan not yet finished */
	if (prGlueInfo->prScanRequest != NULL) {
		DBGLOG(REQ, INFO, ("prGlueInfo->prScanRequest != NULL\n"));
		return -EBUSY;
	}

	if (request->n_ssids == 0) {
		rScanRequest.rSsid.u4SsidLen = 0;
	} else if (request->n_ssids == 1) {
		COPY_SSID(rScanRequest.rSsid.aucSsid, rScanRequest.rSsid.u4SsidLen,
			request->ssids[0].ssid, request->ssids[0].ssid_len);
	} else {
		DBGLOG(REQ, INFO, ("request->n_ssids:%d\n", request->n_ssids));
		return -EINVAL;
	}

	if (request->ie_len > 0) {
		rScanRequest.u4IELength = request->ie_len;
		rScanRequest.pucIE = (PUINT_8) (request->ie);
	} else {
		rScanRequest.u4IELength = 0;
	}

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetBssidListScanExt,
			&rScanRequest,
			sizeof(PARAM_SCAN_REQUEST_EXT_T),
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, INFO, ("scan error:%x\n", (unsigned int)rStatus));
		return -EINVAL;
	}

	prGlueInfo->prScanRequest = request;

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .connect — conectar a un ESS. VERBATIM del stock: es el "command
 *        stream" exacto (SetInfrastructureMode -> SetAuthMode ->
 *        SetEncryptionStatus -> [SetAddWep] -> SetSsid/SetBssid) que fija
 *        eEncStatus=ENUM_ENCRYPTION3_ENABLED para CCMP (clave para el DHCP).
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_connect(
	struct wiphy *wiphy,
	struct net_device *ndev,
	struct cfg80211_connect_params *sme
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus;
	UINT_32 u4BufLen;
	ENUM_PARAM_ENCRYPTION_STATUS_T eEncStatus;
	ENUM_PARAM_AUTH_MODE_T eAuthMode;
	UINT_32 cipher;
	PARAM_SSID_T rNewSsid;
	BOOLEAN fgCarryWPSIE = FALSE;
	ENUM_PARAM_OP_MODE_T eOpMode;

	(void) ndev;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	DBGLOG(REQ, INFO, ("[wlan] mtk_cfg80211_connect 0x%p %u\n",
			sme->ie, (unsigned int)sme->ie_len));

	if (prGlueInfo->prAdapter->rWifiVar.rConnSettings.eOPMode > NET_TYPE_AUTO_SWITCH)
		eOpMode = NET_TYPE_AUTO_SWITCH;
	else
		eOpMode = prGlueInfo->prAdapter->rWifiVar.rConnSettings.eOPMode;

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetInfrastructureMode,
			&eOpMode,
			sizeof(eOpMode),
			FALSE,
			FALSE,
			TRUE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("wlanoidSetInfrastructureMode fail 0x%x\n", (unsigned int)rStatus));
		return -EFAULT;
	}

	/* despues de fijar el modo, la tabla de claves queda limpia */

	/* reset wpa info */
	prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
	prGlueInfo->rWpaInfo.u4KeyMgmt = 0;
	prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_NONE;
	prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_NONE;
	prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
#if CFG_SUPPORT_802_11W
	prGlueInfo->rWpaInfo.u4Mfp = IW_AUTH_MFP_DISABLED;
#endif

	if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_1)
		prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_WPA;
	else if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_2)
		prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_WPA2;
	else
		prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

	switch (sme->auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
		prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
		break;
	case NL80211_AUTHTYPE_SHARED_KEY:
		prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_SHARED_KEY;
		break;
	default:
		prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM | IW_AUTH_ALG_SHARED_KEY;
		break;
	}

	if (sme->crypto.n_ciphers_pairwise) {
		prGlueInfo->prAdapter->rWifiVar.rConnSettings.rRsnInfo.au4PairwiseKeyCipherSuite[0] =
			sme->crypto.ciphers_pairwise[0];
		switch (sme->crypto.ciphers_pairwise[0]) {
		case WLAN_CIPHER_SUITE_WEP40:
			prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_WEP40;
			break;
		case WLAN_CIPHER_SUITE_WEP104:
			prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_WEP104;
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_TKIP;
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_CCMP;
			break;
		case WLAN_CIPHER_SUITE_AES_CMAC:
			prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_CCMP;
			break;
		default:
			DBGLOG(REQ, WARN, ("invalid cipher pairwise (%d)\n",
					sme->crypto.ciphers_pairwise[0]));
			return -EINVAL;
		}
	}

	if (sme->crypto.cipher_group) {
		prGlueInfo->prAdapter->rWifiVar.rConnSettings.rRsnInfo.u4GroupKeyCipherSuite =
			sme->crypto.cipher_group;
		switch (sme->crypto.cipher_group) {
		case WLAN_CIPHER_SUITE_WEP40:
			prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_WEP40;
			break;
		case WLAN_CIPHER_SUITE_WEP104:
			prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_WEP104;
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_TKIP;
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_CCMP;
			break;
		case WLAN_CIPHER_SUITE_AES_CMAC:
			prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_CCMP;
			break;
		default:
			DBGLOG(REQ, WARN, ("invalid cipher group (%d)\n",
					sme->crypto.cipher_group));
			return -EINVAL;
		}
	}

	if (sme->crypto.n_akm_suites) {
		prGlueInfo->prAdapter->rWifiVar.rConnSettings.rRsnInfo.au4AuthKeyMgtSuite[0] =
			sme->crypto.akm_suites[0];
		if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_WPA) {
			switch (sme->crypto.akm_suites[0]) {
			case WLAN_AKM_SUITE_8021X:
				eAuthMode = AUTH_MODE_WPA;
				break;
			case WLAN_AKM_SUITE_PSK:
				eAuthMode = AUTH_MODE_WPA_PSK;
				break;
			default:
				DBGLOG(REQ, WARN, ("invalid cipher group (%d)\n",
						sme->crypto.cipher_group));
				return -EINVAL;
			}
		} else if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_WPA2) {
			switch (sme->crypto.akm_suites[0]) {
			case WLAN_AKM_SUITE_8021X:
				eAuthMode = AUTH_MODE_WPA2;
				break;
			case WLAN_AKM_SUITE_PSK:
				eAuthMode = AUTH_MODE_WPA2_PSK;
				break;
			default:
				DBGLOG(REQ, WARN, ("invalid cipher group (%d)\n",
						sme->crypto.cipher_group));
				return -EINVAL;
			}
		}
	}

	if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_DISABLED) {
		eAuthMode = (prGlueInfo->rWpaInfo.u4AuthAlg == IW_AUTH_ALG_OPEN_SYSTEM) ?
			AUTH_MODE_OPEN : AUTH_MODE_AUTO_SWITCH;
	}

	prGlueInfo->rWpaInfo.fgPrivacyInvoke = sme->privacy;

	prGlueInfo->fgWpsActive = FALSE;
#if CFG_SUPPORT_HOTSPOT_2_0
	prGlueInfo->fgConnectHS20AP = FALSE;
#endif

	if (sme->ie && sme->ie_len > 0) {
		PUINT_8 prDesiredIE = NULL;

#if CFG_SUPPORT_WAPI
		if (wextSrchDesiredWAPIIE(sme->ie,
				sme->ie_len, (PUINT_8 *)&prDesiredIE)) {
			rStatus = kalIoctl(prGlueInfo,
					wlanoidSetWapiAssocInfo,
					prDesiredIE,
					IE_SIZE(prDesiredIE),
					FALSE,
					FALSE,
					FALSE,
					FALSE,
					&u4BufLen);

			if (rStatus != WLAN_STATUS_SUCCESS) {
				DBGLOG(SEC, WARN, ("[wapi] set wapi assoc info error:%x\n", (unsigned int)rStatus));
			}
		}
#endif

		DBGLOG(REQ, INFO, ("[wlan] wlanoidSetWapiAssocInfo: .fgWapiMode = %d\n",
				prGlueInfo->prAdapter->rWifiVar.rConnSettings.fgWapiMode));

#if CFG_SUPPORT_WPS2
		if (wextSrchDesiredWPSIE(sme->ie,
				sme->ie_len,
				0xDD,
				(PUINT_8 *)&prDesiredIE)) {
			prGlueInfo->fgWpsActive = TRUE;
			fgCarryWPSIE = TRUE;

			rStatus = kalIoctl(prGlueInfo,
					wlanoidSetWSCAssocInfo,
					prDesiredIE,
					IE_SIZE(prDesiredIE),
					FALSE,
					FALSE,
					FALSE,
					FALSE,
					&u4BufLen);
			if (rStatus != WLAN_STATUS_SUCCESS) {
				DBGLOG(SEC, WARN, ("WSC] set WSC assoc info error:%x\n", (unsigned int)rStatus));
			}
		}
#endif
	}

	/* clear WSC Assoc IE buffer si no se detecto WPS IE */
	if (fgCarryWPSIE == FALSE) {
		kalMemZero(&prGlueInfo->aucWSCAssocInfoIE, 200);
		prGlueInfo->u2WSCAssocInfoIELen = 0;
	}

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetAuthMode,
			&eAuthMode,
			sizeof(eAuthMode),
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			&u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, ("set auth mode error:%x\n", (unsigned int)rStatus));
	}

	cipher = prGlueInfo->rWpaInfo.u4CipherGroup | prGlueInfo->rWpaInfo.u4CipherPairwise;

	if (prGlueInfo->rWpaInfo.fgPrivacyInvoke) {
		if (cipher & IW_AUTH_CIPHER_CCMP) {
			eEncStatus = ENUM_ENCRYPTION3_ENABLED;
		} else if (cipher & IW_AUTH_CIPHER_TKIP) {
			eEncStatus = ENUM_ENCRYPTION2_ENABLED;
		} else if (cipher & (IW_AUTH_CIPHER_WEP104 | IW_AUTH_CIPHER_WEP40)) {
			eEncStatus = ENUM_ENCRYPTION1_ENABLED;
		} else if (cipher & IW_AUTH_CIPHER_NONE) {
			if (prGlueInfo->rWpaInfo.fgPrivacyInvoke)
				eEncStatus = ENUM_ENCRYPTION1_ENABLED;
			else
				eEncStatus = ENUM_ENCRYPTION_DISABLED;
		} else {
			eEncStatus = ENUM_ENCRYPTION_DISABLED;
		}
	} else {
		eEncStatus = ENUM_ENCRYPTION_DISABLED;
	}

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetEncryptionStatus,
			&eEncStatus,
			sizeof(eEncStatus),
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			&u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, ("set encryption mode error:%x\n", (unsigned int)rStatus));
	}

	if (sme->key_len != 0 && prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_DISABLED) {
		P_PARAM_WEP_T prWepKey = (P_PARAM_WEP_T) wepBuf;

		kalMemSet(prWepKey, 0, sizeof(prWepKey));
		prWepKey->u4Length = 12 + sme->key_len;
		prWepKey->u4KeyLength = (UINT_32) sme->key_len;
		prWepKey->u4KeyIndex = (UINT_32) sme->key_idx;
		prWepKey->u4KeyIndex |= BIT(31);
		if (prWepKey->u4KeyLength > 32) {
			DBGLOG(REQ, WARN, ("Too long key length (%u)\n", (unsigned int)prWepKey->u4KeyLength));
			return -EINVAL;
		}
		kalMemCopy(prWepKey->aucKeyMaterial, sme->key, prWepKey->u4KeyLength);

		rStatus = kalIoctl(prGlueInfo,
				wlanoidSetAddWep,
				prWepKey,
				prWepKey->u4Length,
				FALSE,
				FALSE,
				TRUE,
				FALSE,
				&u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, INFO, ("wlanoidSetAddWep fail 0x%x\n", (unsigned int)rStatus));
			return -EFAULT;
		}
	}

	if (sme->ssid_len > 0) {
		/* connect by SSID */
		COPY_SSID(rNewSsid.aucSsid, rNewSsid.u4SsidLen, sme->ssid, sme->ssid_len);

		rStatus = kalIoctl(prGlueInfo,
				wlanoidSetSsid,
				(PVOID) &rNewSsid,
				sizeof(PARAM_SSID_T),
				FALSE,
				FALSE,
				TRUE,
				FALSE,
				&u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(REQ, WARN, ("set SSID:%x\n", (unsigned int)rStatus));
			return -EINVAL;
		}
	} else {
		/* connect by BSSID */
		rStatus = kalIoctl(prGlueInfo,
				wlanoidSetBssid,
				(PVOID) sme->bssid,
				MAC_ADDR_LEN,
				FALSE,
				FALSE,
				TRUE,
				FALSE,
				&u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(REQ, WARN, ("set BSSID:%x\n", (unsigned int)rStatus));
			return -EINVAL;
		}
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .disconnect — desasociar del ESS actual. VERBATIM del stock.
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_disconnect(
	struct wiphy *wiphy,
	struct net_device *ndev,
	u16 reason_code
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus;
	UINT_32 u4BufLen;

	(void) ndev;
	(void) reason_code;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetDisassociate,
			NULL,
			0,
			FALSE,
			FALSE,
			TRUE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, ("disassociate error:%x\n", (unsigned int)rStatus));
		return -EFAULT;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .set_power_mgmt — perfil de ahorro de energia. VERBATIM del stock.
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_set_power_mgmt(
	struct wiphy *wiphy,
	struct net_device *ndev,
	bool enabled,
	int timeout
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus;
	UINT_32 u4BufLen;
	PARAM_POWER_MODE ePowerMode;

	(void) ndev;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	if (enabled) {
		if (timeout == -1) {
			ePowerMode = Param_PowerModeFast_PSP;
		} else {
			ePowerMode = Param_PowerModeMAX_PSP;
		}
	} else {
		ePowerMode = Param_PowerModeCAM;
	}

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSet802dot11PowerSaveProfile,
			&ePowerMode,
			sizeof(ePowerMode),
			FALSE,
			FALSE,
			TRUE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, ("set_power_mgmt error:%x\n", (unsigned int)rStatus));
		return -EFAULT;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .set_pmksa — cachear un PMKID para un BSSID. VERBATIM del stock.
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_set_pmksa(
	struct wiphy *wiphy,
	struct net_device *ndev,
	struct cfg80211_pmksa *pmksa
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus;
	UINT_32 u4BufLen;
	P_PARAM_PMKID_T prPmkid;

	(void) ndev;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	prPmkid = (P_PARAM_PMKID_T) kalMemAlloc(8 + sizeof(PARAM_BSSID_INFO_T), VIR_MEM_TYPE);
	if (!prPmkid) {
		DBGLOG(INIT, INFO, ("Can not alloc memory for IW_PMKSA_ADD\n"));
		return -ENOMEM;
	}

	prPmkid->u4Length = 8 + sizeof(PARAM_BSSID_INFO_T);
	prPmkid->u4BSSIDInfoCount = 1;
	kalMemCopy(prPmkid->arBSSIDInfo->arBSSID, pmksa->bssid, 6);
	kalMemCopy(prPmkid->arBSSIDInfo->arPMKID, pmksa->pmkid, IW_PMKID_LEN);

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetPmkid,
			prPmkid,
			sizeof(PARAM_PMKID_T),
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("add pmkid error:%x\n", (unsigned int)rStatus));
	}
	kalMemFree(prPmkid, VIR_MEM_TYPE, 8 + sizeof(PARAM_BSSID_INFO_T));

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .del_pmksa — no-op en el stock (devuelve 0). Conservado.
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_del_pmksa(
	struct wiphy *wiphy,
	struct net_device *ndev,
	struct cfg80211_pmksa *pmksa
	)
{
	(void) wiphy;
	(void) ndev;
	(void) pmksa;
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .flush_pmksa — vaciar la cache de PMKID. VERBATIM del stock.
 */
/*----------------------------------------------------------------------------*/
static int
mtk_cfg80211_flush_pmksa(
	struct wiphy *wiphy,
	struct net_device *ndev
	)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus;
	UINT_32 u4BufLen;
	P_PARAM_PMKID_T prPmkid;

	(void) ndev;

	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	prPmkid = (P_PARAM_PMKID_T) kalMemAlloc(8, VIR_MEM_TYPE);
	if (!prPmkid) {
		DBGLOG(INIT, INFO, ("Can not alloc memory for IW_PMKSA_FLUSH\n"));
		return -ENOMEM;
	}

	prPmkid->u4Length = 8;
	prPmkid->u4BSSIDInfoCount = 0;

	rStatus = kalIoctl(prGlueInfo,
			wlanoidSetPmkid,
			prPmkid,
			sizeof(PARAM_PMKID_T),
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			&u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, ("flush pmkid error:%x\n", (unsigned int)rStatus));
	}
	kalMemFree(prPmkid, VIR_MEM_TYPE, 8);

	return 0;
}

/*******************************************************************************
*        struct cfg80211_ops mtk_cfg80211_ops  (NO-weak: gana al stub)
********************************************************************************/
/*
 * NO-weak: sustituye al `__weak struct cfg80211_ops mtk_cfg80211_ops` de
 * gl_init.c. wiphy_new(&mtk_cfg80211_ops, sizeof(GLUE_INFO_T)) en wlanNetCreate
 * lo referencia por nombre; el enlazador prefiere este simbolo fuerte.
 *
 * Solo los 12 handlers STA. NO se cablean: IBSS (join/leave_ibss),
 * mgmt-tx / remain-on-channel / mgmt_frame_register (P2P), testmode,
 * change/add/del_station (TDLS), assoc (usa req->bss, no lo pide wpa_s en STA
 * puro por esta ruta).
 */
struct cfg80211_ops mtk_cfg80211_ops = {
	.change_virtual_intf	= mtk_cfg80211_change_iface,
	.add_key		= mtk_cfg80211_add_key,
	.get_key		= mtk_cfg80211_get_key,
	.del_key		= mtk_cfg80211_del_key,
	.set_default_key	= mtk_cfg80211_set_default_key,
	.get_station		= mtk_cfg80211_get_station,
	.scan			= mtk_cfg80211_scan,
	.connect		= mtk_cfg80211_connect,
	.disconnect		= mtk_cfg80211_disconnect,
	.set_power_mgmt		= mtk_cfg80211_set_power_mgmt,
	.set_pmksa		= mtk_cfg80211_set_pmksa,
	.del_pmksa		= mtk_cfg80211_del_pmksa,
	.flush_pmksa		= mtk_cfg80211_flush_pmksa,
};
