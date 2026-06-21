// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-wifi.c — driver WiFi (cfg80211 full-MAC) del combo CONSYS del MT6582.
 *
 * ============================ LA DECISIÓN ARQUITECTÓNICA ============================
 * El WiFi del MT6582 NO usa el enlace BTIF/STP (el que mt6582-btif.c usa para BT y GPS).
 * Tiene su PROPIO bloque HIF estilo-SDIO mapeado en el bus AHB en 0x180F0000 (registros
 * "MCR": WCIR/WHLPCR/WHISR/WTDR0/WRDR0/...) y un canal PDMA propio en 0x11000180.
 * STP/WMT sólo se usa para (a) encender la radio (func_on(WIFI)) y (b) coordinar reset.
 * El firmware WIFI_RAM_CODE se descarga por el PUERTO DE DATOS del HIF (WTDR0) como
 * comandos INIT_CMD_ID_DOWNLOAD_BUF — NO por STP. Ver WIFI-DESIGN.md §0.
 * ===================================================================================
 *
 * Estado de este fichero: SCAFFOLD de la Fase 0 (ver WIFI-ROADMAP.md):
 *   probe -> EPROBE_DEFER hasta mt6582_consys_ready -> mt6582_consys_func_on(WIFI)
 *   -> ioremap(HIF + PDMA) -> driver-own -> leer WCIR(==0x6582) -> descargar firmware
 *   -> INIT_CMD_ID_WIFI_START -> poll WLAN_READY.
 * Disparado por debugfs (igual que el bring-up del btif). Las Fases 1-3 (cmd/event,
 * scan, connect, netdev) están como TODOs marcados.
 *
 * Estilo calcado de mt6582-btif.c: kernel C, tabs, helpers rd()/wr(), dev_info/dev_err.
 */
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/firmware.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/crc32.h>

#include "mt6582-wifi-reg.h"

/* ===== dependencias del resto del CONSYS (ya implementadas) ===== */
/* exportado por mt6582-consys.c: el MCU del CONSYS corre y el chip-id leyó 0x6582 */
extern bool mt6582_consys_ready;

/* Provista (EXPORT_SYMBOL_GPL) por mt6582-btif.c — manda el comando WMT func_on y espera el EVT,
 * thread-safe con el RX-thread del BT. type: 0=BT 1=FM 2=GPS 3=WIFI 4=WMT. */
extern int mt6582_consys_func_on(u8 type);

/* Provista por mt6582-consys.c: enciende/apaga VCN33_WIFI (LDO RF). El OEM lo hace en func_on(WIFI),
 * no al boot del consys; sin el flanco off->on el MAC no afirma WLAN_READY. */
extern int mt6582_consys_wifi_vcn33(bool on);

#define WMTDRV_TYPE_WIFI	3	/* == canal STP WIFI; type-id del func_on para la radio WiFi */

/* nombre del firmware del MAC WiFi en /lib/firmware (contenido = el blob de ~160KB).
 * Downstream: CFG_FW_FILENAME="WIFI_RAM_CODE" con sufijo de chip ("_SOC" para 6582). */
#define WIFI_FW_NAME		"mediatek/mt6582/WIFI_RAM_CODE"

#define DRIVER_OWN_POLL		1000	/* iteraciones de poll para driver-own / ready */
#define DLM_BUF_SZ		(sizeof(struct init_hif_tx_header) + \
				 sizeof(struct init_cmd_download_buf) + CMD_PKT_SIZE_FOR_IMAGE)

