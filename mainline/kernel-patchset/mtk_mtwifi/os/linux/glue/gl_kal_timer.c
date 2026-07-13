// SPDX-License-Identifier: GPL-2.0
/*
 * os/linux/glue/gl_kal_timer.c — SHIM del temporizador unico del GLUE para el port del
 * MediaTek MT6582 'mt_wifi' full-MAC a linux-7.0.12 (postmarketOS / BQ krillin).
 *
 * Reemplaza la parte "timer" de la gl_kal.c stock (Android 3.10) implementando el contrato
 * de gl_kal_prototypes.h:
 *
 *     kalOsTimerInitialize()  -> timer_setup()          (antes init_timer + .function/.data)
 *     kalSetTimer()           -> timer_delete_sync + mod_timer
 *     kalCancelTimer()        -> timer_delete_sync
 *     kalTimeoutHandler()     -> callback (setea GLUE_FLAG_TIMEOUT + despierta el tx_thread)
 *
 * Este temporizador es el BACKEND del cnm_timer del core stock: el core arma un unico
 * timer_list (GLUE_INFO_T->tickfn) via kalSetTimer(); al expirar, el callback marca
 * GLUE_FLAG_TIMEOUT_BIT y despierta GlueInfo->waitq, con lo que el tx_thread unico recorre
 * cnmTimerDoTimeOutCheck() (ver el bucle principal de gl_kal_thread.c, rama
 * test_and_clear_bit(GLUE_FLAG_TIMEOUT_BIT)).
 *
 * DELTAS 3.10 -> 7.0.12 aplicados aqui:
 *   - init_timer()/.data/.function ELIMINADOS -> timer_setup(&t, cb, 0). El callback ya no
 *     recibe 'unsigned long', recibe 'struct timer_list*'; recuperamos el GLUE_INFO_T con
 *     timer_container_of() (NO from_timer, que se renombro/elimino en mainline reciente).
 *   - del_timer_sync() -> timer_delete_sync().
 *   - mod_timer() sin cambios de firma.
 *
 * COMPAT DE CONTRATO: gl_kal_prototypes.h declara kalTimeoutHandler(unsigned long arg) con la
 * firma stock. La mantenemos definida (por si algun caller del core la referencia directamente),
 * pero al timer core NO le pasamos esa funcion: registramos el trampolin kalTimerTrampoline()
 * con la firma nueva (struct timer_list*), que a su vez delega en el cuerpo comun.
 */

#include "precomp.h"                /* GLUE_INFO_T completo: tickfn, u4Flag, waitq, GLUE_FLAG_* */
#include "gl_kal_prototypes.h"      /* contrato: kalOsTimerInitialize/kalSetTimer/kalCancelTimer */

#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/bitops.h>

/*
 * Valores de flag defensivos: el core (gl_os.h) ya los define; los replicamos con guarda por si
 * el orden de include de una TU suelta no los arrastro. Deben coincidir con gl_os.h stock.
 */
#ifndef GLUE_FLAG_TIMEOUT_BIT
#define GLUE_FLAG_TIMEOUT_BIT       (3)
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief Cuerpo comun del callback del timeout: marca el evento y despierta el tx_thread.
 *
 * \param[in] prGlueInfo Puntero a la estructura GLUE.
 */
