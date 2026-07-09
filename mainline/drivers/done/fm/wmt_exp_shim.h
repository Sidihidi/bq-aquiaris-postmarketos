/* wmt_exp.h — shim para el driver stock mtk_fm_drv.
 * Declara las APIs WMT que el stock FM espera, implementadas en fm_glue.c. */
#ifndef _WMT_EXP_SHIM_H
#define _WMT_EXP_SHIM_H

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

typedef enum {
	WMTDRV_TYPE_BT = 0,
	WMTDRV_TYPE_FM = 1,
	WMTDRV_TYPE_GPS = 2,
	WMTDRV_TYPE_WIFI = 3,
	WMTDRV_TYPE_LPBK = 4,
} ENUM_WMTDRV_TYPE_T;

typedef enum {
	WMTHWVER_NONE = 0,
	WMTHWVER_6627 = 0x8a00,
} ENUM_WMTHWVER_TYPE_T;

typedef enum {
	WMTMSG_TYPE_POWER_ON = 0,
	WMTMSG_TYPE_POWER_OFF = 1,
	WMTMSG_TYPE_RESET = 2,
	WMTMSG_TYPE_STP_RDY = 3,
	WMTMSG_TYPE_HW_FUNC_ON = 4,
} ENUM_WMTMSG_TYPE_T;

typedef enum {
	WMTRSTMSG_RESET_START = 0x0,
	WMTRSTMSG_RESET_END = 0x1,
	WMTRSTMSG_MAX
} ENUM_WMTRSTMSG_TYPE_T;

extern MTK_WCN_BOOL mtk_wcn_wmt_func_on(ENUM_WMTDRV_TYPE_T type);
extern MTK_WCN_BOOL mtk_wcn_wmt_func_off(ENUM_WMTDRV_TYPE_T type);
extern INT32 mtk_wcn_wmt_chipid_query(VOID);
extern ENUM_WMTHWVER_TYPE_T mtk_wcn_wmt_hwver_get(VOID);

typedef void (*PF_WMT_CB)(ENUM_WMTDRV_TYPE_T, ENUM_WMTDRV_TYPE_T,
			  ENUM_WMTMSG_TYPE_T, VOID *, UINT32);
extern INT32 mtk_wcn_wmt_msgcb_reg(ENUM_WMTDRV_TYPE_T type, PF_WMT_CB cb);
extern INT32 mtk_wcn_wmt_msgcb_unreg(ENUM_WMTDRV_TYPE_T type);

#endif /* _WMT_EXP_SHIM_H */
