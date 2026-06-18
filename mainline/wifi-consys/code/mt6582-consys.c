// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-consys.c — bring-up del subsistema de conectividad (CONSYS) del MT6582.
 *
 * MILESTONE 1 (este fichero): encender el CONSYS (reguladores + MTCMOS por SPM,
 * mismo patron que mt6582-mfg-power.c) y LEER EL CHIP-ID (0x18070008) para PROBAR
 * que el hardware responde. Si lee 0x6582 -> el CONSYS esta vivo (equivalente a la
 * prueba de power del GPU/MFG). El WiFi funcional (WMT + firmware + cfg80211) es
 * trabajo posterior (ver HITO-WIFI-CONSYS.md), pero todo empieza por este encendido.
 *
 * Secuencia portada de downstream 3.10:
 *   mtk_wcn_consys_hw_reg_ctrl() -> conn_power_on() -> spm_mtcmos_ctrl_connsys(ON)
 *   + reguladores VCN_1V8/VCN28 (PMIC MT6323) + poll CONSYS_CHIP_ID_REG==0x6582.
 *
 * NOTA: SPM e INFRACFG son bloques compartidos -> los mapeamos con ioremap directo
 * (no platform_get_resource, para no reservarlos en exclusiva). Igual que el MFG.
 */
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

/* --- bases fisicas MT6582 (downstream 0xF0xxxxxx/0xF8xxxxxx -> fisico) --- */
#define SPM_PHYS		0x10006000
#define INFRACFG_AO_PHYS	0x10001000
#define CONN_MCU_CONFIG_PHYS	0x18070000

/* --- SPM (power MTCMOS del CONSYS) --- */
#define SPM_POWERON_CFG		0x000	/* key de desbloqueo */
#define SPM_KEY			0x0b160001
#define SPM_CONN_PWR_CON	0x280	/* CONSYS_TOP1_PWR_CTRL_REG */
#define SPM_PWR_STATUS		0x60c
#define SPM_PWR_STATUS_S	0x610
#define CONN_PWR_STA_MASK	(1U << 1)

/* bits de SPM_CONN_PWR_CON (identicos al MFG) */
#define PWR_RST_B		(1U << 0)
#define PWR_ISO			(1U << 1)
#define PWR_ON			(1U << 2)
#define PWR_ON_S		(1U << 3)
#define PWR_CLK_DIS		(1U << 4)
#define CONN_SRAM_PDN		(1U << 8)	/* MD_SRAM_PDN */

/* --- INFRACFG_AO (proteccion de bus TOPAXI) --- */
#define TOPAXI_PROT_EN		0x220
#define TOPAXI_PROT_STA1	0x228
#define CONN_PROT_MASK		0x0104

/* --- CONN_MCU_CONFIG --- */
#define CONSYS_CHIP_ID		0x008
#define CONSYS_CHIP_ID_6582	0x6582
#define CONSYS_CHIP_ID_6572	0x6572

/* --- activación del subsistema interno del CONSYS ---
 * El M1 sólo hacía power SPM (basta p/chip-id). Para que el MCU corra el ROM y el
 * enlace BTIF clockee (el shift TEMT se quedaba a 0), faltan estos 3 pasos que el
 * downstream difería al "FW patch" (mtk_wcn_consys_hw_reg_ctrl). */
#define TOPCKGEN_PHYS		0x10000000
#define AP_RGU_PHYS		0x10007000
#define TOP_CLKCG_CLR		0x084		/* CONSYS_TOP_CLKCG_CLR */
#define TOP_CLKCG_CONSYS	(1U << 26)
#define INFRA_PDN_CLR		0x044		/* ungate (CONNMCU = bit 12) */
#define INFRA_CG_CONNMCU	(1U << 12)
#define CONSYS_CPU_SW_RST	0x018		/* AP_RGU + 0x18 */
#define CPU_SW_RST_BIT		(1U << 12)
#define CPU_SW_RST_KEY		(0x88U << 24)