/*----------------------------------------------------------------------------*/
static VOID
kalTimerFire(P_GLUE_INFO_T prGlueInfo)
{
    if (!prGlueInfo)
        return;

    /* Notificar al tx_thread unico el evento de timeout (backend del cnm_timer). */
    set_bit(GLUE_FLAG_TIMEOUT_BIT, &prGlueInfo->u4Flag);
    wake_up_interruptible(&prGlueInfo->waitq);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Trampolin del timer core de 7.0.12 (recibe struct timer_list*, no unsigned long).
 *
 * Recupera el GLUE_INFO_T contenedor a partir del miembro 'tickfn' con timer_container_of()
 * (equivalente moderno de container_of para timers; NO usar from_timer).
 *
 * \param[in] t Puntero al timer_list embebido (GLUE_INFO_T->tickfn).
 */
/*----------------------------------------------------------------------------*/
static VOID
kalTimerTrampoline(struct timer_list *t)
{
    P_GLUE_INFO_T prGlueInfo = timer_container_of(prGlueInfo, t, tickfn);

    kalTimerFire(prGlueInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Callback de timeout con la firma STOCK (unsigned long).
 *
 * Se mantiene por compatibilidad con el prototipo declarado en gl_kal_prototypes.h y con
 * cualquier caller del core que lo invoque directamente. NO se registra en el timer core de
 * 7.0.12 (que exige struct timer_list*); ahi se usa kalTimerTrampoline().
 *
 * \param[in] arg (P_GLUE_INFO_T) casteado a unsigned long, como en el driver stock.
 */
/*----------------------------------------------------------------------------*/
VOID
kalTimeoutHandler(unsigned long arg)
{
    kalTimerFire((P_GLUE_INFO_T) arg);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Inicializa el temporizador unico del GLUE.
 *
 * En 7.0.12 no se puede fijar .function/.data a mano: timer_setup() liga el callback y
 * marca el timer como inicializado. El parametro prTimerHandler del contrato stock se ignora
 * a proposito (el callback fijo es kalTimerTrampoline, que ya despierta el tx_thread, el cual
 * ejecuta la logica del cnm_timer); asi respetamos el modelo de "un solo thread" del port.
 *
 * \param[in] prGlueInfo     Estructura GLUE.
 * \param[in] prTimerHandler (ignorado en 7.0.12; ver nota anterior).
 */
/*----------------------------------------------------------------------------*/
VOID
kalOsTimerInitialize(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            prTimerHandler
    )
{
    ASSERT(prGlueInfo);

    (void) prTimerHandler;

    timer_setup(&prGlueInfo->tickfn, kalTimerTrampoline, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief (Re)arma el temporizador a 'u4Interval' ms desde ahora.
 *
 * Firma unificada (P_GLUE_INFO_T, UINT_32)->BOOLEAN, con u4Interval en ms (MSEC_TO_SYSTIME).
 * Usa timer_delete_sync() para cancelar un disparo pendiente y mod_timer() para re-armar; la
 * conversion ms->jiffies replica la del stock (u4Interval * HZ / MSEC_PER_SEC) pero via
 * msecs_to_jiffies(), que es exacta y sin overflow.
 *
 * \param[in] prGlueInfo Estructura GLUE.
 * \param[in] u4Interval Intervalo en milisegundos.
 *
 * \retval TRUE Siempre (el timer queda armado).
 */
/*----------------------------------------------------------------------------*/
BOOLEAN
kalSetTimer(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_32          u4Interval
    )
{
    ASSERT(prGlueInfo);

    /* Cancelar un disparo previo pendiente antes de re-armar (como el stock). */
    timer_delete_sync(&prGlueInfo->tickfn);

    mod_timer(&prGlueInfo->tickfn,
              jiffies + msecs_to_jiffies(MSEC_TO_SYSTIME(u4Interval)));

    return TRUE; /* success */
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Cancela el temporizador y limpia el flag de timeout pendiente.
 *
 * \param[in] prGlueInfo Estructura GLUE.
 *
 * \retval TRUE  El timer estaba armado y se cancelo.
 * \retval FALSE El timer no estaba armado.
 */
/*----------------------------------------------------------------------------*/
BOOLEAN
kalCancelTimer(
    IN P_GLUE_INFO_T    prGlueInfo
    )
{
    ASSERT(prGlueInfo);

    /* Descartar cualquier evento de timeout ya marcado pero no procesado. */
    clear_bit(GLUE_FLAG_TIMEOUT_BIT, &prGlueInfo->u4Flag);

    /* timer_delete_sync() devuelve 1 si desactivo un timer pendiente, 0 si no estaba armado. */
    if (timer_delete_sync(&prGlueInfo->tickfn) > 0)
        return TRUE;

    return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Tick de sistema en milisegundos.
 *
 * gl_kal.h stock ya define kalGetTimeTick() como MACRO (jiffies_to_msecs(jiffies)); solo se
 * provee aqui como funcion si esa macro NO esta presente, para evitar colision de simbolo.
 */
/*----------------------------------------------------------------------------*/
#ifndef kalGetTimeTick
UINT_32
kalGetTimeTick(VOID)
{
    return (UINT_32) jiffies_to_msecs(jiffies);
}
#endif /* kalGetTimeTick */
