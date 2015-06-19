#include "stdafx.h"
#include "traymain.h"
#include "traySettings.h"
#include "sharingData.h"

TrayMainObject g_tray;
SharingManager g_sharing;

int get_name(lua_State *L)
{
    lua_pushstring(L, "Плагин оповещения в трее");
    return 1;
}

int get_description(lua_State *L)
{
    luaT_Props p(L);
    u8string prefix;
    p.cmdPrefix(&prefix);
    u8string text("Плагин добавляет в клиент команду "); text.append(prefix);
    text.append("tray. Данная команда выводит текстовое сообщение\r\n"
         "(параметры команды) в правый нижний угол рабочего стола в виде всплывающей подсказки.\r\n"
         "Если были получены новые сообщения, а клиент работал в фоновом режиме, то иконка\r\n"
         "клиента в панели задач будет переодически мигать. См. настройки в меню клиента.");
    lua_pushstring(L, text.c_str());
    return 1;
}

int get_version(lua_State *L)
{
    lua_pushstring(L, "1.0");
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

void parse_color(const u8string& text, COLORREF *color)
{
    int r = 0; int g = 0; int b = 0;
    sscanf(text.c_str(), "%d,%d,%d", &r, &g, &b);
    unsigned char rb = 0; unsigned char gb = 0; unsigned char bb = 0;
    if (check_color(r, &rb) && check_color(g, &gb) && check_color(b, &bb))
    {
        *color = RGB(rb, gb, bb);
    }
}

int init(lua_State *L)
{
    g_sharing.init();

    base::addCommand(L, "tray");
    base::addMenu(L, "Плагины/Оповещения (tray)...", 2, 1);

    g_tray.create();
    luaT_Props p(L);
    g_tray.setFont(p.currentFont());
    g_tray.setAlarmWnd(base::getParent(L));
    g_tray.setActivated(p.activated());

    luaT_run(L, "getPath", "s", "config.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    TraySettings &s = g_tray.traySettings();
    s.timeout = 5;
    s.interval = 15;
    s.showactive = 0;
    s.text = GetSysColor(COLOR_INFOTEXT);
    s.background = GetSysColor(COLOR_INFOBK);

    xml::node ld;
    if (ld.load(path.c_str()) && ld.move("params"))
    {
        if (ld.get("timeout", &s.timeout))
            check_minmax(&s.timeout, 1, 5, MAX_TIMEOUT, MAX_TIMEOUT);
        if (ld.get("interval", &s.interval))
            check_minmax(&s.interval, 5, 5, MAX_INTERVAL, MAX_INTERVAL);
        int showactive = 0;
        if (ld.get("showactive", &showactive))
            check_minmax(&showactive, 0, 0, 1, 0);
        s.showactive = showactive ? true : false;
        u8string text, bkgnd;
        if (ld.get("textcolor", &text))
            parse_color(text, &s.text);
        if (ld.get("bkgndcolor", &bkgnd))
            parse_color(bkgnd, &s.background);
        ld.move("/");
    }
    ld.deletenode();
    return 0;
}

int release(lua_State *L)
{
    TraySettings &s = g_tray.traySettings();

    xml::node sv("tray");
    sv.create("params");
    sv.set("timeout", s.timeout);
    sv.set("interval", s.interval);
    sv.set("showactive", s.showactive ? 1 : 0);
    utf8 buffer[16];
    sprintf(buffer, "%d,%d,%d", GetRValue(s.text), GetGValue(s.text), GetBValue(s.text));
    sv.set("textcolor", buffer);
    sprintf(buffer, "%d,%d,%d", GetRValue(s.background), GetGValue(s.background), GetBValue(s.background));
    sv.set("bkgndcolor", buffer);
    sv.move("/");

    luaT_run(L, "getPath", "s", "config.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    if (!sv.save(path.c_str()))
    {
        sv.deletenode();
        u8string error("Ошибка записи файла с настройками плагина : ");
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
        u8string cmd(lua_tostring(L, -1));
        lua_pop(L, 1);
        if (cmd == "tray")
        {
            int n = luaL_len(L, -1);
            u8string text;
            for (int i=2; i<=n; ++i)
            {
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                if (lua_isstring(L, -1))
                {
                    if (!text.empty())
                        text.append(" ");
                    text.append(lua_tostring(L, -1));
                }
                lua_pop(L, 1);
            }
            if (!text.empty())
                 g_tray.showMessage(text);
            lua_pop(L, 1);
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
    lua_setglobal(L, "tray");
    return 0;
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
