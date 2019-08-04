#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <glib.h>

#include "hexchat-plugin.h"
#include "strb.h"
#include "wcsb.h"
#include "promise.h"

#define PNAME "OSU!Referee"
#define PDESC "Simple Edition Automated Referee"
#define PVERSION "0.1"

static hexchat_plugin* ph;      /* plugin handle */
static int enable = 1;

//-------------------------------------------------------------------------------

#define CHAR_MAX 0xff
#define PVER_KEY "_pVersion"

#define ARR_LEN(a) (sizeof(a) / sizeof(*a))
#define GET_UNAME(prefix) ( \
    substr(prefix, 1, indexof_char(prefix, '!')) \
)

typedef struct
{
    char staff[CHAR_MAX];
    char set[CHAR_MAX];
    char password[CHAR_MAX];
    char sleep[CHAR_MAX];

    char bot[CHAR_MAX];         // 用于存放bot用户
    char* staff2;
} referee_ini_t;

static char* attrs[] = { "staff","set","password","sleep" };
static char* attrs_default_value[] = { "","2 0 16","998","300" };
static referee_ini_t cfg;

static void utf8_command(const char* cmd);
static void utf8_commandf(const char* format, ...);
static void utf8_print(const char* text);
static void utf8_printf(const char* format, ...);

static int is_staff(uname)
{
    if (hexchat_nickcmp(ph, cfg.bot, uname) == 0)
        return 1;
    else if (cfg.staff != NULL)
    {
        char staff[CHAR_MAX];
        strcpy(staff, cfg.staff); // strtok 会改变 staff 的值，strcpy 保证了原字符串的完整性

        char* s = strtok(staff, ",");
        while (s != NULL)
        {
            if (hexchat_nickcmp(ph, s, uname) == 0)
                return 1;

            s = strtok(NULL,",");
        }
    }

    return 0;
}

static void init_cfg()
{
    char* pver[CHAR_MAX];
    if (hexchat_pluginpref_get_str(ph, PVER_KEY, pver)
        && strcmp(pver, PVERSION) == 0)
        return;

    int len = ARR_LEN(attrs);
    while (len--)
        if (hexchat_pluginpref_get_str(ph, attrs[len], NULL) == 0)
            hexchat_pluginpref_set_str(ph, attrs[len], attrs_default_value[len]);
    hexchat_pluginpref_set_str(ph, PVER_KEY, PVERSION);
}

static void reload_cfg()
{
    int len = ARR_LEN(attrs);
    while (len--)
    {
        char* attr = &cfg.staff + len;
        hexchat_pluginpref_get_str(ph, attrs[len], attr);

        // 剔除脏值
        int size = strlen(attr) + 1;
        memset(attr + size, '\0', CHAR_MAX - size);
    }

    if (strlen(cfg.staff) > 0)
        cfg.staff2 = &cfg.staff;
    else
        cfg.staff2 = &cfg.bot;
}

// hook command
//-------------------------------------------------------------------------------

static int
bb_cb(char* word[], char* word_eol[], void* userdata)
{
    if (strlen(word_eol[2]) == 0)
    {
        utf8_command("HELP BB");
        goto end;
    }

    utf8_commandf("MSG BanchoBot %s", word_eol[2]);

end:
    return HEXCHAT_EAT_ALL;
}

static int
ini_cb(char* word[], char* word_eol[], void* userdata)
{
    if (strlen(word[2]) == 0)
    {
        utf8_command("HELP INI");
        goto end;
    }

    char* old_value[CHAR_MAX];
    int success = hexchat_pluginpref_get_str(ph, word[2], old_value);

    if (strlen(word[3]) > 0)
    {
        if (hexchat_pluginpref_set_str(ph, word[2], word[3]) == 0)
        {
            utf8_printf("属性 %s 设置失败");
            goto end;
        }

        if (success)
            utf8_printf("属性 %s 修改成功，其原值为：%s", word[2], old_value);
        else
            utf8_printf("新增 %s 属性", word[2]);

        reload_cfg();
    }
    else
    {
        if (success)
            utf8_printf("属性 %s 值为：%s", word[2], old_value);
        else
            utf8_printf("属性 %s 尚未设置", word[2]);
    }

end:
    return HEXCHAT_EAT_ALL;
}

