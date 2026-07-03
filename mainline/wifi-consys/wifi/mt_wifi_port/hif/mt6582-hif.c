// SPDX-License-Identifier: GPL-2.0
/*
 * os/linux/hif/mt6582-hif.c — BACKEND HIF (AHB slave, PIO) del port del mt_wifi a linux-7.0.12.
 *
 * Implementa el contrato de os/linux/glue/hif.h (kalDevRegRead/Write/PortRead/PortWrite +
 * glBusSetIrq/glBusFreeIrq + HifIsFwOwn) EXTRAYENDO las primitivas PIO PROBADAS del driver
 * funcional mt6582-wifi.c:
 *   - wifi_hif_alive()      -> hif_alive()      : guard driver-own via WHLPCR (dominio always-on)
 *                                                + ventana disc_settle post-disconnect.
 *   - wifi_set_driver_own() -> kalDevSetDriverOwn()
 *   - wifi_set_fw_own()     -> kalDevSetFwOwn()
 *   - wifi_hstcr()          -> hif_hstcr()       : HSTCR con erratum WHIER dummy-read.
 *   - wifi_port_{read,write}_pio() con guard WCIR por-palabra -> port_pio_{read,write}().
 *   - el caso Port==MCR_WHISR -> bloque enhance de 88B por HSTCR target #4 (re-arma RX0_DONE).
 *   - wifi_isr()            -> glHifIsr()        : mask WHLPCR INT_EN_CLR + GLUE_FLAG_INT + wake.
 *
 * El core stock accede al chip por las macros HAL_* (<nic/hal.h>), que llaman a estos kalDev*.
 * Todos los offsets y bits vienen de <mt6582-wifi-reg.h>.
 *
 * Modelo de propiedad del HIF (CAUSA RAIZ del brick ciclico, sesion 0624c): mientras el FW es
 * dueno (FW_OWN / power-save), leer/escribir un registro NORMAL (WCIR/WHIER/WRDR0/WTDR0/WRPLR)
 * NO completa en el bus AHB -> hard-lockup mudo -> WDT 31s. WHLPCR esta en el dominio ALWAYS-ON:
 * leerlo NUNCA cuelga. Por eso TODO acceso al data-port pasa por hif_alive() (comprueba WHLPCR).
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>

#include "precomp.h"		/* GLUE_INFO_T completo: rHifInfo, prAdapter, u4Flag, waitq, GLUE_FLAG_* */
#include "hif.h"			/* nuestro contrato (os/linux/glue/hif.h) */

#ifndef HIF_MOD_NAME
#define HIF_MOD_NAME		"mt6582-hif"	/* nombre para request_irq() */
#endif

/* GLUE_FLAG_INT_BIT / GLUE_FLAG_HALT los define el core (gl_os.h). Fallback defensivo por si el
 * orden de include no los trajo aun (mismos valores que el stock gl_os.h). */
#ifndef GLUE_FLAG_INT_BIT
#define GLUE_FLAG_INT_BIT	2
#endif
#ifndef GLUE_FLAG_HALT
#define GLUE_FLAG_HALT		BIT(0)
#endif

#define DRIVER_OWN_POLL		1000	/* iteraciones de poll (x50us = 50ms) para driver-own */

/* helpers de acceso crudo al bloque MCR (== rd()/wr() del driver funcional) */
static inline u32 hifrd(GL_HIF_INFO_T *h, u32 off)
{
	return readl(h->HifRegBaseAddr + off);
}
static inline void hifwr(GL_HIF_INFO_T *h, u32 off, u32 val)
{
	writel(val, h->HifRegBaseAddr + off);
}

/* ======================================================================
 *  Guard driver-own / settle (== wifi_hif_alive del driver funcional).
 *  Devuelve true = es SEGURO tocar registros/puertos normales (tenemos driver-own).
 * ====================================================================== */
