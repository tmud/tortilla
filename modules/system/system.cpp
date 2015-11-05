#include "stdafx.h"
#pragma comment(lib, "lua.lib")

int system_messagebox(lua_State *L)
{
    HWND parent = base::getParent(L);
    if (!::IsWindow(parent))
        parent = NULL;

    bool params_ok = false;

    std::wstring text;
    std::wstring caption(L"Tortilla Mud Client");
    UINT buttons = MB_OK;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        text.assign(luaT_towstring(L, 1));
        params_ok = true;
    }
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
    {
        caption.assign(luaT_towstring(L, 1));
        text.assign(luaT_towstring(L, 2));
        params_ok = true;
    }
    else if (luaT_check(L, 3, LUA_TSTRING, LUA_TSTRING, LUA_TSTRING))
    {
        caption.assign(luaT_towstring(L, 1));
        text.assign(luaT_towstring(L, 2));

        std::wstring b(luaT_towstring(L, 3));
        if (b == L"ok,cancel") buttons = MB_OKCANCEL;
        else if (b == L"cancel,ok") buttons = MB_OKCANCEL|MB_DEFBUTTON2;
        else if (b == L"yes,no") buttons = MB_YESNO;
        else if (b == L"no,yes") buttons = MB_YESNO|MB_DEFBUTTON2;

        if (wcsstr(b.c_str(), L"error")) buttons |= MB_ICONERROR;
        else if (wcsstr(b.c_str(), L"stop")) buttons |= MB_ICONERROR;
        else if (wcsstr(b.c_str(), L"info")) buttons |= MB_ICONINFORMATION;
        else if (wcsstr(b.c_str(), L"information")) buttons |= MB_ICONINFORMATION;
        else if (wcsstr(b.c_str(), L"warning")) buttons |= MB_ICONWARNING;
        else if (wcsstr(b.c_str(), L"question")) buttons |= MB_ICONQUESTION;

        params_ok = true;
    }
    UINT result = 0;
    if (params_ok)
        result = MessageBox(parent, text.c_str(), caption.c_str(), buttons);

    lua_pushinteger(L, result);
    return 1;
}

int system_debugstack(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        std::wstring label(luaT_towstring(L, -1));
        luaT_showLuaStack(L, label.c_str());
        return 0;
    }
    luaT_showLuaStack(L, NULL);
    return 0;
}

int system_dbgtable(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TTABLE, LUA_TSTRING))
    {
        std::wstring label(luaT_towstring(L, -1));
        luaT_showTableOnTop(L, label.c_str());
        return 0;
    }
    if (luaT_check(L, 1, LUA_TTABLE))
        luaT_showTableOnTop(L, NULL);
    return 0;
}

void formatByType(lua_State* L, int index, std::wstring *buf)
{
    int i = index;
    int type = lua_type(L, i);
    wchar_t dbuf[32];
    buf->clear();
    switch (type)
    {
    case LUA_TNIL:
        buf->append(L"nil");
        break;
    case LUA_TNUMBER:
        swprintf(dbuf, L"%d", lua_tointeger(L, i));
        buf->append(dbuf);
        break;
    case LUA_TBOOLEAN:
        swprintf(dbuf, L"%s", (lua_toboolean(L, i) == 0) ? L"false" : L"true");
        buf->append(dbuf);
        break;
    case LUA_TSTRING:
        buf->append(luaT_towstring(L, i));
        break;
    default:
        buf->append(L"[?]");
        break;
    }
}

int system_dbglog(lua_State *L)
{
    std::wstring msg;
    for (int i=1,e=lua_gettop(L); i<=e; ++i)
    {
        formatByType(L, i, &msg);
    }
    OutputDebugString(msg.c_str());
    return 0;
}

int system_sleep(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int wait = lua_tointeger(L, 1);
        if (wait > 0)
            ::Sleep(wait);
    }
    return 0;
}

static const luaL_Reg system_methods[] =
{
    { "dbgstack", system_debugstack},
    { "dbgtable", system_dbgtable },
    { "dbglog", system_dbglog },
    { "msgbox", system_messagebox },
    { "sleep", system_sleep },
    { NULL, NULL }
};

int luaopen_system(lua_State *L)
{
    luaL_newlib(L, system_methods);
    return 1;
}
