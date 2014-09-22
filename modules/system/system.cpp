#include "stdafx.h"
#pragma comment(lib, "lua.lib")

int system_test(lua_State *L)
{
    //MessageBox(NULL, L"1", L"test", MB_OK);
    return 0;
}

int system_outputdebugstring(lua_State *L)
{
    int n = lua_gettop(L);
    for (int i = 1; i <= n; ++i)
    {
        switch (lua_type(L, i))
        {
        case LUA_TNUMBER:
        case LUA_TSTRING:
        case LUA_TBOOLEAN:
            OutputDebugString( convert_utf8_to_wide(lua_tostring(L, i)) );
        break;
        case LUA_TNIL:
            OutputDebugString(L"<nil>");
        break;
        case LUA_TTABLE:
            OutputDebugString(L"<table>");
        break;
        case LUA_TLIGHTUSERDATA:
            OutputDebugString(L"<light userdata>");
        break;
        case LUA_TUSERDATA:
            OutputDebugString(L"<userdata>");
        break;
        case LUA_TFUNCTION:
            OutputDebugString(L"<function>");
        break;
        case LUA_TTHREAD:
            OutputDebugString(L"<thread>");
        break;
        default:
            OutputDebugString(L"<unknown>");
        }
        OutputDebugString(L"\r\n");        
    }
    return 0;
}

int luaopen_system(lua_State *L)
{
    lua_newtable(L);
    lua_pushstring(L, "debuglog");
    lua_pushcfunction(L, system_outputdebugstring);
    lua_settable(L, -3);
    return 1;
}