static bool hif_alive(GL_HIF_INFO_T *h)
{
	int t;

	/* 0624d: tras un .disconnect el FW REINICIA el MAC ~1s; leer un registro NORMAL en esa
	 * ventana no completa en el AHB (hard-lockup). No tocar el HIF hasta que asiente. */
	if (time_before(jiffies, h->DiscSettleUntil))
		return false;

	if (h->fgHifDead)		/* ya marcado muerto en un acceso previo */
		return false;

	/* WHLPCR es always-on: leerlo nunca cuelga aunque el FW duerma */
	if (hifrd(h, MCR_WHLPCR) & WHLPCR_IS_DRIVER_OWN)
		return true;		/* caso normal: ya somos driver-own */

	/* el FW tomo la propiedad -> pedirla de vuelta (FW_OWN_REQ_CLR) y poll */
	hifwr(h, MCR_WHLPCR, WHLPCR_FW_OWN_REQ_CLR);
	for (t = 0; t < DRIVER_OWN_POLL; t++) {
		if (hifrd(h, MCR_WHLPCR) & WHLPCR_IS_DRIVER_OWN)
			return true;
		udelay(50);
	}

	/* el FW no devuelve la propiedad (dormido sin despertar / muerto) -> abortar el acceso
	 * en vez de colgar el CPU. Marcar el HIF muerto. */
	if (h->Dev)
		dev_err(h->Dev,
			"HIF: driver-own NO recuperado (WHLPCR=0x%08x) -> FW dormido/muerto, abandono PIO\n",
			hifrd(h, MCR_WHLPCR));
	h->fgHifDead = true;
	return false;
}

/* HifIsFwOwn lo provee el CORE STOCK (nic/nic.c, firma P_ADAPTER_T). Nuestro guard interno
 * es hif_alive(), usado directamente dentro de kalDevPortRead/Write; no exportamos otro
 * HifIsFwOwn(P_GLUE_INFO_T) para no chocar con la firma del stock. */

/* pedir driver-own explicito (== wifi_set_driver_own). 0 = conseguido. */
int kalDevSetDriverOwn(P_GLUE_INFO_T prGlueInfo)
{
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;
	int t;

	hifwr(h, MCR_WHLPCR, WHLPCR_FW_OWN_REQ_CLR);
	for (t = 0; t < DRIVER_OWN_POLL; t++) {
		if (hifrd(h, MCR_WHLPCR) & WHLPCR_IS_DRIVER_OWN) {
			h->fgHifDead = false;
			return 0;
		}
		udelay(50);
	}
	if (h->Dev)
		dev_err(h->Dev, "driver-own TIMEOUT (WHLPCR=0x%08x)\n",
			hifrd(h, MCR_WHLPCR));
	return -ETIMEDOUT;
}
EXPORT_SYMBOL(kalDevSetDriverOwn);

void kalDevSetFwOwn(P_GLUE_INFO_T prGlueInfo)
{
	hifwr(&prGlueInfo->rHifInfo, MCR_WHLPCR, WHLPCR_FW_OWN_REQ_SET);
}
EXPORT_SYMBOL(kalDevSetFwOwn);

/* ======================================================================
 *  HSTCR (== wifi_hstcr): programa burst 4DW + puerto destino + nº de bytes ANTES de
 *  CADA acceso al puerto de datos. Es lo que hace avanzar el FIFO del chip.
 *  Erratum "HIF 92B, 4B" (ahb.c:2153): leer WHIER ANTES de HSTCR o las transferencias
 *  cuya longitud cae en el caso malo se corrompen.
 * ====================================================================== */
static void hif_hstcr(GL_HIF_INFO_T *h, u32 target, u32 size)
{
	u32 cnt = (size & 0x3) ? (size + 4) : size;

	(void)hifrd(h, MCR_WHIER);	/* erratum dummy-read (registro no-func0) */

	hifwr(h, MCR_HSTCR,
	      (HIF_BURST_4DW << HSTCR_BURST_OFFSET) |
	      (target << HSTCR_TARGET_OFFSET) |
	      (cnt & HSTCR_TRANS_CNT));
}

/* mapear un offset de PUERTO de datos a su HSTCR target (== la dispatch de kalDevPortRead
 * stock: WRDR0->RXD0, WRDR1->RXD1, WHISR->WHISR). Devuelve -1 si no es un puerto (no arma HSTCR). */
static int port_read_target(u16 port)
{
	switch (port) {
	case MCR_WRDR0:	return HIF_TARGET_RXD0;
	case MCR_WRDR1:	return HIF_TARGET_RXD1;
	case MCR_WHISR:	return HIF_TARGET_WHISR;	/* bloque enhance: re-arma RX0_DONE */
	default:	return -1;
	}
}
static int port_write_target(u16 port)
{
	switch (port) {
	case MCR_WTDR0:	return HIF_TARGET_TXD0;
	case MCR_WTDR1:	return HIF_TARGET_TXD1;
	default:	return -1;
	}
}

