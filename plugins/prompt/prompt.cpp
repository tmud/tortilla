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
        L"с командой #drop, а также различные фильтры, которые убирают целиком строку.");
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.02");
    return 1;
}


//std::wstring last_string;
std::wstring last_prompt;
int empty_counter = 0;
/*
void saveLastString(luaT_ViewData &vd)
{
    if (vd.isLast())
        last_string.clear();
    else
    {
        int strings_count = vd.size();
        vd.select(strings_count);
        if (vd.isDropped()) 
          last_string.clear();
        else {
          std::wstring text;
          vd.getText(&text);
          last_string.append(text);
        }
    }
}

void clearLastString()
{
    last_string.clear();
}
*/
int insertEmpty(luaT_ViewData &vd)
{
    int inserted = empty_counter;
    while (empty_counter > 0)
    {
       vd.insertString();
       empty_counter--;        
    }
    last_prompt.clear();
    return inserted;
}

void clearEmpty()
{
    empty_counter = 0;
    last_prompt.clear();
}

//------------------------------------------------------------------------
void checkDoublePrompt(luaT_ViewData &vd)
{
    int strings_count = vd.size();
    if (strings_count == 0)
        return;

    std::wstring text;
    std::vector<int> not_empty;
    int dropped = 0;
    for (int i = 1; i <= strings_count; ++i)
    {
        vd.select(i);
        if (vd.isDropped())
         {
             dropped++;
             continue;
        }
        if (vd.isGameCmd())
            { not_empty.push_back(i); continue; }
        if ((i == 1 && vd.isFirst()) || 
           (i == strings_count && !vd.isLast()))
            { not_empty.push_back(i); continue; }
        vd.getText(&text);
        if (!text.empty()) 
            { not_empty.push_back(i); }
    }

    if (not_empty.empty())                // all vd strings are empty
    {
        // count empty strings
        empty_counter += (strings_count - dropped);
        // delete all strings from vd
        for (int j=strings_count; j >= 1; --j)
        {
            vd.select(j);
            vd.deleteString();
        }
        return;
    }

    // check not empty
    for (int i=0,e=not_empty.size();i<e;++i)
    {
        vd.select(not_empty[i]);
        if (vd.isPrompt())
        {
            if (last_prompt.empty())
            {
                vd.getPrompt(&last_prompt);            
            }
            

        }
    }



    /*
    for (int i = 1; i <= strings_count; ++i)
    {
        vd.select(i);
        if (vd.isDropped())
            continue;

        if (vd.isGameCmd())
        {
            if (i == 1 && vd.isFirst())
            {
              clearEmpty();
              continue;
            }
            int count = insertEmpty(vd);
            strings_count += count;
            i = i + count;
            continue;
        }       

        if (i == strings_count && !vd.isLast())
        {
            break;
        }

        if (i == 1 && vd.isFirst()) {
           vd.getText(&text);
           last_string.append(text);
           text.swap(last_string);     // text with first full string
           last_string.clear();
        } else {
           vd.getText(&text);
        }
        if (text.empty())
        {
            empty_counter++;
            vd.deleteString();
            strings_count = vd.size();
            i = i - 1;
            continue;
        }

        if (!vd.isPrompt())
        {
            int count = insertEmpty(vd);
            strings_count = vd.size();
            i = i + count;
            continue;
        }
        if (last_prompt.empty())
        {
            vd.getPrompt(&last_prompt);

            continue;
        }

        std::wstring prompt;
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
            if (!strings_count)
                break;
            i = empty[0];
            empty.clear();
        }
        else
        {
            last_prompt.clear();
            empty.clear();
        }
    }

    if (vd.isLast())
        last_string.clear();
    else
    {
        vd.select(strings_count);
        if (vd.isDropped()) 
          last_string.clear();
        else {
          vd.getText(&text);
          last_string.append(text);
        }
    }*/
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

int disconnect(lua_State *L)
{
    clearEmpty();
    //clearLastString();
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
    lua_setglobal(L, "prompt");
    return 0;
}
