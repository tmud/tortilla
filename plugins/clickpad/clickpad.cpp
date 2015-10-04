#include "stdafx.h"
#include "resource.h"
#include "clickpad.h"
#include "mainwnd.h"

lua_State *m_pL = NULL;
HWND m_hwnd_float = NULL;
HWND m_hwnd_mudclient = NULL;
luaT_window m_parent_window;
ClickpadMainWnd* m_clickpad = NULL;
luaT_window m_settings_window;
SettingsDlg* m_settings = NULL;
luaT_window m_select_image_window;
SelectImageDlg* m_select_image = NULL;

lua_State* getLuaState()
{
    return m_pL;
}

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
    m_pL = L;

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

    bool ok = false;
    if (m_parent_window.create(L, "Игровая панель Clickpad", 400, 100, true))
    {
        HWND parent = m_parent_window.hwnd();
        m_clickpad = new ClickpadMainWnd();
        RECT rc; ::GetClientRect(parent, &rc);
        HWND res = m_clickpad->Create(parent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        m_parent_window.attach(res);
        m_parent_window.setFixedSize(rc.right, rc.bottom);
        m_hwnd_float = m_parent_window.floathwnd();
        ok = true;
    }
    if (ok && m_settings_window.create(L, "Настройки Clickpad", 250, 250, false))
    {
        m_settings = new SettingsDlg();
        m_settings->Create(m_settings_window.hwnd());
        m_settings->setSettings(m_clickpad);
        CWindow sd(m_settings->m_hWnd);
        RECT rc; sd.GetClientRect(&rc);
        m_settings_window.attach(sd);
        m_settings_window.setFixedSize(rc.right, rc.bottom);
    
    } else { ok = false; }
    if (ok && m_select_image_window.create(L, "Иконка для кнопки", 580, 500, false))
    {
        SIZE sz = m_select_image_window.getSize();
        RECT rc = { 0, 0, sz.cx, sz.cy };
        m_select_image = new SelectImageDlg();
        m_select_image->Create(m_select_image_window.hwnd(), rc);
        m_select_image->setNotify(m_settings->m_hWnd, WM_USER);
        m_select_image_window.attach(m_select_image->m_hWnd);
        m_select_image_window.block("left,right,top,bottom");
    } else { ok = false; }

    if (!ok)
        return luaT_error(L, "Не удалось создать окно для Clickpad");

    base::addMenu(L, "Плагины/Игровая панель Clickpad...", 1);

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
       if (!result) 
       {
          u8string error("Ошибка загрузки списка с кнопками: ");
          error.append(path);
          luaT_log(L, error.c_str());
       }
       ld.deletenode();
    }

    // todo remove lines
    base::checkMenu(L, 1);
    m_settings_window.show();
    m_select_image_window.show();
    m_clickpad->setEditMode(true);    
    return 0;
}

int release(lua_State *L)
{
    if (m_select_image)
    {
        m_select_image->DestroyWindow();
        delete m_select_image;
        m_select_image = NULL;    
    }

    if (m_settings)
    {
        m_settings->DestroyWindow();
        delete m_settings;
        m_settings = NULL;
    }

    if (m_clickpad)
    {
        xml::node tosave("clickpad");
        m_clickpad->save(tosave);

        u8string path;
        base::getProfilePath(L, &path);

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
        m_select_image_window.hide();
        m_settings_window.hide();
        m_clickpad->setEditMode(false);
    }
    else if (hwnd == m_select_image_window.hwnd())
    {
        m_select_image_window.hide();
    }
    return 0;
}

int propsblocked(lua_State *L)
{
    if (m_settings)
        m_settings->setSettingsBlock(true);
    return 0;
}

int propsunblocked(lua_State *L)
{
    if (m_settings)
        m_settings->setSettingsBlock(false);
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
    { "propsblocked", propsblocked },
    { "propsunblocked", propsunblocked },
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

void processGameCommand(const tstring& cmd, bool template_cmd)
{
    if (cmd.empty())
        return;
    TW2U c(cmd.c_str());
    if (!template_cmd)
        base::runCommand(pL, c);
    else
        base::setCommand(pL, c);
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

CAppModule _Module;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        _Module.Init(NULL, hModule);
        break;
    case DLL_PROCESS_DETACH:
        _Module.Term();
        break;
    }
    return TRUE;
}
