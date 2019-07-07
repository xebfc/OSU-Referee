#include <wchar.h>
#include <windows.h>
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <gmodule.h>

#include "hexchat-plugin.h"

#include "strb.h"
#include "wcsb.h"

#define PNAME "OSU!Referee"
#define PDESC "Simple Edition Automated Referee"
#define PVERSION "0.1"

static hexchat_plugin* ph;      /* plugin handle */
static int enable = 1;

static int y_cb(char* word[], char* word_eol[], void* userdata)
{
	wchar_t* w = from_utf8(word[4]);
	if (wcscmp(w, L":!y") != 0)
		goto end;
	free(w);

	w = from_utf8(word[1]);
	int index = wcschr(w, L'!') - w;
	wchar_t* sender = (wchar_t*)malloc(WCS_SIZEOF(index));
	memcpy(sender, w + 1, WCS_SIZEOF(index - 1));
	*(sender + index - 1) = L'\0';
	free(w);

	char* msg = _G_LOCALE_TO_UTF8("Ë¹¹þË¹¹þ");
	hexchat_commandf(ph, "MSG %ls %s", sender, msg);
	free(sender);
	free(msg);

end:
	return HEXCHAT_EAT_NONE;
}

int hexchat_plugin_init(
	hexchat_plugin* plugin_handle,
	char** plugin_name, char** plugin_desc, char** plugin_version,
	char* arg)
{
	/* we need to save this for use with any hexchat_* functions */
	ph = plugin_handle;

	/* tell HexChat our info */
	*plugin_name = PNAME;
	*plugin_desc = PDESC;
	*plugin_version = PVERSION;

	hexchat_hook_server(ph, "PRIVMSG", HEXCHAT_PRI_NORM, y_cb, NULL);

	hexchat_printf(ph, "%s loaded successfully!\n", PNAME);
	return 1;       /* return 1 for success */
}

int hexchat_plugin_deinit(hexchat_plugin* plugin_handle)
{
	hexchat_printf(ph, "Unloading %s.\n", PNAME);
	return 1;       /* return 1 for success */
}