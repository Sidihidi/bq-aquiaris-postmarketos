// SPDX-License-Identifier: GPL-2.0-only
/*
 * MMC3516x - MEMSIC 3-axis Magnetic Sensor (as found in the BQ Aquaris E4.5)
 *
 * IIO driver for MMC3516x (7-bit I2C slave address 0x30).
 *
 * Modeled on mmc35240.c (same register topology) with the measurement
 * flow, timings and Z-axis inversion taken from the MEMSIC/MTK downstream
 * driver (drivers/misc/mediatek/magnetometer/mmc3516x_auto/mmc3516x.c):
 *   - degauss (REFILL + SET / REFILL + RESET pulse cycles) at init and
 *     every MMC3516X_RESET_INTV measurements
 *   - single-shot TM + DS-bit poll (no continuous mode)
 *   - 16-bit unsigned output, null-field 32768, 2048 counts/Gauss
 *   - Z axis inverted (downstream does vec[2] = 65536 - raw)
 *
 * No OTP compensation on this part (unlike MMC35240).
 */

#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/pm.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define MMC3516X_DRV_NAME		"mmc3516x"

#define MMC3516X_REG_XOUT_L		0x00
#define MMC3516X_REG_STATUS		0x06
#define MMC3516X_REG_CTRL0		0x07
#define MMC3516X_REG_CTRL1		0x08
#define MMC3516X_REG_ID			0x20

#define MMC3516X_STATUS_MEAS_DONE_BIT	BIT(0)

#define MMC3516X_CTRL0_TM_BIT		BIT(0)
#define MMC3516X_CTRL0_CM_BIT		BIT(1)
#define MMC3516X_CTRL0_NOBOOST_BIT	BIT(4)
#define MMC3516X_CTRL0_SET_BIT		BIT(5)
#define MMC3516X_CTRL0_RESET_BIT	BIT(6)
#define MMC3516X_CTRL0_REFILL_BIT	BIT(7)

/* CTRL1: output resolution / measurement time */
#define MMC3516X_CTRL1_BW_MASK		GENMASK(1, 0)
#define MMC3516X_CTRL1_16_BITS_SLOW	0x00	/* 7.92 ms */
#define MMC3516X_CTRL1_16_BITS_FAST	0x01	/* 4.08 ms */
#define MMC3516X_CTRL1_14_BITS		0x02	/* 2.16 ms */

/* timings from the downstream driver */
#define MMC3516X_WAIT_CHARGE_PUMP_MS	50	/* MMC3516X_DELAY_SET/RST */
#define MMC3516X_WAIT_COIL_PULSE_MS	1
#define MMC3516X_WAIT_TM_MS		10	/* MMC3516X_DELAY_TM */

/* degauss (SET/RESET) every N measurements, like the downstream driver */
#define MMC3516X_RESET_INTV		150

/* 16-bit mode: unsigned, null field output and counts per Gauss */
#define MMC3516X_NFO			32768
#define MMC3516X_SENS			2048	/* counts / Gauss */

enum mmc3516x_axis {
	AXIS_X = 0,
	AXIS_Y,
	AXIS_Z,
};

struct mmc3516x_data {
	struct i2c_client *client;
	struct mutex mutex;
	struct regmap *regmap;
	unsigned int meas_count;
};

#define MMC3516X_CHANNEL(_axis) { \
	.type = IIO_MAGN, \
	.modified = 1, \
	.channel2 = IIO_MOD_ ## _axis, \
	.address = AXIS_ ## _axis, \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW), \
	.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE), \
}

static const struct iio_chan_spec mmc3516x_channels[] = {
	MMC3516X_CHANNEL(X),
	MMC3516X_CHANNEL(Y),
	MMC3516X_CHANNEL(Z),
};

/*
 * One coil pulse, downstream style: REFILL, wait for the charge pump,
 * pulse the coil bit, then explicitly clear CTRL0 (the downstream driver
 * does not rely on the self-clearing behaviour).
 */
static int mmc3516x_coil_pulse(struct mmc3516x_data *data, u8 coil_bit)
{
	int ret;

	ret = regmap_write(data->regmap, MMC3516X_REG_CTRL0,
			   MMC3516X_CTRL0_REFILL_BIT);
	if (ret < 0)
		return ret;
	msleep(MMC3516X_WAIT_CHARGE_PUMP_MS);

	ret = regmap_write(data->regmap, MMC3516X_REG_CTRL0, coil_bit);
	if (ret < 0)
		return ret;
	msleep(MMC3516X_WAIT_COIL_PULSE_MS);

	ret = regmap_write(data->regmap, MMC3516X_REG_CTRL0, 0);
	if (ret < 0)
		return ret;
	msleep(MMC3516X_WAIT_COIL_PULSE_MS);

	return 0;
}

