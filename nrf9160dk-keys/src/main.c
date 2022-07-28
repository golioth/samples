/*
 * Copyright (c) 2022 Golioth Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <modem/modem_key_mgmt.h>
#include <dk_buttons_and_leds.h>
#include <string.h>
#include <sys/util.h>

#define BUTTON_1    1<<0
#define BUTTON_2    1<<1
uint8_t button_mask;

static void button_handler(uint32_t button_states, uint32_t has_changed)
{
	if (has_changed & button_states & BUTTON_1) {
		button_mask = BUTTON_1;
	}
    else if (has_changed & button_states & BUTTON_2) {
		button_mask = BUTTON_2;
	}
}

int write_credentials_to_modem(bool id_in_prj, bool psk_in_prj)
{
    int err;

    #define AT_BUF_LEN  512
    char at_buf[AT_BUF_LEN] = {0};

    if ((id_in_prj | psk_in_prj) == 0)
    {
        /* This should never happen */
        printk("ERROR: No credentials available to write\n");
        return -EINVAL;
    }
    if ((id_in_prj & psk_in_prj) == 0)
    {
        printk("WARNING: Only one of the two credential types is available to write. I hope you know what you're doing!\n");
    }
    if (id_in_prj)
    {
        printk("Attempting to write Golioth PSK-ID to modem with sec_tag=1...\n");
        err = modem_key_mgmt_write(1,
                    MODEM_KEY_MGMT_CRED_TYPE_IDENTITY,
                    CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID,
                    strlen(CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID)
                    );
        if (err) printk("Could not write CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID to modem: %d\n", err);
        else printk("Success!\n");
    }
    if (psk_in_prj)
    {
        printk("Encoding PSK...\n");
        err = bin2hex(CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK, strlen(CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK), at_buf, AT_BUF_LEN);
        if (err != strlen(CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK)*2)
        {
            printk("Error encoding PSK in an AT command; Expected encoded length of %d but got %d\n",
                    strlen(CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK)*2,
                    err);
            return err;
        }
        printk("Attempting to write Golioth PSK to modem with sec_tag=1...\n");
        err = modem_key_mgmt_write(1,
                    MODEM_KEY_MGMT_CRED_TYPE_PSK,
                    at_buf,
                    strlen(at_buf)
                    );
        if (err) printk("Could not write CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK to modem: %d\n", err);
        else printk("Success!\n");
    }
    return 0;
}

int delete_credentials_from_modem(bool id_in_modem, bool psk_in_modem)
{
    int err;

    if ((id_in_modem | psk_in_modem) == 0)
    {
        /* This should never happen */
        printk("ERROR: No credentials available to erase from modem\n");
        return -EINVAL;
    }
    printk("Attempting to delete credentials with sec_tag=1 from modem...\n");
    if (id_in_modem)
    {
        printk("Deleting PSK-ID from modem...\n");
        err = modem_key_mgmt_delete(1, MODEM_KEY_MGMT_CRED_TYPE_IDENTITY);
        if (err) printk("Could not delete PSK-ID: %d", err);
        else printk("Success!\n");
    }
    if (psk_in_modem)
    {
        printk("Deleting PSK from modem...\n");
        err = modem_key_mgmt_delete(1, MODEM_KEY_MGMT_CRED_TYPE_PSK);
        if (err) printk("Could not delete PSK: %d", err);
        else printk("Success!\n");
    }
    return 0;
}

void show_prompt(bool in_modem, bool in_prj) {
    printk("\nManage Modem PSK-ID/PSK Storage:\n");
    printk("================================\n\n");
    if ((in_modem | in_prj) == 0)
    {
        printk("No credentials found in modem or in Kconfig (ie: prj.conf\n");
        printk("Nothing to do here!\n\n");
        return;
    }
    if (in_prj)
    {
        printk("Press Button 1 to write Golioth credentials from Kconfig (ie: prj.conf) to modem\n");
    }
    if (in_modem)
    {
        printk("Press Button 2 to ERASE the PSK-ID and PSK credentials with sec_tag=1 from modem (DANGER!)\n");
    }
    printk("\n");
}

int check_credentials(bool *psk_id_in_modem, bool *psk_in_modem, bool *psk_id_in_prj, bool *psk_in_prj)
{
    int err;
    static const char FOUND[] = "FOUND";
    static const char ABSENT[] = "ABSENT";

    printk("\nChecking credentials...\n");
    printk("=======================\n");

    /* Check for credentials in modem */
    err = modem_key_mgmt_exists(1, MODEM_KEY_MGMT_CRED_TYPE_IDENTITY, psk_id_in_modem);
    if (err) {
        printk("Error reading PSK-ID from modem: %d\n", err);
    }
    else
    {
        printk("%10s: PSK-ID with sec_tag=1 stored in modem\n", *psk_id_in_modem ? FOUND : ABSENT);
    }
    err = modem_key_mgmt_exists(1, MODEM_KEY_MGMT_CRED_TYPE_PSK, psk_in_modem);
    if (err) {
        printk("Error reading PSK from modem: %d\n", err);
    }
    else
    {
        printk("%10s: PSK with sec_tag=1 stored in modem\n", *psk_in_modem ? FOUND : ABSENT);
    }

    /* Check for credentials in prj.conf */
    #ifdef CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID
    if (strlen(CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK_ID) > 0) *psk_id_in_prj = 1;
    #endif
    printk("%10s: Golioth PSK-ID from Kconfig (ie: prj.conf)\n", *psk_id_in_prj ? FOUND : ABSENT);

    #ifdef CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK
    if (strlen(CONFIG_GOLIOTH_SYSTEM_CLIENT_PSK) > 0) *psk_in_prj = 1;
    #endif
    printk("%10s: Golioth PSK from Kconfig (ie: prj.conf)\n", *psk_in_prj ? FOUND : ABSENT);

    return 0;
}

void main(void)
{
    bool psk_id_in_modem = 0;
    bool psk_in_modem = 0;
    bool psk_id_in_prj = 0;
    bool psk_in_prj = 0;

    check_credentials(&psk_id_in_modem, &psk_in_modem, &psk_id_in_prj, &psk_in_prj);

    /* Buttons */
    button_mask = 0;
    dk_buttons_init(button_handler);

    show_prompt((psk_id_in_modem | psk_in_modem), (psk_id_in_prj | psk_in_prj));

    while(1)
    {
        if (button_mask) {
            if (button_mask & BUTTON_1)
            {
                write_credentials_to_modem(psk_id_in_prj, psk_in_prj);
            }
            else if (button_mask & BUTTON_2)
            {
                delete_credentials_from_modem(psk_id_in_modem, psk_in_modem);
            }
            k_sleep(K_SECONDS(2));  /* Some timeout to signal to user that something important happened */
            check_credentials(&psk_id_in_modem, &psk_in_modem, &psk_id_in_prj, &psk_in_prj);
            show_prompt((psk_id_in_modem | psk_in_modem), (psk_id_in_prj | psk_in_prj));
            button_mask = 0;
        }
        else
        {
            k_sleep(K_MSEC(100));
        }
    }
}
