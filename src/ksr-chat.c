#include <glib.h>
#include <gio/gio.h>

gchar* g_credentials_get_pid(GCredentials *credentials)
{
	gchar *pid;
	gchar *credentials_string;
	GRegex *pid_regex;

	credentials_string = g_credentials_to_string(credentials);
	g_print("credentials : %s\n",credentials_string);

	pid_regex = g_regex_new(".*:", G_REGEX_CASELESS, G_REGEX_MATCH_NEWLINE_CRLF, NULL);
	credentials_string = g_regex_replace(pid_regex,credentials_string,g_utf8_strlen(credentials_string,256),0,g_strdup(""),G_REGEX_MATCH_NEWLINE_CRLF,NULL);
	pid_regex = g_regex_new(",.*", G_REGEX_CASELESS, G_REGEX_MATCH_NEWLINE_CRLF, NULL);
	credentials_string = g_regex_replace(pid_regex,credentials_string,g_utf8_strlen(credentials_string,256),0,g_strdup(""),G_REGEX_MATCH_NEWLINE_CRLF,NULL);
	pid_regex = g_regex_new("pid=", G_REGEX_CASELESS, G_REGEX_MATCH_NEWLINE_CRLF, NULL);
	credentials_string = g_regex_replace(pid_regex,credentials_string,g_utf8_strlen(credentials_string,256),0,g_strdup(""),G_REGEX_MATCH_NEWLINE_CRLF,NULL);

	g_print("pid : %s\n",credentials_string);

	return credentials_string;
}
