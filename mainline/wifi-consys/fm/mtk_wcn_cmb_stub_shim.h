/* mtk_wcn_cmb_stub.h — shim vacío para el driver stock mtk_fm_drv.
 * El stock referencia mtk_wcn_cmb_stub_audio_ctrl pero la única llamada está
 * COMENTADA (mt6627_fm_lib.c:675). El path de audio del FM es HW analógico.
 * Definimos los tipos consys aquí (no depende de fm_typedef.h por orden includes). */
#ifndef _MTK_WCN_CMB_STUB_SHIM_H
#define _MTK_WCN_CMB_STUB_SHIM_H

#ifndef _FM_CONSYS_TYPES
#define _FM_CONSYS_TYPES
typedef signed int INT32;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef void VOID;
#endif

typedef enum {
	CMB_STUB_AIF_0 = 0,
	CMB_STUB_AIF_1,
	CMB_STUB_AIF_2,
	CMB_STUB_AIF_3,
} CMB_STUB_AIF_X;

static inline INT32 mtk_wcn_cmb_stub_audio_ctrl(CMB_STUB_AIF_X aif) { return 0; }

#endif /* _MTK_WCN_CMB_STUB_SHIM_H */
