//
// C++ Implementation: ProgramData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <string.h>

#include "ProgramData.h"
#include "CoreThread.h"
#include "utils.h"

extern ProgramData progdt;

/**
 * 类构造函数.
 */
ProgramData::ProgramData():nickname(NULL), mygroup(NULL),
 myicon(NULL), path(NULL),  sign(NULL), codeset(NULL), encode(NULL),
 palicon(NULL), font(NULL), flags(0), transtip(NULL), msgtip(NULL),
 volume(1.0), sndfgs(~0), netseg(NULL), urlregex(NULL), xcursor(NULL),
 lcursor(NULL), table(NULL), settings(NULL)
{
        gettimeofday(&timestamp, NULL);
        pthread_mutex_init(&mutex, NULL);
}

/**
 * 类析构函数.
 */
ProgramData::~ProgramData()
{
        g_free(nickname);
        g_free(mygroup);
        g_free(myicon);
        g_free(path);
        g_free(sign);

        g_free(codeset);
        g_free(encode);
        g_free(palicon);
        g_free(font);

        g_free(msgtip);
        g_free(transtip);

        for (GSList *tlist = netseg; tlist; tlist = g_slist_next(tlist))
                delete (NetSegment *)tlist->data;
        g_slist_free(netseg);

        if (urlregex)
                g_regex_unref(urlregex);
        if (xcursor)
                gdk_cursor_unref(xcursor);
        if (lcursor)
                gdk_cursor_unref(lcursor);
        if (table)
                g_object_unref(table);
        if (settings)
	        g_object_unref(settings);
        pthread_mutex_destroy(&mutex);
}

/**
 * 初始化相关类成员数据.
 */
void ProgramData::InitSublayer()
{
        AddGSettingsNotify();
        ReadProgData();
        CheckIconTheme();
        CreateRegex();
        CreateCursor();
        CreateTagTable();
}

/**
 * 写出程序数据.
 */
void ProgramData::WriteProgData()
{
        gettimeofday(&timestamp, NULL); //更新时间戳

        g_settings_set_string(settings, "nick-name", nickname);
        g_settings_set_string(settings, "belong-group", mygroup);
        g_settings_set_string(settings, "my-icon", myicon);
        g_settings_set_string(settings, "archive-path", path);
        g_settings_set_string(settings, "personal-sign", sign);
        g_settings_set_string(settings, "candidacy-encode", codeset);
        g_settings_set_string(settings, "preference-encode", encode);
        g_settings_set_string(settings, "pal-icon", palicon);
        g_settings_set_string(settings, "panel-font", font);
        g_settings_set_boolean(settings, "open-chat", FLAG_ISSET(flags, 7) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "hide-startup", FLAG_ISSET(flags, 6) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "open-transmission", FLAG_ISSET(flags, 5) ? TRUE : FALSE);
	g_settings_set_boolean(settings, "use-enter-key", FLAG_ISSET(flags, 4) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "clearup-history", FLAG_ISSET(flags, 3) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "record-log", FLAG_ISSET(flags, 2) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "open-blacklist", FLAG_ISSET(flags, 1) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "proof-shared", FLAG_ISSET(flags, 0) ? TRUE : FALSE);
        g_settings_set_string(settings, "trans-tip", transtip);
        g_settings_set_string(settings, "msg-tip", msgtip);
        g_settings_set_double(settings, "volume-degree", volume);
        g_settings_set_boolean(settings, "transnd-support", FLAG_ISSET(sndfgs, 2) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "msgsnd-support", FLAG_ISSET(sndfgs, 1) ? TRUE : FALSE);
        g_settings_set_boolean(settings, "sound-support", FLAG_ISSET(sndfgs, 0) ? TRUE : FALSE);

        WriteNetSegment(settings);
}

/**
 * 深拷贝一份网段数据.
 * @return 网段数据
 */
GSList *ProgramData::CopyNetSegment()
{
        NetSegment *ns, *pns;
        GSList *tlist, *nseg;

        nseg = NULL;
        tlist = netseg;
        while (tlist) {
                pns = (NetSegment *)tlist->data;
                ns = new NetSegment;
                nseg = g_slist_append(nseg, ns);
                ns->startip = g_strdup(pns->startip);
                ns->endip = g_strdup(pns->endip);
                ns->description = g_strdup(pns->description);
                tlist = g_slist_next(tlist);
        }

        return nseg;
}

