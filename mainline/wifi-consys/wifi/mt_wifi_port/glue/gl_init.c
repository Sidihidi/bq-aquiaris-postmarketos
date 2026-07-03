// SPDX-License-Identifier: GPL-2.0
/*
 * gl_init.c — FASE 3: el PROBE REAL del port del mt_wifi (MediaTek MT6582) a
 * linux-7.0.12 (postmarketOS / BQ krillin, armv7, cfg80211 full-MAC, STA-only).
 *
 * ============================ ARQUITECTURA ============================
 * Port HIBRIDO:
 *   [arriba] core stock INTACTO (mgmt/ nic/ common/) dieted STA-only.
 *   [medio]  glue NUEVO 7.0.12 (gl_kal_*.o + ESTE gl_init.o + gl_cfg80211.o).
 *   [abajo]  backend HIF PIO probado (mt6582-hif.o).
 *
 * Este fichero SUSTITUYE al gl_init.c MINIMO (que solo enlazaba con g_u4HaltFlag=1).
 * Reemplaza la capa de PLATAFORMA del stock (glBusInit/glBusFuncOn/SDIO) por el
 * bring-up del CONSYS de nuestro driver A que YA funciona en HW:
 *   probe -> EPROBE_DEFER(!mt6582_consys_ready) -> func_on(WIFI) -> VCN33 always-on
 *         -> ioremap(HIF@0x180F0000 + MCU@0x18070000 + PDMA@0x11000180) en rHifInfo
 *         -> wlanNetCreate (wiphy+netdev+adapter) -> glBusSetIrq (GIC_SPI 184)
 *         -> [FW-download STOCK] wlanAdapterStart(...) <- ARRANCA EL FW (hito M1)
 *         -> MAC via kalIoctl(wlanoidQueryCurrentAddr) -> eth_hw_addr_set
 *         -> wlanNetRegister (wiphy_register + apply_custom_regulatory + register_netdev)
 *         -> u4ReadyFlag=1 ; g_u4HaltFlag=0.
 *
 * PUNTO CLAVE: TODO el bring-up de REGISTROS del HIF (leer chip-id, WHCR/WHIER/WHLPCR,
 * descargar el FW por WTDR0 como INIT_CMD_ID_DOWNLOAD_BUF, WIFI_START, poll WLAN_READY)
 * lo hace el CORE STOCK dentro de wlanAdapterStart() a traves de las macros HAL_* de
 * <nic/hal.h>, que llaman a kalDevRegRead/Write/PortRead/PortWrite (mt6582-hif.c).
 * Por eso ESTE probe NO toca registros directamente: solo enciende el consys, puebla
 * rHifInfo con los ioremaps, y delega en wlanAdapterStart. (En el driver A monolitico
 * ese trabajo lo hacia wifi_bringup()/wifi_download_firmware(); aqui vive en el core.)
 *
 * Deltas 7.0.12 aplicados: eth_hw_addr_set (dev_addr const), NL80211_BAND_2GHZ,
 * wiphy_new(ops,sizeof), alloc_netdev(...,ether_setup), register_netdev,
 * wiphy_apply_custom_regulatory + REGULATORY_CUSTOM_REG, request_firmware (en gl_kal_fw.c),
 * platform_driver.remove devuelve void.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/dma-mapping.h>
#include <net/cfg80211.h>

#include "precomp.h"			/* GLUE_INFO_T/ADAPTER_T completos, wlanAdapterStart/Stop, wlanoid*, QUEUE_* */
#include "gl_kal_prototypes.h"		/* tx_thread, kalIoctl, kalDev*, shims KAL */
#include "hif.h"			/* GL_HIF_INFO_T, glBusSetIrq/glBusFreeIrq, MCR/WIFI_*_PHYS defines */

/* ===================== dependencias del CONSYS (driver A, ya en HW) =====================
 * Exportadas por mt6582-consys.c / mt6582-btif.c. El WiFi comparte el bring-up del combo:
 * STP/WMT solo enciende la radio (func_on) y el rail RF (VCN33); el data-path va por el HIF. */
extern bool mt6582_consys_ready;		/* el MCU del CONSYS corre y leyo chip-id 0x6582 */
extern int  mt6582_consys_func_on(u8 type);	/* WMT func_on: 0=BT 1=FM 2=GPS 3=WIFI 4=WMT */
extern int  mt6582_consys_func_off(u8 type);	/* WMT func_off (power-cycle diagnostico -> chip frio) */
extern int  mt6582_consys_wifi_vcn33(bool on);	/* LDO RF del WiFi (VCN33_WIFI) */

#define WMTDRV_TYPE_WIFI	3		/* type-id del func_on para la radio WiFi */

static void glLoadNvram(P_GLUE_INFO_T prGlueInfo, P_REG_INFO_T prRegInfo);	/* fwd (def. abajo) */

