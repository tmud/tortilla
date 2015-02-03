#pragma  once
#include "api/api.h"
#include "plugin.h"

class luaT_towstring
{
    Utf8ToWide value;
public:
    luaT_towstring(lua_State *L, int index)
    {
        value.convert(lua_tostring(L, index));
    }
    operator const wchar_t*() const
    {
        return value;
    }
};

class luaT_pushwstring
{
    WideToUtf8 value;
public:
    luaT_pushwstring(lua_State *L, const wchar_t* string)
    {
        value.convert(string, -1);
        lua_pushstring(L, value);
    }
};

class PluginsIdTableControl
{
public:
    PluginsIdTableControl(int firstid, int lastid);
    UINT registerPlugin(Plugin*p, int code, bool button);
    UINT unregisterByCode(Plugin*p, int code, bool button);
    void unregisterById(Plugin*p, UINT id);
    UINT findId(Plugin*p, int code, bool button);
    void runPluginCmd(UINT id);   

private:
    int  getIndex(Plugin*p, int code, bool button);
    struct idplugin { int code; Plugin *plugin; bool button;  };
    std::vector<idplugin> plugins_id_table;
    int first_id;
    int last_id;
};