/**
 * 查询(ipv4)所在网段的描述串.
 * @param ipv4 ipv4
 * @return 描述串
 */
char *ProgramData::FindNetSegDescription(in_addr_t ipv4)
{
        in_addr_t startip, endip;
        NetSegment *pns;
        GSList *tlist;
        char *description;

        ipv4 = ntohl(ipv4);
        description = NULL;
        tlist = netseg;
        while (tlist) {
                pns = (NetSegment *)tlist->data;
                inet_pton(AF_INET, pns->startip, &startip);
                startip = ntohl(startip);
                inet_pton(AF_INET, pns->endip, &endip);
                endip = ntohl(endip);
                ipv4_order(&startip, &endip);
                if (ipv4 >= startip && ipv4 <= endip) {
                        description = g_strdup(pns->description);
                        break;
                }
                tlist = g_slist_next(tlist);
        }

        return description;
}

/**
 * 读取程序数据.
 */
void ProgramData::ReadProgData()
{
	nickname = g_settings_get_string(settings, "nick-name");
	if(strlen(nickname) == 0) nickname = g_strdup(g_get_user_name());
	mygroup = g_settings_get_string(settings, "belong-group");
	if(strlen(mygroup) == 0) mygroup = g_strdup("");
        myicon = g_settings_get_string(settings, "my-icon");
	if(strlen(myicon) == 0) myicon = g_strdup("icon-tux.png");
	path = g_settings_get_string(settings, "archive-path");
	if(strlen(path) == 0) path = g_strdup(g_get_home_dir());
        sign = g_settings_get_string(settings, "personal-sign");
	if(strlen(sign) == 0) sign = g_strdup("");
	codeset = g_settings_get_string(settings, "candidacy-encode");
	if(strlen(codeset) == 0) codeset = g_strdup("utf-16");
        encode = g_settings_get_string(settings, "preference-encode");
	if(strlen(encode) == 0) encode = g_strdup("utf-8");
        palicon = g_settings_get_string(settings, "pal-icon");
	if(strlen(palicon) == 0) palicon = g_strdup("icon-qq.png");
        font = g_settings_get_string(settings, "panel-font");
	if(strlen(font) == 0) font = g_strdup("Sans Serif 10");

        if(g_settings_get_boolean(settings, "open-chat")) FLAG_SET(flags, 7);
        if(g_settings_get_boolean(settings, "hide-startup")) FLAG_SET(flags, 6);
        if(g_settings_get_boolean(settings, "open-transmission")) FLAG_SET(flags, 5);
        if(g_settings_get_boolean(settings, "use-enter-key")) FLAG_SET(flags, 4);
        if(g_settings_get_boolean(settings, "clearup-history")) FLAG_SET(flags, 3);
        if(g_settings_get_boolean(settings, "record-log")) FLAG_SET(flags, 2);
        if(g_settings_get_boolean(settings, "open-blacklist")) FLAG_SET(flags, 1);
        if(g_settings_get_boolean(settings, "proof-shared")) FLAG_SET(flags, 0);

        msgtip = g_settings_get_string(settings, "msg-tip");
	if(strlen(msgtip) == 0) msgtip = g_strdup(__SOUND_PATH "/msg.ogg");
        transtip = g_settings_get_string(settings, "trans-tip");
	if(strlen(transtip) == 0) transtip = g_strdup(__SOUND_PATH "/trans.ogg");

	volume = g_settings_get_double(settings, "volume-degree");
	if(!g_settings_get_boolean(settings, "transnd-support")) FLAG_CLR(sndfgs, 2);
	if(!g_settings_get_boolean(settings, "msgsnd-support")) FLAG_CLR(sndfgs, 1);
	if(!g_settings_get_boolean(settings, "sound-support")) FLAG_CLR(sndfgs, 0);

        ReadNetSegment(settings);
}

/**
 * 监视程序配置文件信息数据的变更.
 */
