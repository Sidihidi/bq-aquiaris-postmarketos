// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-btif.c — BTIF (PIO) + STP framing + 1er comando WMT, para el CONSYS MT6582.
 *
 * ===== FASE STP+WMT (2026-06-18) =====
 * La Fase C+D v2 dejó el TX del BTIF funcionando (resync TX OK) pero RX=0 al resync
 * solo. Para que el CONSYS CONTESTE hay que mandarle un comando WMT real envuelto en
 * STP. Aquí:
 *   1) init BTIF PIO (igual que C+D v2: HANDSHAKE + TRI_LVL, sin IER) + WAK (despierta consys).
 *   2) resync STP (4×0x7F).
 *   3) paquete STP{ type=WMT(0), payload = WMT_QUERY_BAUD } con CRC16.
 *   4) RX-poll ~500 ms y log de los bytes crudos: ¿llega un EVT WMT?  -> M3a.
 *
 * STP (de downstream stp_core.c, conn_soc/mt6582):
 *   hdr[0]=0x80|(seq<<3)|ack ; hdr[1]=(type<<4)|((len>>8)&0x0f) ; hdr[2]=len&0xff ;
 *   hdr[3]=(h0+h1+h2)&0xff ; luego payload ; luego CRC16(payload) little-endian.
 *   Secuencia inicial: seq=0, ack=7. resync = 4×0x7F.
 * WMT QUERY_BAUD = {01 04 01 00 02}.
 */
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define BTIF_RBR	0x00
#define BTIF_THR	0x00
#define BTIF_IER	0x04
#define BTIF_FAKELCR	0x0c
#define BTIF_LSR	0x14
#define BTIF_DMA_EN	0x4c
#define BTIF_TRI_LVL	0x60
#define BTIF_WAK	0x64
#define BTIF_HANDSHAKE	0x6c

#define LSR_DR		(1u << 0)
#define LSR_THRE	(1u << 5)
#define LSR_TEMT	(1u << 6)
#define HANDSHAKE_EN	(1u << 0)
#define WAK_BIT		(1u << 0)
#define DMA_EN_RX	(1u << 0)
#define DMA_EN_TX	(1u << 1)
#define DMA_EN_AUTORST	(1u << 2)
#define TRI_LVL_VAL	(((8u) & 0xf) | (((1u) & 0x7) << 4))
#define TRI_LOOP_EN	(1u << 7)	/* loopback interno (diagnóstico del RX) */

#define PERICFG_PHYS	0x10003000
#define PERICFG_SIZE	0x100
#define PERI_PDN0_CLR	0x10
#define PERI_CG_BTIF	(1u << 20)

#define STP_TYPE_WMT	0	/* task index del canal WMT en STP */
#define BTIF_RX_LOG	128

/* lo pone a true mt6582-consys.c cuando el MCU del CONSYS ya corre el ROM */
extern bool mt6582_consys_ready;

static const u16 crc16_table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static u16 stp_crc16(const u8 *buf, unsigned int len)
{
	u16 crc = 0;

	while (len--)
		crc = (crc >> 8) ^ crc16_table[(crc ^ (*buf++)) & 0xff];
	return crc;
}

struct mt6582_btif {
	struct device *dev;
	void __iomem *base;
	int irq;
	u8 rxlog[BTIF_RX_LOG];
	unsigned int rxcnt;
};

static inline u32 btif_rd(struct mt6582_btif *b, u32 off) { return readl(b->base + off); }
static inline void btif_wr(struct mt6582_btif *b, u32 off, u32 v) { writel(v, b->base + off); }

static void mt6582_btif_clk_ungate(struct device *dev)
{
	void __iomem *peri = ioremap(PERICFG_PHYS, PERICFG_SIZE);

	if (peri) {
		writel(PERI_CG_BTIF, peri + PERI_PDN0_CLR);
		iounmap(peri);
	}
}

