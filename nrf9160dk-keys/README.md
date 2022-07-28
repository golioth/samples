# nrf9160dk-keys

Manage the PSK-ID and PSK secure storage on the nrf9160. This app is based on NCS 1.7.1.

At run time the app will look for keys stored in the modem, and keys provided in the prj.conf (or other Kconfig delivery like command line arguments). If keys are found in the prj.conf file, the user will be provided with the option to press Button 1 on the nrf9160dk board to write them to modem memory. If keys are found in modem memory, the user will be provided with the option to press Button 2 to delete the keys from modem memory.

## prj.conf credentials format

Entries for credentials already exist in the prj.conf file but are commented out. Uncomment these lines and replace with your actual Golioth PSK-ID/PSK.

```
CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID="psk-id"
CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK="psk"
```

You may also choose to specify a separate file that includes these values. Place the lines as seen above and call the file by location in the build command:

```
west build -b nrf9160dk_nrf9160_ns . -D OVERLAY_CONFIG=~/Desktop/nrf9160dk.conf -p
```

## Example terminal output

```
*** Booting Zephyr OS build v2.6.99-ncs1-1  ***

Checking credentials...
=======================
     FOUND: PSK-ID with sec_tag=1 stored in modem
     FOUND: PSK with sec_tag=1 stored in modem
     FOUND: Golioth PSK-ID from Kconfig (ie: prj.conf)
     FOUND: Golioth PSK from Kconfig (ie: prj.conf)

Manage Modem PSK-ID/PSK Storage:
================================

Press Button 1 to write Golioth credentials from Kconfig (ie: prj.conf) to modem
Press Button 2 to ERASE the PSK-ID and PSK credentials with sec_tag=1 from modem (DANGER!)

Attempting to delete credentials with sec_tag=1 from modem...
Deleting PSK-ID from modem...
Success!
Deleting PSK from modem...
Success!

Checking credentials...
=======================
    ABSENT: PSK-ID with sec_tag=1 stored in modem
    ABSENT: PSK with sec_tag=1 stored in modem
     FOUND: Golioth PSK-ID from Kconfig (ie: prj.conf)
     FOUND: Golioth PSK from Kconfig (ie: prj.conf)

Manage Modem PSK-ID/PSK Storage:
================================

Press Button 1 to write Golioth credentials from Kconfig (ie: prj.conf) to modem

Attempting to write Golioth PSK-ID to modem with sec_tag=1...
Success!
Encoding PSK...
Attempting to write Golioth PSK to modem with sec_tag=1...
Success!

Checking credentials...
=======================
     FOUND: PSK-ID with sec_tag=1 stored in modem
     FOUND: PSK with sec_tag=1 stored in modem
     FOUND: Golioth PSK-ID from Kconfig (ie: prj.conf)
     FOUND: Golioth PSK from Kconfig (ie: prj.conf)

Manage Modem PSK-ID/PSK Storage:
================================

Press Button 1 to write Golioth credentials from Kconfig (ie: prj.conf) to modem
Press Button 2 to ERASE the PSK-ID and PSK credentials with sec_tag=1 from modem (DANGER!)
```