/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2008  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define HCID_DEFAULT_DISCOVERABLE_TIMEOUT 180 /* 3 minutes */

/* When all services should trust a remote device */
#define GLOBAL_TRUST "[all]"

enum {
	HCID_SET_NAME,
	HCID_SET_CLASS,
	HCID_SET_VOICE,
	HCID_SET_INQMODE,
	HCID_SET_PAGETO,
	HCID_SET_DISCOVTO,
	HCID_SET_PTYPE,
	HCID_SET_LM,
	HCID_SET_LP,
};

/*
 * Scanning modes, used by DEV_SET_MODE
 * off: remote devices are not allowed to find or connect to this device
 * connectable: remote devices are allowed to connect, but they are not
 *              allowed to find it.
 * discoverable: remote devices are allowed to connect and find this device
 * limited: limited discoverable - GIAC + IAC enabled and set limited
 *          bit on device class.
 */

#define MODE_OFF		0x00
#define MODE_CONNECTABLE	0x01
#define MODE_DISCOVERABLE	0x02
#define MODE_LIMITED		0x03
#define MODE_UNKNOWN		0xff

struct device_opts {
	unsigned long flags;
	char    *name;
	uint32_t class;
	uint16_t voice;
	uint8_t  inqmode;
	uint16_t pageto;
	uint16_t pkt_type;
	uint16_t link_mode;
	uint16_t link_policy;
	uint8_t  scan;
	uint8_t  mode;
	uint32_t discovto;
};

extern struct device_opts default_device;

struct device_list {
	char *ref;			/* HCI device or Bluetooth address */
	struct device_list *next;
	struct device_opts opts;
};

struct hcid_opts {
	char    host_name[40];
	int     auto_init;
	int	offmode;
	char    deviceid[15];

	int     sock;
};

extern struct hcid_opts hcid;

void hci_req_queue_remove(int dev_id, bdaddr_t *dba);

#define HCID_OFFMODE_DEVDOWN	0
#define HCID_OFFMODE_NOSCAN	1

uint8_t get_startup_scan(int hdev);
uint8_t get_startup_mode(int hdev);
int get_discoverable_timeout(int dev_id);

void start_security_manager(int hdev);
void stop_security_manager(int hdev);

void set_pin_length(bdaddr_t *sba, int length);

gboolean plugin_init(GKeyFile *config);
void plugin_cleanup(void);

void __probe_servers(const char *adapter);
void __remove_servers(const char *adapter);
