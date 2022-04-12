Golioth Desired-State Sample
############################

Overview
********

Demonstrate using "actual state" and "desired state" data objects on LightDB.

Pressing the button toggles the LED on and off. This change will be written to
the ``state`` endpoint on LightDB.

Commands can be sent to the board by writing a timestamp to ``cmd/led_off`` or
``cmd/led_on``. If both endpoints are present, the on with the the more recent
timestamp will be serviced. After the device services this desired state, it
will delete these endpoints.

.. code-block:: json

   "cmd" : {
      "led_off": 1649694069,
      "led_on": 1649694041
   }

Requirements
************

- Dev board that has ``sw0`` and ``led0`` defined in devicetree
- Golioth credentials
- Network connectivity

Building and Running
********************

Configure the following Kconfig options based on your Golioth credentials:

- GOLIOTH_SYSTEM_CLIENT_PSK_ID  - PSK ID of registered device
- GOLIOTH_SYSTEM_CLIENT_PSK     - PSK of registered device

by adding these lines to configuration file (e.g. ``prj.conf``):

.. code-block:: cfg

   CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID="my-psk-id"
   CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK="my-psk"

Build and flash:

.. code-block:: console

   $ west build -b <board name> .
   $ west flash