/* ======================================================================
 *  kalDevRegRead / kalDevRegWrite — acceso de REGISTRO de 32 bits (PIO).
 *  El core los usa para WCIR/WHLPCR/WHCR/WHIER/WRPLR/mailboxes... (via HAL_MCR_RD/WR).
 *  OJO: NO son el puerto de datos; el guard hif_alive lo aplican los callers de puerto,
 *  pero WHLPCR/WCIR son always-on y el core los lee para el propio guard -> aqui NO
 *  metemos hif_alive (evita recursion: hif_alive lee WHLPCR via hifrd, no via kalDevRegRead).
 * ====================================================================== */
BOOL kalDevRegRead(P_GLUE_INFO_T prGlueInfo, UINT_32 u4Register, PUINT_32 pu4Value)
{
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;

	if (unlikely(!pu4Value))
		return false;
	*pu4Value = hifrd(h, u4Register);
	return true;
}
EXPORT_SYMBOL(kalDevRegRead);

BOOL kalDevRegWrite(P_GLUE_INFO_T prGlueInfo, UINT_32 u4Register, UINT_32 u4Value)
{
	hifwr(&prGlueInfo->rHifInfo, u4Register, u4Value);
	return true;
}
EXPORT_SYMBOL(kalDevRegWrite);

/* ======================================================================
 *  kalDevPortRead — leer 'u2Len' bytes del puerto de datos (WRDR0/WRDR1/WHISR) en PIO.
 *  (== wifi_port_read_pio / wifi_port1_read_pio / wifi_read_enhance unificados.)
 *  Guard hif_alive de entrada + re-sondeo WCIR por-palabra (aborta si el core muere a
 *  mitad de burst, en vez de colgar el bus AHB).
 * ====================================================================== */
BOOL kalDevPortRead(P_GLUE_INFO_T prGlueInfo, UINT_16 u2Port, UINT_16 u2Len,
		    PUINT_8 pucBuf, UINT_16 u2ValidOutBufSize)
{
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;
	u32 *p = (u32 *)pucBuf;
	u32 i, words;
	int target;

	if (unlikely(!pucBuf || u2Len > u2ValidOutBufSize))
		return false;

	/* guard: FW-own / muerto / settle -> no tocar el data-port (evita hard-lockup) */
	if (!hif_alive(h)) {
		memset(pucBuf, 0, u2Len);
		return false;
	}

	target = port_read_target(u2Port);
	if (target >= 0)
		hif_hstcr(h, (u32)target, u2Len);	/* avanza el FIFO; para WHISR = read-clear enhance */

	/* DIAG Fase3 (M1): PRE sin POST = cuelga en el readl del puerto (ACK de descarga o enhance). */
	{ static u32 dr; if (dr++ < 12 && h->Dev)
		dev_info(h->Dev, "DIAG portR #%u port=0x%x len=%u tgt=%d PRE\n", dr, u2Port, u2Len, target); }
	words = (u2Len + 3) / 4;
	for (i = 0; i < words; i++) {
		/* audit 0624: re-sondear WCIR (registro estatico, no toca el FIFO) ANTES de cada
		 * palabra: si el core murio a mitad de burst, abortar en vez de colgar el AHB. */
		if ((hifrd(h, MCR_WCIR) & WCIR_CHIP_ID) != WIFI_CHIP_ID_6582) {
			h->fgHifDead = true;
			memset(&p[i], 0, (words - i) * 4);
			return false;
		}
		p[i] = hifrd(h, u2Port);	/* WRDR0/WRDR1/WHISR: stream del bloque */
	}
	{ static u32 drp; if (drp++ < 12 && h->Dev)
		dev_info(h->Dev, "DIAG portR #%u POST (%u palabras) OK\n", drp, words); }
	return true;
}
EXPORT_SYMBOL(kalDevPortRead);

/* ======================================================================
 *  kalDevPortWrite — escribir 'u2Len' bytes al puerto de datos (WTDR0/WTDR1) en PIO.
 *  (== wifi_port_write_pio / wifi_port1_write_pio unificados.)
 * ====================================================================== */
