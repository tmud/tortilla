#include "stdafx.h"
#include "jmc3ImportImpl.h"

int get_name(lua_State *L)
{
    luaT_pushwstring(L, L"Импорт из JMC3");
    return 1;
}

int get_description(lua_State *L)
{
    luaT_pushwstring(L, L"Позволяет импортировать триггеры, макросы и другие игровые элементы\r\nиз игровых сетов мад-клиента Jaba Mud Client 3.x");
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.11");
    return 1;
}

int init(lua_State *L)
{
    base::addMenu(L, L"Плагины/Импорт из JMC3...", 1);
    return 0;
}

int menucmd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    int menuid = lua_tointeger(L, 1);
    lua_pop(L, 1);
    if (menuid == 1)
    {
        HWND parent = base::getParent(L);
        std::vector<std::wstring> errors;
        Jmc3Import jmc3(L);
        if (jmc3.import(parent, &errors))
        {
            if (!errors.empty())
            {
                base::log(L, L"Ошибки импорта из JMC3 (неверный синтаксис или такой элемент уже есть):");
                for (int i = 0, e = errors.size(); i < e; ++i)
                {
                    std::wstring msg(L"Ошибка: ");
                    msg.append(errors[i].c_str());
                    base::log(L, msg.c_str());
                }
            }
            else
            {
                base::log(L, L"Импорт прошел без ошибок.");
            }
        }
    }
    return 0;
}

static const luaL_Reg jmc3_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "menucmd", menucmd },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, jmc3_methods);
    return 1;
}
