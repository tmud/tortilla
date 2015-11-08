#pragma once

#include "plugin.h"

#define PLUGING_MENUID_START 33000
#define PLUGING_MENUID_END 57000
void collectGarbage();
bool initPluginsSystem();
bool loadModules();
void unloadModules();
void tmcLog(const tstring& msg);
void pluginLoadError(const tchar* msg, const wchar_t *plugin_name);
int  pluginInvArgs(lua_State *L, const tchar* fname);
int  pluginLoadFail(lua_State *L, const tchar* fname, const tchar* file);
int  pluginError(const tchar* fname, const tchar* error);
int  pluginError(const tchar* error);
int  pluginLog(const tchar* msg);
int  pluginOut(const tchar* msg);

void pluginsMenuCmd(UINT id);
void pluginsUpdateActiveObjects(int type);

void regFunction(lua_State *L, const char* name, lua_CFunction f);
void regIndexMt(lua_State *L);
PluginData& find_plugin();

void pluginFormatByType(lua_State* L, int index, tstring *buf);