BOOL kalDevPortWrite(P_GLUE_INFO_T prGlueInfo, UINT_16 u2Port, UINT_16 u2Len,
		     PUINT_8 pucBuf, UINT_16 u2ValidInBufSize)
{
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;
	const u32 *p = (const u32 *)pucBuf;
	u32 i, words;
	int target;

	if (unlikely(!pucBuf || u2Len > u2ValidInBufSize))
		return false;

	if (!hif_alive(h))		/* FW-own / muerto / settle: no-op return (guard) */
		return false;

	target = port_write_target(u2Port);
	if (target >= 0)
		hif_hstcr(h, (u32)target, u2Len);

	/* DIAG Fase3 (M1): localizar el cuelgue de la descarga. PRE sin POST = cuelga en el writel.
	 * En la 1a escritura, volcar la config del HIF para comparar con los valores buenos del driver A
	 * (WHCR: MAX_HIF_RX_LEN_NUM bits4-7=0, RX_ENHANCE bit16=0, W_INT_CLR bit1=0). */
	{ static u32 dw; if (dw++ < 12 && h->Dev) {
		if (dw == 1) {
			u32 wcir = hifrd(h, MCR_WCIR);
			/* WCIR_WLAN_READY: si sigue a 1 aqui, el power-cycle NO enfrio el chip (FW vivo) -> la
			 * re-descarga cuelga. =0 => chip frio y el cuelgue es otra cosa. */
			dev_info(h->Dev, "DIAG pre-DL: WCIR=0x%08x (chip=0x%04x WLAN_READY=%d) WHCR=0x%08x WHIER=0x%08x WHLPCR=0x%08x WHISR=0x%08x\n",
				 wcir, (u32)(wcir & 0xffff), !!(wcir & WCIR_WLAN_READY),
				 hifrd(h, MCR_WHCR), hifrd(h, MCR_WHIER), hifrd(h, MCR_WHLPCR), hifrd(h, MCR_WHISR));
		}
		dev_info(h->Dev, "DIAG portW #%u port=0x%x len=%u tgt=%d PRE\n", dw, u2Port, u2Len, target);
	} }
	words = (u2Len + 3) / 4;
	for (i = 0; i < words; i++)
		hifwr(h, u2Port, p[i]);	/* WTDR0/WTDR1 */
	{ static u32 dwp; if (dwp++ < 12 && h->Dev)
		dev_info(h->Dev, "DIAG portW #%u POST (%u palabras) OK\n", dwp, words); }
	return true;
}
EXPORT_SYMBOL(kalDevPortWrite);

/* ======================================================================
 *  ISR del HIF (== HifAhbISR stock + wifi_isr del driver funcional).
 *  Corre en hardirq: enmascara el INT global (WHLPCR always-on -> seguro en hardirq),
 *  marca GLUE_FLAG_INT y despierta el tx_thread unico del core (GlueInfo->waitq).
 *  dev_id == GLUE_INFO_T* (lo pasamos como pvCookie en glBusSetIrq).
 * ====================================================================== */
static irqreturn_t glHifIsr(int irq, void *dev_id)
{
	P_GLUE_INFO_T prGlueInfo = dev_id;
	GL_HIF_INFO_T *h;

	if (unlikely(!prGlueInfo))
		return IRQ_HANDLED;
	h = &prGlueInfo->rHifInfo;

	/* durante el HALT (unload) solo enmascarar y salir (stock ahb.c:1860) */
	if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
		hifwr(h, MCR_WHLPCR, WHLPCR_INT_EN_CLR);
		return IRQ_HANDLED;
	}

	hifwr(h, MCR_WHLPCR, WHLPCR_INT_EN_CLR);	/* enmascarar (always-on, no cuelga) */

	set_bit(GLUE_FLAG_INT_BIT, &prGlueInfo->u4Flag);
	wake_up_interruptible(&prGlueInfo->waitq);	/* despertar el tx_thread unico */
	return IRQ_HANDLED;
}

/* ======================================================================
 *  glBusSetIrq / glBusFreeIrq — firmas del stock hif.h.
 *  pvData   = struct net_device* (para saca GlueInfo de netdev_priv, como el stock).
 *  pfnIsr   = ISR del core (IGNORADO: usamos glHifIsr).
 *  pvCookie = GLUE_INFO_T* (dev_id de request_irq).
 *  El IRQ del HIF es GIC_SPI 184 (platform_get_irq(pdev, 0) del DT). Level-low.
 * ====================================================================== */
