Golioth weather-sensor Sample
#############################

Overview
********

This sample demonstrates how to stream BME280 sensor data to Golioth. This
include temperature, humidity, and pressure. If a BME280 sensor is not present,
it will send simulated data from a lookup table (100 samples that repeat).

.. code-block:: json

   "environment":{
      "humidity":30.202148
      "press":98.282675
      "temp":27.77
   }

This is a useful data source for testing out integrations like the `Datacake/Golioth integration`_:

* `Datacake guide`_

Requirements
************

- Golioth credentials
- Network connectivity
- Golioth Zephyr SDK v0.1.0

To checkout SDK v0.1.0 for vanilla Zephyr:

```
cd ~/golioth-zephyr-workspace/modules/lib/golioth
git checkout v0.1.0
west update
```

Similarly, to checkout SDK v0.1.0 for NCS Zephyr:

```
cd ~/golioth-ncs-workspace/modules/lib/golioth
git checkout v0.1.0
west update
```

Building and Running
********************

Configure the following Kconfig options based on your Golioth credentials:

- GOLIOTH_SYSTEM_CLIENT_PSK_ID  - PSK ID of registered device
- GOLIOTH_SYSTEM_CLIENT_PSK     - PSK of registered device

by adding these lines to configuration file (e.g. ``prj.conf``):

.. code-block:: cfg

   CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID="my-psk-id"
   CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK="my-psk"

Platform specific configuration
===============================

ESP32
-----

Configure the following Kconfig options based on your WiFi AP credentials:

- GOLIOTH_SAMPLE_WIFI_SSID  - WiFi SSID
- GOLIOTH_SAMPLE_WIFI_PSK   - WiFi PSK

by adding these lines to configuration file (e.g. ``prj.conf`` or
``board/esp32.conf``):

.. code-block:: cfg

   CONFIG_GOLIOTH_SAMPLE_WIFI_SSID="my-wifi"
   CONFIG_GOLIOTH_SAMPLE_WIFI_PSK="my-psk"

Build and flash the project from inside this directory:

.. code-block:: console

   $ west build -b esp32 .
   $ west flash

See `ESP32`_ for details on how to use ESP32 board.

nRF9160 Feather
---------------

Build the project from inside this directory:

.. code-block:: console

   $ west build -b circuitdojo_feather_nrf9160_ns weather-sensor

Enter bootloader and use ``mcumgr`` (or ``newtmgr``) to flash firmware:

.. code-block:: console

   $ mcumgr --conntype serial --connstring /dev/ttyUSB0,baudrate=1000000 build/zephyr/app_update.bin

See `nRF9160 Feather Programming and Debugging`_ for details.

Sample output
=============

This is the output from the serial console:

.. code-block:: console

   [00:04:40.939,880] <inf> golioth_bme380: temp: 27.750000; press: 98.283949; humidity: 30.223632
   [00:04:45.941,589] <inf> golioth_bme380: temp: 27.750000; press: 98.283781; humidity: 30.223632
   [00:04:50.943,267] <inf> golioth_bme380: temp: 27.750000; press: 98.283558; humidity: 30.223632

TODO
****
- Add mcumgr support to this repo, since it is no longer supported in the Golioth
  Zephyr SDK after v0.1.0. That would allow this sample to stay up to date with the SDK,
  and not be stuck on v0.1.0.

.. _Datacake/Golioth integration: https://docs.golioth.io/cloud/output-streams/datacake
.. _Datacake guide: https://docs.datacake.de/integrations/golioth
.. _ESP32: https://docs.zephyrproject.org/latest/boards/xtensa/esp32/doc/index.html
.. _nRF9160 Feather Programming and Debugging: https://docs.jaredwolff.com/nrf9160-programming-and-debugging.html
