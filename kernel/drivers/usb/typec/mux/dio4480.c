// SPDX-License-Identifier: GPL-2.0+
/*
 * USB Type-C Analog Audio Switch Driver
 *
 * Copyright (c) 2021-2022 Jake Wu <jake.wu@rock-chips.com>
 */
#define DEBUG
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/of_gpio.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/extcon.h>
#include <linux/extcon-provider.h>

#define DIO_TRY_JACK_COUNT	(3)

/* register map */
#define DIO_DEV_ID 		0x00
#define DIO_OVP_MASK 		0x01
#define DIO_OVP_INT_CLR 	0x02

#define DIO_SW_EN 		0x04
#define DIO_SW_SEL 		0x05
#define DIO_SW_STAT0 		0x06
#define DIO_SW_STAT1 		0x07
#define DIO_AS_LCH_CTRL 	0x08
#define DIO_AS_RCH_CTRL 	0x09
#define DIO_MS_CTRL 		0x0A
#define DIO_SS_CTRL 		0x0B
#define DIO_AGS_CTRL 		0x0C
#define DIO_DELAY_RS_LS 	0x0D
#define DIO_DELAY_SS_LS 	0x0F
#define DIO_DELAY_AGS_LS 	0x10
#define DIO_AUDIO_ACC_STAT 	0x11
#define DIO_FUNC_EN 		0x12

#define DIO_JACK_STAT 		0x17
#define DIO_JACK_DET_INT 	0x18
#define DIO_JACK_DET_MASK 	0x19

#define DIO_MIC_TH0 		0x1C
#define DIO_MIC_TH1 		0x1D
#define DIO_RESET 		0x1E
#define DIO_CUR_SRC_SET 	0x1F

/* macro */
#define DIO4480_DEV_ID 		0x11

struct dio4480_chip {
	struct i2c_client *client;
	struct mutex lock;
	struct regmap *regmap;
	struct device *dev;
	struct kthread_worker *wq;
	struct kthread_work audio_work;
	bool is_connected;
	int mode;

	struct extcon_dev *edev;
	struct notifier_block det_nb;
	struct gpio_desc *enable_gpio;
};

static int dio4480_read8(struct dio4480_chip *chip, unsigned int reg, u8 *val)
{
	return regmap_raw_read(chip->regmap, reg, val, sizeof(u8));
}

static int dio4480_write8(struct dio4480_chip *chip, unsigned int reg, u8 val)
{
	return regmap_raw_write(chip->regmap, reg, &val, sizeof(u8));
}

static int dio4480_reg_print(struct dio4480_chip *chip, u8 reg_index)
{
	u8 reg = 0;

	dio4480_read8(chip, reg_index, &reg);

	dev_info(chip->dev, "==> reg%02x = %04X", reg_index, reg);
	return reg;
}

static int dio4480_init_state(struct dio4480_chip *chip)
{
	dio4480_write8(chip, DIO_SW_SEL, 0x18);
	usleep_range(50, 55);
	dio4480_write8(chip, DIO_SW_EN, 0xF8);

	/* DET pin open drain */
	dio4480_write8(chip, DIO_FUNC_EN, 0x48);

	return 0;
}

static int typec_audio_callback(struct notifier_block *nb,
			     unsigned long event, void *ptr)
{
	struct dio4480_chip *chip = container_of(nb, struct dio4480_chip, det_nb);

	if (extcon_get_state(chip->edev, EXTCON_MECHANICAL)){
		dev_info(chip->dev, "headset on\n");
		chip->mode = 1;
	} else {
		dev_info(chip->dev, "headset off\n");
		chip->mode = 0;
	}

	kthread_queue_work(chip->wq, &chip->audio_work);

	return 0;
}

static int dio4480_detect_callback(struct dio4480_chip *chip)
{
	unsigned char reg17 =0, val = 0;
	bool headphone;
	bool microphone;

	if (chip->mode == 1 && chip->is_connected == false) {
		dev_info(chip->dev, "%s:audio_adapter attached!\n", __func__);
		dev_info(chip->dev, "%s:open the audio switch!\n", __func__);

		chip->is_connected = true;

		dio4480_write8(chip, DIO_SW_EN, 0x9F);
		usleep_range(50, 55);
		dio4480_write8(chip, DIO_SW_SEL, 0x00);
		msleep_interruptible(1);

		dio4480_write8(chip, DIO_FUNC_EN, 0x4D);

		msleep_interruptible(30);

		dio4480_read8(chip, DIO_JACK_STAT, &reg17);

		dio4480_reg_print(chip, DIO_SW_STAT0);
		dio4480_reg_print(chip, DIO_SW_STAT1);

		msleep_interruptible(10);
		val = reg17;

		switch (val) {
		case 2:
			headphone = true;
			microphone = false;
			dev_info(chip->dev,  "3 pole SENSE->SBU2, SBU1 open\n");
			break;
		case 4:
			headphone = true;
			microphone = true;
			/* automatically configure if it is 4 pole audio jack */
			dev_info(chip->dev,
				"4 pole OMTP: MIC->SBU1, SENSE->SBU2\n");
			break;
		case 8:
			headphone = true;
			microphone = true;
			/* automatically configure if it is 4 pole audio jack */
			dev_info(chip->dev,
				"4 pole CTIA: MIC->SBU2, SENSE->SBU1\n");
			break;
		case 1:
		default:
			headphone = false;
			microphone = false;
			dev_info(chip->dev, "No audio accessory\n");
		}

		extcon_set_state_sync(chip->edev, EXTCON_JACK_HEADPHONE, headphone);
		extcon_set_state_sync(chip->edev, EXTCON_JACK_MICROPHONE, microphone);
		usleep_range(100, 105);
		gpiod_direction_output(chip->enable_gpio, headphone);
	}else if (chip->mode == 0 && chip->is_connected == true) {
		dev_info(chip->dev, "%s:audio_adapter removal!\n", __func__);
		dev_info(chip->dev, "%s:close the audio switch!\n", __func__);

		chip->is_connected = false;

		dio4480_init_state(chip);

		gpiod_direction_output(chip->enable_gpio, false);
		extcon_set_state_sync(chip->edev, EXTCON_JACK_HEADPHONE, false);
		extcon_set_state_sync(chip->edev, EXTCON_JACK_MICROPHONE, false);
	}

	return 0;
}

