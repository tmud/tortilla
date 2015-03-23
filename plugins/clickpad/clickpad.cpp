#include "stdafx.h"
#include <vector>
#include "mainwnd.h"

luaT_window m_parent_window;
ClickpadMainWnd* m_clickpad = NULL;

int get_name(lua_State *L)
{
    lua_pushstring(L, "Панель Clickpad");
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
    if (!m_parent_window.create(L, "Clickpad", 400, 400))
        return luaT_error(L, "Не удалось создать окно для Clickpad");

    HWND parent = m_parent_window.hwnd();
    m_clickpad = new ClickpadMainWnd();
    RECT rc; ::GetClientRect(parent, &rc);
    if (rc.right == 0) rc.right = 400; // requeires for splitter inside map window (if parent window hidden)
    HWND res = m_clickpad->Create(parent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    m_parent_window.attach(res);
    return 0;
}

int release(lua_State *L)
{
    if (m_clickpad)
        m_clickpad->DestroyWindow();
    delete m_clickpad;
    return 0;
}

static const luaL_Reg clickpad_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "release", release },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, clickpad_methods);
    lua_setglobal(L, "clickpad");
    return 0;
}