/* ===================== globales del ciclo probe/remove (CONSERVADAS del stub minimo) ===== */
struct semaphore g_halt_sem;
int              g_u4HaltFlag = 1;	/* 1 = halted; el probe lo pone a 0 al terminar el bring-up */
BOOLEAN          fgIsResetting = FALSE;	/* CFG_CHIP_RESET_SUPPORT=0 en el port STA-only */
UINT_8           aucDebugModule[DBG_MODULE_NUM];

/* handle de la instancia (una sola tarjeta) para el .remove */
static struct wireless_dev *g_prWdev;
static struct platform_device *g_pdev;

/* =======================================================================================
 *  TABLAS cfg80211 / netdev (STA-only, 2.4GHz). Portadas del driver A (funcional en HW).
 * ======================================================================================= */
#define CHAN2G(_ch, _freq) { .band = NL80211_BAND_2GHZ, .center_freq = (_freq), \
			     .hw_value = (_ch), .max_power = 20 }
static struct ieee80211_channel mtk_2ghz_channels[] = {
	CHAN2G(1, 2412), CHAN2G(2, 2417), CHAN2G(3, 2422), CHAN2G(4, 2427),
	CHAN2G(5, 2432), CHAN2G(6, 2437), CHAN2G(7, 2442), CHAN2G(8, 2447),
	CHAN2G(9, 2452), CHAN2G(10, 2457), CHAN2G(11, 2462), CHAN2G(12, 2467),
	CHAN2G(13, 2472),
};
static struct ieee80211_rate mtk_rates[] = {
	{ .bitrate = 10 }, { .bitrate = 20 }, { .bitrate = 55 }, { .bitrate = 110 },
	{ .bitrate = 60 }, { .bitrate = 90 }, { .bitrate = 120 }, { .bitrate = 180 },
	{ .bitrate = 240 }, { .bitrate = 360 }, { .bitrate = 480 }, { .bitrate = 540 },
};
static struct ieee80211_supported_band mtk_band_2ghz = {
	.band = NL80211_BAND_2GHZ,
	.channels = mtk_2ghz_channels,
	.n_channels = ARRAY_SIZE(mtk_2ghz_channels),
	.bitrates = mtk_rates,
	.n_bitrates = ARRAY_SIZE(mtk_rates),
};
/* ciphers -> wpa_supplicant habilita WPA2/CCMP (sin esto no asocia a redes cifradas) */
static const u32 mtk_cipher_suites[] = {
	WLAN_CIPHER_SUITE_CCMP,
	WLAN_CIPHER_SUITE_TKIP,
};
/* dominio regulatorio custom (mundo, 2.4G ch1-13) — no depende de regulatory.db */
static const struct ieee80211_regdomain mtk_regd = {
	.n_reg_rules = 1,
	.alpha2 = "99",
	.reg_rules = {
		REG_RULE(2412 - 10, 2472 + 10, 40, 0, 20, NL80211_RRF_NO_IR),
	},
};

/* =======================================================================================
 *  mtk_cfg80211_ops — STUB MINIMO (TODO FASE 4: gl_cfg80211.c).
 *
 *  El gl_cfg80211.c real (con .scan/.connect/.disconnect/.add_key que HABLAN con el FW,
 *  como wifi_cfg_* del driver A) AUN NO EXISTE en el arbol. Para que ESTE fichero enlace
 *  y el probe llegue al hito M1 (insmod -> wlanAdapterStart completa -> el FW arranca),
 *  proveemos aqui un cfg80211_ops MINIMO cuyos handlers devuelven -EOPNOTSUPP. wiphy_new()
 *  solo necesita un ops valido; con esto wlan0 se registra y el FW corre, aunque scan/connect
 *  no funcionen hasta la Fase 4.
 *
 *  >>> Cuando exista gl_cfg80211.c, BORRAR este bloque y hacer `extern struct cfg80211_ops
 *      mtk_cfg80211_ops;` (o el nombre que exporte), y quitar el __weak de abajo. <<<
 * ======================================================================================= */
static int mtk_stub_scan(struct wiphy *wiphy, struct cfg80211_scan_request *req)
{ (void)wiphy; (void)req; return -EOPNOTSUPP; }
static int mtk_stub_connect(struct wiphy *wiphy, struct net_device *ndev,
			    struct cfg80211_connect_params *sme)
{ (void)wiphy; (void)ndev; (void)sme; return -EOPNOTSUPP; }
static int mtk_stub_disconnect(struct wiphy *wiphy, struct net_device *ndev, u16 reason)
{ (void)wiphy; (void)ndev; (void)reason; return -EOPNOTSUPP; }
static int mtk_stub_add_key(struct wiphy *wiphy, struct net_device *ndev, int link_id,
			    u8 idx, bool pairwise, const u8 *mac,
			    struct key_params *params)
{ (void)wiphy; (void)ndev; (void)link_id; (void)idx; (void)pairwise; (void)mac; (void)params;
  return -EOPNOTSUPP; }

/* __weak: si Fase 4 define un mtk_cfg80211_ops NO-weak (mismo nombre) en gl_cfg80211.c,
 * el enlazador se queda con AQUEL y descarta este. Asi no hay que tocar gl_init.c al llegar Fase 4. */
