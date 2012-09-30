#include <glib.h>
#include <gio/gio.h>
#include "ksr-chat.h"

static gchar *opt_name           = NULL;
static gchar *opt_address        = NULL;
static gchar *object_path		 = NULL;
static GDBusConnection *connection;
static GRegex *disallowed_chars  = NULL;

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
	GVariant *value;
	gchar *input;
	gsize *len;

	error = NULL;

	ret = g_io_channel_read_line (io_channel, &input, len, NULL, &error);
	if (ret == G_IO_STATUS_ERROR)
	{
		g_error ("Error reading: %s\n", error->message);
		return FALSE;
	}

	input = g_regex_replace(disallowed_chars,input,g_utf8_strlen(input,MAX_CHAR),0,g_strdup(""),0,NULL);
	value = g_dbus_connection_call_sync (connection,
										   NULL, /* bus_name */
										   object_path,
										   INTERFACE_PATH,
										   "SendMessage",
										   g_variant_new ("(ss)", opt_name, input),
										   NULL,
										   G_DBUS_CALL_FLAGS_NONE,
										   -1,
										   NULL,
										   &error);
	g_free(input);
	return TRUE;
}

static void
handle_signals (GDBusConnection *connection,
        const gchar *sender_name,
        const gchar *object_path,
        const gchar *interface_name,
        const gchar *signal_name,
        GVariant *parameters,
        gpointer user_data)
{


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
	GDBusConnectionFlags connection_flags;
	GIOChannel *io_channel;
	GDBusProxyFlags proxy_flags;
	GDBusMessage *message;
	GVariant *value;
	gchar *client_ident;
	gchar *input;
	gchar *response;
	gsize *len;
	gint ret,ret_loc;

	ret = 1;
	g_type_init ();
	error = NULL;
	object_path = NULL;

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

	g_print ("Connected to server!\n");

	disallowed_chars = g_regex_new("\n", 0, 0, NULL);
	object_path = g_strdup_printf ("%s/%s", OBJECT_PATH,opt_name);

	// Now register the nickname
	error = NULL;
	value = g_dbus_connection_call_sync (connection,
											   NULL,
											   TMP_OBJECT_PATH,
											   INTERFACE_PATH,
											   "RegisterMe",
											   g_variant_new ("(s)", opt_name),
											   NULL,
											   G_DBUS_CALL_FLAGS_NONE,
											   -1,
											   NULL,
											   &error);

	if (value == NULL)
	{
		g_printerr ("Could not register!\n");
		goto out;
	}
	else
	{
		g_variant_get (value, "(&s)", &response);
		if (g_strcmp0(response,REGISTRATION_RESPONSE_OK) == 0)
			g_print ("Registered a nickname %s , the chat is ready!\n",opt_name);
		else if (g_strcmp0(response,REGISTRATION_RESPONSE_NOT_OK) == 0)
		{
			g_print ("Could not register your nickname %s, please change it!\n",opt_name);
			goto out;
		}
	}

	g_dbus_connection_signal_subscribe(connection,
										NULL,
										INTERFACE_PATH,
										NULL,
										NULL,
										NULL,
										G_DBUS_SIGNAL_FLAGS_NONE,
										handle_signals,
										NULL,
										NULL);

	loop = g_main_loop_new (NULL, FALSE);

	// FD = 0 = stdin
	io_channel = g_io_channel_unix_new(0);

	if (!g_io_add_watch_full (io_channel, G_PRIORITY_HIGH, G_IO_IN, handle_input, NULL, NULL))
			g_error ("Cannot add watch on GIOChannel!\n");

	g_main_loop_run (loop);

	g_main_loop_unref (loop);

	ret = 0;

	out:
	g_option_context_free (opt_context);
	return ret;
}