void ProgramData::AddGSettingsNotify()
{
     settings = get_gsettings("iptux.programdata");
     g_signal_connect(settings, "changed", G_CALLBACK(GSettingsOnKeyChanged), NULL);
}

/**
 * 确保头像数据被存放在主题库中.
 */
void ProgramData::CheckIconTheme()
{
        char pathbuf[MAX_PATHLEN];
        GdkPixbuf *pixbuf;

        snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", myicon);
        if (access(pathbuf, F_OK) != 0) {
                snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
                                 g_get_user_config_dir(), myicon);
                if ( (pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
                        gtk_icon_theme_add_builtin_icon(myicon, MAX_ICONSIZE, pixbuf);
                        g_object_unref(pixbuf);
                }
        }

        snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", palicon);
        if (access(pathbuf, F_OK) != 0) {
                snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
                                 g_get_user_config_dir(), palicon);
                if ( (pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
                        gtk_icon_theme_add_builtin_icon(palicon, MAX_ICONSIZE, pixbuf);
                        g_object_unref(pixbuf);
                }
        }
}

/**
 * 创建识别URL的正则表达式.
 */
void ProgramData::CreateRegex()
{
        urlregex = g_regex_new(URL_REGEX, GRegexCompileFlags(0),
                                 GRegexMatchFlags(0), NULL);
}

/**
 * 创建鼠标光标.
 */
void ProgramData::CreateCursor()
{
        xcursor = gdk_cursor_new(GDK_XTERM);
        lcursor = gdk_cursor_new(GDK_HAND2);
}

/**
 * 创建用于(text-view)的一些通用tag.
 * @note 给这些tag一个"global"标记，表示这些对象是全局共享的
 */
void ProgramData::CreateTagTable()
{
        GtkTextTag *tag;

        table = gtk_text_tag_table_new();

        tag = gtk_text_tag_new("pal-color");
        g_object_set(tag, "foreground", "blue", NULL);
        g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
        gtk_text_tag_table_add(table, tag);
        g_object_unref(tag);

        tag = gtk_text_tag_new("me-color");
        g_object_set(tag, "foreground", "green", NULL);
        g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
        gtk_text_tag_table_add(table, tag);
        g_object_unref(tag);

        tag = gtk_text_tag_new("error-color");
        g_object_set(tag, "foreground", "red", NULL);
        g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
        gtk_text_tag_table_add(table, tag);
        g_object_unref(tag);

        tag = gtk_text_tag_new("sign-words");
        g_object_set(tag, "indent", 10, "foreground", "#1005F0",
                                 "font", "Sans Italic 8", NULL);
        g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
        gtk_text_tag_table_add(table, tag);
        g_object_unref(tag);

        tag = gtk_text_tag_new("url-link");
        g_object_set(tag, "foreground", "blue",
                 "underline", PANGO_UNDERLINE_SINGLE, NULL);
        g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
        gtk_text_tag_table_add(table, tag);
        g_object_unref(tag);
}

/**
 * 写出网段数据.
 * @param pSettings GSettings
 */
void ProgramData::WriteNetSegment(GSettings *pSettings)
{
        NetSegment *pns;
        GSList *tlist;
   
        pthread_mutex_lock(&mutex);
        tlist = netseg;
	gchar **pList = new gchar *[g_slist_length(tlist) * 3 + 1];
	int i = 0;
        while (tlist) {
                pns = (NetSegment *)tlist->data;
                pList[i] = pns->startip;
                pList[i + 1] = pns->endip;
                pList[i + 2] = pns->description ? pns->description : (char *)"";
		i += 3;
                tlist = g_slist_next(tlist);
        }
	pList[i] = NULL;
        g_settings_set_strv(pSettings, "scan-net-segment", pList);
        pthread_mutex_unlock(&mutex);
	delete [] pList;
}

/**
 * 读取网段数据.
 * @param pSettings GSettings
 */
void ProgramData::ReadNetSegment(GSettings *pSettings)
{
        NetSegment *ns;
	gchar **pList = g_settings_get_strv(pSettings, "scan-net-segment");
	
        pthread_mutex_lock(&mutex);
        for(int i = 0; pList[i]; i += 3)
	{
                ns = new NetSegment;
                netseg = g_slist_append(netseg, ns);
                ns->startip = pList[i];
                ns->endip = pList[i + 1];
                ns->description = pList[i + 2];
        }
        pthread_mutex_unlock(&mutex);
	delete [] pList;
}