static int mt6582_btif_tx(struct mt6582_btif *b, const u8 *buf, size_t len)
{
	size_t i;
	int t;

	for (i = 0; i < len; i++) {
		for (t = 0; t < 100000; t++) {
			if (btif_rd(b, BTIF_LSR) & (LSR_THRE | LSR_TEMT))
				break;
			udelay(1);
		}
		if (t == 100000)
			return -ETIMEDOUT;
		btif_wr(b, BTIF_THR, buf[i]);
	}
	for (t = 0; t < 100000; t++) {
		if (btif_rd(b, BTIF_LSR) & LSR_TEMT)
			break;
		udelay(1);
	}
	return 0;
}

static void mt6582_btif_rx_poll(struct mt6582_btif *b, unsigned int ms)
{
	unsigned int loops = ms * 10;
	u8 c;

	while (loops--) {
		while (btif_rd(b, BTIF_LSR) & LSR_DR) {
			c = btif_rd(b, BTIF_RBR) & 0xff;
			if (b->rxcnt < BTIF_RX_LOG)
				b->rxlog[b->rxcnt] = c;
			b->rxcnt++;
		}
		udelay(100);
	}
}

/* Envía un paquete STP: header(4) + payload(len) + CRC16(payload) little-endian.
 * seq=0, ack=7 (estado inicial de la máquina STP del CONSYS). */
static int mt6582_btif_stp_send(struct mt6582_btif *b, u8 type,
				const u8 *payload, unsigned int len)
{
	u8 pkt[4 + 64 + 2];
	u16 crc;
	unsigned int n = 0;

	if (len > 64)
		return -EINVAL;
	pkt[0] = 0x80 | (0 << 3) | 7;			/* seq=0, ack=7 */
	pkt[1] = (type << 4) | ((len >> 8) & 0x0f);
	pkt[2] = len & 0xff;
	pkt[3] = (pkt[0] + pkt[1] + pkt[2]) & 0xff;
	n = 4;
	memcpy(pkt + n, payload, len);
	n += len;
	crc = stp_crc16(payload, len);
	pkt[n++] = crc & 0xff;
	pkt[n++] = (crc >> 8) & 0xff;
	dev_info(b->dev, "STP TX %u bytes (hdr %02x %02x %02x %02x, crc %04x): %*ph\n",
		 n, pkt[0], pkt[1], pkt[2], pkt[3], crc, min_t(unsigned int, n, 16), pkt);
	return mt6582_btif_tx(b, pkt, n);
}

static void mt6582_btif_hw_init(struct mt6582_btif *b)
{
	u32 v;

	btif_wr(b, BTIF_FAKELCR, 0);
	/* HANDSHAKE ON: ahora que el MCU del CONSYS corre (mt6582-consys.c lo activó), el
	 * "new handshake" se completa y habilita el flujo BIDIRECCIONAL (necesario p/ el RX). */
	v = btif_rd(b, BTIF_HANDSHAKE);
	btif_wr(b, BTIF_HANDSHAKE, v | HANDSHAKE_EN);
	btif_wr(b, BTIF_TRI_LVL, TRI_LVL_VAL);
	v = btif_rd(b, BTIF_DMA_EN);
	v &= ~(DMA_EN_TX | DMA_EN_RX);
	v |= DMA_EN_AUTORST;
	btif_wr(b, BTIF_DMA_EN, v);
	btif_wr(b, BTIF_IER, 0);			/* sin IRQ */
	/* despertar el MCU del CONSYS via WAK (ap_wakeup_consys): pulso clr->set. El MCU
	 * parece dormido (CPUPCR fijo 0x23d8); ahora que el enlace clockea el WAK no bloquea. */
	btif_wr(b, BTIF_WAK, 0);
	udelay(80);
	btif_wr(b, BTIF_WAK, WAK_BIT);
	udelay(80);
	/* (obsoleto) NOTA: el WAK (despertar ap_wakeup_consys) se probó y dejaba LSR=0x0 en el
	 * 2º TX -> fuera por ahora; el CONSYS ya está on (M1). */
}

