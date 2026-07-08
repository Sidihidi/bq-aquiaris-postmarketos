// fm_glue.c — Glue layer entre el driver stock mtk_fm_drv y nuestro CONSYS/btif.
// Implementa las APIs externas que el stock FM espera (stp_exp.h, wmt_exp.h,
// mtk_wcn_cmb_stub.h) traduciéndolas a mt6582-btif.c / mt6582-consys.c.

#include <linux/types.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>

/* === Declaraciones de nuestras funciones (definidas en btif.c/consys.c) === */
extern int mt6582_stp_fm_send(const u8 *data, u32 len);
extern int mt6582_stp_fm_register_rx(void (*cb)(const u8 *data, u32 len));
extern int mt6582_consys_func_on(u8 type);
extern int mt6582_consys_func_off(u8 type);

/* === Tipos del stock (deben coincidir con los shims stp_exp.h/wmt_exp.h) === */
typedef int INT32;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef void VOID;
typedef int MTK_WCN_BOOL;
#define MTK_WCN_BOOL_TRUE  1
#define MTK_WCN_BOOL_FALSE 0

enum fm_wmtdrv_type {
	WMTDRV_TYPE_BT = 0,
	WMTDRV_TYPE_FM = 1,
	WMTDRV_TYPE_GPS = 2,
	WMTDRV_TYPE_WIFI = 3,
	WMTDRV_TYPE_LPBK = 4,
	WMTDRV_TYPE_WMT = 5,
};

enum fm_wmthwver_type {
	WMTHWVER_NONE = 0,
	WMTHWVER_6627 = 0x8a00,
};

enum fm_wmtmsg_type {
	WMTMSG_TYPE_POWER_ON = 0,
	WMTMSG_TYPE_POWER_OFF = 1,
	WMTMSG_TYPE_RESET = 2,
	WMTMSG_TYPE_STP_RDY = 3,
	WMTMSG_TYPE_HW_FUNC_ON = 4,
};

/* Callback de RX asíncrono registrado por el driver FM (firma void(*)(void)
 * según fm_eint.c; el driver drena con mtk_wcn_stp_receive_data). */
static void (*g_fm_event_cb)(void);
static DEFINE_SPINLOCK(g_fm_cb_lock);

/* Buffer de recepción: el driver FM llama a mtk_wcn_stp_receive_data() para
 * drenar los datos que llegaron asíncronamente. */
static DEFINE_KFIFO(g_fm_rx_fifo, u8, 8192);

/* Callback que el btif invoca cuando llega un frame STP canal 1 (FM). */
static void fm_btif_rx_cb(const u8 *data, u32 len)
{
	kfifo_in(&g_fm_rx_fifo, data, min_t(u32, len, kfifo_avail(&g_fm_rx_fifo)));
	/* avisar al driver FM que hay datos (su callback es void(void)) */
	spin_lock(&g_fm_cb_lock);
	if (g_fm_event_cb)
		g_fm_event_cb();
	spin_unlock(&g_fm_cb_lock);
}

/* === API STP (stp_exp.h) === */

INT32 mtk_wcn_stp_send_data(const UINT8 *buffer, const UINT32 length, const UINT8 type)
{
	if (type != 1)
		return -1;
	return mt6582_stp_fm_send(buffer, length);
}
EXPORT_SYMBOL(mtk_wcn_stp_send_data);

INT32 mtk_wcn_stp_receive_data(UINT8 *buffer, UINT32 length, UINT8 type)
{
	unsigned int avail, n;

	if (type != 1)
		return -1;
	avail = kfifo_len(&g_fm_rx_fifo);
	n = min_t(unsigned int, length, avail);
	if (n && kfifo_out(&g_fm_rx_fifo, buffer, n) != n)
		return -1;
	return (INT32)n;
}
EXPORT_SYMBOL(mtk_wcn_stp_receive_data);

INT32 mtk_wcn_stp_register_event_cb(UINT8 type, void (*cb)(void))
{
	if (type != 1)
		return -1;
	spin_lock(&g_fm_cb_lock);
	g_fm_event_cb = cb;
	spin_unlock(&g_fm_cb_lock);
	/* Registrar nuestro callback fijo en el btif */
	mt6582_stp_fm_register_rx(fm_btif_rx_cb);
	return 0;
}
EXPORT_SYMBOL(mtk_wcn_stp_register_event_cb);

/* === API WMT (wmt_exp.h) === */

MTK_WCN_BOOL mtk_wcn_wmt_func_on(enum fm_wmtdrv_type type)
{
	int ret = mt6582_consys_func_on((u8)type);
	return (ret == 0) ? MTK_WCN_BOOL_TRUE : MTK_WCN_BOOL_FALSE;
}
EXPORT_SYMBOL(mtk_wcn_wmt_func_on);

MTK_WCN_BOOL mtk_wcn_wmt_func_off(enum fm_wmtdrv_type type)
{
	int ret = mt6582_consys_func_off((u8)type);
	return (ret == 0) ? MTK_WCN_BOOL_TRUE : MTK_WCN_BOOL_FALSE;
}
EXPORT_SYMBOL(mtk_wcn_wmt_func_off);

INT32 mtk_wcn_wmt_chipid_query(VOID)
{
	return 0x6627;
}
EXPORT_SYMBOL(mtk_wcn_wmt_chipid_query);

enum fm_wmthwver_type mtk_wcn_wmt_hwver_get(VOID)
{
	return WMTHWVER_6627;
}
EXPORT_SYMBOL(mtk_wcn_wmt_hwver_get);

/* Callback de whole-chip-reset — por ahora no-op. */
static void (*g_fm_rst_cb)(enum fm_wmtdrv_type, enum fm_wmtdrv_type,
			   enum fm_wmtmsg_type, void *, unsigned int);

/* mtk_wcn_wmt_msgcb_reg: el stock pasa PF_WMT_CB (5 args). Lo guardamos. */
INT32 mtk_wcn_wmt_msgcb_reg(enum fm_wmtdrv_type type,
			    void (*cb)(enum fm_wmtdrv_type, enum fm_wmtdrv_type,
				       enum fm_wmtmsg_type, void *, unsigned int))
{
	if (type == WMTDRV_TYPE_FM)
		g_fm_rst_cb = cb;
	return 0;
}
EXPORT_SYMBOL(mtk_wcn_wmt_msgcb_reg);

INT32 mtk_wcn_wmt_msgcb_unreg(enum fm_wmtdrv_type type)
{
	if (type == WMTDRV_TYPE_FM)
		g_fm_rst_cb = NULL;
	return 0;
}
EXPORT_SYMBOL(mtk_wcn_wmt_msgcb_unreg);

/* === CMB stub (audio path) — no-op === */
INT32 mtk_wcn_cmb_stub_audio_ctrl(UINT32 aif)
{
	return 0;
}
EXPORT_SYMBOL(mtk_wcn_cmb_stub_audio_ctrl);

MODULE_LICENSE("GPL");
