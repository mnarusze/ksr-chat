#include <glib.h>
#include <gio/gio.h>
#include "ksr-chat.h"

gchar *opt_address;
guint temp_registration_id;
static GArray *users;

static GDBusNodeInfo *introspection_data = NULL;

static const gchar introspection_xml[] =
  "<node>"
  " <interface name='org.ksr.chat.Interface'>"
  "  <method name='RegisterMe'>"
  "   <arg name='nick' type='s' direction='in'/>"
  "   <arg name='response' type='s' direction='out'/>"
  "  </method>"
  "  <method name='SendMessage'>"
  "   <arg name='nick' type='s' direction='in'/>"
  "   <arg name='body' type='s' direction='in'/>"
  "  </method>"
  "  <method name='SendPrivateMessage'>"
  "   <arg name='nick' type='s' direction='in'/>"
  "   <arg name='target' type='s' direction='in'/>"
  "   <arg name='body' type='s' direction='in'/>"
  "  </method>"
  "  <method name='ListUsers'>"
  "   <arg name='nick' type='s' direction='in'/>"
  "  </method>"
  "  <method name='QuitChat'>"
  "   <arg name='nick' type='s' direction='in'/>"
  "  </method>"
  "  <signal name='message'>"
  "   <arg name='nick' type='s'/>"
  "   <arg name='body' type='s'/>"
  "  </signal>"
  "  <signal name='users_list'>"
  "   <arg name='list' type='s'/>"
  "  </signal>"
  "  <signal name='joined_chat'>"
  "   <arg name='nick' type='s'/>"
  "  </signal>"
  "  <signal name='left_chat'>"
  "   <arg name='nick' type='s'/>"
  "  </signal>"
  " </interface>"
  "</node>";


static const GDBusInterfaceVTable interface_vtable =
{
	handle_method_call,
	NULL,
	NULL,
};

