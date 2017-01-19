#include "stdafx.h"
#include "traymain.h"
#include "traySettings.h"

TrayMainObject g_tray;

int get_name(lua_State *L)
{
    luaT_pushwstring(L, L"Плагин оповещения в трее");
    return 1;
}

int get_description(lua_State *L)
{
    luaT_Props p(L);
    std::wstring prefix;
    p.cmdPrefix(&prefix);
    std::wstring text(L"Плагин добавляет в клиент команду ");
    text.append(prefix);
    text.append(L"tray. Данная команда выводит текстовое сообщение\r\n"
         L"(параметры команды) в правый нижний угол рабочего стола в виде всплывающей подсказки.\r\n"
         L"Если были получены новые сообщения, а клиент работал в фоновом режиме, то иконка\r\n"
         L"клиента в панели задач будет переодически мигать. См. настройки в меню клиента.");
    luaT_pushwstring(L, text.c_str());
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.12");
    return 1;
}

void check_minmax(int *val, int min, int setmin, int max, int setmax)
{
    if (*val < min) *val = setmin;
    else if (*val > max) *val = setmax;
}

bool check_color(int color, unsigned char *c)
{
    if (color < 0 || color > 255) return false;
    *c = (unsigned char)color;
    return true;
}

void parse_color(const std::wstring& text, COLORREF *color)
{
    int r = 0; int g = 0; int b = 0;
    swscanf(text.c_str(), L"%d,%d,%d", &r, &g, &b);
    unsigned char rb = 0; unsigned char gb = 0; unsigned char bb = 0;
    if (check_color(r, &rb) && check_color(g, &gb) && check_color(b, &bb))
    {
        *color = RGB(rb, gb, bb);
    }
}

int init(lua_State *L)
{
    if (!g_tray.create())
    {
        base::log(L, L"Критическая ошибка.");
        base::terminate(L);
        return 0;
    }

    base::addCommand(L, L"tray");
    base::addMenu(L, L"Плагины/Оповещения (tray)...", 1);

    luaT_Props p(L);
    g_tray.setFont(p.currentFont());
    g_tray.setAlarmWnd(base::getParent(L));
    g_tray.setActivated(p.activated());

    std::wstring path;
    base::getPath(L, L"config.xml", &path);

    TraySettings &s = g_tray.traySettings();
    s.timeout = 5;
    s.interval = 15;
    s.showactive = 1;
    s.text = GetSysColor(COLOR_INFOTEXT);
    s.background = GetSysColor(COLOR_INFOBK);

    xml::node ld;
    if (ld.load(path.c_str()) && ld.move(L"params"))
    {
        if (ld.get(L"timeout", &s.timeout))
            check_minmax(&s.timeout, 1, 5, MAX_TIMEOUT, MAX_TIMEOUT);
        if (ld.get(L"interval", &s.interval))
            check_minmax(&s.interval, 5, 5, MAX_INTERVAL, MAX_INTERVAL);
        int showactive = 1;
        if (ld.get(L"showactive", &showactive))
            check_minmax(&showactive, 0, 0, 1, 0);
        s.showactive = showactive ? true : false;
        std::wstring text, bkgnd;
        if (ld.get(L"textcolor", &text))
            parse_color(text, &s.text);
        if (ld.get(L"bkgndcolor", &bkgnd))
            parse_color(bkgnd, &s.background);
        ld.move(L"/");
    }
    ld.deletenode();
    return 0;
}

int release(lua_State *L)
{
    TraySettings &s = g_tray.traySettings();

    xml::node sv(L"tray");
    sv.create(L"params");
    sv.set(L"timeout", s.timeout);
    sv.set(L"interval", s.interval);
    sv.set(L"showactive", s.showactive ? 1 : 0);
    wchar_t buffer[16];
    swprintf(buffer, L"%d,%d,%d", GetRValue(s.text), GetGValue(s.text), GetBValue(s.text));
    sv.set(L"textcolor", buffer);
    swprintf(buffer, L"%d,%d,%d", GetRValue(s.background), GetGValue(s.background), GetBValue(s.background));
    sv.set(L"bkgndcolor", buffer);
    sv.move(L"/");

    std::wstring path;
    base::getPath(L, L"config.xml", &path);   
    if (!sv.save(path.c_str()))
    {
        sv.deletenode();
        std::wstring error(L"Ошибка записи файла с настройками плагина : ");
        error.append(path);
        return luaT_error(L, error.c_str());
    }
    sv.deletenode();
    if (g_tray.IsWindow())
        g_tray.DestroyWindow();
    return 0;
}

int menucmd(lua_State *L)
{
    TraySettingsDlg dlg;
    dlg.settings = g_tray.traySettings();
    if (dlg.DoModal() == IDOK)
        g_tray.traySettings() = dlg.settings;
    return 0;
}

int syscmd(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TTABLE))
    {
        lua_pushinteger(L, 1);
        lua_gettable(L, -2);
        std::wstring cmd(luaT_towstring(L, -1));
        lua_pop(L, 1);
        if (cmd == L"tray")
        {
            int n = luaL_len(L, -1);
            std::deque<std::wstring> text;
            for (int i=2; i<=n; ++i)
            {
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                if (lua_isstring(L, -1))
                {
                    std::wstring tmp(luaT_towstring(L, -1));
                    text.push_back(tmp);
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
            if (!text.empty())
            {
                Msg m;
                COLORREF text_color = 0; COLORREF bgnd_color = 0;
                if (base::translateColors(L, text[0].c_str(), &text_color, &bgnd_color))
                {
                    m.textcolor = text_color;
                    m.bkgndcolor = bgnd_color;
                }
                else
                {
                    m.textcolor =  g_tray.traySettings().text;
                    m.bkgndcolor = g_tray.traySettings().background;
                }
                text.pop_front();
                for (int i=0,e=text.size();i<e;++i) {
                   if (i!=0) m.text.append(L" ");
                   m.text.append(text[i]);
                }
                g_tray.addMessage(m);
            }
            lua_pushnil(L);
            return 1;
        }
    }
    return 1;
}

int activated(lua_State *L)
{
    g_tray.setActivated(true);
    return 0;
}

int deactivated(lua_State *L)
{
    g_tray.setActivated(false);
    return 0;
}

static const luaL_Reg tray_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "release", release },
    { "menucmd", menucmd },
    { "syscmd", syscmd },
    { "activated", activated },
    { "deactivated", deactivated },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, tray_methods);
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
