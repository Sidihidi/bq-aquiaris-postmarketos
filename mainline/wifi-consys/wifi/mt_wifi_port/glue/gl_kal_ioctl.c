// SPDX-License-Identifier: GPL-2.0
/*
 * os/linux/glue/gl_kal_ioctl.c — SHIM KAL: pipeline OID / completions del port del
 * mt_wifi (MediaTek MT6582 full-MAC) a linux-7.0.12 (postmarketOS / BQ krillin).
 *
 * Reemplaza el tramo kalIoctl/kalOid* de la gl_kal.c Android-3.10-era. Implementa el
 * contrato de os/linux/glue/gl_kal_prototypes.h:
 *
 *      kalIoctl              - bomba OID unica (OidEntry) protegida por g_halt_sem +
 *                             ioctl_sem, con wait_for_completion_TIMEOUT (~2500ms).
 *      kalOidComplete        - el tx_thread la llama al terminar el OID -> despierta al waiter.
 *      kalOidClearance       - libera al waiter si el OID no se llego a completar.
 *      kalOidCmdClearance    - saca el OID pendiente de la rCmdQueue y lo aborta.
 *
 * kalIndicateStatusAndComplete NO vive aqui (va en gl_kal_indicate.c).
 * kalEnqueueCommand y kalClear{Security,Mgmt}Frames* tampoco (thread.c / mem.c).
 *
 * DELTA CRITICO 3.10 -> 7.0.12 (wiphy_lock):
 *   Desde 5.12 los cfg80211 ops (.scan/.connect/.add_key/.disconnect) corren con el
 *   wiphy mutex COGIDO. Este FW cuelga documentadamente en SET_BSS_INFO. El stock hacia
 *   wait_for_completion() SIN timeout: si el FW no completa el OID, el thread que sostiene
 *   el wiphy_lock se bloquea PARA SIEMPRE -> rfkill/ip/wpa_supplicant muertos, el sistema
 *   entero de red se cuelga. Aqui usamos wait_for_completion_timeout(~2500ms): al expirar,
 *   marcamos el timeout, purgamos el OID pendiente de la cmd-queue (kalOidCmdClearance) y
 *   devolvemos WLAN_STATUS_FAILURE, liberando el wiphy_lock.
 */

#include "precomp.h"
#include "gl_kal_prototypes.h"

/*
 * g_halt_sem / g_u4HaltFlag: definidos en gl_init.c (owner del ciclo probe/remove).
 * Los externamos aqui igual que la gl_kal.c stock (gl_kal.c:710-711).
 */
extern struct semaphore g_halt_sem;
extern int              g_u4HaltFlag;

/*
 * fgIsResetting: en el port STA-only CFG_CHIP_RESET_SUPPORT=0, asi que no existe el
 * subsistema de reset (gl_rst esta guard-out). El stock consultaba 'extern BOOLEAN
 * fgIsResetting' al entrar en kalIoctl; aqui lo dejamos como constante 0 local para
 * conservar la forma del chequeo sin arrastrar gl_rst.
 */
#ifndef CFG_CHIP_RESET_SUPPORT
#define CFG_CHIP_RESET_SUPPORT 0
#endif

/*
 * Timeout del wait del OID. ~2.5s: mas que cualquier command-response sano del FW
 * (los OID normales completan en <200ms), pero acotado para NO retener el wiphy_lock
 * indefinidamente si el FW se cuelga en SET_BSS_INFO. MSEC_TO_SYSTIME convierte ms a
 * jiffies; usamos msecs_to_jiffies directo para no depender de la macro del core.
 */
#define KAL_OID_WAIT_TIMEOUT_MS   2500

/*----------------------------------------------------------------------------*/
/*!
 * \brief El tx_thread llama a esta rutina cuando el OID handler termino, para
 *        despertar al hilo bloqueado en kalIoctl y entregarle el estado.
 *
 * \param prGlueInfo         Puntero a la estructura glue
 * \param fgSetQuery         Set (TRUE) o Query (FALSE)
 * \param u4SetQueryInfoLen  Longitud de datos devuelta
 * \param rOidStatus         Estado final del OID
 */
