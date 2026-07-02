// SPDX-License-Identifier: GPL-2.0
/*
 * os/linux/glue/gl_kal_mem.c
 *
 * SHIM KAL "memmisc" del port del driver full-MAC MediaTek MT6582 'mt_wifi'
 * (Android 3.10-era) a linux-7.0.12 (postmarketOS / BQ krillin, armv7).
 *
 * Este es uno de los seis translation-units que reemplazan al gl_kal.c stock,
 * segun el contrato de "gl_kal_prototypes.h". Aqui viven:
 *
 *   - memoria / buffer / paquete:
 *       kalPacketAlloc / kalPacketFree        (dev_alloc_skb / dev_kfree_skb)
 *       kalProcessRxPacket                    (re-punta skb al buffer de RX)
 *   - clasificador de TX (CLAVE del fix de cifrado):
 *       kalQoSFrameClassifierAndPacketInfo    (ethertype/UP; EAPOL 0x888E -> is1X)
 *   - completado de TX:
 *       kalSendCompleteAndAwakeQueue          (libera skb + reanuda las colas netdev)
 *       kalSecurityFrameSendComplete
 *   - MAC / config:
 *       kalUpdateMACAddress
 *       kalRetrieveNetworkAddress
 *   - STUBS de config/estado para este port STA-only sin NVRAM:
 *       kalIsCardRemoved -> FALSE
 *       kalIsResetting   -> FALSE   (CFG_CHIP_RESET_SUPPORT=0)
 *       kalCfgDataRead16 / kalCfgDataWrite16 -> fallo (fgNvramAvailable=FALSE)
 *       kalGetConfigurationVersion -> 0
 *       kalWriteToFile   -> -EPERM
 *
 * NOTA IMPORTANTE sobre lo que NO va aqui:
 *   kalMemAlloc / kalMemFree / kalMemCopy / kalMemSet / kalMemZero / kalMemCmp,
 *   kalCopyFrame, kalQueryBufferPointer / kalQueryValidBufferLength,
 *   kalGetTimeTick, kalPrint, kalSendComplete
 * son MACROS definidas en <os/linux/include/gl_kal.h> (lineas ~717-891). NO se
 * re-implementan como funciones aqui: hacerlo colisionaria con las macros (el
 * preprocesador las expandiria sobre la definicion). El core stock las consume
 * ya expandidas; este .c solo aporta los SIMBOLOS de funcion reales que faltan.
 *
 * ENUM_SPIN_LOCK_CATEGORY_E, PHY_MEM_TYPE/VIR_MEM_TYPE, ETH_P_1X (0x888E),
 * ETH_TYPE_LEN_OFFSET, PARAM_MAC_ADDR_LEN, etc. ya los declaran los headers
 * stock (gl_kal.h / gl_os.h / wlan_oid.h) portados a include/; por tanto no se
 * declaran de nuevo (el contrato pide declarar el enum SOLO si faltara).
 */

#include "precomp.h"		/* GLUE_INFO_T completo, ADAPTER_T, macros kal*, ENUM_* */
#include "gl_kal_prototypes.h"	/* contrato unico de los shims KAL */

#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jiffies.h>
#include <linux/ip.h>
#include <linux/errno.h>

/* ==========================================================================
 *  Fallbacks defensivos: si el orden de include del port no arrastro alguna
 *  constante de gl_os.h (ETH_*), definirla con el valor stock. Guarded para no
 *  redefinir cuando gl_os.h SI la trajo.
 * ========================================================================== */
#ifndef ETH_TYPE_LEN_OFFSET
#define ETH_TYPE_LEN_OFFSET	12
#endif
#ifndef ETH_HLEN
#define ETH_HLEN		14
#endif
#ifndef ETH_P_IP
#define ETH_P_IP		0x0800
#endif
#ifndef ETH_P_1X
#define ETH_P_1X		0x888E	/* EAPOL: security frame (802.1X) */
#endif
#ifndef ETH_P_PRE_1X
#define ETH_P_PRE_1X		0x88C7
#endif
#ifndef PARAM_MAC_ADDR_LEN
#define PARAM_MAC_ADDR_LEN	6
#endif
#ifndef USER_PRIORITY_DEFAULT
#define USER_PRIORITY_DEFAULT	0
#endif

