#include <gio/gio.h>
#include "chat-generated.h"

static gchar *opt_name         = NULL;
static gchar *opt_address      = NULL;
static gchar *opt_server_mode  = NULL;

static GOptionEntry opt_entries[] =
{
	{ "server", 's', 0, G_OPTION_ARG_NONE, &opt_server_mode, "Start as server", NULL },
	{ "name", 'n', 0, G_OPTION_ARG_STRING, &opt_name, "Client's name", NULL },
	{ "address", 'a', 0, G_OPTION_ARG_STRING, &opt_address, "Address of the server we want to connect to.", NULL },
	{ NULL }
};

static gboolean
input_is_valid ()
{
	if (opt_address == NULL && opt_server_mode == NULL && opt_name == NULL)
	{
		g_printerr ("Use \"ksr-chat -h\" for help.\n");
		return 0;
	}
	if (opt_address == NULL)
	{
		g_printerr ("Address must be specified with --address.\n");
		return 0;
	}
	if (opt_server_mode == NULL && opt_name == NULL)
	{
		g_printerr ("If not in --server mode, --name must be specified.\n");
		return 0;
	}
	return 1;
}

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
	if (g_strcmp0 (method_name, "HelloWorld") == 0)
	{
		const gchar *greeting;
		gchar *response;

		g_variant_get (parameters, "(&s)", &greeting);
		response = g_strdup_printf ("You said '%s'. KTHXBYE!", greeting);
		g_dbus_method_invocation_return_value (invocation,
											   g_variant_new ("(s)", response));
		g_free (response);
		g_print ("Client said: %s\n", greeting);
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

	g_print ("Client connected.\n");
	g_object_ref (connection);

	return TRUE;
}

int
main (int argc, char *argv[])
{
	GOptionContext *opt_context;
	GError *error;
	gint ret;

	ret = 1;
	g_type_init ();
	error = NULL;

	opt_context = g_option_context_new ("");
	g_option_context_set_summary (opt_context,
								  "To connect to server under address as \"nickname\" and start chatting, use:\n"
								  "  \"ksr-chat -n nickname -a address\" \n\n"
								  "To start a local server under address, use:\n"
								  "  \"ksr-chat -a address --server\"");
	g_option_context_add_main_entries (opt_context, opt_entries, NULL);

	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_printerr ("Error parsing options: %s\n", error->message);
		g_error_free(error);
		goto out;
	}

	if (!input_is_valid())
		goto out;

	if (opt_server_mode)
	{
		GDBusServer *server;
		gchar *guid;
		GMainLoop *loop;
		GDBusServerFlags server_flags;

		guid = g_dbus_generate_guid ();
		server_flags = G_DBUS_SERVER_FLAGS_NONE;
		error = NULL;

		server = g_dbus_server_new_sync (opt_address,
									     server_flags,
									     guid,
									     NULL,
									     NULL,
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

		g_object_unref (server);
		g_main_loop_unref (loop);
	}
	else
	{
		GDBusConnection *connection;
		GDBusConnectionFlags connection_flags;
		GVariant *value;

		connection_flags = G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT;
		error = NULL;
		connection = g_dbus_connection_new_for_address_sync (opt_address,
															 connection_flags,
															 NULL,
															 NULL,
															 &error);

		if (connection == NULL)
		{
			g_printerr("Could not connect to server under address %s : %s\n", opt_address, error->message);
			g_error_free (error);
			goto out;
		}

		g_print ("Connected to server!\n");



		if (value != NULL)
			g_variant_unref (value);
		g_object_unref (connection);
	}

	ret = 0;

	out:
	g_option_context_free (opt_context);
	return ret;
}
