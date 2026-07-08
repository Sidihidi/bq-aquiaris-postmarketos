/* stp_exp.h — shim para el driver stock mtk_fm_drv.
 * Declara las APIs STP que el stock FM espera, implementadas en fm_glue.c
 * (que enruta a mt6582-btif.c canal STP 1). Los tipos UINT8/UINT32/INT32/VOID
 * no están en fm_typedef.h del stock (vienen del CONSYS downstream); los
 * definimos aquí para satisfacer las firmas sin tocar el stock. */
#ifndef _STP_EXP_SHIM_H
#define _STP_EXP_SHIM_H

/* Tipos del CONSYS downstream que fm_typedef.h no define. */
#ifndef _FM_CONSYS_TYPES
#define _FM_CONSYS_TYPES
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef signed int INT32;
typedef void VOID;
typedef int MTK_WCN_BOOL;
#define MTK_WCN_BOOL_TRUE  1
#define MTK_WCN_BOOL_FALSE 0
#endif

#define BT_TASK_INDX   0
#define FM_TASK_INDX   1
#define GPS_TASK_INDX  2
#define WIFI_TASK_INDX 3

extern INT32 mtk_wcn_stp_send_data(const UINT8 *buffer, const UINT32 length, const UINT8 type);
extern INT32 mtk_wcn_stp_receive_data(UINT8 *buffer, UINT32 length, UINT8 type);

/* El callback del stock para RX asíncrono (RDS, eventos). */
typedef void (*FM_STP_EVENT_CB)(const UINT8 *data, UINT32 len);
extern INT32 mtk_wcn_stp_register_event_cb(UINT8 type, FM_STP_EVENT_CB cb);

#endif /* _STP_EXP_SHIM_H */