gchar * get_users_list ()
{
	GString *list;
	GUser user;
	gint iter;
	list = g_string_new ("Users: ");
	for (iter = 0; iter < users->len; iter++)
	{
		user = g_array_index (users, GUser, iter);
		g_string_append (list, g_strdup(" "));
		g_string_append (list, &user.nickname);
	}
	return list->str;
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
	if (g_strcmp0 (method_name, "RegisterMe") == 0)
	{
		GError *error;
		gchar *nickname;
		gint registration_id;
		gint i;
		gchar *client_object;
		gchar *response;
		gboolean unregistered;
		gboolean unique_username;
		GUser user;

		g_variant_get (parameters, "(&s)", &nickname);
		client_object = g_strdup_printf ("%s/%s", OBJECT_PATH,nickname);
		error = NULL;
		unique_username = TRUE;

		for (i = 0 ; i < users->len; i++)
		{
			user = g_array_index (users, GUser, i);
			if (g_strcmp0(user.nickname, nickname) == 0)
			{
				unique_username = FALSE;
				break;
			}
		}

		if (unique_username == FALSE)
		{
			g_print ("%s already registered, not registering the new one!\n",nickname);
			response = g_strdup (&REGISTRATION_RESPONSE_NOT_OK);
			g_dbus_method_invocation_return_value (invocation,
													 g_variant_new ("(s)", response));
			g_free (response);
			return;
		}

		unregistered = g_dbus_connection_unregister_object(connection,temp_registration_id);

		if (unregistered == FALSE)
		{
			g_printerr ("Error unregistering the temp object!\n");
		}

		registration_id = g_dbus_connection_register_object (connection,
																client_object,
																introspection_data->interfaces[0],
																&interface_vtable,
																NULL,
																NULL,
																&error);
		if (registration_id <= 0)
		{
			g_print ("%s not registered!\n",nickname);
			response = g_strdup (&REGISTRATION_RESPONSE_NOT_OK);
			g_dbus_method_invocation_return_value (invocation,
													 g_variant_new ("(s)", response));
		    g_free (response);
		}
		else if (error == NULL)
		{
			gint iter;

			user.registration_id = registration_id;
			i = 0;
			for (; i < MAX_CHAR; i++)
			{
				user.nickname[i] = nickname[i];
				if (nickname[i] == 0)
					break;
			}
			user.connection = connection;
			g_print ("%s joined the chat.\n",user.nickname);

			g_array_append_val(users, user);

			for (iter = 0; iter < users->len ; iter++)
			{
				user = g_array_index (users, GUser, iter);
				if (g_strcmp0(nickname,user.nickname) == 0)
					continue;
				object_path = g_strdup_printf("%s/%s", OBJECT_PATH, &user.nickname);
				g_dbus_connection_emit_signal(user.connection,
												NULL,
												object_path,
												INTERFACE_PATH,
												"joined_chat",
												g_variant_new ("(s)",
																nickname),
												&error);

			}

			response = g_strdup (&REGISTRATION_RESPONSE_OK);
			g_dbus_method_invocation_return_value (invocation,
													 g_variant_new ("(s)", response));
			g_free (response);
		}
	}
	else if (g_strcmp0 (method_name, "SendMessage") == 0)
	{
		gchar *nick;
		gchar *content;
		gchar *object_path;
		gint iter;
		GError *error;
		GUser user;

		g_variant_get (parameters, "(&s&s)", &nick, &content);
		error = NULL;

		g_print ("%s: %s\n", nick, content);

		for (iter = 0; iter < users->len ; iter++)
		{
			user = g_array_index (users, GUser, iter);
			if (g_strcmp0(nick,user.nickname) == 0)
				continue;
			object_path = g_strdup_printf("%s/%s", OBJECT_PATH, &user.nickname);
			g_dbus_connection_emit_signal(user.connection,
											NULL,
											object_path,
											INTERFACE_PATH,
											"message",
											g_variant_new ("(ss)",
															nick,
															content),
											&error);

		}

	}
	else if (g_strcmp0 (method_name, "ListUsers") == 0)
	{
		gchar *list;
		GError *error;
		gchar *nickname;
		GUser user;
		gint iter;

		list = get_users_list();
		error = NULL;

		g_variant_get (parameters, "(&s)", &nickname);
		object_path = g_strdup_printf ("%s/%s", OBJECT_PATH,nickname);
		for (iter = 0; iter < users->len; iter++)
		{
			user = g_array_index (users, GUser, iter);
			if (g_strcmp0(nickname,user.nickname) == 0)
			{
				g_dbus_connection_emit_signal(user.connection,
												NULL,
												object_path,
												INTERFACE_PATH,
												"users_list",
												g_variant_new ("(s)",
																list),
												&error);
				break;
			}
		}

	}
	else if (g_strcmp0 (method_name, "QuitChat") == 0)
	{
		gchar *list;
		GError *error;
		gchar *nickname;
		GUser user;
		gint iter;
		gint position;
		gboolean unregistered;

		position = 0;
		list = get_users_list();
		error = NULL;
		g_variant_get (parameters, "(&s)", &nickname);

		for (iter = 0; iter < users->len; iter++)
		{
			user = g_array_index (users, GUser, iter);
			if (g_strcmp0(nickname,user.nickname) == 0)
			{
				// Save position for later
				position = iter;

				// Unregister object on dbus
				unregistered = g_dbus_connection_unregister_object(connection, user.registration_id);
				if (unregistered == FALSE)
				{
					g_printerr ("Error unregistering the object!\n");
				}

				// Unref the connection
				g_object_unref (user.connection);
			}
			else
			{
				object_path = g_strdup_printf ("%s/%s", OBJECT_PATH,&user.nickname);
				g_dbus_connection_emit_signal(user.connection,
												NULL,
												object_path,
												INTERFACE_PATH,
												"left_chat",
												g_variant_new ("(s)",
																nickname),
												&error);
			}
		}
		// Remove the user from users list
		g_array_remove_index(users,position);

		g_print ("%s left the chat!\n", nickname);
	}
	else
	{
		 g_printerr ("Unknown method!\n");
	}
}

static gboolean
on_new_connection (GDBusServer *server,
                   GDBusConnection *connection,
                   gpointer user_data)
{
	g_object_ref (connection);
	temp_registration_id = g_dbus_connection_register_object (connection,
															TMP_OBJECT_PATH,
															introspection_data->interfaces[0],
															&interface_vtable,
															NULL,  /* user_data */
															NULL,  /* user_data_free_func */
															NULL); /* GError** */
	g_assert (temp_registration_id > 0);

	return TRUE;
}

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

GOptionEntry opt_entries[] =
{
	{ "address", 'a', 0, G_OPTION_ARG_STRING, &opt_address, "Address under which the server will reside", NULL },
	{ NULL }
};

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
	users = g_array_new (FALSE,TRUE,sizeof (GUser));

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
