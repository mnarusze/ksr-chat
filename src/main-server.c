#include <gio/gio.h>
#include "chat-generated.h"

gchar *opt_name;

GOptionEntry opt_entries[] =
{
  { "name", 'n', 0, G_OPTION_ARG_STRING, &opt_name, "Name to acquire", NULL },
  { NULL }
};

static gboolean
input_is_valid ()
{
	if (opt_name == NULL)
	{
		g_printerr ("Incorrect usage, use \"ksr-chat --help\" for help.\n");
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
	GBusNameOwnerFlags name_flags;

	gint ret;
	gchar *object_path;
	guint owner_id;

	ret = 1;
	g_type_init ();
	error = NULL;

	opt_context = g_option_context_new ("ksr-chat-server() usage:");
	g_option_context_set_summary (opt_context,
									"To start a local server located under org.ksr.chat.my_server use:\n"
									"  \"ksr-chat-server -n my_server\"");

	g_option_context_add_main_entries (opt_context, opt_entries, NULL);

	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_printerr ("Error parsing options: %s\n", error->message);
		return 1;
	}

	if (!input_is_valid())
		return 1;

	name_flags = G_BUS_NAME_OWNER_FLAGS_NONE;
	error = NULL;
	object_path = g_strdup_printf("org.ksr.chat.%s", opt_name);

	owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
								object_path,
								name_flags,
								on_bus_acquired,
								on_name_acquired,
								on_name_lost,
								NULL,
								NULL);

	loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (loop);

	g_bus_unown_name (owner_id);
	g_main_loop_unref (loop);

	return 0;
}
