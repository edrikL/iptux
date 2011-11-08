//
// C++ Interface: UseGSettings
//
// Description:
// 使用GSettings读写配置文件。
//
// Author: zxln <huifengfuliu@gmail.com>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifdef HAVE_CONFIG_H
     #include <config.h>
#endif

#include <string.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>
#include "UseGSettings.h"

#define MAX_PATH 1024

UseGSettings::UseGSettings()
{
     m_pSettings = NULL;
}

UseGSettings::~UseGSettings()
{
     freeSettings();
}

/**
 * 初始化环境变量。
 * @param sPath /glib-2.0/schemas/gschemas.compiled的安装路径，如果为NULL，则默认为__DATA_PATH。
 * @note GSettings需要读取XDG_DATA_DIRS下的/glib-2.0/schemas/gschemas.compiled。
 */
void UseGSettings::initGSettings(const char *sPath)
{
     gchar sEnv[MAX_PATH];
     if(sPath)
     {
	  g_sprintf(sEnv, "%s:%s", sPath, g_getenv("XDG_DATA_DIRS"));
     }
     else
     {
	  g_sprintf(sEnv, "%s:%s", __DATA_PATH, g_getenv("XDG_DATA_DIRS"));
     }
     g_setenv("XDG_DATA_DIRS", sEnv, true);
}

/**
 * 释放GSEttings资源。
 * @note 对象析构时调用，无需手动调用。
 */
void UseGSettings::freeSettings()
{
     if(m_pSettings)
     {
	  g_object_unref(G_SETTINGS(m_pSettings));
	  m_pSettings = NULL;
     }
}

/**
 * 获取当前用户的配置信息。
 * @param sSchema 用户配置项的名称。
 * @param sFileName 配置文件的全路径，如果为NULL，则默认为用户配置路径下的/$PACKAGE_NAME/settings.ini。
 */
void UseGSettings::getSettings(const char *sSchema, const char *sFileName)
{
     freeSettings();
     GSettingsBackend *pBkend;
     if(sFileName)
     {
	  pBkend = g_keyfile_settings_backend_new(sFileName, "/", NULL);
     }
     else
     {
	  gchar sSettingsFile[MAX_PATH];
	  g_sprintf(sSettingsFile, "%s/%s/settings.ini", g_get_user_config_dir(), PACKAGE_NAME);
	  pBkend = g_keyfile_settings_backend_new(sSettingsFile, "/", NULL);
     }
     m_pSettings = g_settings_new_with_backend(sSchema, pBkend);
     g_object_unref(pBkend);
}

/**
 * 获取配置文件中skey键的字符串键值。
 * @param sKey 需要查询的键。
 * @param sDefault 如果键值为空，则返回该默认字符串。
 * @return 字符串键值。
 * @note 返回的字符串需要手动释放。
 */
char *UseGSettings::getString(const char *sKey, const char *sDefault)
{
     char *str;
     str = g_settings_get_string(G_SETTINGS(m_pSettings), sKey);
     if(strlen(str) <= 0)
     {
	  str = g_strdup(sDefault);
     }
     return str;
}

/**
 * 设置配置文件中sKey键的字符串键值。
 * @param sKey 需要设置的键。
 * @param sValue 字符串键值。
 */
void UseGSettings::setString(const char *sKey, const char *sValue)
{
     g_settings_set_string(G_SETTINGS(m_pSettings), sKey, sValue);
}

/**
 * 获取配置文件中skey键的整数键值。
 * @param sKey 需要查询的键。
 * @return 整数键值。
 * @note 布尔型数值可以用整数代替，0为FALSE，非0为TRUE.
 */
int UseGSettings::getInt(const char *sKey)
{
     return g_settings_get_int(G_SETTINGS(m_pSettings), sKey);
}

/**
 * 设置配置文件中sKey键的整数键值。
 * @param sKey 需要设置的键。
 * @param dValue 整数键值。
 */
void UseGSettings::setInt(const char *sKey, int iValue)
{
     g_settings_set_int(G_SETTINGS(m_pSettings), sKey, iValue);
}

/**
 * 获取配置文件中skey键的浮点数键值。
 * @param sKey 需要查询的键。
 * @return 浮点数键值。
 */
double UseGSettings::getDouble(const char *sKey)
{
     return g_settings_get_double(G_SETTINGS(m_pSettings), sKey);
}

/**
 * 设置配置文件中sKey键的浮点数键值。
 * @param sKey 需要设置的键。
 * @param dValue 浮点数键值。
 */
void UseGSettings::setDouble(const char *sKey, double dValue)
{
     g_settings_set_double(G_SETTINGS(m_pSettings), sKey, dValue);
}

/**
 * 获取配置文件中skey键的字符串数组键值。
 * @param sKey 需要查询的键。
 * @return 字符串数组键值。
 * @note 返回的字符串数组需要手动删除。
 */
char **UseGSettings::getStrV(const char *sKey)
{
     return g_settings_get_strv(G_SETTINGS(m_pSettings), sKey);
}

/**
 * 设置配置文件中sKey键的字符串数组键值。
 * @param sKey 需要设置的键。
 * @param pValue 字符串数组键值。
 */
void UseGSettings::setStrV(const char *sKey, char **pValue)
{
     g_settings_set_strv(G_SETTINGS(m_pSettings), sKey, pValue);
}