/**
 * 配置文件信息数据变更的响应处理函数.
 * 当本程序写出数据时，程序会自动更新时间戳，所以若当前时间与时间戳间隔太短，
 * 便认为是本程序写出数据导致配置文件信息数据发生了变化，在这种情况下，
 * 响应函数无需理睬数值的变更.\n
 * @param pSettings GSettings。
 * @param sKey 发生改变的键。
 */
void ProgramData::GSettingsOnKeyChanged(GSettings *pSettings, const gchar *sKey)
{
        struct timeval stamp;
        const char *str;
        bool update;

        /* 如果没有值则直接跳出 */
        if (!pSettings || !sKey) return;
        /* 如果间隔太短则直接跳出 */
        gettimeofday(&stamp, NULL);
        if (difftimeval(stamp, progdt.timestamp) < 1.0)
                return;

        /* 匹配键值并修正 */
        update = false; //预设更新标记为假
        if (strcmp(sKey, "nick-name") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.nickname);
                        progdt.nickname = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(sKey, "belong-group") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.mygroup);
                        progdt.mygroup = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(sKey, "my-icon") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.myicon);
                        progdt.myicon = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(sKey, "archive-path") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.path);
                        progdt.path = g_strdup(str);
                }
        } else if (strcmp(sKey, "personal-sign") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.sign);
                        progdt.sign = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(sKey, "candidacy-encode") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.codeset);
                        progdt.codeset = g_strdup(str);
                }
        } else if (strcmp(sKey, "preference-encode") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.encode);
                        progdt.encode = g_strdup(str);
                }
        } else if (strcmp(sKey, "pal-icon") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.palicon);
                        progdt.palicon = g_strdup(str);
                }
        } else if (strcmp(sKey, "panel-font") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.font);
                        progdt.font = g_strdup(str);
                }
        } else if (strcmp(sKey, "hide-startup") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.flags, 6);
                else
                        FLAG_CLR(progdt.flags, 6);
        } else if (strcmp(sKey, "open-transmission") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.flags, 5);
                else
                        FLAG_CLR(progdt.flags, 5);
        } else if (strcmp(sKey, "use-enter-key") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.flags, 4);
                else
                        FLAG_CLR(progdt.flags, 4);
        } else if (strcmp(sKey, "clearup-history") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.flags, 3);
                else
                        FLAG_CLR(progdt.flags, 3);
        } else if (strcmp(sKey, "record-log") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.flags, 2);
                else
                        FLAG_CLR(progdt.flags, 2);
        } else if (strcmp(sKey, "open-blacklist") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.flags, 1);
                else
                        FLAG_CLR(progdt.flags, 1);
        } else if (strcmp(sKey, "proof-shared") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.flags, 0);
                else
                        FLAG_CLR(progdt.flags, 0);
        } else if (strcmp(sKey, "trans-tip") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.transtip);
                        progdt.transtip = g_strdup(str);
                }
        } else if (strcmp(sKey, "msg-tip") == 0) {
                if ( (str = g_settings_get_string(pSettings, sKey))) {
                        g_free(progdt.transtip);
                        progdt.transtip = g_strdup(str);
                }
        } else if (strcmp(sKey, "volume-degree") == 0) {
	     progdt.volume = g_settings_get_double(pSettings, sKey);
        } else if (strcmp(sKey, "transnd_support") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.sndfgs, 2);
                else
                        FLAG_CLR(progdt.sndfgs, 2);
        } else if (strcmp(sKey, "msgsnd-support") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.sndfgs, 1);
                else
                        FLAG_CLR(progdt.sndfgs, 1);
        } else if (strcmp(sKey, "sound-support") == 0) {
                if (g_settings_get_boolean(pSettings, sKey))
                        FLAG_SET(progdt.sndfgs, 0);
                else
                        FLAG_CLR(progdt.sndfgs, 0);
        }

        /* 如果需要更新则调用更新处理函数 */
        if (update)
                CoreThread::UpdateMyInfo();
}

