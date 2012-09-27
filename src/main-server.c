#include <gio/gio.h>
#include "chat-generated.h"
#include "ksr-chat.h"

gchar *opt_address;

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

static gboolean
on_new_connection (GDBusServer *server,
					GDBusConnection *connection,
					gpointer user_data)
{
	 g_print ("Client connected.\n");
	 return TRUE;
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
