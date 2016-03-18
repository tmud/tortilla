#include "stdafx.h"

int get_name(lua_State *L)
{
    lua_pushstring(L, "Пример плагина");
    return 1;
}

int get_description(lua_State *L)
{
    lua_pushstring(L, "Этот плагин собран из примера в SDK мад-клиента");
    return 1;
}

int get_version(lua_State *L)
{
    lua_pushstring(L, "1.0");
    return 1;
}

int init(lua_State *L)
{
    luaT_run(L, "addMenu", "sdd", "Плагины/Пример...", 1, 2);
    return 0;
}

int menucmd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    int menuid = lua_tointeger(L, 1);
    lua_pop(L, 1);
    if (menuid == 1)
    {
        MessageBox(NULL, L"Мы в методе плагина", L"plugin", MB_OK);
    }
    return 0;
}

static const luaL_Reg plugin_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "menucmd", menucmd },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, plugin_methods);
    return 1;
}