static void dio4480_audio_work(struct kthread_work *work)
{
	struct dio4480_chip *chip =
		container_of(work, struct dio4480_chip, audio_work);

	mutex_lock(&chip->lock);

	dio4480_detect_callback(chip);

	mutex_unlock(&chip->lock);
}

static const struct regmap_config dio4480_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0xFF, /* 0x80 .. 0xFF are vendor defined */
};

static int dio4480_check_dev_id(struct i2c_client *i2c)
{
	int ret;

	ret = i2c_smbus_read_byte_data(i2c, DIO_DEV_ID);
	if (ret < 0) {
		dev_err(&i2c->dev, "fail to read dev id(%d)\n", ret);
		return ret;
	}

	dev_info(&i2c->dev, "dev id : 0x%04x\n", ret);

	return 0;
}

static int dio4480_extcon_init(struct dio4480_chip *chip)
{
	int ret;

	if (!of_property_read_bool(chip->dev->of_node, "extcon")) {
		dev_err(chip->dev, "no extcon node!\n");
		return -1;
	}

	chip->edev = extcon_get_edev_by_phandle(chip->dev, 0);
	if (IS_ERR(chip->edev)) {
		dev_err(chip->dev, "couldn't get extcon device");
		return -EPROBE_DEFER;
	}

	chip->det_nb.notifier_call = typec_audio_callback;
	ret = devm_extcon_register_notifier(chip->dev, chip->edev, EXTCON_MECHANICAL, &chip->det_nb);
	if (ret < 0) {
		dev_err(chip->dev, "fail register extcon notifier");
		return ret;
	}

	return 0;
}

static int dio4480_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct dio4480_chip *chip;
	int ret;

	dev_info(&client->dev, "===dio4480 probe===\n");

	printk("===dio4480 probe===\n");
	ret = dio4480_check_dev_id(client);
	if (ret < 0) {
		dev_err(&client->dev, "check device id fail(%d)\n", ret);
		printk("dio4480 check device id fail(%d)\n", ret);
		return ret;
	}

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->dev = &client->dev;
	mutex_init(&chip->lock);

	chip->enable_gpio = devm_gpiod_get_optional(&client->dev, "enable", GPIOD_ASIS);
	if (IS_ERR(chip->enable_gpio)) {
		ret = PTR_ERR(chip->enable_gpio);
		if (ret != -EPROBE_DEFER)
			dev_err(&client->dev, "failed to get enable GPIO: %d\n", ret);
		return ret;
	}

	chip->regmap = devm_regmap_init_i2c(client, &dio4480_regmap_config);
	if (IS_ERR(chip->regmap))
		return PTR_ERR(chip->regmap);

	i2c_set_clientdata(client, chip);

	dio4480_init_state(chip);

	ret = dio4480_extcon_init(chip);
	if (ret < 0)
		return ret;

	chip->wq = kthread_create_worker(0, dev_name(dev));
	if (IS_ERR(chip->wq))
		return PTR_ERR(chip->wq);
	sched_set_fifo(chip->wq->task);

	kthread_init_work(&chip->audio_work, dio4480_audio_work);

	return 0;
}

static int dio4480_remove(struct i2c_client *client)
{
	struct dio4480_chip *chip = i2c_get_clientdata(client);

	kthread_destroy_worker(chip->wq);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id dio4480_of_match[] = {
	{ .compatible = "dioo,dio4480" },
	{},
};
MODULE_DEVICE_TABLE(of, dio4480_of_match);
#endif

static int dio4480_pm_suspend(struct device *dev)
{
	return 0;
}

static int dio4480_pm_resume(struct device *dev)
{
	struct dio4480_chip *chip = dev->driver_data;

	if (extcon_get_state(chip->edev, EXTCON_MECHANICAL)){
		dev_info(chip->dev, "headset on\n");
		chip->mode = 1;
		chip->is_connected = false;
	} else {
		dev_info(chip->dev, "headset off\n");
		chip->mode = 0;
		chip->is_connected = true;
	}

	kthread_queue_work(chip->wq, &chip->audio_work);

	return 0;
}

static const struct dev_pm_ops dio4480_pm_ops = {
	.suspend = dio4480_pm_suspend,
	.resume = dio4480_pm_resume,
};

static struct i2c_driver dio4480_driver = {
	.driver = {
		.name = "dio4480",
		.pm = &dio4480_pm_ops,
		.of_match_table = of_match_ptr(dio4480_of_match),
	},
	.probe		= dio4480_probe,
	.remove		= dio4480_remove,
};

module_i2c_driver(dio4480_driver);

MODULE_AUTHOR("Jake Wu <jake.wu@rock-chips.com>");
MODULE_DESCRIPTION("USB Type-C Analog Audio Switch Driver");
MODULE_LICENSE("GPL");
