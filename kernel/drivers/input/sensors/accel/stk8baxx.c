/*
 *  stk8baxx.c - Linux kernel modules for sensortek  stk8ba50 / stk8ba50-R /
 *  stk8ba53 accelerometer
 *
 *  Copyright (C) 2012~2016 Lex Hsieh / Sensortek
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/math64.h>
#include <linux/init.h>
#include <linux/sensor-dev.h>
#include "stk8baxx.h"

static int sensor_active(struct i2c_client *client, int enable, int rate)
{
	int ret = 0;

	if (!enable) {
		ret = sensor_write_reg(client, STK8BAXX_POWMODE, STK8BAXX_MD_SUSPEND);
		if (ret < 0) {
			dev_err(&client->dev,
				"set sensor suspend power mode failed.\n");
			return ret;
		}
	} else {
		ret = sensor_write_reg(client, STK8BAXX_POWMODE, STK8BAXX_MD_NORMAL);
		if (ret < 0) {
			dev_err(&client->dev,
				"set sensor normal power mode failed.\n");
			return ret;
		}
	}

	return 0;
}

int stk8baxx_flag = 0;
static int sensor_init(struct i2c_client *client)
{
	struct sensor_private_data *sensor =
		(struct sensor_private_data *)i2c_get_clientdata(client);

	int ret = 0;
	u8 chipid;

	chipid = sensor_read_reg(client, STK8BAXX_CHIPID);
	switch (chipid)
	{
	case STK8BA50_ID:
		dev_info(&client->dev, "CHIP_ID : 0x%02x, gsensor is stk8ba50\n", chipid);
		break;
	case STK8323_ID:
		dev_info(&client->dev, "CHIP_ID : 0x%02x, gsensor is stk8323 or stk8321\n", chipid);
		break;
	case STK8329_ID:
		dev_info(&client->dev, "CHIP_ID : 0x%02x, gsensor is stk8329\n", chipid);
		break;
	case STK8327_ID:
		dev_info(&client->dev, "CHIP_ID : 0x%02x, gsensor is stk8327\n", chipid);
		break;
	case STK8BA50R_ID:
		dev_info(&client->dev, "CHIP_ID : 0x%02x, gsensor is stk8ba50r\n", chipid);
		break;
	case STK8BA53_ID:
		dev_info(&client->dev, "CHIP_ID : 0x%02x, gsensor is stk8ba53 or stk8ba58\n", chipid);
		break;
	default:
		dev_err(&client->dev, "CHIP_ID : 0x%02x, id not match\n", chipid);
		return -1;
		break;
	}

	ret = sensor_write_reg(client, STK8BAXX_POWMODE, STK8BAXX_MD_NORMAL);
	if (ret < 0) {
		dev_err(&client->dev,
			"set sensor normal power mode failed.\n");
		return ret;
	}

	if (sensor->pdata->irq_enable) {
		/* map new data int to int1 */
		ret = sensor_write_reg(client, STK8BAXX_INTMAP2, 0x01);
		if (ret < 0) {
			dev_err(&client->dev,
				"failed to write reg 0x%02x, error=%d\n", STK8BAXX_INTMAP2, ret);
			return ret;
		}
		/* enable new data in */
		ret = sensor_write_reg(client, STK8BAXX_INTEN2, 0x10);
		if (ret < 0) {
			dev_err(&client->dev,
				"failed to write reg 0x%02x, error=%d\n", STK8BAXX_INTEN2, ret);
			return ret;
		}
		/* non-latch int */
		ret = sensor_write_reg(client, STK8BAXX_INTCFG2, 0x00);
		if (ret < 0) {
			dev_err(&client->dev,
				"failed to write reg 0x%02x, error=%d\n", STK8BAXX_INTCFG2, ret);
			return ret;
		}

		/* int1, push-pull, active high */
		ret = sensor_write_reg(client, STK8BAXX_INTCFG1, 0x01);
		if (ret < 0) {
			dev_err(&client->dev,
				"failed to write reg 0x%02x, error=%d\n", STK8BAXX_INTCFG1, ret);
			return ret;
		}
	}

	if(chipid == STK8BA53_ID) {
		stk8baxx_flag = 1;
		/* +-4g */
		ret = sensor_write_reg(client, STK8BAXX_RANGESEL, STK8BAXX_RNG_4G);
		if (ret < 0) {
			dev_err(&client->dev,
				"failed to write reg 0x%02x, error=%d\n", STK8BAXX_RANGESEL, ret);
			return ret;
		}
	} else {
		/* +-2g */
		ret = sensor_write_reg(client, STK8BAXX_RANGESEL, STK8BAXX_RNG_2G);
		if (ret < 0) {
			dev_err(&client->dev,
				"failed to write reg 0x%02x, error=%d\n", STK8BAXX_RANGESEL, ret);
			return ret;
		}
	}

	/* ODR = 62.5 Hz */
	ret = sensor_write_reg(client, STK8BAXX_BWSEL, STK8BAXX_BWSEL_INIT_ODR);
	if (ret < 0) {
		dev_err(&client->dev,
			"failed to write reg 0x%02x, error=%d\n", STK8BAXX_RANGESEL, ret);
		return ret;
	}

	return ret;
}

static int gsensor_report_value(struct i2c_client *client,
				struct sensor_axis *axis)
{
	struct sensor_private_data *sensor =
		(struct sensor_private_data *)i2c_get_clientdata(client);
	if (sensor->status_cur == SENSOR_ON) {
		/* Report gsensor sensor information */
		input_report_abs(sensor->input_dev, ABS_X, axis->x);
		input_report_abs(sensor->input_dev, ABS_Y, axis->y);
		input_report_abs(sensor->input_dev, ABS_Z, axis->z);
		input_sync(sensor->input_dev);
	}

