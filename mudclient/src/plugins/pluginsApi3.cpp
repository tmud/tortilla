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
        pd.state = _cp->state() ? 1 : 0;
        pdv->push_back(pd);
        index = pdv->size() - 1;
    }
    return pdv->at(index);
}