struct mt6582_wifi {
	struct device		*dev;
	void __iomem		*hif;		/* registros MCR @ 0x180F0000 */
	void __iomem		*mcu;		/* @ 0x18070000 (chip-id/CPUPCR) */
	void __iomem		*pdma;		/* canal PDMA @ 0x11000180 */
	struct mutex		hif_lock;	/* serializa acceso al puerto HIF */
	struct dentry		*dbg;
	bool			started;	/* firmware corriendo + WLAN_READY */
	u8			cmd_seq;	/* secuencia de comandos init/runtime */
	int			dbg_evt;	/* limita el log de depuración de eventos init */
	u8			*dlm;		/* buffer de descarga (DMA-able) */
	dma_addr_t		dlm_phys;
	/* TODO(Fase 1): struct wireless_dev *wdev; struct net_device *ndev; etc. */
};

static struct mt6582_wifi *g_wifi;

static inline u32 rd(void __iomem *b, u32 o) { return readl(b + o); }
static inline void wr(void __iomem *b, u32 o, u32 v) { writel(v, b + o); }

/* ======================================================================
 *  Capa HIF: propiedad driver-own y (Fase 0) escritura/lectura PIO del puerto.
 *  En Fase 3 esto pasa a PDMA (0x11000180) + IRQ (WHISR). Ver ahb_pdma.c.
 * ====================================================================== */

/* pedir driver-own: escribir FW_OWN_REQ_CLR y poll de IS_DRIVER_OWN (mtreg.h WHLPCR) */
static int wifi_set_driver_own(struct mt6582_wifi *w)
{
	int t;

	wr(w->hif, MCR_WHLPCR, WHLPCR_FW_OWN_REQ_CLR);
	for (t = 0; t < DRIVER_OWN_POLL; t++) {
		if (rd(w->hif, MCR_WHLPCR) & WHLPCR_IS_DRIVER_OWN)
			return 0;
		udelay(50);
	}
	dev_err(w->dev, "driver-own TIMEOUT (WHLPCR=0x%08x)\n", rd(w->hif, MCR_WHLPCR));
	return -ETIMEDOUT;
}

static void wifi_set_fw_own(struct mt6582_wifi *w)
{
	wr(w->hif, MCR_WHLPCR, WHLPCR_FW_OWN_REQ_SET);
}

/* Configura HSTCR antes de un acceso al puerto: burst 4DW + puerto destino + nº de bytes
 * (HifAhbDmaEnhanceModeConf, ahb.c:2155). Es lo que hace avanzar el FIFO del chip. */
static void wifi_hstcr(struct mt6582_wifi *w, u32 target, u32 size)
{
	u32 cnt = (size & 0x3) ? (size + 4) : size;

	wr(w->hif, MCR_HSTCR,
	   (HIF_BURST_4DW << HSTCR_BURST_OFFSET) |
	   (target << HSTCR_TARGET_OFFSET) |
	   (cnt & HSTCR_TRANS_CNT));
}

/* escribir 'len' bytes al puerto de datos TX0 en modo PIO (palabras de 32 bits).
 * TODO(Fase 3): sustituir por PDMA (HifPdmaConfig/Start con Dst=WIFI_HIF_PHYS+MCR_WTDR0). */
static void wifi_port_write_pio(struct mt6582_wifi *w, const void *buf, u32 len)
{
	const u32 *p = buf;
	u32 i, words = (len + 3) / 4;

	wifi_hstcr(w, HIF_TARGET_TXD0, len);
	for (i = 0; i < words; i++)
		wr(w->hif, MCR_WTDR0, p[i]);
}

/* leer 'len' bytes del puerto de datos RX0 en modo PIO.
 * TODO(Fase 1): el flujo real es: poll WHISR.RX0_DONE -> leer WRPLR (longitud) -> leer aquí. */
static void wifi_port_read_pio(struct mt6582_wifi *w, void *buf, u32 len)
{
	u32 *p = buf;
	u32 i, words = (len + 3) / 4;

	wifi_hstcr(w, HIF_TARGET_RXD0, len);
	for (i = 0; i < words; i++)
		p[i] = rd(w->hif, MCR_WRDR0);
}

