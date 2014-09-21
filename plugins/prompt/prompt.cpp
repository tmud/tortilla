#include "stdafx.h"
#include "resource.h"
#include "settingsDlg.h"
#include <vector>

u8string m_regexp;
Pcre m_pcre;

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
    lua_pushstring(L, "1.0");
    return 1;
}

int init(lua_State *L)
{
    luaT_run(L, "addMenu", "sdd", "Плагины/Фильтр prompt...", 1, 2);    

    luaT_run(L, "getPath", "s", "config.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    m_regexp.clear();
    xml::node ld;
    if (ld.load(path.c_str()))
    {
        ld.get("pcre/value", &m_regexp);
    }
    ld.deletenode();
    m_pcre.init(m_regexp.c_str());
    return 0;
}

int release(lua_State *L)
{
    xml::node s("prompt");
    s.set("pcre/value", m_regexp.c_str());
    luaT_run(L, "getPath", "s", "config.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    if (!s.save(path.c_str()))
        return luaT_error(L, "Ошибка записи настроек плагина prompt: config.xml");
    s.deletenode();
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
        std::wstring param( convert_utf8_to_wide(m_regexp.c_str()) );
        SettingsDlg dlg(param);
        if (dlg.DoModal() == IDOK)
        {
            m_regexp.assign( convert_wide_to_utf8(dlg.getRegexp()) );
            m_pcre.init(m_regexp.c_str());
        }
    }
    return 0;
}

bool recognizePrompt(u8string &prompt)
{
    if (prompt.find(">") == u8string::npos)
        return false;
    return true;
}

u8string last_prompt;
void checkDoublePrompt(luaT_ViewData &vd)
{
    int strings_count = vd.size();
    if (strings_count == 0) return;

    std::vector<int> empty;
    u8string text;
    for (int i = 0; i < strings_count; ++i)
    {
        vd.select(i);
        if (i == 0 && vd.isfirst())
        {
            last_prompt.clear();
            empty.clear();
        }
        if (vd.isgamecmd())
        {            
            last_prompt.clear();
            empty.clear();
            continue; 
        }
        vd.gettext(&text);
        if (text.empty())
        {
            empty.push_back(i);
            continue;
        }
        if (!recognizePrompt(text))
        {
            last_prompt.clear();
            empty.clear();
            continue;
        }
        if (last_prompt.empty())
        {
            vd.gethash(&last_prompt);
            continue;
        }
        u8string hash;
        vd.gethash(&hash);
        if (hash == last_prompt)
        {
            empty.push_back(i);
            for (int j = empty.size() - 1; j >= 0; --j)
            {
                vd.select(empty[j]);
                vd.deletestring();
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
    int view = lua_tointeger(L, 1);
    if (view != 0)
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
    { "init", init },
    { "release", release },
    { "menucmd", menucmd },
    { "after", afterstr },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, prompt_methods);
    lua_setglobal(L, "prompt");
    return 0;
}
