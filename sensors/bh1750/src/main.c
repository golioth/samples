/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/__assert.h>
//#include "drivers/bh1750.h"

//extern static const struct device *bh1750_dev;

/* This sets up  the sensor to signal valid data when a threshold
 * value is reached.
 */
static void process(const struct device *dev)
{
	int ret;
	int val;
	
	uint16_t sensor_val;
	//struct bh1750_data *data = dev->data;
	struct sensor_value temp_val;




	

	//sensor_val = be16_to_cpu(dev->data   .sample_amb_light[0]);

	while (1) {

		ret = sensor_sample_fetch_chan(dev, SENSOR_CHAN_LIGHT);

		ret = sensor_channel_get(dev, SENSOR_CHAN_LIGHT, &temp_val);

		if (ret) {
			printk("sensor_channel_get failed ret %d\n", ret);
			return;
		}
		printk("BH1750 LUX: %d\n", temp_val.val1);
        
		k_sleep(K_MSEC(2000));
	}
}

void main(void)
{
	const struct device *dev;

	if (IS_ENABLED(CONFIG_LOG_BACKEND_RTT)) {
		/* Give RTT log time to be flushed before executing tests */
		k_sleep(K_MSEC(500));
	}
	dev = device_get_binding("BH1750");
	if (dev == NULL) {
		printk("Failed to get device binding\n");
		return;
	}
	printk("device is %p, name is %s\n", dev, dev->name);
	process(dev);
}