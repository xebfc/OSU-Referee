#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <glib.h>
#include <gmodule.h>

#include "hexchat-plugin.h"
#include "strb.h"
#include "wcsb.h"
#include "tournament.h"
#include "hashmap.h"

#define PNAME "OSU!Referee"
#define PDESC "Simple Edition Automated Referee"
#define PVERSION "0.1"

#define WORD_LIMIT 32

static hexchat_plugin * ph;      /* plugin handle */
static int enable = 1;

typedef int (*hook_callback) (char* word[], char* word_eol[], void* user_data);

typedef struct
{
    void* userdata;
    hook_callback func;
} userdata_t;

static void utf8_command(const char* cmd);
static void utf8_commandf(const char* format, ...);
static void utf8_print(const char* text);
static void utf8_printf(const char* format, ...);
static hexchat_hook* utf8_hook_command(const char* name,
    int pri,
    hook_callback callback,
    const char* help_text,
    void* userdata);
static hexchat_hook* utf8_hook_server(const char* name,
    int pri,
    hook_callback callback,
    void* userdata);
static void* utf8_unhook(hexchat_hook* hook);

static userdata_t*
userdata_new(void* userdata, hook_callback func)
{
    userdata_t* data = (userdata_t*)malloc(sizeof(userdata_t));
    if (data == NULL)
        return NULL;

    data->userdata = userdata;
    data->func = func;

    return data;
}

static int
_param_from_utf8(char* word[], char* word_eol[], void* userdata)
{
    char* local_word[WORD_LIMIT],
        * local_word_eol[WORD_LIMIT];

    int i = WORD_LIMIT;
    while (i-- > 0)
    {
        local_word[i] = _G_LOCALE_FROM_UTF8(word[i]);
        local_word_eol[i] = _G_LOCALE_FROM_UTF8(word_eol[i]);
    }

    userdata_t* data = (userdata_t*)userdata;
    int eat = data->func(&local_word, &local_word_eol, data->userdata);

    i = WORD_LIMIT;
    while (i-- > 0)
    {
        g_free(local_word[i]);
        local_word[i] == NULL;

        g_free(local_word_eol[i]);
        local_word_eol[i] = NULL;
    }

    return eat;
}

//-------------------------------------------------------------------------------

#define CHAR_MAX 0xff
#define PVER_KEY "_pVersion"
#define BANCHOBOT "BanchoBot"
#define DEFAULT_ROOM_NAME "OSAR: (Blue Team) vs (Red Team)"

#define ARR_LEN(a) (sizeof(a) / sizeof(*a))
#define GET_UNAME(prefix) ( \
    substr(prefix, 1, indexof_char(prefix, '!')) \
)
#define IS_BOT(uname) (hexchat_nickcmp(ph, cfg.bot, uname) == 0)

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

