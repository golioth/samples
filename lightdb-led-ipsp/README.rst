Golioth Light DB LED sample over IPSP
#####################################

Overview
********

This Light DB application demonstrates how to connect with Golioth, access
Light DB and change state of onboard LEDs.

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

Testing with a Linux host
=========================

Setup UDP packets forwarding from your host machine to Golioth server:

.. code-block:: console

   # socat -v -x udp6-listen:5684,reuseaddr,fork udp:104.197.107.212:5684

Make sure the Linux kernel has been built with Bluetooth 6LoWPAN module
(CONFIG_BT_6LOWPAN=y) then proceed to enable it with with the following commands
(as root):

.. code-block:: console

   # modprobe bluetooth_6lowpan
   # echo 1 > /sys/kernel/debug/bluetooth/6lowpan_enable

If you connected your board to a UART console, you will see an output similar to
(may vary slightly by application and Zephyr versions):

.. code-block:: console

   [bt] [WRN] set_static_addr: Using temporary static random address
   [bt] [INF] show_dev_info: Identity: cb:af:14:57:d8:6e (random)
   [bt] [INF] show_dev_info: HCI: version 5.0 (0x09) revision 0x0000, manufacturer 0xffff
   [bt] [INF] show_dev_info: LMP: version 5.0 (0x09) subver 0xffff
   [bt] [WRN] bt_pub_key_gen: ECC HCI commands not available
   [ipsp] [INF] init_app: Run IPSP sample
   [ipsp] [INF] listen: Starting to wait

The output above shows the BLE address assigned to your board for the
current session; the address will be different on subsequent sessions.

Alternatively, you may scan for your board on the host. The modern way to do
that is using ``bluetoothctl`` utility (included in the recent versions of
BlueZ package) and its ``scan on`` command:

.. code-block:: console

   $ bluetoothctl
   [NEW] Controller A3:24:97:EB:D6:23 ubuntu-0 [default]
   [NEW] Device D7:5C:D6:18:14:87 Zephyr
   [NEW] Device E1:E7:F9:56:EC:06 Zephyr
   [NEW] Device C8:12:C5:08:86:E1 Zephyr
   [bluetooth]# scan on
   Discovery started
   [NEW] Device DC:98:FB:22:CA:3A Zephyr

When started, ``bluetoothctl`` shows all BLE (and likely, BT/EDR) devices it
knows about. As discussed above, the IPSP uses static random addresses, so
entries for previously connected devices, as shown above, can accumulate and
become stale. You need to be extra careful to find an entry for the active
address. The best approach may be to reset your board after issuing
``scan on`` command. This way it will reinitialize with the BLE address
which will be discovered after the command.

As an alternative to ``bluetoothctl``, you can use the legacy ``hcitool``
utility which talks directly to hardware and always shows fresh scan results:

.. code-block:: console

   $ sudo hcitool lescan
   LE Scan ...
   CB:AF:14:57:D8:6E (unknown)
   CB:AF:14:57:D8:6E Test IPSP node

After you have found the board's BLE address, connect to the board (as root):

.. code-block:: console

   # echo "connect <bdaddr> <type>" > /sys/kernel/debug/bluetooth/6lowpan_control

Where ``<bdaddr>`` is the BLE address and ``<type>`` is BLE address type:
1 for public address and 2 for random address. As you can see from
the IPSP sample output above, it uses a static random address. So, with the
sample output above, the command will be:

.. code-block:: console

   # echo "connect CB:AF:14:57:D8:6E 2" > /sys/kernel/debug/bluetooth/6lowpan_control

Once connected a dedicated interface will be created, usually bt0. You can verify this
with the following command:

.. code-block:: console

   # ifconfig
   bt0       Link encap:UNSPEC  HWaddr F8-2F-A8-FF-FE-EB-6D-8C-00-00-00-00-00-00-00-00
             inet6 addr: fe80::fa2f:a8ff:feeb:6d8c/64 Scope:Link
             UP POINTOPOINT RUNNING MULTICAST  MTU:1280  Metric:1
             RX packets:2 errors:0 dropped:3 overruns:0 frame:0
             TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
             collisions:0 txqueuelen:1000
             RX bytes:92 (92.0 B)  TX bytes:233 (233.0 B)

As can be seen from the output, only a link-local IPv6 address was assigned
to the interface.

At this point, you can test IPv6 connectivity (and discover your board's IPv6
address) by pinging "All local-link nodes" IPv6 address:

.. code-block:: console

   # ping6 -I bt0 ff02::1
   PING ff02::1(ff02::1) from fe80::fa54:a8ff:feeb:218f bt0: 56 data bytes
   64 bytes from fe80::fa54:a8ff:feeb:218f: icmp_seq=1 ttl=64 time=0.088 ms
   64 bytes from fe80::c9af:14ff:fe57:d86e: icmp_seq=1 ttl=64 time=285 ms (DUP!)

For each ping packet, both your host and the BLE board send a reply. You
can see the board's reply marked as ``(DUP!)``. You can ping the board
directly with:

