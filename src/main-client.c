#include <gio/gio.h>
#include "chat-generated.h"
#include "ksr-chat.h"

static gchar *opt_name           = NULL;
static gchar *opt_address        = NULL;

static GOptionEntry opt_entries[] =
{
	{ "name", 'n', 0, G_OPTION_ARG_STRING, &opt_name, "Client's name", NULL },
	{ "address", 'a', 0, G_OPTION_ARG_STRING, &opt_address, "Address of the DBus", NULL },
	{ NULL }
};

static gboolean
handle_input (GIOChannel *io_channel, gpointer *data)
{
	GIOStatus ret;
	GError *error;
	gchar *input;
	gint *len;

	error = NULL;
	ret = g_io_channel_read_line (io_channel, &input, &len, NULL, &error);
	if (ret == G_IO_STATUS_ERROR)
			g_error ("Error reading: %s\n", error->message);

	printf ("Read %u bytes: %s\n", len, input);

	g_free(input);
	return TRUE;
}

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

int
main (int argc, char *argv[])
{
	GOptionContext *opt_context;
	GError *error;
	GMainLoop *loop;
	GDBusConnection *connection;
	GDBusConnectionFlags connection_flags;
	GIOChannel *io_channel;

	gchar *input;
	gint len;
	gint ret,ret_loc;

	ret = 1;
	g_type_init ();
	error = NULL;

	opt_context = g_option_context_new ("ksr-chat-client() usage:");
	g_option_context_set_summary (opt_context,
									"To connect to server under tcp:host=0.0.0.0 as \"maryl\" and start chatting, use:\n"
									"  \"ksr-chat-client -n maryl -a tcp:host=0.0.0.0");
	g_option_context_add_main_entries (opt_context, opt_entries, NULL);

	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_printerr ("Error parsing options: %s\n", error->message);
		g_error_free (error);
		goto out;
	}

	if (!input_is_valid())
		goto out;

	connection_flags = G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT;

	connection = g_dbus_connection_new_for_address_sync (opt_address,
														   connection_flags,
														   NULL, /* GDBusAuthObserver */
														   NULL, /* GCancellable */
														   &error);

	if (connection == NULL)
	{
		g_printerr ("Error connecting to the DBus : %s\n", error->message);
		g_error_free(error);
		goto out;
	}

	g_printf ("Connected to server!\n");


	loop = g_main_loop_new (NULL, FALSE);

	// FD = 0 = stdin
	io_channel = g_io_channel_unix_new(0);

	if (!g_io_add_watch (io_channel, G_IO_IN, handle_input, NULL))
			g_error ("Cannot add watch on GIOChannel!\n");

	g_main_loop_run (loop);

	g_main_loop_unref (loop);

	ret = 0;

	out:

	g_option_context_free (opt_context);
	return ret;
}
