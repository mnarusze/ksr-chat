#include <glib.h>
#include <gio/gio.h>

// CONSTANTS

const gint MAX_CHAR = 256;
const gchar OBJECT_PATH[] = "/org/ksr/chat";
const gchar BUS_PATH[] = "org.ksr.chat";

// ENUMS

enum POSSIBLE_ACTIONS {
	send_message,
	leave_channel,
	send_private_message,
	list_users,
	enter_channel
};

gchar* g_credentials_get_unique_ident(GCredentials *credentials);
