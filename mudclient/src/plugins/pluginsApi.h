#pragma once

#include "plugin.h"

#define PLUGING_MENUID_START 33000
#define PLUGING_MENUID_END 57000
void collectGarbage();

bool initPluginsSystem();
bool loadModules();
void tmcLog(const tstring& msg);
void pluginLog(const tstring& msg);
void pluginLoadError(const wchar_t* msg, const wchar_t *fname);
int  pluginInvArgs(lua_State *L, const utf8* fname);
int  pluginError(const utf8* fname, const utf8* error);
int  pluginError(const utf8* error);
int  pluginLog(const utf8* msg);
void pluginsMenuCmd(UINT id);
void pluginsUpdateActiveObjects(int type, const tstring& pattern);

void regFunction(lua_State *L, const char* name, lua_CFunction f);
void regIndexMt(lua_State *L);
PluginData& find_plugin();

void pluginFormatByType(lua_State* L, int index, u8string *buf);