/* esperar un evento init en RX0 y devolver su 'status' (CMD_RESULT). Versión POLLING simple. */
static int wifi_wait_init_event(struct mt6582_wifi *w, u8 expect_eid, u32 ms)
{
	struct init_wifi_event *ev;
	u8 rxbuf[128];
	u32 wrplr, plen, readlen;
	int loops = ms * 20;	/* el downstream sondea cada 50us (nicRxWaitResponse, nic_rx.c:3359) */

	/* La fase de descarga sondea WRPLR (longitud RX0). El HW añade además un DW (4B) de status
	 * TRAS el paquete: hay que leer ALIGN_4(plen)+4 para drenarlo, o el siguiente evento sale
	 * desplazado 4 bytes. (poll WRPLR de nicRxWaitResponse + DW de nic_rx.c:2526.) */
	while (loops-- > 0) {
		wrplr = rd(w->hif, MCR_WRPLR);
		plen = wrplr & 0xffff;			/* longitud del paquete pendiente en RX0 */
		if (plen == 0) {			/* respuesta aún no lista */
			udelay(50);
			continue;
		}
		readlen = ALIGN(plen, 4);
		if (readlen > sizeof(rxbuf))
			return -EMSGSIZE;
		wifi_port_read_pio(w, rxbuf, readlen);	/* HSTCR (en port_read) avanza el FIFO */
		if (w->dbg_evt < 6) { w->dbg_evt++;
			dev_info(w->dev, "evt WRPLR=0x%08x plen=%u rx=%*ph\n",
				 wrplr, plen, min_t(u32, plen, 16), rxbuf); }
		ev = (struct init_wifi_event *)rxbuf;
		if (ev->eid != expect_eid) {
			dev_warn(w->dev, "init evt EID=0x%x (esperaba 0x%x)\n",
				 ev->eid, expect_eid);
			return -EBADMSG;
		}
		/* para CMD_RESULT, el byte tras la cabecera es ucStatus (0=OK) */
		return rxbuf[sizeof(*ev)];
	}
	return -ETIMEDOUT;
}

/* ======================================================================
 *  Descarga de firmware (Fase 0): WIFI_RAM_CODE por el puerto HIF como
 *  comandos INIT_CMD_ID_DOWNLOAD_BUF. Ver wlan_lib.c:3140 (wlanImageSectionDownload).
 * ====================================================================== */

/* manda UNA sección (troceada en chunks <= CMD_PKT_SIZE_FOR_IMAGE) al destino dado. */
static int wifi_dl_section(struct mt6582_wifi *w, u32 dest, const u8 *data, u32 size)
{
	u32 off;
	int ret;

	for (off = 0; off < size; off += CMD_PKT_SIZE_FOR_IMAGE) {
		u32 chunk = min_t(u32, CMD_PKT_SIZE_FOR_IMAGE, size - off);
		struct init_hif_tx_header *th = (void *)w->dlm;
		struct init_cmd_download_buf *db = (void *)(th + 1);
		u32 total = sizeof(*th) + sizeof(*db) + chunk;

		th->tx_byte_count = cpu_to_le16(total);
		th->ether_type_offset = 0;
		th->cs_flags = 0;
		th->cid = INIT_CMD_ID_DOWNLOAD_BUF;
		th->seq_num = ++w->cmd_seq;
		th->reserved = 0;
		db->address = cpu_to_le32(dest + off);
		db->length = cpu_to_le32(chunk);
		db->crc32 = cpu_to_le32(crc32_le(~0, data + off, chunk) ^ ~0);
		/* ACK + FW CIFRADO: el firmware del MT6582 va cifrado (downstream CFG_ENABLE_FW_ENCRYPTION=1,
		 * wlan_lib.c:3198). Sin este bit el chip rechaza el chunk con status=1 (invalid param). */
		db->data_mode = cpu_to_le32(DOWNLOAD_BUF_ACK_OPTION | DOWNLOAD_BUF_ENCRYPTION_MODE);
		memcpy(db + 1, data + off, chunk);

		mutex_lock(&w->hif_lock);
		wifi_port_write_pio(w, w->dlm, total);
		ret = wifi_wait_init_event(w, INIT_EVENT_ID_CMD_RESULT, 200);
		mutex_unlock(&w->hif_lock);

		if (ret < 0) {
			dev_err(w->dev, "DL chunk @0x%08x: sin ACK (%d)\n", dest + off, ret);
			return ret;
		}
		if (ret != 0) {	/* ucStatus != success */
			dev_err(w->dev, "DL chunk @0x%08x: FW rechazó status=%d\n", dest + off, ret);
			return -EIO;
		}
	}
	return 0;
}

