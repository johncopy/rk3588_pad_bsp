#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>

struct lp6280a_chip {
	struct regmap *regmap;
	struct device *dev;
	struct i2c_client *client;
};

static struct reg_default lp6280a_reg_defaults[] = {
	{ 0x00, 0x14 },
	{ 0x01, 0x14 },
	{ 0x03, 0x03 },
	{ 0xff, 0x80 },
};

static const struct regmap_config lp6280a_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = 0xff,
	.cache_type = REGCACHE_RBTREE,

	.reg_defaults	= lp6280a_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(lp6280a_reg_defaults),
};

static int lp6280a_read8(struct lp6280a_chip *chip, unsigned int reg, u8 *val)
{
	return regmap_raw_read(chip->regmap, reg, val, sizeof(u8));
}

static int lp6280a_write8(struct lp6280a_chip *chip, unsigned int reg, u8 val)
{
	return regmap_raw_write(chip->regmap, reg, &val, sizeof(u8));
}

static int lp6280a_probe(struct i2c_client *client,
					const struct i2c_device_id *i2c_id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lp6280a_chip *chip;
	int ret;
	u8 vlau;
	int i;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->regmap = devm_regmap_init_i2c(client, &lp6280a_regmap_config);
	if (IS_ERR(chip->regmap))
		return PTR_ERR(chip->regmap);

	chip->dev = &client->dev;
	i2c_set_clientdata(client, chip);

	for(i = 0; i < sizeof(lp6280a_reg_defaults) / sizeof(lp6280a_reg_defaults[0]); i++) {
		ret = lp6280a_write8(chip, lp6280a_reg_defaults[i].reg, lp6280a_reg_defaults[i].def);
		if(!ret) {
			msleep(10);
			ret = lp6280a_read8(chip, lp6280a_reg_defaults[i].reg, &vlau);
			if(!ret)
				dev_info(chip->dev, "register 0x%2x is %x", lp6280a_reg_defaults[i].reg, vlau);
			else
				// dev_warn(chip->dev, "Failed to read register 0x%2x (ret = %d, vlau = %d)",
				// 	lp6280a_reg_defaults[i].reg, ret, vlau);
				printk("lp6280 Failed to write register 0x%2x (ret = %d, vlau = %d)",
			 		lp6280a_reg_defaults[i].reg, ret, vlau);
		} else
			// dev_warn(chip->dev, "Failed to write register 0x%2x (ret = %d, vlau = %d)",
			// 		lp6280a_reg_defaults[i].reg, ret, vlau);
			printk("lp6280 Failed to write register 0x%2x (ret = %d, vlau = %d)",
			 		lp6280a_reg_defaults[i].reg, ret, vlau);
	}

	return 0;
}

static const struct i2c_device_id lp6280a_id[] = {
	{.name = "lcd-lp6280a",},
	{},
};
MODULE_DEVICE_TABLE(i2c, lp6280a_id);

static struct i2c_driver lp6280a_i2c_driver = {
	.driver = {
		.name = "lcd-lp6280a",
	},
	.probe = lp6280a_probe,
	.id_table = lp6280a_id,
};

// module_i2c_driver(lp6280a_i2c_driver);

static int __init lp6280a_i2c_init(void)
{
	return i2c_add_driver(&lp6280a_i2c_driver);
}

late_initcall(lp6280a_i2c_init);

MODULE_DESCRIPTION("lp6280a regulator driver");
MODULE_AUTHOR("jackfahdin <maojie_guo@techvision.com.cn>");
MODULE_LICENSE("GPL v2");