__weak struct cfg80211_ops mtk_cfg80211_ops = {
	.scan       = mtk_stub_scan,
	.connect    = mtk_stub_connect,
	.disconnect = mtk_stub_disconnect,
	.add_key    = mtk_stub_add_key,
};

/* netdev_ops MINIMO: el TX/RX real de datos lo cablea Fase 4 (o el core via wlanHardStartXmit).
 * Para M1 solo hace falta que register_netdev tenga un ndo_open/stop/xmit no-NULL. */
static int mtk_ndo_open(struct net_device *ndev)  { netif_start_queue(ndev); return 0; }
static int mtk_ndo_stop(struct net_device *ndev)  { netif_stop_queue(ndev);  return 0; }
static netdev_tx_t mtk_ndo_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	/* TODO FASE 4: encolar en rTxQueue y despertar el tx_thread (wlanHardStartXmit del core).
	 * Hasta entonces, drop silencioso para no colgar la pila. */
	dev_kfree_skb(skb);
	ndev->stats.tx_dropped++;
	return NETDEV_TX_OK;
}
static const struct net_device_ops mtk_netdev_ops = {
	.ndo_open       = mtk_ndo_open,
	.ndo_stop       = mtk_ndo_stop,
	.ndo_start_xmit = mtk_ndo_xmit,
};

/* =======================================================================================
 *  glSetHifInfo / glClearHifInfo — pueblan rHifInfo (el "abajo" que consume mt6582-hif.c).
 *  Sustituyen a glBusInit/glBusRelease del stock (que hacian SDIO). El ioremap del HIF/MCU/
 *  PDMA + Dev/PDev es lo que glBusSetIrq (platform_get_irq(PDev)) y kalDev* necesitan.
 *  Firmas segun INTEGRATION.md: estas viven en gl_init.c. Aqui ioremapeamos en devm del pdev.
 * ======================================================================================= */
static int glSetHifInfo(P_GLUE_INFO_T prGlueInfo, struct platform_device *pdev)
{
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;
	struct device *dev = &pdev->dev;

	memset(h, 0, sizeof(*h));
	h->Dev  = dev;
	h->PDev = pdev;
	h->fgIntReadClear = true;	/* WHCR read-clear (Exp B del driver A): re-arma RX0_DONE */
	h->fgMbxReadClear = true;
	h->fgDmaEnable = false;		/* backend PIO */
	spin_lock_init(&h->DdmaLock);
	h->DiscSettleUntil = jiffies;	/* != 0 (INITIAL_JIFFIES trampa; ver driver A) */

	/* mapas del bus AHB (mismas bases que el driver A probado) */
	h->HifRegBaseAddr = devm_ioremap(dev, WIFI_HIF_PHYS, WIFI_HIF_LEN);
	h->McuRegBaseAddr = devm_ioremap(dev, WIFI_MCU_PHYS, WIFI_MCU_LEN);
	h->DmaRegBaseAddr = devm_ioremap(dev, WIFI_PDMA_PHYS, WIFI_PDMA_LEN);
	if (!h->HifRegBaseAddr || !h->McuRegBaseAddr || !h->DmaRegBaseAddr) {
		dev_err(dev, "gl_init: ioremap HIF/MCU/PDMA fallo\n");
		return -ENOMEM;
	}
	dev_info(dev, "gl_init: HIF@0x%x MCU@0x%x PDMA@0x%x mapeados en rHifInfo\n",
		 WIFI_HIF_PHYS, WIFI_MCU_PHYS, WIFI_PDMA_PHYS);
	return 0;
}

static void glClearHifInfo(P_GLUE_INFO_T prGlueInfo)
{
	/* los ioremaps son devm_* -> se liberan solos al soltar el pdev. Nada que hacer aqui
	 * salvo dejar el struct consistente. (glClearHifInfo lo llama wlanNetDestroy.) */
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;

	h->HifRegBaseAddr = NULL;
	h->McuRegBaseAddr = NULL;
	h->DmaRegBaseAddr = NULL;
}

/* =======================================================================================
 *  wlanNetCreate — PORTADO del stock (2240) a 7.0.12, dieted STA-only (sin P2P/wext/BOW).
 *  Crea wireless_dev + wiphy(priv=GLUE_INFO_T) + netdev wlan0 + adapter, e inicializa el glue.
 * ======================================================================================= */
static struct lock_class_key rSpinKey[SPIN_LOCK_NUM];

static struct wireless_dev *wlanNetCreate(struct platform_device *pdev)
{
	struct wireless_dev *prWdev = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	struct net_device *prDevHandler = NULL;
	struct device *prDev = &pdev->dev;
	UINT_32 i;

	/* <1.1> wireless_dev */
	prWdev = kzalloc(sizeof(struct wireless_dev), GFP_KERNEL);
	if (!prWdev)
		return NULL;