/* arranca el firmware ya descargado: INIT_CMD_ID_WIFI_START + poll WLAN_READY. */
static int wifi_fw_start(struct mt6582_wifi *w)
{
	struct init_hif_tx_header *th = (void *)w->dlm;
	struct init_cmd_wifi_start *st = (void *)(th + 1);
	u32 total = sizeof(*th) + sizeof(*st);
	u32 pc0;
	int t;

	th->tx_byte_count = cpu_to_le16(total);
	th->ether_type_offset = 0;
	th->cs_flags = 0;
	th->cid = INIT_CMD_ID_WIFI_START;
	th->seq_num = ++w->cmd_seq;
	th->reserved = 0;
	st->override = 0;	/* usar la dirección de arranque por defecto del FW */
	st->address = 0;

	mutex_lock(&w->hif_lock);
	wifi_port_write_pio(w, w->dlm, total);
	mutex_unlock(&w->hif_lock);
	/* WIFI_START no lleva ACK (wlanConfigWifiFunc sólo lo envía); se sondea WLAN_READY. */

	pc0 = rd(w->mcu, CONN_MCU_CPUPCR);	/* PC del MCU antes de esperar el ready */
	for (t = 0; t < 500; t++) {	/* ~5 s (downstream: CFG_RESPONSE_POLLING_TIMEOUT x msleep 10) */
		if (rd(w->hif, MCR_WCIR) & WCIR_WLAN_READY)
			return 0;
		msleep(10);
	}
	dev_err(w->dev,
		"WLAN_READY TIMEOUT WCIR=0x%08x WASR=0x%08x WHISR=0x%08x mbox=%08x/%08x/%08x\n",
		rd(w->hif, MCR_WCIR), rd(w->hif, MCR_WASR), rd(w->hif, MCR_WHISR),
		rd(w->hif, MCR_D2HRM0R), rd(w->hif, MCR_D2HRM1R), rd(w->hif, MCR_D2HRM2R));
	{
		u32 a, b, cc;

		a = rd(w->mcu, CONN_MCU_CPUPCR); udelay(60);
		b = rd(w->mcu, CONN_MCU_CPUPCR); udelay(60);
		cc = rd(w->mcu, CONN_MCU_CPUPCR);
		dev_err(w->dev, "CPUPCR pre=0x%08x post=0x%08x/0x%08x/0x%08x (si cambia => MCU ejecutando)\n",
			pc0, a, b, cc);
	}
	return -ETIMEDOUT;
}

