// SPDX-License-Identifier: GPL-2.0
/*
 * Driver para el panel Himax HX8389B del BQ Aquaris E4.5 (krillin / MT6582):
 *   - Truly qHD 540x960, MIPI-DSI 2 lanes, video mode SYNC_PULSE, RGB888.
 *   - power = VGP2 2.8V (MT6323); reset = GPIO112.
 * Secuencia init traducida del LCM driver downstream (init_lcm_registers),
 * estructura clonada de panel-himax-hx8394.c.
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

#define DRV_NAME "panel-himax-hx8389"

struct hx8389 {
	struct device *dev;
	struct drm_panel panel;
	struct gpio_desc *reset_gpio;
	struct regulator *power;	/* VGP2 2.8V */
	bool up;			/* panel traido una vez (init hecho) */
};

static inline struct hx8389 *panel_to_hx8389(struct drm_panel *panel)
{
	return container_of(panel, struct hx8389, panel);
}

/* init traducido 1:1 de init_lcm_registers() del LCM downstream (truly hx8389) */
static void hx8389_init_sequence(struct mipi_dsi_multi_context *dsi_ctx)
{
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xb9, 0xff, 0x83, 0x89); /* SETEXTC */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xde, 0x05, 0x58, 0x10);
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xba, 0x01, 0x92);       /* SETMIPI */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xb1, 0x00, 0x00, 0x04, 0xe3, 0x96,
				     0x10, 0x11, 0x6f, 0xef, 0x28, 0x30, 0x23, 0x23,
				     0x42, 0x00, 0x58, 0xf2, 0x20, 0x00); /* SETPOWER */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xb2, 0x00, 0x00, 0x78, 0x08, 0x03,
				     0x00, 0x80); /* SETDISP */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xb4, 0x80, 0x08, 0x00, 0x32, 0x10,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37,
				     0x0a, 0x40, 0x04, 0x37, 0x0a, 0x40, 0x14, 0x50,
				     0x55, 0x0a); /* SETCYC */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xd5, 0x00, 0x00, 0x4c, 0x00, 0x01,
				     0x00, 0x00, 0x00, 0x60, 0x00, 0x99, 0x88, 0x99,
				     0x88, 0x88, 0x32, 0x88, 0x10, 0x88, 0x76, 0x88,
				     0x54, 0x10, 0x32, 0x88, 0x88, 0x88, 0x88, 0x88); /* SETGIP1 */
#if 0 /* experimento color: sin gamma E0 + DGC C1 (panel usa su default) */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xe0, 0x05, 0x14, 0x18, 0x2d, 0x34,
				     0x3f, 0x20, 0x3c, 0x08, 0x0e, 0x0e, 0x11, 0x13,
				     0x10, 0x12, 0x1a, 0x1c, 0x05, 0x14, 0x18, 0x2d,
				     0x34, 0x3f, 0x20, 0x3c, 0x08, 0x0e, 0x0e, 0x11,
				     0x13, 0x10, 0x12, 0x1a, 0x1c); /* SETGAMMA */
	/* DGC LUT (C1) — 4 bloques, tal cual el LK */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xc1, 0x01, 0x02, 0x08, 0x17, 0x20,
				     0x27, 0x2d, 0x32, 0x38, 0x40, 0x48, 0x4f, 0x57,
				     0x5f, 0x67, 0x70, 0x79, 0x81, 0x89, 0x91, 0x99,
				     0xa1, 0xa8, 0xb0, 0xb9, 0xc2, 0xc9, 0xd0, 0xd9,
				     0xe2, 0xe9, 0xf3);
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xc1, 0xfa, 0xff, 0xd0, 0xaf, 0xdf,
				     0xa9, 0xa9, 0x3d, 0x79, 0x92, 0xc0, 0x02, 0x08,
				     0x17, 0x20, 0x27, 0x2d, 0x32, 0x38, 0x40, 0x48,
				     0x4f, 0x57, 0x5f, 0x67, 0x70, 0x79, 0x81, 0x89,
				     0x91, 0x99, 0xa1);
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xc1, 0xa8, 0xb0, 0xb9, 0xc2, 0xc9,
				     0xd0, 0xd9, 0xe2, 0xe9, 0xf3, 0xfa, 0xff, 0xd0,
				     0xaf, 0xdf, 0xa9, 0xa9, 0x3d, 0x79, 0x92, 0xc0,
				     0x02, 0x08, 0x17, 0x20, 0x27, 0x2d, 0x32, 0x38,
				     0x40, 0x48, 0x4f);
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xc1, 0x57, 0x5f, 0x67, 0x70, 0x79,
				     0x81, 0x89, 0x91, 0x99, 0xa1, 0xa8, 0xb0, 0xb9,
				     0xc2, 0xc9, 0xd0, 0xd9, 0xe2, 0xe9, 0xf3, 0xfa,
				     0xff, 0xd0, 0xaf, 0xdf, 0xa9, 0xa9, 0x3d, 0x79,
				     0x92, 0xc0);
	#endif
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xcc, 0x02);             /* SETPANEL */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xb6, 0x00, 0xaf, 0x00, 0xaf); /* SETVCOM */
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xcb, 0x07, 0x07);
	mipi_dsi_dcs_write_seq_multi(dsi_ctx, 0xbb, 0x00, 0x00, 0xff, 0x80); /* SETOTP */
}