	/* <1.2> wiphy (private = GLUE_INFO_T entero, idiom stock -> wiphy_priv) */
	prWdev->wiphy = wiphy_new(&mtk_cfg80211_ops, sizeof(GLUE_INFO_T));
	if (!prWdev->wiphy) {
		kfree(prWdev);
		return NULL;
	}
	set_wiphy_dev(prWdev->wiphy, prDev);

	/* <1.4> configurar wireless_dev + wiphy (STA-only, 2.4GHz) */
	prWdev->iftype = NL80211_IFTYPE_STATION;
	prWdev->wiphy->max_scan_ssids  = 1;
	prWdev->wiphy->max_scan_ie_len = 512;
	prWdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
	prWdev->wiphy->bands[NL80211_BAND_2GHZ] = &mtk_band_2ghz;
	prWdev->wiphy->signal_type     = CFG80211_SIGNAL_TYPE_MBM;
	prWdev->wiphy->cipher_suites   = mtk_cipher_suites;
	prWdev->wiphy->n_cipher_suites = ARRAY_SIZE(mtk_cipher_suites);
	prWdev->wiphy->regulatory_flags = REGULATORY_CUSTOM_REG;	/* apply_custom_regulatory en Register */

	/* <2> glue = wiphy_priv */
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);
	if (!prGlueInfo)
		goto netcreate_err;

	/* <3.1> netdev wlan0 (STA-only: alloc_netdev + ether_setup; sin _mq/CFG_MAX_TXQ_NUM) */
	prDevHandler = alloc_netdev(sizeof(P_GLUE_INFO_T), "wlan%d",
				    NET_NAME_UNKNOWN, ether_setup);
	if (!prDevHandler)
		goto netcreate_err;
	prGlueInfo->prDevHandler = prDevHandler;

	/* netdev_priv guarda un P_GLUE_INFO_T* (idiom stock -> doble deref en tx_thread/remove) */
	*((P_GLUE_INFO_T *) netdev_priv(prDevHandler)) = prGlueInfo;
	prDevHandler->netdev_ops = &mtk_netdev_ops;
	netif_carrier_off(prDevHandler);
	netif_tx_stop_all_queues(prDevHandler);

	/* <3.1.2> co-relacion bidireccional con wiphy */
	prDevHandler->ieee80211_ptr = prWdev;
	prWdev->netdev = prDevHandler;
	SET_NETDEV_DEV(prDevHandler, wiphy_dev(prWdev->wiphy));

	/* <3.2> variables del glue */
	prGlueInfo->eParamMediaStateIndicated = PARAM_MEDIA_STATE_DISCONNECTED;
	prGlueInfo->ePowerState = ParamDeviceStateD0;
	prGlueInfo->fgIsMacAddrOverride = FALSE;
	prGlueInfo->fgIsRegistered = FALSE;
	prGlueInfo->prScanRequest = NULL;

	init_completion(&prGlueInfo->rScanComp);
	init_completion(&prGlueInfo->rHaltComp);
	init_completion(&prGlueInfo->rPendComp);	/* kalIoctl (wiphy_lock-safe timeout) */
	init_completion(&prGlueInfo->rChannelReq);

	/* timer del OID-timeout checker (timer_setup en gl_kal_timer.c) */
	kalOsTimerInitialize(prGlueInfo, kalTimeoutHandler);

	for (i = 0; i < SPIN_LOCK_NUM; i++) {
		spin_lock_init(&prGlueInfo->rSpinLock[i]);
		lockdep_set_class(&prGlueInfo->rSpinLock[i], &rSpinKey[i]);
	}
	sema_init(&prGlueInfo->ioctl_sem, 1);
	sema_init(&g_halt_sem, 1);
	g_u4HaltFlag = 0;	/* el stock lo pone aqui; lo re-afirmamos al final del probe */

	/* <4> adapter del core */
	prAdapter = (P_ADAPTER_T) wlanAdapterCreate(prGlueInfo);
	if (!prAdapter)
		goto netcreate_err;
	prGlueInfo->prAdapter = prAdapter;

	return prWdev;

netcreate_err:
	if (prAdapter)
		wlanAdapterDestroy(prAdapter);
	if (prGlueInfo && prGlueInfo->prDevHandler) {
		free_netdev(prGlueInfo->prDevHandler);
		prGlueInfo->prDevHandler = NULL;
	}
	if (prWdev->wiphy)
		wiphy_free(prWdev->wiphy);
	kfree(prWdev);
	return NULL;
}

/* =======================================================================================
 *  wlanNetRegister / wlanNetUnregister / wlanNetDestroy — portados del stock (2119/2178/2438).
 * ======================================================================================= */
static int wlanNetRegister(struct wireless_dev *prWdev)
{
	P_GLUE_INFO_T prGlueInfo;
	int ret;

	if (!prWdev)
		return -EINVAL;
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);

	ret = wiphy_register(prWdev->wiphy);
	if (ret < 0) {
		DBGLOG(INIT, ERROR, ("wlanNetRegister: wiphy_register=%d\n", ret));
		return ret;
	}
	/* custom regdomain DESPUES de wiphy_register (7.0.12) */
	wiphy_apply_custom_regulatory(prWdev->wiphy, &mtk_regd);

	ret = register_netdev(prWdev->netdev);
	if (ret < 0) {
		DBGLOG(INIT, ERROR, ("wlanNetRegister: register_netdev=%d\n", ret));
		wiphy_unregister(prWdev->wiphy);
		return ret;
	}
	prGlueInfo->fgIsRegistered = TRUE;
	return 0;
}

