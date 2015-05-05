#include "stdafx.h"
#include "resource.h"
#include <vector>
#include "mainwnd.h"
#include "settingsDlg.h"

HWND m_hwnd_client = NULL;
luaT_window m_parent_window;
ClickpadMainWnd* m_clickpad = NULL;
luaT_window m_settings_window;
SettingsDlg* m_settings = NULL;

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

    if (!m_parent_window.create(L, "Игровая панель Clickpad", 400, 100) ||
        !m_settings_window.create(L, "Настройки Clickpad", 250, 250, false))
            return luaT_error(L, "Не удалось создать окно для Clickpad");
    
    base::addMenu(L, "Плагины/Окно Clickpad...", 2, 2);
    base::checkMenu(L, 2);
    base::addMenu(L, "Плагины/Настройки Clickpad...", 1, 2);

    HWND parent = m_parent_window.hwnd();
    m_clickpad = new ClickpadMainWnd();
    RECT rc; ::GetClientRect(parent, &rc);
    HWND res = m_clickpad->Create(parent, rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
    m_parent_window.attach(res);
    m_parent_window.setBlocked(0,0);

    parent = m_settings_window.hwnd();
    m_settings = new SettingsDlg();
    res = m_settings->Create(parent); 
    m_settings->GetClientRect(&rc);

    m_settings_window.attach(res);
    m_settings_window.setBlocked(rc.right, rc.bottom);

    luaT_run(L, "getPath", "s", "buttons.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    DWORD fa = GetFileAttributes(TU2W(path.c_str()));
    if (fa != INVALID_FILE_ATTRIBUTES && fa&FILE_ATTRIBUTE_NORMAL && !(fa&FILE_ATTRIBUTE_DIRECTORY))
    {
        xml::node ld;
        if (!ld.load(path.c_str())) {
            u8string error("Ошибка загрузки списка с кнопками: ");
            error.append(path);
            luaT_log(L, error.c_str());
            m_clickpad->initDefault();
        }
        else
        {
            m_clickpad->load(ld);
        }
        ld.deletenode();
    }
    else
    {
        m_clickpad->initDefault();
    }
    return 0;
}

int release(lua_State *L)
{
    if (!m_clickpad)
        return 0;

    xml::node tosave("clickpad");
    m_clickpad->save(tosave);

    luaT_run(L, "getPath", "s", "buttons.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    bool result = true; //tosave.save(path.c_str());

    tosave.deletenode();
    m_clickpad->DestroyWindow();
    delete m_clickpad;
    m_settings->DestroyWindow();
    delete m_settings;

    if (!result)
    {
        u8string error("Ошибка записи списка кнопок: ");
        error.append(path);
        return luaT_error(L, error.c_str());
    }
    return 0;
}

int menucmd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    int id = lua_tointeger(L, 1);
    lua_pop(L, 1);
    if (id == 1)
    {
        if (!m_settings_window.isVisible())
        {
            base::checkMenu(L, 1);
            m_settings_window.show();        
        }
        else
        {
            base::uncheckMenu(L, 1);
            m_settings_window.hide();        
        }
    }
    if (id == 2)
    {
        if (!m_parent_window.isVisible())
        {
            base::checkMenu(L, 2);
            m_parent_window.show();
        }
        else
        {
            base::uncheckMenu(L, 2);
            m_parent_window.hide();
        }                        
    }
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