.. code-block:: console

   # ping6 fe80::c9af:14ff:fe57:d86e%bt0
   PING fe80::c9af:14ff:fe57:d86e%bt0(fe80::c9af:14ff:fe57:d86e) 56 data bytes
   64 bytes from fe80::c9af:14ff:fe57:d86e: icmp_seq=1 ttl=64 time=177 ms
   64 bytes from fe80::c9af:14ff:fe57:d86e: icmp_seq=2 ttl=64 time=53.0 ms

Note that the command uses a "scoped IPv6 address", where the scope is
defined by the networking interface, with ``%bt0`` appended in this case.
A specification like that is an alternative to passing ``-I bt0`` to
``ping6`` (and works with other networking tools like ``telnet``, ``nc``,
``curl``, etc.)

While we can use a link-local address, it's not very convenient, as it must be
scoped and will change on each run. Instead, the IPSP sample is configured with
``2001:db8::1`` static address and we'll configure the host's interface to
access that address by configuring ``bt0`` with the complementary address
``2001:db8::2``:

.. code-block:: console

   # ip address add 2001:db8::2/64 dev bt0

Now we can ping the board's static address with:

.. code-block:: console

   # ping6 2001:db8::1
   PING 2001:db8::1(2001:db8::1) 56 data bytes
   64 bytes from 2001:db8::1: icmp_seq=1 ttl=64 time=282 ms


Sample output
=============

This is the output from the serial console:

.. code-block:: console

   [00:00:00.261,077] <inf> golioth_system: Initializing
   [00:00:00.263,183] <inf> bt_hci_core: HW Platform: Nordic Semiconductor (0x0002)
   [00:00:00.263,183] <inf> bt_hci_core: HW Variant: nRF52x (0x0002)
   [00:00:00.263,214] <inf> bt_hci_core: Firmware: Standard Bluetooth controller (0x00) Version 2.6 Build 99
   [00:00:00.263,977] <inf> bt_hci_core: Identity: D9:24:8E:86:A4:F6 (random)
   [00:00:00.263,977] <inf> bt_hci_core: HCI: version 5.2 (0x0b) revision 0x0000, manufacturer 0x05f1
   [00:00:00.263,977] <inf> bt_hci_core: LMP: version 5.2 (0x0b) subver 0xffff
   [00:00:00.264,526] <inf> net_config: Initializing network
   [00:00:00.264,526] <inf> net_config: Waiting interface 1 (0x20003220) to be up...
   [00:00:05.683,288] <inf> net_config: Interface 1 (0x20003220) coming up
   [00:00:05.683,868] <wrn> bt_l2cap: No credits to transmit packet
   [00:00:05.784,393] <inf> net_config: IPv6 address: 2001:db8::1
   [00:00:05.784,820] <dbg> golioth_lightdb.main: Start Light DB LED sample
   [00:00:05.784,912] <inf> golioth_system: Starting connect
   [00:00:05.791,748] <wrn> bt_l2cap: No credits to transmit packet
   [00:00:08.883,575] <wrn> bt_l2cap: No credits to transmit packet
   [00:00:08.883,728] <wrn> bt_l2cap: No credits to transmit packet
   [00:00:09.638,031] <inf> golioth_system: Client connected!
   [00:00:09.835,296] <dbg> golioth_lightdb: Payload
                                             a3 61 30 f5 61 32 f5 61  31 f5                   |.a0.a2.a 1.
   [00:00:09.835,327] <inf> golioth_lightdb: LED 0 -> 1
   [00:00:09.835,357] <inf> golioth_lightdb: LED 2 -> 1
   [00:00:09.835,388] <inf> golioth_lightdb: LED 1 -> 1

Monitor counter value
=====================

Device increments counter every 5s and updates ``/counter`` resource in Light DB
with its value. Current value can be fetched using following command:

.. code-block:: console

   goliothctl lightdb get <device-id> /counter

Control LEDs
============

Multiple LEDs can be changed simultaneously using following command:

.. code-block:: console

   goliothctl lightdb set <device-id> /led -b '{"0":true,"1":false,"2":true,"3":true}'

This request should result in following serial console output:

.. code-block:: console

   [00:00:04.050,000] <dbg> golioth_lightdb: Payload
                                             a4 61 33 f5 61 30 f5 61  31 f4 61 32 f5          |.a3.a0.a 1.a2.
   [00:00:04.050,000] <inf> golioth_lightdb: LED 3 -> 1
   [00:00:04.050,000] <inf> golioth_lightdb: LED 0 -> 1
   [00:00:04.050,000] <inf> golioth_lightdb: LED 1 -> 0
   [00:00:04.050,000] <inf> golioth_lightdb: LED 2 -> 1

Additionally board LEDs will be changed, if they are configured in device-tree
as:

- ``/aliases/led0``
- ``/aliases/led1``
- ``/aliases/led2``
- ``/aliases/led3``

TODO
****
- Add tinycbor support to this repo, since it is no longer supported in the Golioth
  Zephyr SDK after v0.1.0. That would allow this sample to stay up to date with the SDK,
  and not be stuck on v0.1.0.
