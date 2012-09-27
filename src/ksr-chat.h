#include <glib.h>
#include <gio/gio.h>

// CONSTANTS

const int MAX_CHAR = 256;
const char *BUS_PATH = "/org/ksr/chat";

// ENUMS

enum POSSIBLE_ACTIONS {
	send_message,
	leave_channel,
	send_private_message,
	list_users,
	enter_channel
};
