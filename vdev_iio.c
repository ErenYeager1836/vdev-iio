// SPDX-License-Identifier: GPL-2.0+
/*
 * Virtual Industrial I/O (IIO) 3-Axis Accelerometer & Temperature Driver
 *
 * Demonstrates Linux kernel driver development:
 * - IIO device allocation & registration (devm_iio_device_alloc, devm_iio_device_register)
 * - Channel specification (iio_chan_spec) for X/Y/Z acceleration & temperature
 * - Sysfs ABI hooks (read_raw for IIO_CHAN_INFO_RAW and IIO_CHAN_INFO_SCALE)
 * - Platform device & driver binding
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/random.h>

#define VDEV_IIO_NAME "vdev-iio-accel"

struct vdev_iio_state {
	struct mutex lock;
	int scale_val;
	int scale_val2;
};

static const struct iio_chan_spec vdev_iio_channels[] = {
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_X,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_Y,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_Z,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
};

static int vdev_iio_read_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int *val, int *val2, long mask)
{
	struct vdev_iio_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&st->lock);
		switch (chan->type) {
		case IIO_ACCEL:
			if (chan->channel2 == IIO_MOD_Z)
				*val = 9806 + (int)(get_random_u32() % 20) - 10; // ~1g (9.8 m/s^2)
			else
				*val = (int)(get_random_u32() % 40) - 20;        // near zero
			break;
		case IIO_TEMP:
			*val = 25000 + (int)(get_random_u32() % 500) - 250;     // ~25.0 C in milli-degrees
			break;
		default:
			mutex_unlock(&st->lock);
			return -EINVAL;
		}
		mutex_unlock(&st->lock);
		return IIO_VAL_INT;

	case IIO_CHAN_INFO_SCALE:
		mutex_lock(&st->lock);
		*val = st->scale_val;
		*val2 = st->scale_val2;
		mutex_unlock(&st->lock);
		return IIO_VAL_INT_PLUS_MICRO;

	default:
		return -EINVAL;
	}
}

static const struct iio_info vdev_iio_info = {
	.read_raw = vdev_iio_read_raw,
};

static int vdev_iio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct iio_dev *indio_dev;
	struct vdev_iio_state *st;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);
	mutex_init(&st->lock);
	st->scale_val = 0;
	st->scale_val2 = 1000; // 0.001 scale

	indio_dev->name = VDEV_IIO_NAME;
	indio_dev->info = &vdev_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = vdev_iio_channels;
	indio_dev->num_channels = ARRAY_SIZE(vdev_iio_channels);

	return devm_iio_device_register(dev, indio_dev);
}

static struct platform_driver vdev_iio_driver = {
	.driver = {
		.name = VDEV_IIO_NAME,
	},
	.probe = vdev_iio_probe,
};

static struct platform_device *vdev_iio_pdev;

static int __init vdev_iio_init(void)
{
	int ret;

	ret = platform_driver_register(&vdev_iio_driver);
	if (ret)
		return ret;

	vdev_iio_pdev = platform_device_register_simple(VDEV_IIO_NAME, -1, NULL, 0);
	if (IS_ERR(vdev_iio_pdev)) {
		platform_driver_unregister(&vdev_iio_driver);
		return PTR_ERR(vdev_iio_pdev);
	}

	pr_info("vdev_iio: registered synthetic 3-axis accelerometer and temperature sensor\n");
	return 0;
}

static void __exit vdev_iio_exit(void)
{
	platform_device_unregister(vdev_iio_pdev);
	platform_driver_unregister(&vdev_iio_driver);
	pr_info("vdev_iio: driver unregistered successfully\n");
}

module_init(vdev_iio_init);
module_exit(vdev_iio_exit);

MODULE_AUTHOR("Nithish Velan");
MODULE_DESCRIPTION("Synthetic Industrial I/O (IIO) 3-Axis Accelerometer & Temperature Kernel Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
