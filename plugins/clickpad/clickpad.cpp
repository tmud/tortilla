#include "stdafx.h"
#include "resource.h"
#include <vector>
#include "mainwnd.h"
#include "settingsDlg.h"

luaT_window m_parent_window;
HWND m_hwnd_client = NULL;
ClickpadMainWnd* m_clickpad = NULL;

int get_name(lua_State *L)
{
    lua_pushstring(L, "Игровая панель Clickpad");
    return 1;
}

int get_description(lua_State *L)
{
    lua_pushstring(L, "Данный плагин позволяет играть в мад используя мышь.\r\n"
        "С его помощью можно создать панель кнопок-горячих клавиш с нужными командами.");
    return 1;
}

int get_version(lua_State *L)
{
    lua_pushstring(L, "1.0");
    return 1;
}

int init(lua_State *L)
{
    HWND client_wnd = NULL;
    luaT_run(L, "getParent", "");
    if (lua_isnumber(L, -1))
    {
        HWND wnd = (HWND)lua_tounsigned(L, -1);
        lua_pop(L, 1);
        if (::IsWindow(wnd))
            client_wnd = wnd;
    }
    if (client_wnd)
        m_hwnd_client = client_wnd;
    else
        return luaT_error(L, "Не удалось получить доступ к главному окну клиента");

    if (!m_parent_window.create(L, "Игровая панель Clickpad", 400, 400))
        return luaT_error(L, "Не удалось создать окно для Clickpad");

    luaT_run(L, "addMenu", "sdd", "Плагины/Настройки Clickpad...", 1, 2);

    HWND parent = m_parent_window.hwnd();
    m_clickpad = new ClickpadMainWnd();
    RECT rc; ::GetClientRect(parent, &rc);
    HWND res = m_clickpad->Create(parent, rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
    m_parent_window.attach(res);
    m_parent_window.block("left,right,top,bottom");
    return 0;
}

int release(lua_State *L)
{
    if (m_clickpad)
        m_clickpad->DestroyWindow();
    delete m_clickpad;
    return 0;
}

int menucmd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    int id = lua_tointeger(L, 1);
    lua_pop(L, 1);
    if (id == 1)
        m_clickpad->switchEditMode();
    return 0;
}

static const luaL_Reg clickpad_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "release", release },
    { "menucmd", menucmd },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, clickpad_methods);
    lua_setglobal(L, "clickpad");
    return 0;
}