/*----------------------------------------------------------------------------*/
VOID
kalOidComplete(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN BOOLEAN          fgSetQuery,
    IN UINT_32          u4SetQueryInfoLen,
    IN WLAN_STATUS      rOidStatus
    )
{
    ASSERT(prGlueInfo);

    /* quitar el timer de watchdog del OID (armado por wlanoidSetTimeoutCheck) */
    wlanoidClearTimeoutCheck(prGlueInfo->prAdapter);

    /*
     * Publicar la longitud de retorno en el OidEntry para el lado query. El stock
     * no la copiaba aqui (la escribia el propio handler via pu4QryInfoLen), pero
     * mantenerla es inocuo y ayuda a los sets que informan longitud.
     */
    if (prGlueInfo->OidEntry.pu4QryInfoLen != NULL)
        *prGlueInfo->OidEntry.pu4QryInfoLen = u4SetQueryInfoLen;

    prGlueInfo->rPendStatus = rOidStatus;

    /*
     * Marcar completado ANTES del complete(): kalOidClearance mira este flag para
     * no soltar el completion dos veces. Ordenar la escritura del flag antes del
     * complete() (que lleva su propia barrera) es suficiente en este uso.
     */
    prGlueInfo->u4OidCompleteFlag = 1;
    complete(&prGlueInfo->rPendComp);
} /* kalOidComplete */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Libera al hilo bloqueado en kalIoctl cuando el OID NO llego a completarse
 *        por la via normal (p.ej. el tx_thread lo descarto). Idempotente respecto
 *        a kalOidComplete via u4OidCompleteFlag.
 *
 * \param prGlueInfo   Puntero a la estructura glue
 */
/*----------------------------------------------------------------------------*/
VOID
kalOidClearance(
    IN P_GLUE_INFO_T    prGlueInfo
    )
{
    ASSERT(prGlueInfo);

    if (prGlueInfo->u4OidCompleteFlag != 1)
        complete(&prGlueInfo->rPendComp);
} /* kalOidClearance */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Saca el OID pendiente de la cmd-queue y lo aborta (timeout handler o
 *        NOT_ACCEPTED). Se invoca desde kalIoctl al expirar el wait, y desde el
 *        teardown para no dejar comandos OID colgados en la cola.
 *
 * \param prGlueInfo   Puntero a la estructura glue
 */
/*----------------------------------------------------------------------------*/
VOID
kalOidCmdClearance(
    IN P_GLUE_INFO_T    prGlueInfo
    )
{
    P_QUE_T         prCmdQue;
    QUE_T           rTempCmdQue;
    P_QUE_T         prTempCmdQue = &rTempCmdQue;
    P_QUE_ENTRY_T   prQueueEntry = (P_QUE_ENTRY_T)NULL;
    P_CMD_INFO_T    prCmdInfo    = (P_CMD_INFO_T)NULL;

    GLUE_SPIN_LOCK_DECLARATION();

    ASSERT(prGlueInfo);

    prCmdQue = &prGlueInfo->rCmdQueue;

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
    QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

    /* recorrer la copia: el primer entry con fgIsOid es el OID pendiente; el resto
     * se re-encola tal cual. */
    QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
    while (prQueueEntry) {
        if (((P_CMD_INFO_T)prQueueEntry)->fgIsOid) {
            prCmdInfo = (P_CMD_INFO_T)prQueueEntry;
            break;
        } else {
            QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
        }
        QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
    }

    QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);

    if (prCmdInfo) {
        if (prCmdInfo->pfCmdTimeoutHandler) {
            prCmdInfo->pfCmdTimeoutHandler(prGlueInfo->prAdapter, prCmdInfo);
        } else {
            kalOidComplete(prGlueInfo,
                           prCmdInfo->fgSetQuery,
                           0,
                           WLAN_STATUS_NOT_ACCEPTED);
        }

        prGlueInfo->u4OidCompleteFlag = 1;
        cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
    }
} /* kalOidCmdClearance */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Traduce un ioctl linux a un OID y ejecuta el pipeline: compone el
 *        GL_IO_REQ_T unico (OidEntry), agenda el bit OID, despierta al tx_thread
 *        y bloquea con TIMEOUT hasta que el OID complete.
 *
 * \param prGlueInfo     Puntero a la estructura glue
 * \param pfnOidHandler  Handler OID (wlanoidXxx)
 * \param pvInfoBuf      Buffer de datos
 * \param u4InfoBufLen   Longitud del buffer
 * \param fgRead         TRUE si es query
 * \param fgWaitResp     TRUE si hay que esperar respuesta del FW
 * \param fgCmd          TRUE si el OID compone command packet
 * \param fgIsP2pOid     TRUE si es OID de P2P (ignorado: STA-only)
 * \param pu4QryInfoLen  OUT longitud de datos devuelta
 *
 * \retval WLAN_STATUS_SUCCESS / _PENDING resuelto / _FAILURE / _ADAPTER_NOT_READY
 */
