#pragma once

#define PLUGING_MENUID_START 33000
#define PLUGING_MENUID_END 57000

bool initPluginsSystem();
bool loadModules();
void tmcLog(const tstring& msg);
void pluginLog(const tstring& msg);
int  pluginInvArgs(lua_State *L, const utf8* fname);
int  pluginError(lua_State *L, const utf8* fname, const utf8* error);
int  pluginError(lua_State *L, const utf8* error);
int  pluginLog(lua_State *L, const utf8* msg);
void pluginsMenuCmd(UINT id);
void pluginsUpdateActiveObjects(int type);