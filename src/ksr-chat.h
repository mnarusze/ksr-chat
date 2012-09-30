#include <glib.h>
#include <gio/gio.h>

#define MAX_CHAR 256
#define OBJECT_PATH "/org/ksr/chat"
#define INTERFACE_PATH "org.ksr.chat.Interface"
#define TMP_OBJECT_PATH "/org/ksr/chat/tmp"
#define REGISTRATION_RESPONSE_OK "REGISTERED"
#define REGISTRATION_RESPONSE_NOT_OK "COULD NOT REGISTER"
#define GUser struct _GUser

// ENUMS

enum POSSIBLE_ACTIONS {
	send_message,
	leave_channel,
	send_private_message,
	list_users,
	enter_channel
};

struct _GUser
{
	gchar                  nickname[MAX_CHAR];
	gint				   registration_id;
	GDBusConnection		   *connection;
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