static void wlanNetUnregister(struct wireless_dev *prWdev)
{
	P_GLUE_INFO_T prGlueInfo;

	if (!prWdev)
		return;
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);

	unregister_netdev(prWdev->netdev);
	wiphy_unregister(prWdev->wiphy);
	prGlueInfo->fgIsRegistered = FALSE;
}

static void wlanNetDestroy(struct wireless_dev *prWdev)
{
	P_GLUE_INFO_T prGlueInfo;

	if (!prWdev)
		return;
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);

	kalCancelTimer(prGlueInfo);		/* parar el OID-timeout timer */
	glClearHifInfo(prGlueInfo);

	if (prGlueInfo->prAdapter) {
		wlanAdapterDestroy(prGlueInfo->prAdapter);
		prGlueInfo->prAdapter = NULL;
	}
	/* netdev_priv solo guarda un P_GLUE_INFO_T* (puntero); el GLUE_INFO_T real vive en
	 * wiphy_priv (wiphy_new(ops, sizeof(GLUE_INFO_T))). Por eso free_netdev NO libera el glue. */
	free_netdev(prWdev->netdev);		/* libera netdev + el slot del puntero */
	wiphy_free(prWdev->wiphy);		/* libera wiphy + GLUE_INFO_T (wiphy_priv) */
	kfree(prWdev);
}

/* =======================================================================================
 *  wlanProbe — PORTADO del stock (2913). glBusInit/func_on-SDIO -> consys de nuestro driver A.
 * ======================================================================================= */
static int mtk_wlanProbe(struct platform_device *pdev)
{
	struct wireless_dev *prWdev = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	int i4Status = 0;
	int ret;

	/* <1a> esperar al CONSYS (igual que el btif): sin esto el HIF@0x180F0000 no responde */
	if (!mt6582_consys_ready)
		return -EPROBE_DEFER;

	dma_coerce_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));

	/* <1a.5> DIAG/CHIP-FRIO: el driver A ya arranco el FW al boot (chip CALIENTE) -> wlanAdapterStart
	 * no re-inicia un FW vivo. Forzamos chip FRIO: func_off(WIFI)+VCN33 off, asentar, y el func_on de
	 * abajo lo re-arranca en frio. (El flanco VCN33 off->on pierde la cal-RF -> el scan/TX no iran
	 * finos, pero para el TEST del bring-up del FW da igual; esto es DIAGNOSTICO, se quita luego.) */
	dev_info(&pdev->dev, "DIAG: power-cycle del consys WiFi (func_off+VCN33 off -> frio)\n");
	mt6582_consys_func_off(WMTDRV_TYPE_WIFI);
	mt6582_consys_wifi_vcn33(false);
	msleep(120);

	/* <1b> encender la radio WiFi por WMT (via btif). Deja el HIF accesible. */
	ret = mt6582_consys_func_on(WMTDRV_TYPE_WIFI);
	if (ret) {
		dev_err(&pdev->dev, "func_on(WIFI) fallo (%d)\n", ret);
		return ret;
	}
	/* <1c> VCN33_WIFI SIEMPRE ON: NO conmutar off->on (ese flanco TIRA la cal RF del boot
	 * -> WLAN_READY nunca). Con el rail ya on esto es no-op (refcount++). */
	ret = mt6582_consys_wifi_vcn33(true);
	if (ret)
		dev_warn(&pdev->dev, "VCN33_WIFI enable=%d (sigo)\n", ret);
	usleep_range(2000, 3000);	/* asentar antes de tocar el HIF */

	/* <2> crear netdev/adapter/glue/wiphy (wlanNetCreate portado) */
	prWdev = wlanNetCreate(pdev);
	if (!prWdev) {
		dev_err(&pdev->dev, "wlanProbe: sin memoria para dev/priv\n");
		i4Status = -ENOMEM;
		goto err_funcoff;
	}
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);
	prAdapter  = prGlueInfo->prAdapter;
	g_prWdev = prWdev;
	platform_set_drvdata(pdev, prWdev);

	/* <2.5> setear el ioaddr del HIF (ioremap HIF+MCU+PDMA en rHifInfo). En el stock esto era
	 * glSetHifInfo((UINT_32)pvData); aqui puebla rHifInfo con los mapas del bus AHB + Dev/PDev. */
	ret = glSetHifInfo(prGlueInfo, pdev);
	if (ret) {
		i4Status = ret;
		goto err_netdestroy;
	}

	/* main-thread queues + waitq (stock <2.5>/<3>) */
	init_waitqueue_head(&prGlueInfo->waitq);
	QUEUE_INITIALIZE(&prGlueInfo->rCmdQueue);
	QUEUE_INITIALIZE(&prGlueInfo->rTxQueue);

	prGlueInfo->u4ReadyFlag = 0;

	/* <4> IRQ del HIF (GIC_SPI 184). glBusSetIrq usa rHifInfo.PDev (platform_get_irq) y arma
	 * el INT. Firma stock: glBusSetIrq(netdev, ISR-ignorado, GLUE_INFO_T). Si el DT no declara
	 * 'interrupts' -> el backend cae a polling del tx_thread (retro-compatible), NO es fatal. */
	ret = glBusSetIrq(prWdev->netdev, NULL, prGlueInfo);
	if (ret != 0)
		dev_warn(&pdev->dev, "glBusSetIrq=%d -> RX por polling\n", ret);

	/* <5> ARRANCAR EL FW (hito M1): bloque FW-download STOCK.
	 * wlanAdapterStart hace TODO el bring-up del HIF via HAL_x/kalDevX (chip-id, WHCR/WHIER,
	 * descarga WIFI_RAM_CODE por WTDR0 como INIT_CMD_ID_DOWNLOAD_BUF, WIFI_START, poll WLAN_READY,
	 * BASIC_CONFIG/NIC_CAPABILITY, SET_DOMAIN_INFO). */
