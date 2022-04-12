/*
 * Copyright (c) 2022 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <data/json.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(golioth_desired_state, LOG_LEVEL_DBG);

#include <net/coap.h>
#include <net/golioth/system_client.h>
#include <net/golioth/wifi.h>

#include <stdlib.h>

static struct golioth_client *client = GOLIOTH_SYSTEM_CLIENT_GET();
static struct coap_reply coap_replies[1];

uint8_t led_state = 1;
bool send_status_update = false;

/* Framework for parsing settings JSON data received from Golioth */
#define LED_OFF         (1 << 0)
#define LED_ON          (1 << 1)

struct desired_state {
	uint32_t led_off;
	uint32_t led_on;
};

static const struct json_obj_descr desired_state_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct desired_state, led_off, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct desired_state, led_on, JSON_TOK_NUMBER),
};

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE        DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      { 0 });
static struct gpio_callback button_cb_data;

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     { 0 });

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
	led_state ^= 0x01;
	gpio_pin_set_dt(&led, led_state);
	send_status_update = true;
}

/*
 * This function is registed to be called when the data
 * stored at `/observed` changes.
 */
static int on_update(const struct coap_packet *response,
		     struct coap_reply *reply,
		     const struct sockaddr *from)
{
	int err;
	char str[64];
	uint16_t payload_len;
	const uint8_t *payload;

	payload = coap_packet_get_payload(response, &payload_len);
	if (!payload) {
		LOG_WRN("packet did not contain data");
		return -ENOMSG;
	}

	if (payload_len + 1 > ARRAY_SIZE(str)) {
		payload_len = ARRAY_SIZE(str) - 1;
	}

	memcpy(str, payload, payload_len);
	str[payload_len] = '\0';

	LOG_DBG("payload: %s", log_strdup(str));

	struct desired_state received_state;

	int ret = json_obj_parse(str, sizeof(str),
				 desired_state_descr,
				 ARRAY_SIZE(desired_state_descr),
				 &received_state);

	/* Get timestamps from endpoints and delete them to indicate they have been serviced */
	uint32_t on_timestamp = 0;
	uint32_t off_timestamp = 0;
	if (ret < 0) {
		LOG_ERR("JSON Parse Error: %d", ret);
	} else   {
		if (ret & LED_OFF) {
			off_timestamp = received_state.led_off;
			err = golioth_lightdb_delete(client,
						     GOLIOTH_LIGHTDB_PATH("cmd/led_off")
						     );
			if (err) {
				LOG_WRN("Failed to delete cmd/led_off endpoint: %d", err);
			}
		}
		if (ret & LED_ON) {
			on_timestamp = received_state.led_on;
			err = golioth_lightdb_delete(client,
						     GOLIOTH_LIGHTDB_PATH("cmd/led_on")
						     );
			if (err) {
				LOG_WRN("Failed to delete cmd/led_on endpoint: %d", err);
			}

		}
	}

	/* Process the LED changes based on the timestamps */
	uint8_t new_led_val = 0;
	if ((on_timestamp | off_timestamp) > 0) {
		if (on_timestamp > off_timestamp) {
			new_led_val = 1;
		}
		LOG_DBG("setting LED to: %d", new_led_val);
		led_state = new_led_val;
		gpio_pin_set_dt(&led, led_state);
		send_status_update = true; /* Flag an update to actual state on cloud */
	}

	return 0;
}

/*
 * In the `main` function, this function is registed to be
 * called when the device connects to the Golioth server.
 */
static void golioth_on_connect(struct golioth_client *client)
{
	struct coap_reply *observe_reply;
	int err;

	coap_replies_clear(coap_replies, ARRAY_SIZE(coap_replies));

	observe_reply = coap_reply_next_unused(coap_replies,
					       ARRAY_SIZE(coap_replies));

	/*
	 * Observe the data stored at `/observed` in LightDB.
	 * When that data is updated, the `on_update` callback
	 * will be called.
	 * This will get the value when first called, even if
	 * the value doesn't change.
	 */
	err = golioth_lightdb_observe(client,
				      GOLIOTH_LIGHTDB_PATH("cmd"),
				      COAP_CONTENT_FORMAT_TEXT_PLAIN,
				      observe_reply, on_update);

	if (err) {
		LOG_WRN("failed to observe lightdb path: %d", err);
	}
}

/*
 * In the `main` function, this function is registed to be
 * called when the device receives a packet from the Golioth server.
 */
static void golioth_on_message(struct golioth_client *client,
			       struct coap_packet *rx)
{
	/*
	 * In order for the observe callback to be called,
	 * we need to call this function.
	 */
	coap_response_received(rx, NULL, coap_replies,
			       ARRAY_SIZE(coap_replies));
}

void main(void)
{
	int ret;

	LOG_DBG("Start Desired State sample");

	if (!device_is_ready(button.port)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
		       ret, button.port->name, button.pin);
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	printk("Set up button at %s pin %d\n", button.port->name, button.pin);

	if (led.port && !device_is_ready(led.port)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led.port->name);
		led.port = NULL;
	}
	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}

	gpio_pin_set_dt(&led, led_state);
	send_status_update = true;

	if (IS_ENABLED(CONFIG_GOLIOTH_SAMPLE_WIFI)) {
		LOG_INF("Connecting to WiFi");
		wifi_connect();
	}

	client->on_connect = golioth_on_connect;
	client->on_message = golioth_on_message;
	golioth_system_client_start();

	while (true) {
		k_sleep(K_MSEC(150));
		if (send_status_update) {
			/* Flag will be set by button callback */
			send_status_update = false;
			LOG_DBG("Updating LED status to: %d", led_state);
			char sbuf[2] = "0";
			if (led_state & 0x01) {
				sbuf[0] = '1';
			}
			int err = golioth_lightdb_set(client,
						      GOLIOTH_LIGHTDB_PATH("state/led"),
						      COAP_CONTENT_FORMAT_TEXT_PLAIN,
						      sbuf, strlen(sbuf));
			if (err) {
				LOG_WRN("Failed to update state/led: %d", err);
			}
		}
	}
}
