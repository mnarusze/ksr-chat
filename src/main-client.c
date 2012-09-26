#include <gio/gio.h>
#include "chat-generated.h"

static gchar *opt_name           = NULL;
static gchar *opt_address        = NULL;

static GOptionEntry opt_entries[] =
{
	{ "name", 'n', 0, G_OPTION_ARG_STRING, &opt_name, "Client's name", NULL },
	{ "address", 'a', 0, G_OPTION_ARG_STRING, &opt_address, "Address of the DBus", NULL },
	{ NULL }
};

static gboolean
input_is_valid ()
{
	if (opt_name == NULL || opt_address == NULL)
	{
		g_printerr ("Incorrect usage, use \"ksr-chat-client --help\" for help.\n");
		return 0;
	}
	return 1;
}

static void
on_bus_acquired (GDBusConnection *connection,
					const gchar     *name,
					gpointer         user_data)
{
	/* This is where we'd export some objects on the bus */
}

static void
on_name_acquired (GDBusConnection *connection,
					const gchar     *name,
					gpointer         user_data)
{
	g_print ("Acquired the name %s on the session bus\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
				const gchar     *name,
				gpointer         user_data)
{
	g_print ("Lost the name %s on the session bus\n", name);
}

int
main (int argc, char *argv[])
{
	GOptionContext *opt_context;
	GError *error;
	GMainLoop *loop;
	GDBusConnection *connection;
	GDBusConnectionFlags connection_flags;
	GBusNameOwnerFlags name_owner_flags;

	gint ret;
	gchar *object_path;
	guint owner_id;

	ret = 1;
	g_type_init ();
	error = NULL;

	opt_context = g_option_context_new ("ksr-chat-client() usage:");
	g_option_context_set_summary (opt_context,
									"To connect to server under address as \"nickname\" and start chatting, use:\n"
									"  \"ksr-chat -n nickname -a address");
	g_option_context_add_main_entries (opt_context, opt_entries, NULL);

	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_printerr ("Error parsing options: %s\n", error->message);
		g_error_free (error);
		goto out;
	}

	if (!input_is_valid())
		goto out;

	object_path = g_strdup_printf("org.ksr.chat.%s", opt_name);
	connection_flags = G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT;

	g_printf ("Connecting to bus under %s...\n",opt_address);
	connection = g_dbus_connection_new_for_address_sync(opt_address,
														connection_flags,
														NULL,
														NULL,
														error);

	if (connection == NULL)
	{
		g_printerr ("Error connecting to the DBus : %s\n", error->message);
		goto out;
	}

	g_printf ("Connected to the DBus! Connecting to the chat...\n");

	name_owner_flags = G_BUS_NAME_OWNER_FLAGS_NONE;
	owner_id = g_bus_own_name_on_connection(connection,
											object_path,
											name_owner_flags,
											on_name_acquired,
											on_name_lost,
											error,
											NULL);

	if (owner_id == NULL)
		goto out;

	loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (loop);

	/*
	 * Cleanup
	 */
	if (g_dbus_connection_close_sync(connection,NULL,error))
		g_printf ("Closed connection successfully!\n");
	else
		g_printerr ("Error closing the connection : %s\n", error->message);

	g_bus_unown_name (owner_id);
	g_main_loop_unref (loop);
	g_free(object_path);

	ret = 0;

	out:

	g_option_context_free (opt_context);
	return ret;
}