#if CFG_ENABLE_FW_DOWNLOAD
	{
		UINT_32 u4FwSize = 0;
		PVOID prFwBuffer = NULL;
		P_REG_INFO_T prRegInfo = &prGlueInfo->rRegInfo;

		kalMemSet(prRegInfo, 0, sizeof(REG_INFO_T));
		prRegInfo->u4StartAddress = CFG_FW_START_ADDRESS;
		prRegInfo->u4LoadAddress  = CFG_FW_LOAD_ADDRESS;

		/* NVRAM: en el port no hay NVRAM -> defaults (glLoadNvram stub deja fgNvramAvailable=FALSE) */
		glLoadNvram(prGlueInfo, prRegInfo);

		prRegInfo->u4PowerMode  = CFG_INIT_POWER_SAVE_PROF;
		prRegInfo->fgEnArpFilter = TRUE;

		if (kalFirmwareImageMapping(prGlueInfo, &prFwBuffer, &u4FwSize) == NULL) {
			dev_err(&pdev->dev, "DIAG: kalFirmwareImageMapping FALLO (fw no mapeado)\n");
			i4Status = -EIO;
		} else {
			WLAN_STATUS st;

			dev_info(&pdev->dev, "DIAG: FW mapeado OK (%u bytes) -> wlanAdapterStart...\n",
				 (unsigned int)u4FwSize);
			st = wlanAdapterStart(prAdapter, prRegInfo, prFwBuffer, u4FwSize);
			dev_info(&pdev->dev, "DIAG: wlanAdapterStart status=0x%08x (0=SUCCESS)\n",
				 (unsigned int)st);
			if (st != WLAN_STATUS_SUCCESS)
				i4Status = -EIO;
			kalFirmwareImageUnmapping(prGlueInfo, NULL, prFwBuffer);
		}
		if (i4Status < 0)
			goto err_freeirq;
	}
#else
	kalMemSet(&prGlueInfo->rRegInfo, 0, sizeof(REG_INFO_T));
	glLoadNvram(prGlueInfo, &prGlueInfo->rRegInfo);
	prGlueInfo->rRegInfo.u4PowerMode = CFG_INIT_POWER_SAVE_PROF;
	if (wlanAdapterStart(prAdapter, &prGlueInfo->rRegInfo, NULL, 0) != WLAN_STATUS_SUCCESS) {
		i4Status = -EIO;
		goto err_freeirq;
	}
#endif

	/* main thread del core (procesa rCmdQueue/rTxQueue + IST despertado por el ISR) */
	prGlueInfo->main_thread = kthread_run(tx_thread, prGlueInfo->prDevHandler, "tx_thread");

	/* <MAC> leer la MAC permanente del FW por OID y ponerla en wlan0.
	 * 7.0.12: dev_addr es const -> eth_hw_addr_set (NO memcpy dev_addr como el stock). */
	{
		WLAN_STATUS rStatus;
		UINT_8 aucMac[PARAM_MAC_ADDR_LEN] = {0};
		UINT_32 u4SetInfoLen = 0;

		rStatus = kalIoctl(prGlueInfo, wlanoidQueryCurrentAddr,
				   aucMac, PARAM_MAC_ADDR_LEN,
				   TRUE, TRUE, TRUE, FALSE, &u4SetInfoLen);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, WARN, ("wlanProbe: query MAC fallo 0x%x\n", (unsigned int)rStatus));
			prGlueInfo->u4ReadyFlag = 0;
		} else {
			eth_hw_addr_set(prGlueInfo->prDevHandler, aucMac);
			prGlueInfo->u4ReadyFlag = 1;
			DBGLOG(INIT, INFO, ("wlanProbe: MAC = %pM\n", aucMac));
		}
	}

	/* <3> registrar la tarjeta (wiphy_register + apply_custom_regulatory + register_netdev) */
	ret = wlanNetRegister(prWdev);
	if (ret < 0) {
		DBGLOG(INIT, ERROR, ("wlanProbe: wlanNetRegister=%d\n", ret));
		i4Status = -ENXIO;
		goto err_adapterstop;
	}

	g_u4HaltFlag = 0;			/* bring-up completo: el driver esta VIVO */
	dev_info(&pdev->dev, "*** mtk_mtwifi: wlan0 ARRIBA (FW arrancado, cfg80211 registrado) ***\n");
	DBGLOG(INIT, INFO, ("wlanProbe ok\n"));
	return 0;

