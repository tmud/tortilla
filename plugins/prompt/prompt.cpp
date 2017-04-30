#include "stdafx.h"
#include <vector>

int get_name(lua_State *L)
{
    luaT_pushwstring(L, L"Фильтр строки приглашения (prompt)");
    return 1;
}

int get_description(lua_State *L)
{
    luaT_pushwstring(L, L"Фильтрует(отбрасывает) prompt-строку, если она идет подряд без изменений.\r\n"
        L"Плагин позволяет не забивать окно мада строками prompt, если используются триггеры\r\n"
        L"с командой #drop, а также другие фильтры, которые убирают целиком всю строку.\r\n"
        L"Плагин отбрасывает также и пустые строки между одинаковыми prompt.");
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.03");
    return 1;
}

std::wstring last_prompt;
void checkDoublePrompt(luaT_ViewData &vd)
{
    int strings_count = vd.size();
    if (strings_count == 0)
        return;

    std::wstring text;
    std::vector<int> todelete;

    for (int i = 1; i <= strings_count; ++i)
    {
        vd.select(i);
        if (vd.isDropped()) { continue; }
        if (vd.isGameCmd())
            { todelete.clear(); last_prompt.clear(); continue; }
        vd.getText(&text);
        if (text.empty())
        {
            if (!last_prompt.empty())
                todelete.push_back(i);
            continue;
        }
        if (vd.isPrompt())
        {
            if (last_prompt.empty())
                vd.getPrompt(&last_prompt);
            else
            {
                vd.getPrompt(&text);
                if (text == last_prompt) 
                    todelete.push_back(i);
                else
                {
                    last_prompt = text;
                    todelete.clear();
                }
            }
            continue;
        }
        todelete.clear();
        last_prompt.clear();
    }
/*
        static int k = 0;
        for (int i=1;i<=strings_count; ++i)
        {
            vd.select(i);
            vd.getText(&text);
            OutputDebugString(L"[");
            OutputDebugString(text.c_str());
            OutputDebugString(L"]");

            bool todel = false;
            for (int j=0,e=todelete.size();j<e; ++j)
            {
                if (todelete[j] == i) { todel = true; break; }
            }
            if (todel)
                 OutputDebugString(L" --- deleted");
            if (vd.isDropped())
                 OutputDebugString(L" --- dropped");
            OutputDebugString(L" \r\n");
        }
        wchar_t buffer[16];
        _itow(k++, buffer, 10);
        OutputDebugString(buffer);
        OutputDebugString(L" ############\r\n");
*/
    if (!todelete.empty())
    {
        if (todelete.size() == strings_count)
        {
            vd.deleteAllStrings();
        }
        else
        {
            for (int j = todelete.size() - 1; j >= 0; --j)
            {
                vd.select(todelete[j]);
                vd.deleteString();
            }
        }
    }
}

int afterstr(lua_State *L)
{
    if (!luaT_check(L, 2, LUA_TNUMBER, LUAT_VIEWDATA))
        return 0;
    if (lua_tointeger(L, 1) != 0) // view number, only for main window
        return 0;
    luaT_ViewData vd;
    vd.init(L, luaT_toobject(L, 2));
    checkDoublePrompt(vd);
    return 0;
}

int disconnect(lua_State *L)
{
    last_prompt.clear();
    return 0;
}

static const luaL_Reg prompt_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "after", afterstr },
    { "disconnect", disconnect },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, prompt_methods);
    return 1;
}