static int
make_cb(char* word[], char* word_eol[], void* userdata)
{
    utf8_commandf("MSG %s !make %s", cfg.bot, word_eol[2]);
    return HEXCHAT_EAT_ALL;
}

// hook server
//-------------------------------------------------------------------------------

static int
y_scb(char* word[], char* word_eol[], void* userdata)
{
    wchar_t* w = from_utf8(word[4]);
    if (wcscmp(w, L":!y") != 0)
        goto end;
    free(w);

    w = from_utf8(word[1]);
    int index = indexof_wc(w, L'!');
    wchar_t* sender = subws(w, 1, index);
    free(w);

    utf8_commandf("MSG %ls 斯哈斯哈", sender);
    free(sender);

end:
    return HEXCHAT_EAT_NONE;
}

static int
make_scb(char* word[], char* word_eol[], void* userdata)
{
    if (strcmp(word[4], ":!make") != 0)
        goto end;

    char* uname = GET_UNAME(word[1]);
    if (!is_staff(uname))
    {
        utf8_commandf("MSG %s 您的权限不足", uname);
        goto clear;
    }

    utf8_commandf("BB !mp make %s", word_eol[5]);

clear:
    free(uname);

end:
    return HEXCHAT_EAT_NONE;
}

static int
forward_scb(char* word[], char* word_eol[], void* userdata)
{
    if (strlen(cfg.staff) == 0)
        goto end;

    char* uname = GET_UNAME(word[1]);
    // 确保是bancho发送的信息 && 屏蔽mp房间内私聊消息
    if (strcmp(uname, "BanchoBot") == 0 && hexchat_nickcmp(ph, word[3], cfg.bot) == 0)
        utf8_commandf("MSG %s %s", cfg.staff2, word_eol[4] + 1);
    free(uname);

end:
    return HEXCHAT_EAT_NONE;
}

static int
join_scb()
{
}

//-------------------------------------------------------------------------------

static void
utf8_command(const char* cmd)
{
    gchar* cmd_utf8 = _G_LOCALE_TO_UTF8(cmd);
    hexchat_command(ph, cmd_utf8);
    g_free(cmd_utf8);
}

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
utf8_print(const char* text)
{
    gchar* text_utf8 = _G_LOCALE_TO_UTF8(text);
    hexchat_print(ph, text_utf8);
    g_free(text_utf8);
}

static void
utf8_printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    strb_t* text = strb_new(NULL);
    strb_vsprintf(text, format, args);

    utf8_print(text->str);

    strb_free(text);
    va_end(args);
}

static hexchat_hook*
utf8_hook_command(const char* name,
    int pri,
    int (*callback) (char* word[], char* word_eol[], void* user_data),
    const char* help_text,
    void* userdata)
{
    gchar* help_text_utf8 = _G_LOCALE_TO_UTF8(help_text);
    hexchat_hook* hook = hexchat_hook_command(ph, name, pri, callback, help_text_utf8, userdata);
    g_free(help_text_utf8);
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

    // referee.ini
    init_cfg();
    const char* nick = hexchat_get_info(ph, "nick");
    strcpy(cfg.bot, nick);
    reload_cfg();

    // hook command
    utf8_hook_command("BB", HEXCHAT_PRI_NORM, bb_cb, "Usage: BB <message> 给BanchoBot发送消息", NULL);
    utf8_hook_command("INI", HEXCHAT_PRI_NORM, ini_cb, "Usage: INI <key> [value] 查看或修改referee.ini配置", NULL);
    utf8_hook_command("MAKE", HEXCHAT_PRI_NORM, make_cb, "See: !make", NULL);

    // hook server
    hexchat_hook_server(ph, "PRIVMSG", HEXCHAT_PRI_NORM, y_scb, NULL);
    hexchat_hook_server(ph, "PRIVMSG", HEXCHAT_PRI_NORM, make_scb, NULL);
    //hexchat_hook_server(ph, "PRIVMSG", HEXCHAT_PRI_NORM, forward_scb, NULL); // 将bancho私聊消息转发给host

    hexchat_printf(ph, "%s loaded successfully!\n", PNAME);
    return 1;       /* return 1 for success */
}

int
hexchat_plugin_deinit(hexchat_plugin* plugin_handle)
{
    hexchat_printf(ph, "Unloading %s.\n", PNAME);
    return 1;       /* return 1 for success */
}