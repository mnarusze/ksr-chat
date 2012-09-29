#include "ksr-chat.h"
#include <glib.h>
#include <gio/gio.h>

gchar *opt_address;

static GDBusNodeInfo *introspection_data = NULL;

static const gchar introspection_xml[] =
  "<node>"
  " <interface name='org.ksr.chat'>"
  "  <method name='SendMessage'>"
  "   <arg name='nick' type='s' direction='in'/>"
  "   <arg name='content' type='s' direction='in'/>"
  "  </method>"
  "  <signal name='action'>"
  "   <arg name='nick' type='s' direction='out'/>"
  "   <arg name='action_type' type='s' direction='out'/>"
  "  </signal>"
  " </interface>"
  "</node>";

static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data)
{
	if (g_strcmp0 (method_name, "SendMessage") == 0)
	{
		gchar *nick;
		gchar *content;

		g_variant_get (parameters, "(&s&s)", &nick, &content);

		g_print ("%s said: %s\n", nick, content);
	}
}

static const GDBusInterfaceVTable interface_vtable =
{
	handle_method_call,
	NULL,
	NULL,
};

static gboolean
on_new_connection (GDBusServer *server,
                   GDBusConnection *connection,
                   gpointer user_data)
{
	guint registration_id;
	gchar *client_object;
	guint client_number;
	GCredentials *client_credentials;

	client_credentials = g_dbus_connection_get_peer_credentials(connection);
	client_number = g_credentials_get_pid(client_credentials);
	client_object = g_strdup_printf ("%s/client%s", OBJECT_PATH,client_number);
	g_print("%s\n",client_object);

	g_print("Client no. %s connected.\n",client_number);

	g_object_ref (connection);
	registration_id = g_dbus_connection_register_object (connection,
															client_object,
															introspection_data->interfaces[0],
															&interface_vtable,
															NULL,  /* user_data */
															NULL,  /* user_data_free_func */
															NULL); /* GError** */
	g_assert (registration_id > 0);

	return TRUE;
}

GOptionEntry opt_entries[] =
	{
	{ "address", 'a', 0, G_OPTION_ARG_STRING, &opt_address, "Address under which the server will reside", NULL },
	{ NULL }
};

static gboolean
input_is_valid ()
{
	if (opt_address == NULL)
	{
		g_printerr ("Incorrect usage, use \"ksr-chat-server --help\" for help.\n");
		return 0;
	}
	return 1;
}

int
main (int argc, char *argv[])
{
	GOptionContext *opt_context;
	GError *error;
	GMainLoop *loop;
	GDBusServer *server;
	gchar *guid;
	GDBusServerFlags server_flags;

	gchar *action;
	gchar *message;
	gint ret;

	introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
	g_assert (introspection_data != NULL);

	guid = g_dbus_generate_guid ();
	server_flags = G_DBUS_SERVER_FLAGS_NONE;
	ret = 1;
	g_type_init ();
	error = NULL;

	opt_context = g_option_context_new ("ksr-chat-server() usage:");
	g_option_context_set_summary (opt_context,
									"To start a local server located under tcp:host=0.0.0.0, use:\n"
									"  \"ksr-chat-server -a tcp:host=0.0.0.0\"");

	g_option_context_add_main_entries (opt_context, opt_entries, NULL);

	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_printerr ("Error parsing options: %s\n", error->message);
		goto out;
	}

	if (!input_is_valid())
		goto out;

	server = g_dbus_server_new_sync (opt_address,
										server_flags,
										guid,
										NULL, /* GDBusAuthObserver */
										NULL, /* GCancellable */
										&error);

	g_dbus_server_start (server);
	g_free (guid);

	if (server == NULL)
	{
		g_printerr ("Error creating server at address %s: %s\n", opt_address, error->message);
		g_error_free (error);
		goto out;
	}

	g_print ("Server is listening at: %s\n", g_dbus_server_get_client_address (server));
	g_signal_connect (server,
						"new-connection",
						G_CALLBACK (on_new_connection),
						NULL);

	loop = g_main_loop_new (NULL, FALSE);

	g_main_loop_run (loop);

	g_main_loop_unref (loop);

	ret = 0;

	out:
	return ret;
}