/* descarga el fichero completo: parsea cabecera 'MTKW' (seccionado) o lo trata como plano. */
static int wifi_download_firmware(struct mt6582_wifi *w)
{
	const struct firmware *fw;
	const struct firmware_divided_download *hdr;
	int ret, i;

	ret = request_firmware(&fw, WIFI_FW_NAME, w->dev);
	if (ret) {
		dev_err(w->dev, "request_firmware(%s)=%d\n", WIFI_FW_NAME, ret);
		return ret;
	}
	dev_info(w->dev, "firmware %s: %zu bytes, primeros 4: %4ph\n",
		 WIFI_FW_NAME, fw->size, fw->data);

	hdr = (const void *)fw->data;
	if (fw->size > sizeof(*hdr) &&
	    le32_to_cpu(hdr->signature) == MTK_WIFI_SIGNATURE) {
		u32 n = le32_to_cpu(hdr->num_entries);

		dev_info(w->dev, "FW seccionado: %u secciones\n", n);
		for (i = 0; i < n; i++) {
			u32 foff = le32_to_cpu(hdr->section[i].offset);
			u32 flen = le32_to_cpu(hdr->section[i].length);
			u32 dest = le32_to_cpu(hdr->section[i].dest_addr);

			if (foff + flen > fw->size) {
				dev_err(w->dev, "sección %d fuera de rango\n", i);
				ret = -EINVAL;
				goto out;
			}
			dev_info(w->dev, "  sec %d: %u bytes -> 0x%08x\n", i, flen, dest);
			ret = wifi_dl_section(w, dest, fw->data + foff, flen);
			if (ret)
				goto out;
		}
	} else {
		/*
		 * TODO(Fase 0): blob PLANO (sin cabecera 'MTKW'). Hay que averiguar la
		 * dirección destino por defecto del MT6582 (constante en wlan_lib.c, suele
		 * ser el inicio de la RAM de instrucciones del MCU WiFi). Volcar los primeros
		 * bytes del blob real para decidir. De momento abortamos con aviso.
		 */
		dev_err(w->dev, "FW sin firma 'MTKW' (blob plano): falta dirección destino; ver TODO\n");
		ret = -EINVAL;
		goto out;
	}

	ret = wifi_fw_start(w);
	if (!ret)
		dev_info(w->dev, "*** firmware ARRANCADO — WLAN_READY=1 ***\n");
out:
	release_firmware(fw);
	return ret;
}

/* ======================================================================
 *  Bring-up de Fase 0 (disparado por debugfs).
 * ====================================================================== */