static int hx8389_prepare(struct drm_panel *panel)
{
	struct hx8389 *ctx = panel_to_hx8389(panel);
	int ret;

	if (ctx->up)		/* ya traido: no tocar (sobrevive al modeset) */
		return 0;

	ret = regulator_enable(ctx->power);
	if (ret) {
		dev_err(ctx->dev, "Failed to enable power supply: %d\n", ret);
		return ret;
	}
	usleep_range(10000, 11000);

	/* reset: alto -> bajo(50ms) -> alto(20ms) (GPIO112, active-low) */
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);	/* deassert (alto) */
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);	/* assert (bajo) */
	msleep(50);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);	/* deassert (alto) */
	msleep(20);

	return 0;
}

static int hx8389_unprepare(struct drm_panel *panel)
{
	/* no-op: NO cortar reset ni regulador -> el panel conserva su init a
	 * traves de los modesets (el kernel no puede re-inicializarlo por DSI). */
	return 0;
}

static int hx8389_enable(struct drm_panel *panel)
{
	struct hx8389 *ctx = panel_to_hx8389(panel);
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	if (ctx->up)		/* init ya enviado; no repetir en cada modeset */
		return 0;

	hx8389_init_sequence(&dsi_ctx);

	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 200);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 10);

	ctx->up = true;
	return dsi_ctx.accum_err;
}

static int hx8389_disable(struct drm_panel *panel)
{
	/* no-op: NO enviar DCS (display_off/sleep) durante un modeset -> esos
	 * transfers agotan el timeout del DSI y resetean el engine a mitad del
	 * atomic commit (commit wait timed out -> negro). El panel se mantiene. */
	return 0;
}

/* 540x960, porches del panel: hsa8/hbp20/hfp22, vsa3/vbp9/vfp9.
 * clock = htotal(590) x vtotal(981) x ~60fps; ajustable si el DSI no engancha. */
static const struct drm_display_mode hx8389_mode = {
	.hdisplay    = 540,
	.hsync_start = 540 + 22,
	.hsync_end   = 540 + 22 + 8,
	.htotal	     = 540 + 22 + 8 + 20,
	.vdisplay    = 960,
	.vsync_start = 960 + 9,
	.vsync_end   = 960 + 9 + 3,
	.vtotal	     = 960 + 9 + 3 + 9,
	.clock	     = 34740,
	.width_mm    = 56,
	.height_mm   = 99,
};

static int hx8389_get_modes(struct drm_panel *panel,
			    struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &hx8389_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs hx8389_funcs = {
	.prepare   = hx8389_prepare,
	.unprepare = hx8389_unprepare,
	.enable	   = hx8389_enable,
	.disable   = hx8389_disable,
	.get_modes = hx8389_get_modes,
};

static int hx8389_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct hx8389 *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->dev = dev;
	mipi_dsi_set_drvdata(dsi, ctx);

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset gpio\n");

	ctx->power = devm_regulator_get(dev, "power");
	if (IS_ERR(ctx->power))
		return dev_err_probe(dev, PTR_ERR(ctx->power),
				     "Failed to get power regulator\n");

	dsi->lanes = 2;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			  MIPI_DSI_MODE_LPM;

	drm_panel_init(&ctx->panel, dev, &hx8389_funcs, DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return ret;

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "mipi_dsi_attach failed\n");
	}

	return 0;
}

static void hx8389_remove(struct mipi_dsi_device *dsi)
{
	struct hx8389 *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id hx8389_of_match[] = {
	{ .compatible = "truly,hx8389-qhd" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, hx8389_of_match);

static struct mipi_dsi_driver hx8389_driver = {
	.probe = hx8389_probe,
	.remove = hx8389_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = hx8389_of_match,
	},
};
module_mipi_dsi_driver(hx8389_driver);

MODULE_DESCRIPTION("Himax HX8389B Truly qHD DSI panel (BQ Aquaris E4.5 krillin)");
MODULE_LICENSE("GPL");