/*
 * Degauss/restore the sensor characteristics: SET cycle followed by a
 * RESET cycle (axis polarity is the RESET one, which is what both the
 * downstream driver and the Z inversion below assume).
 */
static int mmc3516x_degauss(struct mmc3516x_data *data)
{
	int ret;

	ret = mmc3516x_coil_pulse(data, MMC3516X_CTRL0_SET_BIT);
	if (ret < 0)
		return ret;

	return mmc3516x_coil_pulse(data, MMC3516X_CTRL0_RESET_BIT);
}

static int mmc3516x_init(struct mmc3516x_data *data)
{
	int ret;
	unsigned int reg_id;

	ret = regmap_read(data->regmap, MMC3516X_REG_ID, &reg_id);
	if (ret < 0) {
		dev_err(&data->client->dev, "Error reading product id\n");
		return ret;
	}
	dev_dbg(&data->client->dev, "MMC3516x chip id %x\n", reg_id);

	ret = mmc3516x_degauss(data);
	if (ret < 0)
		return ret;

	/* 16-bit / slow measurement mode, like the downstream driver */
	return regmap_update_bits(data->regmap, MMC3516X_REG_CTRL1,
				  MMC3516X_CTRL1_BW_MASK,
				  MMC3516X_CTRL1_16_BITS_SLOW);
}

static int mmc3516x_take_measurement(struct mmc3516x_data *data)
{
	int ret, tries = 10;
	unsigned int reg_status;

	ret = regmap_write(data->regmap, MMC3516X_REG_CTRL0,
			   MMC3516X_CTRL0_TM_BIT);
	if (ret < 0)
		return ret;

	msleep(MMC3516X_WAIT_TM_MS);

	while (tries-- > 0) {
		ret = regmap_read(data->regmap, MMC3516X_REG_STATUS,
				  &reg_status);
		if (ret < 0)
			return ret;
		if (reg_status & MMC3516X_STATUS_MEAS_DONE_BIT)
			return 0;
		usleep_range(1000, 2000);
	}

	dev_err(&data->client->dev, "data not ready\n");
	return -EIO;
}

static int mmc3516x_read_measurement(struct mmc3516x_data *data, __le16 buf[3])
{
	int ret;

	/* periodic SET/RESET, downstream cadence */
	if (!(data->meas_count % MMC3516X_RESET_INTV)) {
		ret = mmc3516x_degauss(data);
		if (ret < 0)
			return ret;
	}
	data->meas_count++;

	ret = mmc3516x_take_measurement(data);
	if (ret < 0)
		return ret;

	return regmap_bulk_read(data->regmap, MMC3516X_REG_XOUT_L, buf,
				3 * sizeof(__le16));
}

/*
 * Convert a raw reading to milli gauss.
 *
 * Output is 16-bit unsigned with null field at 32768 and 2048 counts per
 * Gauss. The Z axis is inverted (the downstream driver does
 * vec[2] = 65536 - raw before subtracting the offset, which is the same
 * as negating the signed value).
 */
static int mmc3516x_raw_to_mgauss(int index, __le16 buf[], int *val)
{
	int raw = le16_to_cpu(buf[index]);
	int mg = (raw - MMC3516X_NFO) * 1000 / MMC3516X_SENS;

	switch (index) {
	case AXIS_X:
	case AXIS_Y:
		*val = mg;
		return 0;
	case AXIS_Z:
		*val = -mg;
		return 0;
	default:
		return -EINVAL;
	}
}

static int mmc3516x_read_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan, int *val,
			     int *val2, long mask)
{
	struct mmc3516x_data *data = iio_priv(indio_dev);
	int ret;
	__le16 buf[3];

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&data->mutex);
		ret = mmc3516x_read_measurement(data, buf);
		mutex_unlock(&data->mutex);
		if (ret < 0)
			return ret;
		ret = mmc3516x_raw_to_mgauss(chan->address, buf, val);
		if (ret < 0)
			return ret;
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		/* raw value is in milli gauss */
		*val = 0;
		*val2 = 1000;
		return IIO_VAL_INT_PLUS_MICRO;
	default:
		return -EINVAL;
	}
}

