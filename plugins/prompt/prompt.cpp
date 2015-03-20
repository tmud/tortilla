#include "stdafx.h"
#include <vector>

int get_name(lua_State *L)
{
    lua_pushstring(L, "Фильтр строки приглашения (prompt)");
    return 1;
}

int get_description(lua_State *L)
{
    lua_pushstring(L, "Фильтрует(отбрасывает) prompt-строку, если она идет подряд без изменений.\r\nПлагин позволяет не забивать окно мада строками prompt, если используются\r\nтриггеры с командой #drop.");
    return 1;
}

int get_version(lua_State *L)
{
    lua_pushstring(L, "-");
    return 1;
}

u8string last_prompt;
void checkDoublePrompt(luaT_ViewData &vd)
{
    int strings_count = vd.size();
    if (strings_count == 0) return;

    std::vector<int> empty;
    u8string text;
    for (int i = 1; i <= strings_count; ++i)
    {
        vd.select(i);
        if (vd.isGameCmd())
        {
            last_prompt.clear();
            empty.clear();
            continue; 
        }
        vd.getText(&text);
        if (text.empty())
        {
            empty.push_back(i);
            continue;
        }
        if (!vd.isPrompt())
        {
            last_prompt.clear();
            empty.clear();
            continue;
        }
        if (last_prompt.empty())
        {
            vd.getPrompt(&last_prompt);
            continue;
        }
        u8string prompt;
        vd.getPrompt(&prompt);
        if (prompt == last_prompt)
        {
            empty.push_back(i);
            for (int j = empty.size() - 1; j >= 0; --j)
            {
                vd.select(empty[j]);
                vd.deleteString();
            }
            strings_count = vd.size();
            i = empty[0];
            empty.clear();
        }
        else
        {
            last_prompt.clear();
            empty.clear();
        }
    }
}

int afterstr(lua_State *L)
{
    if (!luaT_check(L, 2, LUA_TNUMBER, LUAT_VIEWDATA))
        return 0;
    if (lua_tointeger(L, 1) != 0) // view number
        return 0;
    luaT_ViewData vd;
    vd.init(L, luaT_toobject(L, 2));
    checkDoublePrompt(vd);
    return 0;
}

static const luaL_Reg prompt_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "after", afterstr },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, prompt_methods);
    lua_setglobal(L, "prompt");
    return 0;
}
