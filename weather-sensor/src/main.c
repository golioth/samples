/*
 * Copyright (c) 2022 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(golioth_bme380, LOG_LEVEL_DBG);

#include <net/coap.h>
#include <net/golioth/system_client.h>
#include <net/golioth/wifi.h>

#include <devicetree.h>
#include <drivers/sensor.h>
#include <stdlib.h>

#include "simulated_data.h"

static struct golioth_client *client = GOLIOTH_SYSTEM_CLIENT_GET();

/*
 * Initialize the BME280 sensor
 */
static const struct device *get_bme280_device(void)
{
	const struct device *dev = DEVICE_DT_GET_ANY(bosch_bme280);

	if (dev == NULL) {
		/* No such node, or the node does not have status "okay". */
		printk("\nError: no device found.\n");
		return NULL;
	}

	if (!device_is_ready(dev)) {
		printk("\nError: Device \"%s\" is not ready; "
		       "check the driver initialization logs for errors.\n",
		       dev->name);
		return NULL;
	}

	printk("Found device \"%s\", getting sensor data\n", dev->name);
	return dev;
}

/*
 * Simulate data from a lookup table if no BME280 sensor is present
 */
static void get_sim_data(struct sensor_value *humid, struct sensor_value *press, struct sensor_value *temp)
{
	static uint8_t idx = 0;
	humid->val1 = h_p_t_simdata[idx][0];
	humid->val2 = h_p_t_simdata[idx][1];
	press->val1 = h_p_t_simdata[idx][2];
	press->val2 = h_p_t_simdata[idx][3];
	temp->val1 = h_p_t_simdata[idx][4];
	temp->val2 = h_p_t_simdata[idx][5];
	if (++idx >= 100) idx = 0;
}

/*
 * This function sends BME380 data to lightdb stream `/environment`.
 */
static void temperature_set(uint32_t tempD, uint32_t tempF, uint32_t pressD, uint32_t pressF, uint32_t humD, uint32_t humF)
{
	char sbuf[60];
	int err;

	snprintk(sbuf, sizeof(sbuf) - 1, "{\"temp\":%d.%06d,\"press\":%d.%06d,\"humidity\":%d.%06d}",
		      tempD, tempF, pressD, pressF, humD, humF);

	err = golioth_lightdb_set(client,
				  GOLIOTH_LIGHTDB_STREAM_PATH("environment"),
				  COAP_CONTENT_FORMAT_TEXT_PLAIN,
				  sbuf, strlen(sbuf));
	if (err) {
		LOG_WRN("Failed to update counter: %d", err);
	}
}

void main(void)
{
	const struct device *dev = get_bme280_device();

	if (dev==NULL) LOG_DBG("Start sending simulated data (no sensor found)");
	else LOG_DBG("Start BME380 sample");

	if (IS_ENABLED(CONFIG_GOLIOTH_SAMPLE_WIFI)) {
		LOG_INF("Connecting to WiFi");
		wifi_connect();
	}

	golioth_system_client_start();

	struct sensor_value temp, press, humidity;
	while (true) {
		if (dev==NULL)
		{
			// No BME280 sensor found, send simulated data instead
			get_sim_data(&humidity, &press, &temp);
		}
		else
		{
			sensor_sample_fetch(dev);
			sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
			sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);
			sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);
		}		
			
		LOG_INF("temp: %d.%06d; press: %d.%06d; humidity: %d.%06d",
			temp.val1, temp.val2, press.val1, press.val2,
			humidity.val1, humidity.val2);

		temperature_set(temp.val1, temp.val2, press.val1, press.val2,
			humidity.val1, humidity.val2);

		k_sleep(K_SECONDS(5));
	}
}