static const struct iio_info mmc3516x_info = {
	.read_raw	= mmc3516x_read_raw,
};

static bool mmc3516x_is_writeable_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case MMC3516X_REG_CTRL0:
	case MMC3516X_REG_CTRL1:
		return true;
	default:
		return false;
	}
}

static bool mmc3516x_is_readable_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case MMC3516X_REG_XOUT_L:
	case MMC3516X_REG_XOUT_L + 1:
	case MMC3516X_REG_XOUT_L + 2:
	case MMC3516X_REG_XOUT_L + 3:
	case MMC3516X_REG_XOUT_L + 4:
	case MMC3516X_REG_XOUT_L + 5:
	case MMC3516X_REG_STATUS:
	case MMC3516X_REG_ID:
		return true;
	default:
		return false;
	}
}

static bool mmc3516x_is_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case MMC3516X_REG_CTRL0:
	case MMC3516X_REG_CTRL1:
		return false;
	default:
		return true;
	}
}

static const struct reg_default mmc3516x_reg_defaults[] = {
	{ MMC3516X_REG_CTRL0,  0x00 },
	{ MMC3516X_REG_CTRL1,  0x00 },
};

static const struct regmap_config mmc3516x_regmap_config = {
	.name = "mmc3516x_regmap",

	.reg_bits = 8,
	.val_bits = 8,

	.max_register = MMC3516X_REG_ID,
	.cache_type = REGCACHE_FLAT,

	.writeable_reg = mmc3516x_is_writeable_reg,
	.readable_reg = mmc3516x_is_readable_reg,
	.volatile_reg = mmc3516x_is_volatile_reg,

	.reg_defaults = mmc3516x_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(mmc3516x_reg_defaults),
};

static int mmc3516x_probe(struct i2c_client *client)
{
	struct mmc3516x_data *data;
	struct iio_dev *indio_dev;
	struct regmap *regmap;
	int ret;

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	regmap = devm_regmap_init_i2c(client, &mmc3516x_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&client->dev, "regmap initialization failed\n");
		return PTR_ERR(regmap);
	}

	data = iio_priv(indio_dev);
	i2c_set_clientdata(client, indio_dev);
	data->client = client;
	data->regmap = regmap;
	data->meas_count = 0;

	mutex_init(&data->mutex);

	indio_dev->info = &mmc3516x_info;
	indio_dev->name = MMC3516X_DRV_NAME;
	indio_dev->channels = mmc3516x_channels;
	indio_dev->num_channels = ARRAY_SIZE(mmc3516x_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	ret = mmc3516x_init(data);
	if (ret < 0) {
		dev_err(&client->dev, "mmc3516x chip init failed\n");
		return ret;
	}

	return devm_iio_device_register(&client->dev, indio_dev);
}

static int mmc3516x_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct mmc3516x_data *data = iio_priv(indio_dev);

	regcache_cache_only(data->regmap, true);

	return 0;
}

static int mmc3516x_resume(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct mmc3516x_data *data = iio_priv(indio_dev);
	int ret;

	regcache_mark_dirty(data->regmap);
	ret = regcache_sync_region(data->regmap, MMC3516X_REG_CTRL0,
				   MMC3516X_REG_CTRL1);
	if (ret < 0)
		dev_err(dev, "Failed to restore control registers\n");

	regcache_cache_only(data->regmap, false);

	return 0;
}

static DEFINE_SIMPLE_DEV_PM_OPS(mmc3516x_pm_ops, mmc3516x_suspend,
				mmc3516x_resume);

static const struct of_device_id mmc3516x_of_match[] = {
	{ .compatible = "memsic,mmc3516x", },
	{ }
};
MODULE_DEVICE_TABLE(of, mmc3516x_of_match);

static const struct i2c_device_id mmc3516x_id[] = {
	{ "mmc3516x" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mmc3516x_id);

static struct i2c_driver mmc3516x_driver = {
	.driver = {
		.name = MMC3516X_DRV_NAME,
		.of_match_table = mmc3516x_of_match,
		.pm = pm_sleep_ptr(&mmc3516x_pm_ops),
	},
	.probe		= mmc3516x_probe,
	.id_table	= mmc3516x_id,
};

module_i2c_driver(mmc3516x_driver);

MODULE_AUTHOR("pmos-krillin project");
MODULE_DESCRIPTION("MEMSIC MMC3516x magnetic sensor driver");
MODULE_LICENSE("GPL v2");