	return 0;
}

static int sensor_report_value(struct i2c_client *client)
{
	struct sensor_private_data *sensor =
		(struct sensor_private_data *)i2c_get_clientdata(client);
	struct sensor_platform_data *pdata = sensor->pdata;
	int ret = 0;
	short x = 0, y = 0, z = 0;
	short tmp_x = 0, tmp_y = 0, tmp_z = 0;
	struct sensor_axis axis;
	u8 buffer[6] = { 0 };
	char value = 0;

	if (sensor->ops->read_len < 6) {
		dev_err(&client->dev, "%s:lenth is error,len=%d\n", __func__,
			sensor->ops->read_len);
		return ret;
	}

	memset(buffer, 0, 6);

	/* Data bytes from hardware xL, xH, yL, yH, zL, zH */
	do {
		*buffer = sensor->ops->read_reg;
		ret = sensor_rx_data(client, buffer, sensor->ops->read_len);
		if (ret < 0)
			return ret;
	} while (0);

	x = (buffer[1] << 8 | buffer[0]);
	x >>= 4;
	y = (buffer[3] << 8 | buffer[2]);
	y >>= 4;
	z = (buffer[5] << 8 | buffer[4]);
	z >>= 4;

	dev_dbg(&client->dev, "raw data : x=%d, y=%d, z=%d\n", x, y, z);

	tmp_x = x * STK8BAXX_GRAVITY_STEP;
	tmp_y = y * STK8BAXX_GRAVITY_STEP;
	tmp_z = z * STK8BAXX_GRAVITY_STEP;

	dev_dbg(&client->dev, "temp data : x=%d, y=%d, z=%d\n", tmp_x, tmp_y, tmp_z);

	axis.x = (pdata->orientation[0]) * tmp_x +
			 (pdata->orientation[1]) * tmp_y +
			 (pdata->orientation[2]) * tmp_z;
	axis.y = (pdata->orientation[3]) * tmp_x +
			 (pdata->orientation[4]) * tmp_y +
			 (pdata->orientation[5]) * tmp_z;
	axis.z = (pdata->orientation[6]) * tmp_x +
			 (pdata->orientation[7]) * tmp_y +
			 (pdata->orientation[8]) * tmp_z;

	dev_dbg(&client->dev, "report data : x=%d, y=%d, z=%d\n", axis.x, axis.y, axis.z);

	gsensor_report_value(client, &axis);

	mutex_lock(&(sensor->data_mutex));
	sensor->axis = axis;
	mutex_unlock(&(sensor->data_mutex));

	if ((sensor->pdata->irq_enable) && (sensor->ops->int_status_reg >= 0))
		value = sensor_read_reg(client, sensor->ops->int_status_reg);

	return ret;
}

static int sensor_suspend(struct i2c_client *client)
{
	return 0;
}

static int sensor_resume(struct i2c_client *client)
{
	return 0;
}

struct sensor_operate gsensor_stk8baxx_ops = {
	.name				= "gs_stk8baxx",
	.type				= SENSOR_TYPE_ACCEL,			/*sensor type and it should be correct*/
	.id_i2c				= ACCEL_ID_STK8BAXX,			/*i2c id number*/
	.read_reg			= STK8BAXX_XOUT1,			/*read data*/
	.read_len			= 6,					/*data length*/
	.id_reg				= STK8BAXX_CHIPID,			/*read device id from this register*/
	.id_data			= STK8BA53_ID,				/*device id*/
	.precision			= STK8BAXX_PRECISION,			/*12 bit*/
	.ctrl_reg			= STK8BAXX_POWMODE,			/*enable or disable*/
	/*intterupt status register*/
	.int_status_reg			= STK8BAXX_INTSTS2,
	.range				= {-STK8BAXX_RANGE, STK8BAXX_RANGE},	/*range*/
	.trig				= IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
	.active				= sensor_active,
	.init				= sensor_init,
	.report				= sensor_report_value,
	.suspend			= sensor_suspend,
	.resume				= sensor_resume,
};

static int gsensor_stk8baxx_probe(struct i2c_client *client,
				  const struct i2c_device_id *devid)
{
	return sensor_register_device(client, NULL, devid, &gsensor_stk8baxx_ops);
}

static int gsensor_stk8baxx_remove(struct i2c_client *client)
{
	return sensor_unregister_device(client, NULL, &gsensor_stk8baxx_ops);
}

static const struct i2c_device_id gsensor_stk8baxx_id[] = {
	{"gs_stk8baxx", ACCEL_ID_STK8BAXX},
	{}
};

static struct i2c_driver gsensor_stk8baxx_driver = {
	.probe = gsensor_stk8baxx_probe,
	.remove = gsensor_stk8baxx_remove,
	.shutdown = sensor_shutdown,
	.id_table = gsensor_stk8baxx_id,
	.driver = {
		.name = "gsensor_stk8baxx",
	#ifdef CONFIG_PM
		.pm = &sensor_pm_ops,
	#endif
	},
};

module_i2c_driver(gsensor_stk8baxx_driver);

MODULE_AUTHOR("Jackfahdin<maojie_guo@techvision.com.cn>");
MODULE_DESCRIPTION("stkbaxx gsensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(STK_ACC_DRIVER_VERSION);