struct mt6582_consys {
	struct device *dev;
	void __iomem *spm;
	void __iomem *infra;
	void __iomem *mcu_cfg;
	struct regulator *vcn18;	/* VCN_1V8: digital CONSYS */
	struct regulator *vcn28;	/* VCN28: analog */
	struct regulator *vcn33;	/* VCN33_WIFI: RF WiFi */
};

static int consys_spm_power_on(struct mt6582_consys *cs)
{
	void __iomem *spm = cs->spm;
	void __iomem *infra = cs->infra;
	int t;

	/* desbloquear escrituras SPM (idempotente) */
	writel(SPM_KEY, spm + SPM_POWERON_CFG);

	/* PWR_ON + PWR_ON_S */
	writel(readl(spm + SPM_CONN_PWR_CON) | PWR_ON, spm + SPM_CONN_PWR_CON);
	writel(readl(spm + SPM_CONN_PWR_CON) | PWR_ON_S, spm + SPM_CONN_PWR_CON);

	for (t = 0; t < 2000; t++) {
		if ((readl(spm + SPM_PWR_STATUS) & CONN_PWR_STA_MASK) &&
		    (readl(spm + SPM_PWR_STATUS_S) & CONN_PWR_STA_MASK))
			break;
		udelay(10);
	}
	if (t == 2000) {
		dev_err(cs->dev, "timeout esperando CONN_PWR_STA (CON=0x%x)\n",
			readl(spm + SPM_CONN_PWR_CON));
		return -ETIMEDOUT;
	}

	/* quitar clock-disable, des-aislar, soltar reset, encender SRAM */
	writel(readl(spm + SPM_CONN_PWR_CON) & ~PWR_CLK_DIS, spm + SPM_CONN_PWR_CON);
	writel(readl(spm + SPM_CONN_PWR_CON) & ~PWR_ISO, spm + SPM_CONN_PWR_CON);
	writel(readl(spm + SPM_CONN_PWR_CON) | PWR_RST_B, spm + SPM_CONN_PWR_CON);
	writel(readl(spm + SPM_CONN_PWR_CON) & ~CONN_SRAM_PDN, spm + SPM_CONN_PWR_CON);

	/* liberar la proteccion de bus TOPAXI del CONSYS */
	writel(readl(infra + TOPAXI_PROT_EN) & ~CONN_PROT_MASK, infra + TOPAXI_PROT_EN);
	for (t = 0; t < 2000; t++) {
		if (!(readl(infra + TOPAXI_PROT_STA1) & CONN_PROT_MASK))
			break;
		udelay(10);
	}

	udelay(10);	/* 26M listo */
	return 0;
}

/* El BTIF (mt6582-btif.c) espera a esto: hasta que el MCU del CONSYS no corre el
 * ROM, su lado del enlace no clockea y el STP no llega. */
bool mt6582_consys_ready;
EXPORT_SYMBOL_GPL(mt6582_consys_ready);

/* Activa el subsistema interno: clock de TOP + clock del MCU (INFRA) + suelta el
 * reset del MCU -> el ROM corre y escucha STP por BTIF. */
static void consys_activate_mcu(struct mt6582_consys *cs)
{
	void __iomem *topck = ioremap(TOPCKGEN_PHYS, 0x100);
	void __iomem *rgu = ioremap(AP_RGU_PHYS, 0x100);
	u32 v;

	if (topck) {
		writel(TOP_CLKCG_CONSYS, topck + TOP_CLKCG_CLR);	/* quitar clock-gating CONSYS */
		iounmap(topck);
	}
	writel(INFRA_CG_CONNMCU, cs->infra + INFRA_PDN_CLR);		/* ungate clock del MCU */
	if (rgu) {
		v = readl(rgu + CONSYS_CPU_SW_RST);			/* deassert MCU reset (key 0x88) */
		writel((v & ~CPU_SW_RST_BIT) | CPU_SW_RST_KEY, rgu + CONSYS_CPU_SW_RST);
		iounmap(rgu);
	}
	msleep(30);	/* el MCU arranca el ROM y queda listo para escuchar STP */
	dev_info(cs->dev, "CONSYS MCU activado (TOP_CLKCG bit26 + CONNMCU + deassert reset)\n");
}