static int mt6582_btif_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_btif *b;
	static const u8 stp_resync[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
	/* WMT QUERY_BAUD: type=CMD(01) op=04 len=0001 payload=02 */
	static const u8 wmt_query_baud[5] = { 0x01, 0x04, 0x01, 0x00, 0x02 };
	int ret;

	/* esperar a que el CONSYS active su MCU (mt6582-consys.c); sin eso el enlace
	 * BTIF no clockea (el shift TEMT se queda a 0) y el STP no llega. */
	if (!mt6582_consys_ready)
		return -EPROBE_DEFER;

	b = devm_kzalloc(dev, sizeof(*b), GFP_KERNEL);
	if (!b)
		return -ENOMEM;
	b->dev = dev;
	b->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(b->base))
		return PTR_ERR(b->base);
	b->irq = platform_get_irq(pdev, 0);

	mt6582_btif_clk_ungate(dev);
	mt6582_btif_hw_init(b);

	/* --- DIAGNÓSTICO: loopback interno del BTIF. Aísla si el RX del AP funciona,
	 * independientemente del CONSYS. Si recibo lo que envío -> RX OK (el problema es
	 * que el CONSYS no contesta); si RX=0 -> mi config del RX está mal. --- */
	{
		static const u8 lb[4] = { 0xaa, 0xbb, 0xcc, 0xdd };
		u32 tri = btif_rd(b, BTIF_TRI_LVL);

		btif_wr(b, BTIF_TRI_LVL, tri | TRI_LOOP_EN);
		b->rxcnt = 0;
		mt6582_btif_tx(b, lb, sizeof(lb));
		mt6582_btif_rx_poll(b, 50);
		dev_info(dev, "LOOPBACK: envie aa bb cc dd, RX=%u: %*ph -> RX del AP %s\n",
			 b->rxcnt, min_t(unsigned int, b->rxcnt, 8), b->rxlog,
			 b->rxcnt ? "FUNCIONA" : "NO recibe");
		btif_wr(b, BTIF_TRI_LVL, tri);	/* loopback OFF */
		b->rxcnt = 0;
	}

	/* El MCU del CONSYS ya está activo (mt6582-consys.c) y el enlace clockea
	 * (LSR=0x60). Reintentamos resync + QUERY_BAUD varias veces por si el ROM
	 * tarda en estar listo para procesar STP. */
	{
		int attempt;

		for (attempt = 0; attempt < 6; attempt++) {
			b->rxcnt = 0;
			mt6582_btif_tx(b, stp_resync, sizeof(stp_resync));
			usleep_range(2000, 4000);
			ret = mt6582_btif_stp_send(b, STP_TYPE_WMT, wmt_query_baud,
						   sizeof(wmt_query_baud));
			if (ret) {
				dev_err(dev, "STP: WMT TX timeout intento %d (LSR=0x%x)\n",
					attempt, btif_rd(b, BTIF_LSR));
				msleep(20);
				continue;
			}
			mt6582_btif_rx_poll(b, 200);
			dev_info(dev, "STP+WMT intento %d: LSR=0x%x RX=%u: %*ph\n",
				 attempt, btif_rd(b, BTIF_LSR), b->rxcnt,
				 min_t(unsigned int, b->rxcnt, 32), b->rxlog);
			if (b->rxcnt)
				break;
			msleep(50);
		}
	}
	if (b->rxcnt)
		dev_info(dev, "STP+WMT: *** EL CONSYS CONTESTA (%u bytes) — M3a CONSEGUIDO ***\n",
			 b->rxcnt);
	else
		dev_info(dev, "STP+WMT: RX=0 tras 6 intentos (siguiente: WAK / type STP / delimiter)\n");

	platform_set_drvdata(pdev, b);
	return 0;
}

static const struct of_device_id mt6582_btif_of_ids[] = {
	{ .compatible = "mediatek,mt6582-btif" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_btif_of_ids);

static struct platform_driver mt6582_btif_driver = {
	.probe = mt6582_btif_probe,
	.driver = {
		.name = "mt6582-btif",
		.of_match_table = mt6582_btif_of_ids,
	},
};
module_platform_driver(mt6582_btif_driver);

MODULE_DESCRIPTION("MediaTek MT6582 BTIF + STP + WMT QUERY_BAUD (M3a)");
MODULE_LICENSE("GPL");