static int wifi_bringup(struct mt6582_wifi *w)
{
	u32 wcir;
	int ret;

	if (w->started) {
		dev_info(w->dev, "WiFi ya levantado\n");
		return 0;
	}

	dev_info(w->dev, "=== BRING-UP WiFi (func_on + HIF + firmware) ===\n");

	/* 1. encender la radio WiFi por WMT (vía btif). Deja el HIF @0x180F0000 accesible. */
	ret = mt6582_consys_func_on(WMTDRV_TYPE_WIFI);
	if (ret) {
		dev_err(w->dev, "func_on(WIFI) falló (%d) — ¿btif sin exportar la API?\n", ret);
		return ret;
	}
	dev_info(w->dev, "func_on(WIFI) OK\n");

	/*
	 * 1b. encender VCN33_WIFI AHORA (flanco fresco off->on), imitando HifAhbProbe del OEM.
	 * El consys ya NO lo enciende al boot; el MAC WiFi necesita este flanco coincidente con el
	 * arranque del firmware o nunca afirma WLAN_READY (D2HRM0R=0). Es el FIX del bring-up.
	 */
	ret = mt6582_consys_wifi_vcn33(true);
	if (ret)
		dev_warn(w->dev, "VCN33_WIFI enable=%d (sigo de todas formas)\n", ret);
	else
		dev_info(w->dev, "VCN33_WIFI ON (flanco off->on para reset del MAC)\n");
	usleep_range(2000, 3000);	/* asentar el rail RF antes de tocar el HIF */

	/* 2. ganar driver-own para poder tocar los puertos de datos */
	ret = wifi_set_driver_own(w);
	if (ret)
		return ret;

	/* 3. leer chip-id del HIF — comprobante de que el HIF responde (Riesgo B) */
	wcir = rd(w->hif, MCR_WCIR);
	dev_info(w->dev, "WCIR=0x%08x (chip-id=0x%04x rev=%lu ready=%d)\n",
		 wcir, (u32)(wcir & WCIR_CHIP_ID),
		 (wcir & WCIR_REVISION_ID) >> 16, !!(wcir & WCIR_WLAN_READY));
	if ((wcir & WCIR_CHIP_ID) != WIFI_CHIP_ID_6582 &&
	    (wcir & WCIR_CHIP_ID) != WIFI_CHIP_ID_6572) {
		dev_err(w->dev, "HIF no responde el chip-id esperado — parar y revisar HIFSYS clocks\n");
		return -ENODEV;
	}

	/*
	 * nicInitializeAdapter equiv: el FW espera que el host habilite las interrupciones (WHIER)
	 * antes de arrancar, aunque nosotros sondeemos (downstream nic.c:1328 escribe WHIER_DEFAULT
	 * ANTES de la descarga). Sin esto el handshake de boot del FW no completa → WLAN_READY nunca.
	 */
	wr(w->hif, MCR_WHISR, rd(w->hif, MCR_WHISR));	/* limpiar status pendiente (W1C) */
	wr(w->hif, MCR_WHIER, WHIER_DEFAULT);

	/* 4. descargar el firmware del MAC WiFi y arrancarlo */
	ret = wifi_download_firmware(w);
	if (ret)
		return ret;

	w->started = true;

	/*
	 * ================= TODO Fase 1: cmd/event + wiphy + netdev =================
	 * - wifi_send_cmd(CID, set/query, payload): arma struct wifi_cmd (PKT_TYPE=CMD en
	 *   resource_pkttype_cs bits6-7) + payload, escribe WTDR0; ver nic_tx.c:nicTxCmd.
	 * - RX dispatcher (kthread o IRQ): poll WHISR.RX0_DONE -> WRPLR -> leer WRDR0 ->
	 *   struct hif_rx_header; según (packet_type & HIF_RX_PKT_TYPE_MASK):
	 *     EVENT -> despachar por struct wifi_event.eid
	 *     MGMT  -> cfg80211_inform_bss_frame (beacons del scan)
	 *     DATA  -> netif_rx (Fase 3)
	 * - CMD_ID_GET_NIC_CAPABILITY (query) -> EVENT_ID_NIC_CAPABILITY (lee MAC permanente).
	 * - CMD_ID_BASIC_CONFIG (fija MAC), CMD_ID_SET_DOMAIN_INFO (canales 1-13).
	 * - wiphy_new(&mt6582_wifi_cfg80211_ops, ...); banda 2.4G (14 canales); cipher_suites
	 *   {WEP40,WEP104,TKIP,CCMP,AES_CMAC}; iftype STATION; wiphy_register().
	 * - alloc_netdev/register_netdev "wlan0" + wireless_dev.
	 * Ficheros: gl_cfg80211.c, gl_init.c:737-897/2259-2300, nic_cmd_event.h.
	 *
	 * ================= TODO Fase 2: cfg80211 ops connect/key =================
	 * - .scan    -> CMD_ID_SCAN_REQ_V2 (struct CMD_SCAN_REQ_V2) -> SCAN_DONE.
	 * - .connect -> CMD_ID_INFRASTRUCTURE + CMD_ID_SET_BSS_INFO + activar STA record;
	 *               coreografía: estudiar mgmt/ais_fsm.c:aisFsmRunEventJoinRequest.
	 * - .add_key/.del_key/.set_default_key -> CMD_ID_ADD_REMOVE_KEY (CMD_802_11_KEY).
	 * - eventos CONNECTION_STATUS/ASSOC_INFO -> cfg80211_connect_result/roamed/disconnected.
	 *
	 * ================= TODO Fase 3: data path netdev =================
	 * - ndo_start_xmit: prepend HIF_TX_HEADER_T (PKT_TYPE=DATA) -> WTDR0 (PDMA).
	 * - PDMA real (0x11000180) para TX y RX; IRQ WF_HIF_IRQ (WHISR) + tasklet/NAPI.
	 * - control de flujo TX por TC (WTSR0 / EVENT_ID_TX_DONE). Ver nic_tx.c/ahb_pdma.c.
	 * ===========================================================================
	 */

	dev_info(w->dev, "*** Fase 0 COMPLETA. cfg80211/netdev = TODO (Fases 1-3) ***\n");
	return 0;
}

