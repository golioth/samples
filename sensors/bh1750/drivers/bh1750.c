/*
  This is a library for the BH1750FVI Digital Light Sensor breakout board.
  The BH1750 board uses I2C for communication. Two pins are required to
  interface to the device. Configuring the I2C bus is expected to be done
  in user code. The BH1750 library doesn't do this automatically.
  Written by Christopher Laws, March, 2013.
*/

#include "bh1750.h"

#include <device.h>
#include <drivers/sensor.h>
#include <drivers/i2c.h>

#define DT_DRV_COMPAT rohm_bh1750

#define BH1750_ONE_TIME_H_RES_MODE	0x20

static const struct device *bh1750_dev;

static int bh1750_sample_fetch(const struct device *dev, enum sensor_channel chan) {

	struct bh1750_data *data = dev->data;
	uint8_t status;

	if (chan != SENSOR_CHAN_LIGHT) {
		LOG_ERR("Unsupported sensor channel");
		return -ENOTSUP;
	}

	float level = 0;
	uint16_t tmp = 0;

	uint8_t io_buf[2] = {BH1750_ONE_TIME_H_RES_MODE,0};

	if (i2c_write(dev, io_buf, 1, DT_REG_ADDR(DT_DRV_INST(0)))){
		LOG_ERR("Could not write ONE TIME HIGH RES instruction samples");
		return -EIO;
	}
	k_sleep(K_MSEC(200));
	if (i2c_read(dev, io_buf, 2, DT_REG_ADDR(DT_DRV_INST(0)))){
		LOG_ERR("Could not read two bytes high resolution sample");
		return -EIO;
	}

	data->sample_amb_light[0] = io_buf[0];
	data->sample_amb_light[0] <<8;
	data->sample_amb_light[0] = data->sample_amb_light[0] || data->sample_amb_light[1];

	return 0;	
}

static int bh1750_channel_get(const struct device *dev,
			      enum sensor_channel chan,
			      struct sensor_value *val)
{
	struct bh1750_data *data = dev->data;

		val->val1 = sys_le16_to_cpu(
			data->sample_amb_light[0]);
		val->val2 = 0;
	return 0;
}

static const struct sensor_driver_api bh1750_driver_api = {
	.sample_fetch = &bh1750_sample_fetch,
	.channel_get = &bh1750_channel_get,
};

static int bh1750_init(const struct device *dev)
{
	bh1750_dev = dev;

	struct bh1750_data *data = bh1750_dev->data;
	data->i2c = device_get_binding(DT_BUS_LABEL(DT_DRV_INST(0)));

	if (data->i2c == NULL) {
		LOG_ERR("Failed to get pointer to %s device!",
			DT_BUS_LABEL(DT_DRV_INST(0)));

		return -EINVAL;
	}

	return 0;
};

static struct bh1750_data bh1750_data;

DEVICE_DEFINE(bh1750, DT_INST_LABEL(0),
	      bh1750_init, NULL, &bh1750_data, NULL,
	      POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &bh1750_driver_api);
// static inline int i2c_read(const struct device *dev, uint8_t *buf,
// 			   uint32_t num_bytes, uint16_t addr)
// {
  // Measurement result will be stored here
//   float level = -1.0;

//   // Read two bytes from the sensor, which are low and high parts of the sensor
//   // value
//   if (2 == I2C->requestFrom((int)BH1750_I2CADDR, (int)2)) {
//     unsigned int tmp = 0;
//     tmp = __wire_read();
//     tmp <<= 8;
//     tmp |= __wire_read();
//     level = tmp;
//   }

//   int ret = i2c_burst_read

//   	if (i2c_burst_read(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
// 			   BH1749_RED_DATA_LSB,
// 			   (uint8_t *)data->sample_amb_light,
// 			   BH1750_SAMPLES_TO_FETCH * sizeof(uint16_t))) {
// 		LOG_ERR("Could not read sensor samples");
// 		return -EIO;
// 	}


// 	static int sc_ctrl(const struct device *dev,
// 		   uint8_t set,
// 		   uint8_t clear)
// {
// 	struct ds3231_data *data = dev->data;
// 	const struct ds3231_config *cfg = dev->config;
// 	struct register_map *rp = &data->registers;
// 	uint8_t ctrl = (rp->ctrl & ~clear) | set;
// 	int rc = ctrl;

// 	if (rp->ctrl != ctrl) {
// 		uint8_t buf[2] = {
// 			offsetof(struct register_map, ctrl),
// 			ctrl,
// 		};
// 		rc = i2c_write(data->i2c, buf, sizeof(buf), cfg->addr);
// 		if (rc >= 0) {
// 			rp->ctrl = ctrl;
// 			rc = ctrl;
// 		}
// 	}
// 	return rc;
// }

//     static inline int i2c_burst_read(const struct device *dev,
// 				 uint16_t dev_addr,
// 				 uint8_t start_addr,
// 				 uint8_t *buf,
// 				 uint32_t num_bytes)
// {
// 	return i2c_write_read(dev, dev_addr,
// 			      &start_addr, sizeof(start_addr),
// 			      buf, num_bytes);
// }

// static inline int i2c_write_read(const struct device *dev, uint16_t addr,
// 				 const void *write_buf, size_t num_write,
// 				 void *read_buf, size_t num_read)

//   //lastReadTimestamp = millis();

//   if (level != -1.0) {
// // Print raw value if debug enabled
// // #ifdef BH1750_DEBUG
// //     Serial.print(F("[BH1750] Raw value: "));
// //     Serial.println(level);
// // #endif
//     printk("[BH1750] Raw value: %f",level);

//     if (BH1750_MTreg != BH1750_DEFAULT_MTREG) {
//       level *= (float)((byte)BH1750_DEFAULT_MTREG / (float)BH1750_MTreg);
// // Print MTreg factor if debug enabled
// #ifdef BH1750_DEBUG
//       Serial.print(F("[BH1750] MTreg factor: "));
//       Serial.println(
//           String((float)((byte)BH1750_DEFAULT_MTREG / (float)BH1750_MTreg)));
// #endif
//     }
//     if (BH1750_MODE == BH1750::ONE_TIME_HIGH_RES_MODE_2 ||
//         BH1750_MODE == BH1750::CONTINUOUS_HIGH_RES_MODE_2) {
//       level /= 2;
//     }
//     // Convert raw value to lux
//     level /= BH1750_CONV_FACTOR;

// // Print converted value if debug enabled
// #ifdef BH1750_DEBUG
//     Serial.print(F("[BH1750] Converted float value: "));
//     Serial.println(level);
// #endif
//   }

//   return level;
// }

// static int bh1750_read(struct bh1750_data *data, int *val)
// {
// 	int ret;
// 	__be16 result;
// 	const struct bh1750_chip_info *chip_info = data->chip_info;
// 	unsigned long delay = chip_info->mtreg_to_usec * data->mtreg;

// 	/*
// 	 * BH1721 will enter continuous mode on receiving this command.
// 	 * Note, that this eliminates need for bh1750_resume().
// 	 */
// 	ret = i2c_smbus_write_byte(data->client, BH1750_ONE_TIME_H_RES_MODE);
// 	if (ret < 0)
// 		return ret;

// 	usleep_range(delay + 15000, delay + 40000);

// 	ret = i2c_master_recv(data->client, (char *)&result, 2);
// 	if (ret < 0)
// 		return ret;

// 	*val = be16_to_cpu(result);

// 	return 0;
// }

