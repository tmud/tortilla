#include "stdafx.h"
#pragma comment(lib, "lua.lib")

int system_messagebox(lua_State *L)
{
    MessageBox(NULL, L"test msgbox", L"API", MB_OK);
    return 0;
}

int system_outputdebugstring(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        std::string label(lua_tostring(L, -1));
        lua_pop(L, 1);
        luaT_showLuaStack(L, label.c_str());
        return 0;
    }
    luaT_showLuaStack(L, NULL);
    return 0;
}

int system_dbgtable(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        std::string label(lua_tostring(L, -1));
        lua_pop(L, 1);
        luaT_showTableOnTop(L, label.c_str());
        return 0;
    }
    luaT_showTableOnTop(L, NULL);
    return 0;
}

static const luaL_Reg system_methods[] =
{
    { "dbgstack", system_outputdebugstring },
    { "dbgtable", system_dbgtable },
    { "msgbox", system_messagebox },
    { NULL, NULL }
};

int luaopen_system(lua_State *L)
{
    luaL_newlib(L, system_methods);
    return 1;
}