err_adapterstop:
	if (prGlueInfo->main_thread) {
		set_bit(GLUE_FLAG_HALT_BIT, &prGlueInfo->u4Flag);
		wake_up_interruptible(&prGlueInfo->waitq);
		kthread_stop(prGlueInfo->main_thread);
		prGlueInfo->main_thread = NULL;
	}
	wlanAdapterStop(prAdapter);
err_freeirq:
	glBusFreeIrq(prWdev->netdev, prGlueInfo);
err_netdestroy:
	g_prWdev = NULL;
	platform_set_drvdata(pdev, NULL);
	wlanNetDestroy(prWdev);
err_funcoff:
	mt6582_consys_wifi_vcn33(false);
	g_u4HaltFlag = 1;
	return i4Status;
}

/* =======================================================================================
 *  wlanRemove — PORTADO del stock (3232), dieted STA-only.
 * ======================================================================================= */
static void mtk_wlanRemove(struct platform_device *pdev)	/* 7.0.12: remove devuelve void */
{
	struct wireless_dev *prWdev = platform_get_drvdata(pdev);
	P_GLUE_INFO_T prGlueInfo;
	P_ADAPTER_T prAdapter;

	if (!prWdev)
		return;
	prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(prWdev->wiphy);
	prAdapter  = prGlueInfo->prAdapter;

	/* <1> parar el IRQ antes de tocar nada mas */
	glBusFreeIrq(prWdev->netdev, prGlueInfo);

	/* <2> marcar HALT y parar el tx_thread */
	g_u4HaltFlag = 1;
	down(&g_halt_sem);
	set_bit(GLUE_FLAG_HALT_BIT, &prGlueInfo->u4Flag);
	wake_up_interruptible(&prGlueInfo->waitq);
	if (prGlueInfo->main_thread) {
		kthread_stop(prGlueInfo->main_thread);
		prGlueInfo->main_thread = NULL;
	}
	up(&g_halt_sem);

	/* <3> desregistrar wlan0 + wiphy */
	wlanNetUnregister(prWdev);

	/* <4> parar el adapter (devuelve el chip a FW-own dentro del core) */
	wlanAdapterStop(prAdapter);

	/* <5> apagar el rail RF del WiFi */
	mt6582_consys_wifi_vcn33(false);

	/* <6> destruir netdev/adapter/wiphy */
	platform_set_drvdata(pdev, NULL);
	g_prWdev = NULL;
	wlanNetDestroy(prWdev);
}

/* =======================================================================================
 *  glLoadNvram — STUB (el port no tiene NVRAM): defaults seguros en REG_INFO_T.
 *  (El stock leia WIFI_CFG_PARAM_STRUCT via kalCfgDataRead16; aqui fgNvramAvailable=FALSE.)
 * ======================================================================================= */
static void glLoadNvram(P_GLUE_INFO_T prGlueInfo, P_REG_INFO_T prRegInfo)
{
	if (!prGlueInfo || !prRegInfo)
		return;
	/* sin NVRAM: el FW usa su propia calibracion/efuse; no forzamos MAC ni country override. */
	prGlueInfo->fgNvramAvailable = FALSE;
	prRegInfo->u4PowerMode = CFG_INIT_POWER_SAVE_PROF;
}

/* =======================================================================================
 *  platform_driver — misma `compatible` que el driver A ("mediatek,mt6582-wifi").
 * ======================================================================================= */
static const struct of_device_id mtk_mtwifi_of_ids[] = {
	{ .compatible = "mediatek,mt6582-wifi" },
	{ }
};
MODULE_DEVICE_TABLE(of, mtk_mtwifi_of_ids);

static struct platform_driver mtk_mtwifi_driver = {
	.probe  = mtk_wlanProbe,
	.remove = mtk_wlanRemove,
	.driver = {
		.name = "mtk_mtwifi",	/* NO "mt6582-wifi": colisiona con el driver A en sysfs. El match
					 * al nodo DT va por .compatible, no por .name. */
		.of_match_table = mtk_mtwifi_of_ids,
	},
};

/* =======================================================================================
 *  module init/exit — CONSERVAN el sema_init del stub minimo + registran el platform_driver.
 * ======================================================================================= */