/**
* 判断 uname 是否为staff
* 由于bot本身需要登录一个用户，因此bot用户自带staff权限
* 默认情况下当返回值大于0时则判断该用户为staff
* staff用户需要在referee.ini文件中进行配置
*
* @param uanme 用户名
* @return 0：非staff用户 1：bot用户 2：staff用户
*/
static int is_staff(uname)
{
    if (IS_BOT(uname))
        return 1;
    else if (cfg.staff != NULL)
    {
        char staff[CHAR_MAX];
        strcpy(staff, cfg.staff); // strtok 会改变 staff 的值，strcpy 保证了原字符串的完整性

        char* s = strtok(staff, ",");
        while (s != NULL)
        {
            if (hexchat_nickcmp(ph, s, uname) == 0)
                return 2;

            s = strtok(NULL, ",");
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
    while (len-- > 0)
        if (hexchat_pluginpref_get_str(ph, attrs[len], NULL) == 0)
            hexchat_pluginpref_set_str(ph, attrs[len], attrs_default_value[len]);
    hexchat_pluginpref_set_str(ph, PVER_KEY, PVERSION);
}

static void reload_cfg()
{
    int len = ARR_LEN(attrs);
    while (len-- > 0)
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

    utf8_commandf("MSG %s %s", BANCHOBOT, word_eol[2]);

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
    utf8_commandf("MSG %s !mp make %s", cfg.bot
        , strlen(word_eol[2]) > 0 ? word_eol[2] : DEFAULT_ROOM_NAME);
    return HEXCHAT_EAT_ALL;
}

// hook server
//-------------------------------------------------------------------------------

static int
y_scb(char* word[], char* word_eol[], void* userdata)
{
    if (strcmp(word[4], ":!y") != 0)
        goto end;

    char* sender = GET_UNAME(word[1]);
    utf8_commandf("MSG %s 斯哈斯哈", sender);
    free(sender);

end:
    return HEXCHAT_EAT_NONE;
}

static int
x_scb(char* word[], char* word_eol[], void* userdata)
{
    if (strcmp(word[4], ":雪") != 0)
        goto end;

    char* sender = GET_UNAME(word[1]);
    utf8_commandf("MSG %s 哧溜~", sender);
    free(sender);

end:
    return HEXCHAT_EAT_NONE;
}

static int
make_scb(char* word[], char* word_eol[], void* userdata)
{
    static GQueue* creator = NULL;
    if (creator == NULL)
        creator = g_queue_new();
    else if (!g_queue_is_empty(creator))
    {
        char* uname = g_queue_pop_head(creator);
        // 确保是bancho发送的消息 && 屏蔽mp房内消息（必须是私聊给bot用户的消息）
        if (indexof_str(word[1], BANCHOBOT) != -1 && hexchat_nickcmp(ph, word[3], cfg.bot) == 0)
        {
            int only_staff = !IS_BOT(uname);

            if (only_staff)
                utf8_commandf("MSG %s %s", uname, word_eol[4] + 1); // 将bancho私聊消息转发给创建者

            // 当mp房间创建成功时
            if (indexof_str(word_eol[4], "Created the tournament match") != -1)
            {
                strb_t* mp_channel = strb_new(word[8]);
                int beginIndex = strb_last_indexof_char(mp_channel, '/') + 1;
                strb_substr(mp_channel, beginIndex, strb_strlen(mp_channel));
                strb_precat(mp_channel, "#mp_");

                if (only_staff)
                {
                    utf8_commandf("MSG %s !mp invite %s", mp_channel->str, uname); // 邀请创建者进房
                    utf8_commandf("MSG %s !mp addref %s", mp_channel->str, uname); // 允许创建者使用mp指令
                }

                tourney_new(mp_channel->str, word_eol[9]); // 解析房名

                // 设置房间
                utf8_commandf("MSG %s !mp set %s", mp_channel->str, cfg.set);
                utf8_commandf("MSG %s !mp password %s", mp_channel->str, cfg.password);

                strb_free(mp_channel);
            }
        }
        free(uname);
        goto end;
    }

    if (indexof_str(word_eol[4], "!mp make") == -1)
        goto end;

    char* uname = GET_UNAME(word[1]);

    GRegex* reg = g_regex_new("^.+: (.+) vs (.+)$", G_REGEX_RAW, 0, NULL);
    if (strlen(word_eol[6]) > 0 && !g_regex_match(reg, word_eol[6], 0, NULL))
    {
        utf8_commandf("MSG %s 您必须根据 osu!tourney 控制面板列出的模板创建一个多人房间。", uname);
        utf8_commandf("MSG %s 此模板包括：Your_acronym_in_tournament.cfg: (Team Name 1) vs (Team Name 2)", uname);
        utf8_commandf("MSG %s Your_acronym_in_tournament.cfg 您可以在 tournaments.cfg 里改变比赛的简称。", uname);
        utf8_commandf("MSG %s 在我们的例子中，这个简称为 \"Test Tourney\"。", uname);
        utf8_commandf("MSG %s Test Tourney: (Team Name 1) vs (Team Name 2)", uname);
        utf8_commandf(
            "MSG %s 您可以使用您的队伍名称替换 \"Team Name 1\" 和 \"Team Name 2\"，不过请在其周围保留括号。"
            , uname);

        goto err;
    }

    if (is_staff(uname) > 0)
    {
        g_queue_push_tail(creator, uname);
        utf8_commandf("BB !mp make %s", word_eol[6]);
        goto clear;
    }

    utf8_commandf("MSG %s 您的权限不足", uname);

err:
    free(uname);

clear:
    g_regex_unref(reg);

end:
    return HEXCHAT_EAT_NONE;
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
    hook_callback callback,
    const char* help_text,
    void* userdata)
{
    userdata_t* data = userdata_new(userdata, callback);
    if (data == NULL)
        return NULL;

    gchar* help_text_utf8 = _G_LOCALE_TO_UTF8(help_text);
    hexchat_hook* hook = hexchat_hook_command(ph, name, pri, _param_from_utf8, help_text_utf8, data);
    g_free(help_text_utf8);

    return hook;
}

static hexchat_hook*
utf8_hook_server(const char* name,
    int pri,
    hook_callback callback,
    void* userdata)
{
    userdata_t* data = userdata_new(userdata, callback);
    if (data == NULL)
        return NULL;

    return hexchat_hook_server(ph, name, pri, _param_from_utf8, data);
}

static void*
utf8_unhook(hexchat_hook* hook)
{
    userdata_t* data = (userdata_t*)hexchat_unhook(ph, hook);
    void* userdata = data->userdata;
    free(data);

    return userdata;
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
    utf8_hook_server("PRIVMSG", HEXCHAT_PRI_NORM, y_scb, NULL);
    utf8_hook_server("PRIVMSG", HEXCHAT_PRI_NORM, x_scb, NULL);
    utf8_hook_server("PRIVMSG", HEXCHAT_PRI_NORM, make_scb, NULL);

    hexchat_printf(ph, "%s loaded successfully!\n", PNAME);
    return 1;       /* return 1 for success */
}

int
hexchat_plugin_deinit(hexchat_plugin* plugin_handle)
{
    hexchat_printf(ph, "Unloading %s.\n", PNAME);
    return 1;       /* return 1 for success */
}