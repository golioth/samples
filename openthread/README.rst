Golioth OpenThread sample
#########################

Overview
********

A demonstration of a Zephyr device connecting to Golioth over IPv6 via Thread.


Requirements
************

- Golioth device PSK credentials
- A running Thread Border Router with NAT64
- Thread network name and PSK key

You can follow https://openthread.io/codelabs/openthread-border-router to setup an OpenThread Border Router (OTBR).


Usage
*****

The firmware will wait for a serial console to attach over USB.

Once USB is attached, it will proceed with connecting to the pre-configured
thread network.

Upon successful connection, the green LED of the nRF52840 dongle will turn on.

When you press a button, a new log event will be sent to the Golioth Log device service.

A full Zephyr shell is available on the USB serial console, along with openthread commands.

You can list available openthread commands by running ``ot help`` on the console.


Building and Running
********************

Configure the following Kconfig options

- GOLIOTH_SYSTEM_CLIENT_PSK_ID  - PSK ID of your Golioth registered device
- GOLIOTH_SYSTEM_CLIENT_PSK     - PSK of your Golioth registered device
- OPENTHREAD_NETWORK_NAME       - Name of your Thread network
- OPENTHREAD_NETWORKKEY         - Network Key of your Thread network

by changing these lines in the ``prj.conf`` configuration file, e.g.:

.. code-block:: cfg

   CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID="my-psk-id@my-project"
   CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK="my-psk"
   CONFIG_OPENTHREAD_NETWORKKEY="00:11:22:33:44:55:66:77:88:99:aa:bb:cc:dd:ee:ff"
   CONFIG_OPENTHREAD_NETWORK_NAME="OpenThreadDemo"


Run on nRF52840 USB Dongle
*****************************

Initialize
==========

You will need the Golioth Zephyr SDK and the nRF Connected SDK for this demo.

Follow the instructions in https://github.com/golioth/golioth-zephyr-sdk#using-with-nrf-connect-sdk to initialize both.

This demo was last tested with nRF Connect SDK version 1.9.1.


Build
=====

.. code-block:: console

   west build -b nrf52840dongle_nrf52840 ./ -- -DOVERLAY_CONFIG="overlay-usb.conf" -DDTC_OVERLAY_FILE="usb.overlay"


Package
=======

Package as a ZIP archive for ``nrfutil``

.. code-block:: console

   nrfutil pkg generate --hw-version 52 --sd-req=0x00 \
    --application build/zephyr/zephyr.hex --application-version 1 build/zephyr/zephyr.zip


Flash
==================

.. code-block:: console

   nrfutil dfu usb-serial -pkg build/zephyr/zephyr.zip -p /dev/ttyACM0

or use the nRF Connect v3.7.1 Programmer tool.

You might need to replace /dev/ttyACM0 with the serial port (tty device) your device is using.