int glBusSetIrq(void *pvData, void *pfnIsr, void *pvCookie)
{
	struct net_device *prNetDevice = pvData;
	P_GLUE_INFO_T prGlueInfo = pvCookie;
	GL_HIF_INFO_T *h;
	int ret;

	(void)pfnIsr;		/* usamos glHifIsr, no el ISR del core */

	if (!prGlueInfo || !prNetDevice)
		return -EINVAL;
	h = &prGlueInfo->rHifInfo;

	/* IRQ del DT: AHB_SLAVE_HIF = GIC_SPI 184. Si el DT no lo declara -> irq<=0 -> el core
	 * cae a polling del tx_thread (retro-compatible). */
	h->Irq = platform_get_irq(h->PDev, 0);
	if (h->Irq <= 0) {
		if (h->Dev)
			dev_warn(h->Dev, "sin IRQ del DT (platform_get_irq=%d) -> RX por polling\n",
				 h->Irq);
		h->fgIrqOk = false;
		return h->Irq ? h->Irq : -ENXIO;
	}

	ret = request_irq(h->Irq, glHifIsr, IRQF_TRIGGER_LOW, HIF_MOD_NAME, prGlueInfo);
	if (ret) {
		if (h->Dev)
			dev_err(h->Dev, "request_irq(%d) fallo (%d) -> RX por polling\n",
				h->Irq, ret);
		h->fgIrqOk = false;
		return ret;
	}
	h->fgIrqOk = true;
	h->fgIrqDisabled = false;
	if (h->Dev)
		dev_info(h->Dev, "IRQ %d (AHB_SLAVE_HIF, GIC_SPI 184) registrado -> RX por interrupcion\n",
			 h->Irq);

	/* armar el INT en el HIF (WHLPCR always-on) */
	hifwr(h, MCR_WHLPCR, WHLPCR_INT_EN_SET);
	return 0;
}
EXPORT_SYMBOL(glBusSetIrq);

void glBusFreeIrq(void *pvData, void *pvCookie)
{
	P_GLUE_INFO_T prGlueInfo = pvCookie;
	GL_HIF_INFO_T *h;

	(void)pvData;
	if (!prGlueInfo)
		return;
	h = &prGlueInfo->rHifInfo;

	if (!h->fgIrqOk || h->Irq <= 0)
		return;

	/* enmascarar el INT en el HIF antes de soltar la linea (evita un ultimo disparo) */
	hifwr(h, MCR_WHLPCR, WHLPCR_INT_EN_CLR);

	/* re-habilitar el IRQ si estaba enmascarado en el GIC (equilibra el disable_irq del teardown) */
	if (h->fgIrqDisabled) {
		enable_irq(h->Irq);
		h->fgIrqDisabled = false;
	}
	free_irq(h->Irq, prGlueInfo);
	h->fgIrqOk = false;
}
EXPORT_SYMBOL(glBusFreeIrq);

/* ======================================================================
 *  Enmascarar/re-armar el IRQ en el GIC durante el teardown/settle del disconnect.
 *  (== disable_irq/enable_irq + irq_disabled del driver funcional, 0627.)
 *  Con IRQF_TRIGGER_LOW, si el FW deja la linea HIF asserted-low durante su teardown del MAC,
 *  el GIC re-dispara la ISR en bucle -> tormenta de hardirqs -> lockup -> WDT. Cortar el IRQ
 *  ANTES de entrar en la ventana de settle; re-armar en el proximo .connect.
 *  Las expone el core (gl_cfg80211 .disconnect/.connect) para gobernar la ventana.
 * ====================================================================== */
void glHifIrqMask(P_GLUE_INFO_T prGlueInfo, unsigned int settle_ms)
{
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;

	h->DiscSettleUntil = jiffies + msecs_to_jiffies(settle_ms);
	if (h->fgIrqOk && !h->fgIrqDisabled) {
		disable_irq(h->Irq);	/* SYNC: espera la ISR en vuelo (no toma hif_lock -> sin deadlock) */
		h->fgIrqDisabled = true;
	}
}
EXPORT_SYMBOL(glHifIrqMask);

void glHifIrqUnmask(P_GLUE_INFO_T prGlueInfo)
{
	GL_HIF_INFO_T *h = &prGlueInfo->rHifInfo;

	h->fgHifDead = false;		/* dar otra oportunidad al PIO tras el settle */
	if (h->fgIrqOk && h->fgIrqDisabled) {
		enable_irq(h->Irq);
		h->fgIrqDisabled = false;
	}
	/* re-armar el INT en el HIF */
	hifwr(h, MCR_WHLPCR, WHLPCR_INT_EN_SET);
}
EXPORT_SYMBOL(glHifIrqUnmask);
