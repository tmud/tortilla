#include "stdafx.h"
#include "accessors.h"
#include "pluginsApi.h"
#include "api/api.h"
extern Plugin* _cp;

void pluginFormatByType(lua_State* L, int index, tstring *buf)
{
    int i = index;
    int type = lua_type(L, i);
    tchar dbuf[32];
    buf->clear();

    switch (type)
    {
    case LUA_TNIL:
        buf->append(L"[nil]");
        break;
    case LUA_TNUMBER:
        swprintf(dbuf, L"%d", lua_tointeger(L, i));
        buf->append(dbuf);
        break;
    case LUA_TBOOLEAN:
        buf->append( (lua_toboolean(L, i) == 0) ? L"[false]" : L"[true]" );
        break;
    case LUA_TSTRING:
        buf->append(luaT_towstring(L, i));
        break;
    /*case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    case LUA_TTABLE:*/
    default:
        buf->append(L"[?]");
        break;
    }
}

PluginData& find_plugin()
{
    PluginsDataValues* pdv =  tortilla::pluginsData();
    tstring plugin_name(_cp->get(Plugin::FILE));
    int index = -1;
    for (int i = 0, e = pdv->size(); i < e; ++i)
    {
        const PluginData &p = pdv->at(i);
        if (p.name == plugin_name)
        {
            index = i; break;
        }
    }
    if (index == -1)
    {
        PluginData pd;
        pd.name = plugin_name;
        pd.state = _cp->state() ? PluginData::PDS_ON : PluginData::PDS_OFF;
        pdv->push_back(pd);
        index = pdv->size() - 1;
    }
    return pdv->at(index);
}

Pcre16 fregexp;
int string_format(lua_State *L)
{
    int n = lua_gettop(L);
    if (n == 0 || !lua_isstring(L, 1))
        return pluginInvArgs(L, L"string:format");
    tstring format(luaT_towstring(L, 1));
    if (!fregexp.valid())
        fregexp.setRegExp(L"%(?:.[0-9])?[scdiouxXeEfgGq%]", true);
    fregexp.findAllMatches(format);
    if (fregexp.getSize() == 0)
    {
        luaT_pushwstring(L, format.c_str());
        return 1;
    }
    tchar buffer[32];
    int from = 0;
    tstring result;
    for (int i = 1, e = fregexp.getSize(); i < e; ++i)
    {
        int first = fregexp.getFirst(i);
        int last = fregexp.getLast(i);
        result.append(format.substr(from, first - from));
        tstring k;
        fregexp.getString(i, &k);
        int lastsym = k.size()-1;
        tchar op = k.at(lastsym);
        if (op == L'%') { result.append(L"%"); from = last; continue; }
        int p = i + 1;
        if (p > n) continue;
        switch (op)
        {
            case L'i':
            case L'd':
            {
                swprintf(buffer, k.c_str(), lua_tointeger(L, p));
                result.append(buffer);
                break;
            }
            case L's':
            {
                result.append(luaT_towstring(L, p));
                break;
            }
            case L'u':
            case L'c':
            case L'o': 
            case L'x': 
            case L'X':
            {
                swprintf(buffer, k.c_str(), lua_tounsigned(L, p));
                result.append(buffer);
                break;
            }
            case L'f':
            case L'e':
            case L'E':
            case L'g':
            case L'G':
            {
                swprintf(buffer, k.c_str(), lua_tonumber(L, p));
                result.append(buffer);
                break;
            }
        }
        from = last;
    }
    result.append(format.substr(from));
    luaT_pushwstring(L, result.c_str());
    return 1;
}
