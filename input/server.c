/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hidp.h>
#include <bluetooth/hci.h>
#include <bluetooth/sdp.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "logging.h"

#include "adapter.h"
#include "device.h"
#include "server.h"
#include "glib-helper.h"

static const char *HID_UUID = "00001124-0000-1000-8000-00805f9b34fb";
static GSList *servers = NULL;
struct server {
	bdaddr_t src;
	GIOChannel *ctrl;
	GIOChannel *intr;
};

struct authorization_data {
	bdaddr_t src;
	bdaddr_t dst;
};

static gint server_cmp(gconstpointer s, gconstpointer user_data)
{
	const struct server *server = s;
	const bdaddr_t *src = user_data;

	return bacmp(&server->src, src);
}

static void auth_callback(DBusError *derr, void *user_data)
{
	struct authorization_data *auth = user_data;

	if (derr) {
		error("Access denied: %s", derr->message);
		if (dbus_error_has_name(derr, DBUS_ERROR_NO_REPLY))
			btd_cancel_authorization(&auth->src, &auth->dst);

		input_device_close_channels(&auth->src, &auth->dst);
	} else
		input_device_connadd(&auth->src, &auth->dst);

	g_free(auth);
}

static int authorize_device(const bdaddr_t *src, const bdaddr_t *dst)
{
	struct authorization_data *auth;

	auth = g_new0(struct authorization_data, 1);
	bacpy(&auth->src, src);
	bacpy(&auth->dst, dst);

	return btd_request_authorization(src, dst, HID_UUID,
				auth_callback, auth);
}

static void connect_event_cb(GIOChannel *chan, int err, const bdaddr_t *src,
				const bdaddr_t *dst, gpointer data)
{
	int sk, psm = GPOINTER_TO_UINT(data);

	if (err < 0) {
		error("accept: %s (%d)", strerror(-err), -err);
		return;
	}

	sk = g_io_channel_unix_get_fd(chan);

	debug("Incoming connection on PSM %d", psm);

	if (input_device_set_channel(src, dst, psm, sk) < 0) {
		/* Send unplug virtual cable to unknown devices */
		if (psm == L2CAP_PSM_HIDP_CTRL) {
			unsigned char unplug[] = { 0x15 };
			int err;
			err = write(sk, unplug, sizeof(unplug));
		}
		close(sk);
		return;
	}

	if ((psm == L2CAP_PSM_HIDP_INTR) && (authorize_device(src, dst) < 0))
		input_device_close_channels(src, dst);

	return;
}

int server_start(bdaddr_t *src)
{
	struct server *server;
	GIOChannel *ctrl_io, *intr_io;

	ctrl_io = bt_l2cap_listen(src, L2CAP_PSM_HIDP_CTRL, 0, 0,
				connect_event_cb,
				GUINT_TO_POINTER(L2CAP_PSM_HIDP_CTRL));
	if (!ctrl_io) {
		error("Failed to listen on control channel");
		return -1;
	}
	g_io_channel_set_close_on_unref(ctrl_io, TRUE);

	intr_io = bt_l2cap_listen(src, L2CAP_PSM_HIDP_INTR, 0, 0,
				connect_event_cb,
				GUINT_TO_POINTER(L2CAP_PSM_HIDP_INTR));
	if (!intr_io) {
		error("Failed to listen on interrupt channel");
		g_io_channel_unref(ctrl_io);
		ctrl_io = NULL;
		return -1;
	}
	g_io_channel_set_close_on_unref(intr_io, TRUE);

	server = g_new0(struct server, 1);
	bacpy(&server->src, src);
	server->ctrl = ctrl_io;
	server->intr = intr_io;

	servers = g_slist_append(servers, server);

	return 0;
}

void server_stop(bdaddr_t *src)
{
	struct server *server;
	GSList *l;

	l = g_slist_find_custom(servers, src, server_cmp);
	if (!l)
		return;

	server = l->data;

	g_io_channel_unref(server->intr);
	g_io_channel_unref(server->ctrl);

	servers = g_slist_remove(servers, server);
	g_free(server);
}