/* mascaras del header IP para extraer version y precedencia (DSCP) del TOS.
 * En el stock vienen de nic/mac.h; guarded por si este TU no lo arrastra. */
#ifndef IPVH_VERSION_MASK
#define IPVH_VERSION_MASK	0xF0
#endif
#ifndef IPVH_VERSION_OFFSET
#define IPVH_VERSION_OFFSET	4
#endif
#ifndef IPTOS_PREC_MASK
#define IPTOS_PREC_MASK		0xE0
#endif
#ifndef IPTOS_PREC_OFFSET
#define IPTOS_PREC_OFFSET	5
#endif
#ifndef IPVERSION
#define IPVERSION		4
#endif

/* ==========================================================================
 *  MEMORIA / PAQUETE
 * ========================================================================== */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Reservar un skb para el buffer coalescente (driver-own).
 *
 * \param[in]  prGlueInfo   GLUE Data Structure
 * \param[in]  u4Size       tamano en bytes del skb
 * \param[out] ppucData     recibe el puntero a skb->data
 *
 * \return skb (PVOID) o NULL si falla la reserva.
 */
/*----------------------------------------------------------------------------*/
PVOID
kalPacketAlloc(
	IN  P_GLUE_INFO_T	prGlueInfo,
	IN  UINT_32		u4Size,
	OUT PUINT_8		*ppucData
	)
{
	struct sk_buff *prSkb;

	(void)prGlueInfo;

	prSkb = dev_alloc_skb((unsigned int)u4Size);
	if (prSkb == NULL)
		return NULL;

	if (ppucData != NULL)
		*ppucData = (PUINT_8)prSkb->data;

	return (PVOID)prSkb;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Liberar un skb.
 */
/*----------------------------------------------------------------------------*/
VOID
kalPacketFree(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PVOID		pvPacket
	)
{
	(void)prGlueInfo;

	if (pvPacket != NULL)
		dev_kfree_skb((struct sk_buff *)pvPacket);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Preparar un skb ya reservado para entregar el paquete de RX a la pila.
 *
 * Re-apunta data/tail/len del skb al buffer que el HIF acaba de rellenar con
 * la MSDU. NO llama a netif_rx (eso lo hace kalRxIndicatePkts en gl_kal_mem's
 * hermano). En 7.0.12 skb->tail/end son offsets (sk_buff_data_t) cuando
 * NET_SKBUFF_DATA_USES_OFFSET; por eso usamos skb_reset_tail_pointer + skb_put
 * en vez de aritmetica cruda de punteros (el stock hacia skb->tail=ptr+len,
 * roto en mainline con offsets).
 */
/*----------------------------------------------------------------------------*/
WLAN_STATUS
kalProcessRxPacket(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PVOID		pvPacket,
	IN PUINT_8		pucPacketStart,
	IN UINT_32		u4PacketLen,
	IN BOOLEAN		fgIsRetain,
	IN ENUM_CSUM_RESULT_T	aeCSUM[]
	)
{
	struct sk_buff *skb = (struct sk_buff *)pvPacket;

	(void)prGlueInfo;
	(void)fgIsRetain;

	if (skb == NULL || pucPacketStart == NULL)
		return WLAN_STATUS_FAILURE;

	/* El buffer de RX (pucPacketStart) esta dentro del area lineal del skb
	 * (skb->head..skb->end). Fijar data al inicio de la MSDU y estirar la
	 * cola u4PacketLen bytes usando las primitivas de offset de mainline. */
	skb->data = (unsigned char *)pucPacketStart;
	skb_reset_tail_pointer(skb);
	skb->len = 0;
	skb_put(skb, (unsigned int)u4PacketLen);

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	kalUpdateRxCSUMOffloadParam(skb, aeCSUM);
#else
	(void)aeCSUM;
#endif

	return WLAN_STATUS_SUCCESS;
}

/* ==========================================================================
 *  CLASIFICADOR DE TX  (kalQoSFrameClassifierAndPacketInfo)
 *
 *  CLAVE DEL FIX DE CIFRADO: aqui se detecta el ethertype EAPOL (0x888E) y se
 *  marca *pfgIs1X = TRUE. Ese flag hace que nic_tx.c encole la trama como
 *  COMMAND_TYPE_SECURITY_FRAME (ruta de comando, sin cifrar por el datapath),
 *  que es exactamente la ruta que el 4-way handshake necesita. Sin esta marca,
 *  el EAPOL iria por el datapath normal y el FW lo intentaria cifrar con una
 *  PTK aun no instalada.
 * ========================================================================== */
BOOL
kalQoSFrameClassifierAndPacketInfo(
	IN  P_GLUE_INFO_T	prGlueInfo,
	IN  P_NATIVE_PACKET	prPacket,
	OUT PUINT_8		pucPriorityParam,
	OUT PUINT_32		pu4PacketLen,
	OUT PUINT_8		pucEthDestAddr,
	OUT PBOOLEAN		pfgIs1X,
	OUT PBOOLEAN		pfgIsPAL,
	OUT PUINT_8		pucNetworkType
	)
{
	struct sk_buff *prSkb = (struct sk_buff *)prPacket;
	PUINT_8 aucLookAheadBuf;
	UINT_32 u4PacketLen;
	UINT_16 u2EtherTypeLen;
	UINT_8 ucUserPriority = USER_PRIORITY_DEFAULT;

	if (prSkb == NULL || prSkb->data == NULL)
		return FALSE;

	u4PacketLen = prSkb->len;
	if (u4PacketLen < ETH_HLEN)
		return FALSE;

	aucLookAheadBuf = prSkb->data;

	*pfgIs1X  = FALSE;
	*pfgIsPAL = FALSE;	/* BOW deshabilitado en este port; siempre FALSE */

	u2EtherTypeLen = (UINT_16)((aucLookAheadBuf[ETH_TYPE_LEN_OFFSET] << 8) |
				   (aucLookAheadBuf[ETH_TYPE_LEN_OFFSET + 1]));

	if (u2EtherTypeLen == ETH_P_IP && u4PacketLen >= (ETH_HLEN + 20)) {
		/* IPv4: derivar la UP WMM de la precedencia del TOS/DSCP */
		PUINT_8 pucIpHdr = &aucLookAheadBuf[ETH_HLEN];
		UINT_8 ucIpVersion =
			(pucIpHdr[0] & IPVH_VERSION_MASK) >> IPVH_VERSION_OFFSET;

		if (ucIpVersion == IPVERSION) {
			UINT_8 ucIpTos = pucIpHdr[1];

			ucUserPriority =
				(ucIpTos & IPTOS_PREC_MASK) >> IPTOS_PREC_OFFSET;
		}
	} else if (u2EtherTypeLen == ETH_P_1X) {
		/* EAPOL / 802.1X -> trama de seguridad (fix del cifrado) */
		*pfgIs1X = TRUE;
	} else if (u2EtherTypeLen == ETH_P_PRE_1X) {
		*pfgIs1X = TRUE;
	}
#if CFG_SUPPORT_WAPI
	else if (u2EtherTypeLen == ETH_WPI_1X) {
		*pfgIs1X = TRUE;
	}
#endif

	*pucPriorityParam = ucUserPriority;
	*pu4PacketLen = u4PacketLen;

	/* DA = primeros 6 bytes de la trama Ethernet */
	memcpy(pucEthDestAddr, aucLookAheadBuf, PARAM_MAC_ADDR_LEN);

	/* STA-only: sin BOW ni P2P, siempre la red AIS */
	(void)prGlueInfo;
	*pucNetworkType = NETWORK_TYPE_AIS_INDEX;

	return TRUE;
}

/* ==========================================================================
 *  COMPLETADO DE TX
 * ========================================================================== */

/*----------------------------------------------------------------------------*/
/*!
 * \brief El HIF termino de enviar 'pvPacket'; liberar el skb, decrementar los
 *        contadores de pendientes y reanudar las colas netdev si procede.
 *
 * En 7.0.12 se usa netif_wake_subqueue / netif_tx_wake_all_queues (netif_wake_queue
 * sigue valido para 1 cola). Como este port es STA-only con una sola AIS netdev,
 * reanudamos todas las subcolas del prDevHandler.
 */
/*----------------------------------------------------------------------------*/
VOID
kalSendCompleteAndAwakeQueue(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PVOID		pvPacket
	)
{
	struct sk_buff *prSkb = (struct sk_buff *)pvPacket;
	struct net_device *prDev;

	if (prSkb == NULL)
		return;

	GLUE_DEC_REF_CNT(prGlueInfo->i4TxPendingFrameNum);

	prDev = prSkb->dev;
	if (prDev == NULL)
		prDev = prGlueInfo->prDevHandler;

	dev_kfree_skb(prSkb);

	/* Reanudar la(s) cola(s) de TX del netdev si estaban paradas por backpressure. */
	if (prDev != NULL && netif_running(prDev)) {
		if (netif_queue_stopped(prDev))
			netif_tx_wake_all_queues(prDev);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Completado especifico de una trama de seguridad (EAPOL) encolada por
 *        la ruta de comando. Libera el skb y decrementa el contador de tramas
 *        de seguridad pendientes.
 */
/*----------------------------------------------------------------------------*/
VOID
kalSecurityFrameSendComplete(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PVOID		pvPacket,
	IN WLAN_STATUS		rStatus
	)
{
	(void)rStatus;

	if (pvPacket == NULL)
		return;

	dev_kfree_skb((struct sk_buff *)pvPacket);
	GLUE_DEC_REF_CNT(prGlueInfo->i4TxPendingSecurityFrameNum);
}

/* ==========================================================================
 *  MAC / CONFIG
 * ========================================================================== */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Escribir en el netdev la MAC reportada por el FW (EVENT_NIC_CAPABILITY /
 *        BASIC_CONFIG). El core la invoca en el bring-up.
 *
 * En 7.0.12 dev_addr es 'const unsigned char *' -> usar dev_addr_set() (o
 * eth_hw_addr_set) en vez de memcpy directo, que rompe -Werror por const.
 */
/*----------------------------------------------------------------------------*/
VOID
kalUpdateMACAddress(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN PUINT_8		pucMacAddr
	)
{
	struct net_device *prDev;

	if (prGlueInfo == NULL || pucMacAddr == NULL)
		return;

	prDev = prGlueInfo->prDevHandler;
	if (prDev == NULL)
		return;

	if (!ether_addr_equal(prDev->dev_addr, pucMacAddr))
		eth_hw_addr_set(prDev, pucMacAddr);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Obtener la MAC "de fabrica" del interfaz.
 *
 * En este port NO hay NVRAM (fgNvramAvailable = FALSE), asi que la lectura via
 * kalCfgDataRead16 (que hace el stock) fallaria. Modelo de este port:
 *   - Si el usuario forzo una MAC (fgIsMacAddrOverride) -> copiarla.
 *   - Si no -> devolver FALSE. El core entonces conserva la MAC que el FW ya
 *     reporto por kalUpdateMACAddress (BASIC_CONFIG), que es lo deseado.
 */
/*----------------------------------------------------------------------------*/
BOOLEAN
kalRetrieveNetworkAddress(
	IN     P_GLUE_INFO_T		prGlueInfo,
	IN OUT PARAM_MAC_ADDRESS	*prMacAddr
	)
{
	if (prGlueInfo == NULL || prMacAddr == NULL)
		return FALSE;

	if (prGlueInfo->fgIsMacAddrOverride) {
		COPY_MAC_ADDR(prMacAddr, prGlueInfo->rMacAddrOverride);
		return TRUE;
	}

	/* Sin NVRAM ni override: dejar la MAC del BASIC_CONFIG del FW en pie. */
	return FALSE;
}

/* ==========================================================================
 *  STUBS (config/estado ausentes en este port STA-only sin NVRAM)
 * ========================================================================== */

/* La tarjeta es un slave AHB soldado: nunca "se retira". */
BOOLEAN
kalIsCardRemoved(
	IN P_GLUE_INFO_T	prGlueInfo
	)
{
	(void)prGlueInfo;
	return FALSE;
}

#if !defined(CFG_CHIP_RESET_SUPPORT) || (CFG_CHIP_RESET_SUPPORT == 0)
/* CFG_CHIP_RESET_SUPPORT=0: no hay maquina de reset del chip -> nunca reseteando.
 * (Si el header stock ya declaro/definio kalIsResetting bajo el macro, este
 *  bloque queda cubierto por el mismo guard y no colisiona.) */
BOOLEAN
kalIsResetting(
	VOID
	)
{
	return FALSE;
}
#endif /* !CFG_CHIP_RESET_SUPPORT */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Leer 16 bits de la NVRAM/cfg. No hay NVRAM en este port -> fallo.
 *        Se marca fgNvramAvailable = FALSE para que el core no reintente.
 */
/*----------------------------------------------------------------------------*/
BOOLEAN
kalCfgDataRead16(
	IN  P_GLUE_INFO_T	prGlueInfo,
	IN  UINT_32		u4Offset,
	OUT PUINT_16		pu2Data
	)
{
	(void)u4Offset;

	if (prGlueInfo != NULL)
		prGlueInfo->fgNvramAvailable = FALSE;

	if (pu2Data != NULL)
		*pu2Data = 0;

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Escribir 16 bits en la NVRAM/cfg. No hay NVRAM -> fallo (no-op).
 */
/*----------------------------------------------------------------------------*/
BOOLEAN
kalCfgDataWrite16(
	IN P_GLUE_INFO_T	prGlueInfo,
	IN UINT_32		u4Offset,
	IN UINT_16		u2Data
	)
{
	(void)u4Offset;
	(void)u2Data;

	if (prGlueInfo != NULL)
		prGlueInfo->fgNvramAvailable = FALSE;

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Version de la configuracion. Sin cfg -> todo 0.
 */
/*----------------------------------------------------------------------------*/
VOID
kalGetConfigurationVersion(
	IN  P_GLUE_INFO_T	prGlueInfo,
	OUT PUINT_16		pu2Part1CfgOwnVersion,
	OUT PUINT_16		pu2Part1CfgPeerVersion,
	OUT PUINT_16		pu2Part2CfgOwnVersion,
	OUT PUINT_16		pu2Part2CfgPeerVersion
	)
{
	(void)prGlueInfo;

	if (pu2Part1CfgOwnVersion)
		*pu2Part1CfgOwnVersion = 0;
	if (pu2Part1CfgPeerVersion)
		*pu2Part1CfgPeerVersion = 0;
	if (pu2Part2CfgOwnVersion)
		*pu2Part2CfgOwnVersion = 0;
	if (pu2Part2CfgPeerVersion)
		*pu2Part2CfgPeerVersion = 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Escritura a fichero de algun OID de debug. No soportada en este port.
 *
 * \return (UINT_32)-EPERM. (set_fs/get_fs eliminados en 7.0.12; si se quisiera
 *         soportar habria que usar kernel_write sobre filp_open, fuera de fase.)
 */
/*----------------------------------------------------------------------------*/
UINT_32
kalWriteToFile(
	IN const PUINT_8	pucPath,
	IN BOOLEAN		fgDoAppend,
	IN PUINT_8		pucData,
	IN UINT_32		u4Size
	)
{
	(void)pucPath;
	(void)fgDoAppend;
	(void)pucData;
	(void)u4Size;

	return (UINT_32)(-EPERM);
}
