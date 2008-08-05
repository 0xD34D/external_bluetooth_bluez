/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2007  Nokia Corporation
 *  Copyright (C) 2004-2008  Marcel Holtmann <marcel@holtmann.org>
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

#define DEVICE_INTERFACE	"org.bluez.Device"

struct btd_device *device_create(DBusConnection *conn, struct adapter *adapter,
				const gchar *address);
void device_remove(DBusConnection *conn, struct btd_device *device);
gint device_address_cmp(struct btd_device *device, const gchar *address);
int device_browse(struct btd_device *device, DBusConnection *conn,
			DBusMessage *msg, uuid_t *search);
void device_probe_drivers(struct btd_device *device, GSList *uuids, sdp_list_t *recs);
struct adapter *device_get_adapter(struct btd_device *device);
const gchar *device_get_address(struct btd_device *device);
const gchar *device_get_path(struct btd_device *device);
struct agent *device_get_agent(struct btd_device *device);
void device_set_agent(struct btd_device *device, struct agent *agent);
gboolean device_is_busy(struct btd_device *device);
gboolean device_is_temporary(struct btd_device *device);
void device_set_temporary(struct btd_device *device, gboolean temporary);
void device_set_cap(struct btd_device *device, uint8_t cap);
void device_set_auth(struct btd_device *device, uint8_t auth);
uint8_t device_get_auth(struct btd_device *device);