static int __init mtk_mtwifi_init(void)
{
	int _i;

	sema_init(&g_halt_sem, 1);
	/* DIAG: activar los DBGLOG del core (ERROR|WARN|STATE|EVENT|TRACE = 0x1F, sin LOUD/TEMP) para ver
	 * el sub-paso EXACTO donde falla wlanAdapterStart (nicInitializeAdapter / Firmware download / ...). */
	for (_i = 0; _i < DBG_MODULE_NUM; _i++)
		aucDebugModule[_i] = 0x1F;
	pr_info("mtk_mtwifi: cargado (Fase 3 — probe real sobre consys MT6582; DBGLOG on)\n");
	return platform_driver_register(&mtk_mtwifi_driver);
}

static void __exit mtk_mtwifi_exit(void)
{
	platform_driver_unregister(&mtk_mtwifi_driver);
	pr_info("mtk_mtwifi: descargado\n");
}

module_init(mtk_mtwifi_init);
module_exit(mtk_mtwifi_exit);

/* ===================== shims KAL que faltaban (12) — CONSERVADOS del stub minimo ========= */
PVOID kalAllocateIOBuffer(IN UINT_32 u4AllocSize)
{
	return kmalloc(u4AllocSize, GFP_KERNEL);	/* PIO: no requiere contiguidad DMA */
}

VOID kalReleaseIOBuffer(IN PVOID pvAddr, IN UINT_32 u4Size)
{
	(void)u4Size;
	kfree(pvAddr);
}

P_REG_INFO_T kalGetConfiguration(IN P_GLUE_INFO_T prGlueInfo)
{
	return &prGlueInfo->rRegInfo;	/* NVRAM stub -> defaults (fgNvramAvailable=FALSE) */
}

BOOLEAN kalIsConfigurationExist(IN P_GLUE_INFO_T prGlueInfo)
{
	(void)prGlueInfo;
	return FALSE;			/* sin NVRAM en el port */
}

VOID kalHandleAssocInfo(IN P_GLUE_INFO_T prGlueInfo, IN P_EVENT_ASSOC_INFO prAssocInfo)
{
	(void)prGlueInfo;
	(void)prAssocInfo;		/* los IEs de assoc se pasan a cfg80211 en connect_bss */
}

VOID kalQueryTxChksumOffloadParam(IN PVOID pvPacket, OUT PUINT_8 pucFlag)
{
	(void)pvPacket;
	if (pucFlag)
		*pucFlag = 0;		/* sin HW checksum offload */
}

VOID kalUpdateRxCSUMOffloadParam(IN PVOID pvPacket, IN ENUM_CSUM_RESULT_T eCSUM[])
{
	(void)pvPacket;
	(void)eCSUM;
}

UINT_32 kalRandomNumber(VOID)
{
	return get_random_u32();
}

VOID kalReadyOnChannel(IN P_GLUE_INFO_T prGlueInfo, IN UINT_64 u8Cookie,
		       IN ENUM_BAND_T eBand, IN ENUM_CHNL_EXT_T eSco,
		       IN UINT_8 ucChannelNum, IN UINT_32 u4DurationMs)
{
	(void)prGlueInfo; (void)u8Cookie; (void)eBand;
	(void)eSco; (void)ucChannelNum; (void)u4DurationMs;	/* remain-on-channel: Fase 4 */
}

VOID kalRemainOnChannelExpired(IN P_GLUE_INFO_T prGlueInfo, IN UINT_64 u8Cookie,
			       IN ENUM_BAND_T eBand, IN ENUM_CHNL_EXT_T eSco,
			       IN UINT_8 ucChannelNum)
{
	(void)prGlueInfo; (void)u8Cookie; (void)eBand; (void)eSco; (void)ucChannelNum;
}

VOID kalUpdateRSSI(IN P_GLUE_INFO_T prGlueInfo, IN ENUM_KAL_NETWORK_TYPE_INDEX_T eNetTypeIdx,
		   IN INT_8 cRssi, IN INT_8 cLinkQuality)
{
	(void)prGlueInfo; (void)eNetTypeIdx; (void)cRssi; (void)cLinkQuality;
}

BOOLEAN kalWSCGetActiveState(IN P_GLUE_INFO_T prGlueInfo)
{
	(void)prGlueInfo;
	return FALSE;			/* WSC/WPS no activo */
}

/* ===================== stubs aaa_fsm (AP-mode, no STA) — CONSERVADOS ===================== */
VOID aaaFsmRunEventRxAuth(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb)
{
	(void)prAdapter; (void)prSwRfb;
}

WLAN_STATUS aaaFsmRunEventRxAssoc(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb)
{
	(void)prAdapter; (void)prSwRfb;
	return WLAN_STATUS_FAILURE;
}

WLAN_STATUS aaaFsmRunEventTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
				 IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	(void)prAdapter; (void)prMsduInfo; (void)rTxDoneStatus;
	return WLAN_STATUS_SUCCESS;
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek MT6582 full-MAC WiFi (port stock mt_wifi -> 7.0.12, STA-only) — Fase 3 probe");
MODULE_AUTHOR("pmOS krillin port");
MODULE_FIRMWARE("mediatek/mt6582/WIFI_RAM_CODE");
