#include "stdafx.h"
#include "resource.h"
#include "clickpad.h"

#include "mainwnd.h"

HWND m_hwnd_float = NULL;
HWND m_hwnd_mudclient = NULL;
luaT_window m_parent_window;
ClickpadMainWnd* m_clickpad = NULL;
luaT_window m_settings_window;
lua_State *m_pL = NULL;

int get_name(lua_State *L)
{
    lua_pushstring(L, "Игровая панель Clickpad");
    return 1;
}

int get_description(lua_State *L)
{
    lua_pushstring(L, "Данный плагин позволяет играть в мад используя мышь.\r\n"
        "С его помощью можно создать панель кнопок-горячих клавиш с нужными командами.\r\n"
        "С помощью плагина можно перести часть горячих клавиш (hotkeys) на данную панель,\r\n"
        "тем самым освободив их под другие задачи.");
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
        m_hwnd_mudclient = client_wnd;
    else
        return luaT_error(L, "Не удалось получить доступ к главному окну клиента");

    if (!m_parent_window.create(L, "Игровая панель Clickpad", 400, 100, true) ||
        !m_settings_window.create(L, "Настройки Clickpad", 250, 250, false))
        return luaT_error(L, "Не удалось создать окно для Clickpad");

    base::addMenu(L, "Плагины/Настройка Clickpad...", 1);

    HWND parent = m_parent_window.hwnd();
    m_clickpad = new ClickpadMainWnd();
    RECT rc; ::GetClientRect(parent, &rc);
    HWND res = m_clickpad->Create(parent, rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
    m_parent_window.attach(res);
    m_parent_window.setFixedSize(0,0);
    m_hwnd_float = m_parent_window.floathwnd();

    luaT_run(L, "getPath", "s", "buttons.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    DWORD fa = GetFileAttributes(TU2W(path.c_str()));
    if (fa != INVALID_FILE_ATTRIBUTES && !(fa&FILE_ATTRIBUTE_DIRECTORY))
    {
        xml::node ld;
        bool result = ld.load(path.c_str());
        if (result)
            m_clickpad->load(ld);
        if (!result) {
        u8string error("Ошибка загрузки списка с кнопками: ");
        error.append(path);
        luaT_log(L, error.c_str());
    }
    ld.deletenode();
    }

    m_pL = L;
    CWindow sd ( m_clickpad->createSettingsDlg( m_settings_window.hwnd()) );
    sd.GetClientRect(&rc);
    m_settings_window.attach(sd);
    m_settings_window.setFixedSize(rc.right, rc.bottom);

    // todo remove lines
    base::checkMenu(L, 1);
    m_settings_window.show();
    m_clickpad->setEditMode(true);    
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

    bool result = tosave.save(path.c_str());

    tosave.deletenode();
    m_clickpad->DestroyWindow();
    delete m_clickpad;

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
            if (!m_parent_window.isVisible())
                m_parent_window.show();
            m_clickpad->setEditMode(true);
        }
        else
        {
            base::uncheckMenu(L, 1);
            m_settings_window.hide();
            m_clickpad->setEditMode(false);
        }
    }  
    return 0;
}

int closewnd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    HWND hwnd = reinterpret_cast<HWND>(lua_tounsigned(L, 1));
    if (hwnd == m_parent_window.hwnd())
    {
        /*m_parent_window.hide();
        base::uncheckMenu(L, 1);
        m_settings_window.hide();*/
    }
    else if (hwnd == m_settings_window.hwnd())
    {
        base::uncheckMenu(L, 1);
        m_settings_window.hide();
        m_clickpad->setEditMode(false);
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
    { "closewindow", closewnd },
    { NULL, NULL }
};

lua_State *pL = NULL;
int WINAPI plugin_open(lua_State *L)
{
    pL = L;
    luaL_newlib(L, clickpad_methods);
    lua_setglobal(L, "clickpad");
    return 0;
}


void runGameCommand(const tstring& cmd)
{
    if (!cmd.empty())
        base::runCommand(pL, TW2U(cmd.c_str()));
}

void setFocusToMudClient()
{
    if (::IsWindow(m_hwnd_mudclient))
        ::SetFocus(m_hwnd_mudclient);
}

HWND getFloatWnd()
{
    return m_hwnd_float;
}

HWND getMudclientWnd()
{
    return m_hwnd_mudclient;
}

void exitEditMode()
{
    base::pluginName(pL, "clickpad");
    base::uncheckMenu(pL, 1);
    m_settings_window.hide();
    m_clickpad->setEditMode(false);
}

lua_State* getLuaState()
{
    return m_pL;
}