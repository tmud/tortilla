#include "stdafx.h"
#include "pluginsApi.h"
#include "api/api.h"
#include "../MainFrm.h"
extern CMainFrame _wndMain;
extern PropertiesData* _pdata;
extern Plugin* _cp;

void pluginFormatByType(lua_State* L, int index, u8string *buf)
{
    int i = index;
    int type = lua_type(L, i);
    utf8 dbuf[32];
    buf->clear();

    switch (type)
    {
    case LUA_TNIL:
        buf->append("[nil]");
        break;
    case LUA_TNUMBER:
        sprintf(dbuf, "%d", lua_tointeger(L, i));
        buf->append(dbuf);
        break;
    case LUA_TBOOLEAN:
        buf->append( (lua_toboolean(L, i) == 0) ? "[false]" : "[true]" );
        break;
    case LUA_TSTRING:
        buf->append(lua_tostring(L, i));
        break;
    /*case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    case LUA_TTABLE:*/
    default:
        buf->append("[?]");
        break;
    }
}

PluginData& find_plugin()
{
    tstring plugin_name(_cp->get(Plugin::FILE));
    int index = -1;
    for (int i = 0, e = _pdata->plugins.size(); i < e; ++i)
    {
        const PluginData &p = _pdata->plugins[i];
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
        _pdata->plugins.push_back(pd);
        index = _pdata->plugins.size() - 1;
    }
    return _pdata->plugins[index];
}