static ssize_t bringup_write(struct file *f, const char __user *u, size_t n, loff_t *o)
{
	if (g_wifi)
		wifi_bringup(g_wifi);
	return n;
}
static const struct file_operations bringup_fops = {
	.write = bringup_write,
	.owner = THIS_MODULE,
};

/* ======================================================================
 *  probe / remove del platform_driver.
 * ====================================================================== */
static int mt6582_wifi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_wifi *w;

	/* esperar a que el CONSYS esté vivo (igual que el btif) */
	if (!mt6582_consys_ready)
		return -EPROBE_DEFER;

	w = devm_kzalloc(dev, sizeof(*w), GFP_KERNEL);
	if (!w)
		return -ENOMEM;
	w->dev = dev;
	mutex_init(&w->hif_lock);
	dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));

	w->hif = ioremap(WIFI_HIF_PHYS, WIFI_HIF_LEN);
	w->mcu = ioremap(WIFI_MCU_PHYS, WIFI_MCU_LEN);
	w->pdma = ioremap(WIFI_PDMA_PHYS, WIFI_PDMA_LEN);
	if (!w->hif || !w->mcu || !w->pdma)
		return -ENOMEM;

	/* buffer de descarga: DMA-able (en Fase 3 el PDMA escribe desde aquí) */
	w->dlm = dma_alloc_coherent(dev, DLM_BUF_SZ, &w->dlm_phys, GFP_KERNEL);
	if (!w->dlm)
		return -ENOMEM;

	g_wifi = w;
	w->dbg = debugfs_create_dir("mt6582_wifi", NULL);
	debugfs_create_file("bringup", 0200, w->dbg, w, &bringup_fops);
	platform_set_drvdata(pdev, w);

	dev_info(dev, "mapeado HIF@0x%x PDMA@0x%x — echo 1 > /sys/kernel/debug/mt6582_wifi/bringup\n",
		 WIFI_HIF_PHYS, WIFI_PDMA_PHYS);
	dev_info(dev, "(Fase 0: func_on(WIFI) + descarga de WIFI_RAM_CODE. cfg80211 = TODO.)\n");
	return 0;
}

static void mt6582_wifi_remove(struct platform_device *pdev)	/* kernel 7.0.12: remove devuelve void */
{
	struct mt6582_wifi *w = platform_get_drvdata(pdev);

	/* TODO(Fase 1+): unregister_netdev / wiphy_unregister / parar RX-thread o IRQ. */
	if (w->started) {
		wifi_set_fw_own(w);			/* devolver el chip a FW-own */
		mt6582_consys_wifi_vcn33(false);	/* apagar el rail RF del WiFi */
	}
	debugfs_remove_recursive(w->dbg);
	if (w->dlm)
		dma_free_coherent(w->dev, DLM_BUF_SZ, w->dlm, w->dlm_phys);
	if (w->hif)
		iounmap(w->hif);
	if (w->mcu)
		iounmap(w->mcu);
	if (w->pdma)
		iounmap(w->pdma);
	g_wifi = NULL;
}

static const struct of_device_id mt6582_wifi_of_ids[] = {
	{ .compatible = "mediatek,mt6582-wifi" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_wifi_of_ids);

static struct platform_driver mt6582_wifi_driver = {
	.probe = mt6582_wifi_probe,
	.remove = mt6582_wifi_remove,
	.driver = {
		.name = "mt6582-wifi",
		.of_match_table = mt6582_wifi_of_ids,
	},
};
module_platform_driver(mt6582_wifi_driver);

MODULE_DESCRIPTION("MediaTek MT6582 CONSYS WiFi (cfg80211 full-MAC) — scaffold Fase 0");
MODULE_LICENSE("GPL");
MODULE_FIRMWARE(WIFI_FW_NAME);
