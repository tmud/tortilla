#include "stdafx.h"
#pragma comment(lib, "lua.lib")

int system_test(lua_State *L)
{
    //MessageBox(NULL, L"1", L"test", MB_OK);
    return 0;
}

int luaopen_system(lua_State *L)
{
    lua_newtable(L);
    lua_pushstring(L, "test");
    lua_pushcfunction(L, system_test);
    lua_settable(L, -3);    
    return 1;
}
