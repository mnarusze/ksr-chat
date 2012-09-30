#include <glib.h>
#include <gio/gio.h>


// CONSTANTS

const gint MAX_CHAR = 256;
const gchar OBJECT_PATH[] = "/org/ksr/chat";
const gchar INTERFACE_PATH[] = "org.ksr.chat.Interface";
const gchar TMP_OBJECT_PATH[] = "/org/ksr/chat/tmp";
const gchar REGISTRATION_RESPONSE_OK[] = "REGISTERED";
const gchar REGISTRATION_RESPONSE_NOT_OK[] = "COULD NOT REGISTER";

// ENUMS

enum POSSIBLE_ACTIONS {
	send_message,
	leave_channel,
	send_private_message,
	list_users,
	enter_channel
};

struct GUser
{
	gchar                  *nickname;
	gint				   registration_id;
};

static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data);
