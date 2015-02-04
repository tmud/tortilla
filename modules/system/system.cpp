#include "stdafx.h"
#pragma comment(lib, "lua.lib")

int system_messagebox(lua_State *L)
{
    HWND parent = NULL;
    luaT_run(L, "getParent", "");
    if (lua_isnumber(L, -1))
    {
        HWND wnd = (HWND)lua_tounsigned(L, -1);
        lua_pop(L, 1);
        if (::IsWindow(wnd))
            parent = wnd;
    }

    bool params_ok = false;

    u8string text;
    u8string caption("Tortilla Mud Client");
    UINT buttons = MB_OK;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        text.assign(lua_tostring(L, 1));
        params_ok = true;
    }
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
    {
        caption.assign(lua_tostring(L, 1));
        text.assign(lua_tostring(L, 2));
        params_ok = true;
    }
    else if (luaT_check(L, 3, LUA_TSTRING, LUA_TSTRING, LUA_TSTRING))
    {
        caption.assign(lua_tostring(L, 1));
        text.assign(lua_tostring(L, 2));

        u8string b(lua_tostring(L, 3));
        if (b == "ok,cancel") buttons = MB_OKCANCEL;
        else if (b == "cancel,ok") buttons = MB_OKCANCEL|MB_DEFBUTTON2;
        else if (b == "yes,no") buttons = MB_YESNO;
        else if (b == "no,yes") buttons = MB_YESNO|MB_DEFBUTTON2;

        if (strstr(b.c_str(), "error")) buttons |= MB_ICONERROR;
        else if (strstr(b.c_str(), "stop")) buttons |= MB_ICONERROR;
        else if (strstr(b.c_str(), "info")) buttons |= MB_ICONINFORMATION;
        else if (strstr(b.c_str(), "information")) buttons |= MB_ICONINFORMATION;
        else if (strstr(b.c_str(), "warning")) buttons |= MB_ICONWARNING;

        params_ok = true;
    }

    UINT result = 0;
    if (params_ok)
        result = MessageBox(parent, TU2W(text.c_str()), TU2W(caption.c_str()), buttons);

    lua_pushinteger(L, result);
    return 1;
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