static int mt6582_consys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt6582_consys *cs;
	u32 id = 0;
	int ret, t;

	cs = devm_kzalloc(dev, sizeof(*cs), GFP_KERNEL);
	if (!cs)
		return -ENOMEM;
	cs->dev = dev;

	cs->vcn18 = devm_regulator_get(dev, "vcn18");
	cs->vcn28 = devm_regulator_get(dev, "vcn28");
	cs->vcn33 = devm_regulator_get(dev, "vcn33");
	if (IS_ERR(cs->vcn18) || IS_ERR(cs->vcn28) || IS_ERR(cs->vcn33))
		return dev_err_probe(dev, -EPROBE_DEFER, "reguladores VCN no listos\n");

	cs->spm = ioremap(SPM_PHYS, 0x1000);
	cs->infra = ioremap(INFRACFG_AO_PHYS, 0x1000);
	cs->mcu_cfg = ioremap(CONN_MCU_CONFIG_PHYS, 0x1000);
	if (!cs->spm || !cs->infra || !cs->mcu_cfg) {
		ret = -ENOMEM;
		goto err_unmap;
	}

	ret = regulator_enable(cs->vcn18);
	if (ret)
		goto err_unmap;
	ret = regulator_enable(cs->vcn28);
	if (ret)
		goto err_v18;
	ret = regulator_enable(cs->vcn33);
	if (ret)
		goto err_v28;

	ret = consys_spm_power_on(cs);
	if (ret)
		goto err_v33;

	/* activar el subsistema interno (clock TOP + clock MCU + soltar reset) para
	 * que el ROM corra y el enlace BTIF clockee */
	consys_activate_mcu(cs);

	/* poll del chip-id: el CONSYS debe devolver 0x6582 */
	for (t = 0; t < 10; t++) {
		id = readl(cs->mcu_cfg + CONSYS_CHIP_ID);
		if (id == CONSYS_CHIP_ID_6582 || id == CONSYS_CHIP_ID_6572)
			break;
		msleep(20);
	}

	if (id == CONSYS_CHIP_ID_6582 || id == CONSYS_CHIP_ID_6572) {
		dev_info(dev, "CONSYS VIVO: chip-id=0x%04x (PWR_CON=0x%x)\n",
			 id, readl(cs->spm + SPM_CONN_PWR_CON));
		mt6582_consys_ready = true;	/* desbloquea el probe del BTIF */
		platform_set_drvdata(pdev, cs);
		return 0;
	}

	dev_err(dev, "CONSYS no responde: chip-id=0x%08x (PWR_CON=0x%x, PWR_STATUS=0x%x)\n",
		id, readl(cs->spm + SPM_CONN_PWR_CON), readl(cs->spm + SPM_PWR_STATUS));
	ret = -ENODEV;

err_v33:
	regulator_disable(cs->vcn33);
err_v28:
	regulator_disable(cs->vcn28);
err_v18:
	regulator_disable(cs->vcn18);
err_unmap:
	if (cs->spm)
		iounmap(cs->spm);
	if (cs->infra)
		iounmap(cs->infra);
	if (cs->mcu_cfg)
		iounmap(cs->mcu_cfg);
	return ret;
}

static const struct of_device_id mt6582_consys_of_ids[] = {
	{ .compatible = "mediatek,mt6582-consys" },
	{ }
};
MODULE_DEVICE_TABLE(of, mt6582_consys_of_ids);

static struct platform_driver mt6582_consys_driver = {
	.probe = mt6582_consys_probe,
	.driver = {
		.name = "mt6582-consys",
		.of_match_table = mt6582_consys_of_ids,
	},
};
module_platform_driver(mt6582_consys_driver);

MODULE_DESCRIPTION("MediaTek MT6582 CONSYS (WiFi/BT/GPS) bring-up");
MODULE_LICENSE("GPL");