/*----------------------------------------------------------------------------*/
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
    )
{
    P_GL_IO_REQ_T   prIoReq = NULL;
    WLAN_STATUS     ret     = WLAN_STATUS_SUCCESS;
    long            lWaitRet;

    ASSERT(prGlueInfo);

#if CFG_CHIP_RESET_SUPPORT
    if (kalIsResetting())
        return WLAN_STATUS_SUCCESS;
#endif

    /* <1> Serializar contra el halt (probe/remove). down_interruptible: un ^C de
     *     wpa_supplicant no debe dejar el semaforo cogido. */
    if (down_interruptible(&g_halt_sem))
        return WLAN_STATUS_FAILURE;

    if (g_u4HaltFlag) {
        up(&g_halt_sem);
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    /* <2> Un solo OID en vuelo: proteger el OidEntry compartido. */
    if (down_interruptible(&prGlueInfo->ioctl_sem)) {
        up(&g_halt_sem);
        return WLAN_STATUS_FAILURE;
    }

    /* <3> Reset del completion antes de reusarlo (el waiter previo ya salio, pero
     *     un timeout anterior pudo dejarlo semi-señalado). */
    reinit_completion(&prGlueInfo->rPendComp);

    /* <4> Componer la peticion en el OidEntry de la capa glue. */
    prIoReq = &(prGlueInfo->OidEntry);
    ASSERT(prIoReq);

    prIoReq->prAdapter     = prGlueInfo->prAdapter;
    prIoReq->pfnOidHandler = pfnOidHandler;
    prIoReq->pvInfoBuf     = pvInfoBuf;
    prIoReq->u4InfoBufLen  = u4InfoBufLen;
    prIoReq->pu4QryInfoLen = pu4QryInfoLen;
    prIoReq->fgRead        = fgRead;
    prIoReq->fgWaitResp    = fgWaitResp;
    prIoReq->rStatus       = WLAN_STATUS_FAILURE;
#if CFG_ENABLE_WIFI_DIRECT
    prIoReq->fgIsP2pOid    = fgIsP2pOid;
#else
    (void)fgIsP2pOid;
#endif

    /* <5> Reset del estado de la OID pendiente. */
    prGlueInfo->rPendStatus       = WLAN_STATUS_FAILURE;
    prGlueInfo->u4OidCompleteFlag = 0;

    /* <6> ¿Usa la cola de comandos? */
    prIoReq->u4Flag = fgCmd;

    /* <7> Agendar el bit OID para el tx_thread. */
    set_bit(GLUE_FLAG_OID_BIT, &prGlueInfo->u4Flag);

    /* <8> Despertar al tx_thread para que arranque el I/O. */
    wake_up_interruptible(&prGlueInfo->waitq);

    /* <9> Bloquear CON TIMEOUT. Ver DELTA wiphy_lock en la cabecera: sin timeout un
     *     cuelgue del FW en SET_BSS_INFO retendria el wiphy_lock para siempre. */
    lWaitRet = wait_for_completion_timeout(&prGlueInfo->rPendComp,
                                           msecs_to_jiffies(KAL_OID_WAIT_TIMEOUT_MS));

    if (lWaitRet > 0) {
        /* Caso 1: completado a tiempo. Si el handler devolvio PENDING, el estado
         * real esta en rPendStatus (lo publico kalOidComplete). */
        if (prIoReq->rStatus == WLAN_STATUS_PENDING)
            ret = prGlueInfo->rPendStatus;
        else
            ret = prIoReq->rStatus;
    } else {
        /*
         * Caso 2: TIMEOUT (lWaitRet == 0). El FW no completo el OID. Purgar el OID
         * pendiente de la cmd-queue para que el tx_thread no lo complete tarde sobre
         * un OidEntry que vamos a liberar, y salir con FAILURE (libera el wiphy_lock).
         */
        DBGLOG(INIT, WARN,
               ("kalIoctl: OID timeout (%u ms), abortando OID pendiente (FW colgado?)\n",
                (UINT_32)KAL_OID_WAIT_TIMEOUT_MS));

        if (fgCmd)
            kalOidCmdClearance(prGlueInfo);

        /* quitar el watchdog del OID por si sigue armado */
        wlanoidClearTimeoutCheck(prGlueInfo->prAdapter);

        /* limpiar el bit OID: el request ya no es valido */
        clear_bit(GLUE_FLAG_OID_BIT, &prGlueInfo->u4Flag);

        ret = WLAN_STATUS_FAILURE;
    }

    up(&prGlueInfo->ioctl_sem);
    up(&g_halt_sem);

    return ret;
} /* kalIoctl */
