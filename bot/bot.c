#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>

#include "hexchat-plugin.h"

#include "strb.h"
#include "wcsb.h"

#define PNAME "OSU!Referee"
#define PDESC "Simple Edition Automated Referee"
#define PVERSION "0.1"

static void utf8_commandf(const char* format, ...);
static void utf8_command(const char* cmd);

static hexchat_plugin* ph;      /* plugin handle */
static int enable = 1;

static int
y_cb(char* word[], char* word_eol[], void* userdata)
{
	wchar_t* w = from_utf8(word[4]);
	if (wcscmp(w, L":!y") != 0)
		goto end;
	free(w);

	w = from_utf8(word[1]);
	int index = index_of_wc(w, L'!');
	wchar_t* sender = subws(w, 1, index);
	free(w);

	utf8_commandf("MSG %ls 斯哈斯哈", sender);
	free(sender);

end:
	return HEXCHAT_EAT_NONE;
}

static int
bb_cb(char* word[], char* word_eol[], void* userdata)
{
	utf8_commandf("MSG BanchoBot %s", word_eol[2]);
	return HEXCHAT_EAT_ALL;
}

static int
bancho_cb(char* word[], char* word_eol[], void* userdata)
{
	return HEXCHAT_EAT_NONE;
}

//-------------------------------------------------------------------------------

static void
utf8_commandf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	strb_t* cmd = strb_new(NULL);
	strb_vsprintf(cmd, format, args);

	utf8_command(cmd->str);

	strb_free(cmd);
	va_end(args);
}

static void
utf8_command(const char* cmd)
{
	char* cmd_utf8 = _G_LOCALE_TO_UTF8(cmd);
	hexchat_command(ph, cmd_utf8);
	free(cmd_utf8);
}

static hexchat_hook*
utf8_hook_command(const char* name,
	int pri,
	int (*callback) (char* word[], char* word_eol[], void* user_data),
	const char* help_text,
	void* userdata)
{
	char* help_text_utf8 = _G_LOCALE_TO_UTF8(help_text);
	hexchat_hook* hook = hexchat_hook_command(ph, name, pri, callback, help_text_utf8, userdata);
	free(help_text_utf8);
	return hook;
}

int
hexchat_plugin_init(hexchat_plugin* plugin_handle,
	char** plugin_name,
	char** plugin_desc,
	char** plugin_version,
	char* arg)
{
	/* we need to save this for use with any hexchat_* functions */
	ph = plugin_handle;

	/* tell HexChat our info */
	*plugin_name = PNAME;
	*plugin_desc = PDESC;
	*plugin_version = PVERSION;

	hexchat_hook_server(ph, "PRIVMSG", HEXCHAT_PRI_NORM, y_cb, NULL);

	utf8_hook_command("BB", HEXCHAT_PRI_NORM, bb_cb, "Usage: BB <message> 给BanchoBot发送消息", NULL);
	hexchat_hook_server(ph, "PRIVMSG", HEXCHAT_PRI_NORM, bancho_cb, NULL);

	hexchat_printf(ph, "%s loaded successfully!\n", PNAME);
	return 1;       /* return 1 for success */
}

int
hexchat_plugin_deinit(hexchat_plugin* plugin_handle)
{
	hexchat_printf(ph, "Unloading %s.\n", PNAME);
	return 1;       /* return 1 for success */
}