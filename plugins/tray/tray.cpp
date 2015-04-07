#include "stdafx.h"
//#include "resource.h"

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
    u8string text("Плагин добавляет в клиент команду ");
    text.append(prefix);
    text.append("tray. Данная команда выводит текстовое сообшение\r\n"
         "в системный трей Windows в виде всплывающей подсказки. Также хранит историю\r\n"
        "непрочитанных сообщений.");
    lua_pushstring(L, text.c_str());
    return 1;
}

int get_version(lua_State *L)
{
    lua_pushstring(L, "1.0");
    return 1;
}

int init(lua_State *L)
{
    base::addCommand(L, "tray");
    return 0;
}

int release(lua_State *L)
{
    return 0;
}

void show_message(const u8string& msg);
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
                if (lua_isstring(L, i))
                    text.append(lua_tostring(L, i));
                lua_pop(L, 1);
            }
            show_message(text);            
            lua_pop(L, 1);
            lua_pushnil(L);
            return 1;
        }    
    }
    return 1;
}

static const luaL_Reg tray_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "release", release },
    { "syscmd", syscmd },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, tray_methods);
    lua_setglobal(L, "tray");
    return 0;
}
