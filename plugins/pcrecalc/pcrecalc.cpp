#include "stdafx.h"
#include "calcWindow.h"
luaT_window m_parent_window;
CalcWindowDlg m_calc_window;

LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return m_calc_window.HookGetMsgProc(nCode, wParam, lParam);
}

int get_name(lua_State *L)
{
    luaT_pushwstring(L, L"Калькулятор PCRE");
    return 1;
}

int get_description(lua_State *L)
{
    luaT_pushwstring(L, L"Плагин предназначен для написания и проверки регулярных выражений для триггеров.\r\n"
        L"Регулярное выражение применяется к тестовой строке и выводится результат.\r\n"
        L"Пересчет происходит на каждое изменение регулярного выражения или тестовой строки.");
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.02");
    return 1;
}

int init(lua_State *L)
{
    if (m_parent_window.create(L, L"Калькулятор PCRE", 200, 300))
    {
        m_calc_window.Create(m_parent_window.hwnd());
        CWindow sd(m_calc_window.m_hWnd);
        RECT rc; sd.GetClientRect(&rc);
        m_parent_window.attach(sd);
        m_parent_window.setFixedSize(rc.right, rc.bottom);
        base::addMenu(L, L"Плагины/Калькулятор PCRE...", 1);

        if (m_parent_window.isVisible())
        {
            base::checkMenu(L, 1);
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
        if (!m_parent_window.isVisible())
        {
            base::checkMenu(L, 1);
            m_parent_window.show();
        }
        else
        {
            base::uncheckMenu(L, 1);
            m_parent_window.hide();
        }
    }
    return 0;
}

int closewindow(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    HWND hwnd = reinterpret_cast<HWND>(lua_tounsigned(L, 1));
    if (hwnd == m_parent_window.hwnd())
    {
        m_parent_window.hide();
        base::uncheckMenu(L, 1);
    }
    return 0;
}

static const luaL_Reg pcrecalc_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "menucmd", menucmd },
    { "closewindow", closewindow },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, pcrecalc_methods);
    return 1;
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