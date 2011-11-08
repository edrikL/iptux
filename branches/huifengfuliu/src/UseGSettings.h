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
#ifndef USEGSETTINGS_H
#define USEGSETTINGS_H

class UseGSettings
{
public:
     UseGSettings();
     virtual ~UseGSettings();

     static void initGSettings(const char *sPath = NULL);
     void getSettings(const char *sSchema, const char *sFileName = NULL);
     char *getString(const char *sKey, const char *sDefault = "");
     void setString(const char *sKey, const char *sValue);
     int getInt(const char *sKey);
     void setInt(const char *sKey, int iValue);
     double getDouble(const char *sKey);
     void setDouble(const char *sKey, double dValue);
     char **getStrV(const char *sKey);
     void setStrV(const char *sKey, char **pValue);

private:
     void freeSettings();

     void *m_pSettings;
};

#endif
