#include "stdafx.h"
#include "resource.h"
#include "clickpad.h"
#include "mainwnd.h"
#include "imageCollection.h"

lua_State *m_pL = NULL;
HWND m_hwnd_float = NULL;
HWND m_hwnd_mudclient = NULL;
ImageCollection *m_image_collection = NULL;
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
    luaT_pushwstring(L, L"Игровая панель Clickpad");
    return 1;
}

int get_description(lua_State *L)
{
    luaT_pushwstring(L, L"Данный плагин позволяет играть в мад используя мышь.\r\n"
        L"С его помощью можно создать панель кнопок-горячих клавиш с нужными командами.\r\n"
        L"С помощью плагина можно перести часть горячих клавиш (hotkeys) на данную панель,\r\n"
        L"тем самым освободив их под другие задачи.");
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.04");
    return 1;
}

void destroy()
{
    delete m_image_collection;
    m_image_collection = NULL;

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
        m_clickpad->DestroyWindow();
        delete m_clickpad;
        m_clickpad = NULL;
    }
}

int init(lua_State *L)
{
    m_pL = L;

    HWND client_wnd = base::getParent(L);
    if (client_wnd)
        m_hwnd_mudclient = client_wnd;
    else
        return luaT_error(L, L"Не удалось получить доступ к главному окну клиента");

    m_image_collection = new ImageCollection();
    m_image_collection->scanImages();

    xml::node ld;
    std::wstring path;
    base::getProfilePath(L, &path);
    DWORD fa = GetFileAttributes(path.c_str());
    if (fa != INVALID_FILE_ATTRIBUTES && !(fa&FILE_ATTRIBUTE_DIRECTORY))
    {
        bool result = ld.load(path.c_str());
        if (!result)
        {
            std::wstring error(L"Ошибка загрузки списка с кнопками: ");
            error.append(path);
            base::log(L, error.c_str());
        }
    }

    bool ok = false;
    if (m_parent_window.create(L, L"Игровая панель Clickpad", 400, 100, true))
    {
        HWND parent = m_parent_window.hwnd();
        m_clickpad = new ClickpadMainWnd();
        RECT rc; ::GetClientRect(parent, &rc);
        HWND res = m_clickpad->Create(parent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        m_clickpad->load(ld);
        m_parent_window.attach(res);
        m_parent_window.setFixedSize(rc.right, rc.bottom);
        m_hwnd_float = m_parent_window.floathwnd();
        ok = true;
    }
    ld.deletenode();

    if (ok && m_settings_window.create(L, L"Настройки Clickpad", 250, 250, false))
    {
        m_settings = new SettingsDlg();
        m_settings->Create(m_settings_window.hwnd());
        m_settings->setSettings(m_clickpad);
        CWindow sd(m_settings->m_hWnd);
        RECT rc; sd.GetClientRect(&rc);
        m_settings_window.attach(sd);
        m_settings_window.setFixedSize(rc.right, rc.bottom);

    } else { ok = false; }
    if (ok && m_select_image_window.create(L, L"Иконка для кнопки", 580, 500, false))
    {
        SIZE sz = m_select_image_window.getSize();
        RECT rc = { 0, 0, sz.cx, sz.cy };
        m_select_image = new SelectImageDlg();
        m_select_image->Create(m_select_image_window.hwnd(), rc);
        m_select_image->setNotify(m_settings->m_hWnd, WM_USER);
        m_select_image_window.attach(m_select_image->m_hWnd);
        m_select_image_window.block(L"left,right,top,bottom");
    } else { ok = false; }

    if (!ok) {
        destroy();
        return luaT_error(L, L"Не удалось запустить плагин Clickpad");
    }

    base::addMenu(L, L"Плагины/Игровая панель Clickpad...", 1);

#ifdef _DEBUG // open all windows for edit mode (only for debugging)
    /*base::checkMenu(L, 1);
    m_settings_window.show();
    m_select_image_window.show();
    m_clickpad->setEditMode(true);*/
    m_clickpad->setEditMode(false);
#else
    m_clickpad->setEditMode(false);
#endif
    return 0;
}

int release(lua_State *L)
{
    if (m_clickpad)
    {
        xml::node tosave(L"clickpad");
        m_clickpad->save(tosave);

        std::wstring path;
        base::getProfilePath(L, &path);

        bool result = tosave.save(path.c_str());
        tosave.deletenode();

        if (!result)
        {
            destroy();
            std::wstring error(L"Ошибка записи списка кнопок: ");
            error.append(path);
            return luaT_error(L, error.c_str());
        }
    }
    destroy();
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
            m_select_image_window.hide();
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
        m_parent_window.hide();
        base::uncheckMenu(L, 1);
        m_settings_window.hide();
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

int propsupdated(lua_State *L)
{
    if (m_settings)
        m_settings->setSettingsBlock(false);
    if (m_clickpad)
        m_clickpad->updated();
    return 0;
}

int showButtons(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
    {
        int rows = lua_tointeger(L, 1);
        int columns = lua_tointeger(L, 2);
        bool res  = false;
        if (m_clickpad)
            res = m_clickpad->showRowsColumns(rows, columns);
        lua_pushboolean(L, res ? 1 : 0);
        return 1;
    }
    return 0;
}

bool readButton(lua_State *L, int index, ButtonParams* p)
{
    if (!lua_istable(L, index))
        return false;
    lua_pushstring(L, "text");
    lua_gettable(L, index);
    if (lua_isstring(L, -1))
    {
        p->text = luaT_towstring(L, -1);
        p->update |= ButtonParams::TEXT;
    }
    lua_pop(L, 1);

    lua_pushstring(L, "command");
    lua_gettable(L, index);
    if (lua_isstring(L, -1))
    {
        p->cmd = luaT_towstring(L, -1);
        p->update |= ButtonParams::CMD;
    }
    lua_pop(L, 1);

    lua_pushstring(L, "tooltip");
    lua_gettable(L, index);
    if (lua_isstring(L, -1))
    {
        p->tooltip = luaT_towstring(L, -1);
        p->update |= ButtonParams::TOOLTIP;
    }
    lua_pop(L, 1);

    lua_pushstring(L, "image");
    lua_gettable(L, index);
    if (lua_isstring(L, -1))
    {
        p->imagefile = luaT_towstring(L, -1);
        p->update |= ButtonParams::IMAGE;
    }
    lua_pop(L, 1);

    bool imagex = false;
    lua_pushstring(L, "imagex");
    lua_gettable(L, index);
    if (lua_isnumber(L, -1))
    {
        p->imagex = lua_tointeger(L, -1);
        imagex = true;
    }
    lua_pop(L, 1);

    lua_pushstring(L, "imagey");
    lua_gettable(L, index);
    if (lua_isnumber(L, -1) && imagex)
    {
        p->imagey = lua_tointeger(L, -1);
        p->update |= ButtonParams::IMAGEXY;
    }
    else {
        p->imagex = 0;
        p->imagey = 0;
    }
    lua_pop(L, 1);

    lua_pushstring(L, "template");
    lua_gettable(L, index);
    if (lua_isnumber(L, -1) || lua_isboolean(L, -1))
    {
        bool templ = false;
        if (lua_isboolean(L, -1))
            templ = lua_toboolean(L, -1) ? true : false;
        else
        {
            int flag = lua_tointeger(L, -1);
            templ = (flag == 0) ? false : true;
        }
        p->templ = templ;
        p->update |= ButtonParams::TEMPLATE;
    }
    lua_pop(L, 1);
    return true;
}

bool writeButton(lua_State *L, int index, const ButtonParams& p)
{
    if (p.update & ButtonParams::TEXT)
    {
        lua_pushstring(L, "text");
        luaT_pushwstring(L, p.text.c_str());
        lua_settable(L, index);
    }
    if (p.update & ButtonParams::CMD)
    {
        lua_pushstring(L, "command");
        luaT_pushwstring(L, p.cmd.c_str());
        lua_settable(L, index);
    }
    if (p.update & ButtonParams::TOOLTIP)
    {
        lua_pushstring(L, "tooltip");
        luaT_pushwstring(L, p.tooltip.c_str());
        lua_settable(L, index);
    }
    if (p.update & ButtonParams::IMAGE)
    {
        lua_pushstring(L, "image");
        luaT_pushwstring(L, p.imagefile.c_str());
        lua_settable(L, index);
        if (p.update & ButtonParams::IMAGEXY)
        {
            lua_pushstring(L, "imagex");
            lua_pushinteger(L, p.imagex);
            lua_settable(L, index);
            lua_pushstring(L, "imagey");
            lua_pushinteger(L, p.imagey);
            lua_settable(L, index);
        }
    }
    if (p.update & ButtonParams::TEMPLATE)
    {
        lua_pushstring(L, "template");
        lua_pushboolean(L, p.templ);
        lua_settable(L, index);
    }
    return true;
}

int setButton(lua_State *L)
{
    if (luaT_check(L, 3, LUA_TNUMBER, LUA_TNUMBER, LUA_TTABLE))
    {
        int row = lua_tointeger(L, 1);
        int column = lua_tointeger(L, 2);
        bool res = false;
        ButtonParams p;
        if (m_clickpad && readButton(L, 3, &p))
            res = m_clickpad->setButton(row, column, p);
        lua_pushboolean(L, res ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 3, LUA_TNUMBER, LUA_TNUMBER, LUA_TNIL))
    {
        int row = lua_tointeger(L, 1);
        int column = lua_tointeger(L, 2);
        bool res = false;
        if (m_clickpad)
            res = m_clickpad->clearButton(row, column);
        lua_pushboolean(L, res ? 1 : 0);
    }
    return 0;
}

int getButton(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
    {
        int row = lua_tointeger(L, 1);
        int column = lua_tointeger(L, 2);
        bool res = false;
        ButtonParams p;
        if (m_clickpad)
        {
            res = m_clickpad->getButton(row, column, &p);
        }
        if (res)
        {
            lua_newtable(L);
            res = writeButton(L, lua_gettop(L), p);
        }
        if (!res)
            lua_pushnil(L);
        return 1;
    }
    return 0;
}

int updateButton(lua_State *L)
{
    if (luaT_check(L, 3, LUA_TNUMBER, LUA_TNUMBER, LUA_TTABLE))
    {
        int row = lua_tointeger(L, 1);
        int column = lua_tointeger(L, 2);
        bool res = false;
        ButtonParams p;
        if (m_clickpad && readButton(L, 3, &p))
            res = m_clickpad->updateButton(row, column, p);
        lua_pushboolean(L, res ? 1 : 0);
        return 1;
    }
    return 0;
}

int clearButton(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
    {
        int row = lua_tointeger(L, 1);
        int column = lua_tointeger(L, 2);
        bool res = false;
        if (m_clickpad)
            res = m_clickpad->clearButton(row, column);
        lua_pushboolean(L, res ? 1 : 0);
        return 1;
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
    { "propsblocked", propsblocked },
    { "propsupdated", propsupdated },

    { "show", showButtons },
    { "set", setButton },
    { "get", getButton },
    { "update", updateButton },
    { "clear", clearButton },
    { NULL, NULL }
};

lua_State *pL = NULL;
int WINAPI plugin_open(lua_State *L)
{
    pL = L;
    luaL_newlib(L, clickpad_methods);
    return 1;
}

void processGameCommand(const std::wstring& cmd, bool template_cmd)
{
    if (cmd.empty())
        return;
    if (!template_cmd)
        base::runCommand(pL, cmd.c_str());
    else
        base::setCommand(pL, cmd.c_str());
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
    base::pluginName(pL, L"clickpad");
    HWND wnd = base::getParent(pL);
    ::SetFocus(wnd);
    base::pluginName(pL, L"clickpad");
    base::uncheckMenu(pL, 1);
    m_settings_window.hide();
    m_select_image_window.hide();
    m_clickpad->setEditMode(false);
}

bool onlyNumbers(const std::wstring& str)
{
    return (wcsspn(str.c_str(), L"0123456789") != str.length()) ? false : true;
}

bool s2i(const std::wstring& number, int *value)
{
    if (!onlyNumbers(number))
        return false;
    *value = _wtoi(number.c_str());
    return true;
}

void getImagesDir(std::wstring* dir)
{
    base::pluginName(getLuaState(), L"clickpad");
    std::wstring p; 
    base::getResource(getLuaState(), L"", &p);
    dir->assign(p);
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
