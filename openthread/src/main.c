#include <dk_buttons_and_leds.h>
#include <logging/log.h>

#include <drivers/uart.h>
#include <usb/usb_device.h>

#include <net/golioth/system_client.h>

#include <net/openthread.h>
#include <openthread/thread.h>

LOG_MODULE_REGISTER(main, CONFIG_GOLIOTH_THREAD_LOG_LEVEL);

#define CONSOLE_LABEL DT_LABEL(DT_CHOSEN(zephyr_console))
#define OT_CONNECTION_LED DK_LED1

static struct k_work on_connect_work;
static struct k_work on_disconnect_work;

static struct golioth_client *client = GOLIOTH_SYSTEM_CLIENT_GET();

static void golioth_on_message(struct golioth_client *client,
			       struct coap_packet *rx)
{
	uint16_t payload_len;
	const uint8_t *payload;
	uint8_t type;

	type = coap_header_get_type(rx);
	payload = coap_packet_get_payload(rx, &payload_len);

	if (!IS_ENABLED(CONFIG_LOG_BACKEND_GOLIOTH) && payload) {
		LOG_HEXDUMP_DBG(payload, payload_len, "Payload");
	}
}

static void on_ot_connect(struct k_work *item)
{
	ARG_UNUSED(item);

	dk_set_led_on(OT_CONNECTION_LED);

	client->on_message = golioth_on_message;
	golioth_system_client_start();
}

static void on_ot_disconnect(struct k_work *item)
{
	ARG_UNUSED(item);

	dk_set_led_off(OT_CONNECTION_LED);
}


static void on_button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	LOG_DBG("Button %d %d", button_state, has_changed);

	if ((buttons & DK_BTN1_MSK) && button_state == 1) {
		golioth_send_hello(client);
	}
}

static void on_thread_state_changed(uint32_t flags, void *context)
{
	struct openthread_context *ot_context = context;

	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
			k_work_submit(&on_connect_work);
			break;

		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
		default:
			k_work_submit(&on_disconnect_work);
			break;
		}
	}
}

void main(void)
{
	int ret;

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_shell_uart), zephyr_cdc_acm_uart)
	const struct device *dev;
	uint32_t dtr = 0U;

	ret = usb_enable(NULL);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return;
	}

	dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (dev == NULL) {
		LOG_ERR("Failed to find specific UART device");
		return;
	}

	LOG_INF("Waiting for host to be ready to communicate");

	/* Data Terminal Ready - check if host is ready to communicate */
	while (!dtr) {
		ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		if (ret) {
			LOG_ERR("Failed to get Data Terminal Ready line state: %d",
				ret);
			continue;
		}
		k_msleep(100);
	}

	/* Data Carrier Detect Modem - mark connection as established */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
	/* Data Set Ready - the NCP SoC is ready to communicate */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
#endif

	LOG_INF("Start Golioth Thread sample");

	ret = dk_buttons_init(on_button_changed);
	if (ret) {
		LOG_ERR("Cannot init buttons (error: %d)", ret);
		return;
	}

	ret = dk_leds_init();
	if (ret) {
		LOG_ERR("Cannot init leds, (error: %d)", ret);
		return;
	}

	k_work_init(&on_connect_work, on_ot_connect);
	k_work_init(&on_disconnect_work, on_ot_disconnect);

	openthread_set_state_changed_cb(on_thread_state_changed);
	openthread_start(openthread_get_default_context());
}
