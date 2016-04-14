#include "stdafx.h"

std::map<lua_State*, int> m_dict_types;
typedef std::map<lua_State*, int>::iterator iterator;
int get_dict(lua_State *L)
{
    iterator it = m_dict_types.find(L);
    if (it == m_dict_types.end())
        return -1;
    return it->second;
}
void regtype_dict(lua_State *L, int type)
{
    m_dict_types[L] = type;
}

int dict_invalidargs(lua_State *L, const char* function_name)
{
    luaT_push_args(L, function_name);
    return lua_error(L);
}

typedef std::vector<tstring> info;

class Dictonary 
{
public:
    Dictonary();
    ~Dictonary();
    void add(const tstring& name, info *obj) 
    {
        
    }
private:
    std::map<tstring, info*> m_dictonary;
};

int dict_add(lua_State *L)
{
    if (luaT_check(L, 3, get_dict(L), LUA_TSTRING, LUA_TTABLE))
    {
        Dictonary *d = (Dictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        std::vector<tstring> info;
        lua_pushnil(L);                     // first key
        while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
        {
            if (!lua_isstring(L, -1))
                dict_invalidargs(L, "add");
            tstring line(luaT_towstring(L, -1));
            info.push_back(line);
            lua_pop(L, 1);
        }
        return 0;
    }
    return dict_invalidargs(L, "add");
}

int dict_find(lua_State *L)
{
    return 0;
}

int dict_load(lua_State *L)
{
    return 0;
}

int dict_save(lua_State *L)
{
    return 0;
}

int dict_gc(lua_State *L)
{
    return 0;
}

int dict_new(lua_State *L)
{
    if (lua_gettop(L) != 0)
    {
        luaT_push_args(L, "new");
        return lua_error(L);
    }
    if (get_dict(L) == -1)
    {
        int type = luaT_regtype(L, "dictonary");
        if (!type)
            return 0;
        regtype_dict(L, type);
        luaL_newmetatable(L, "dictonary");
        regFunction(L, "add", dict_add);
        regFunction(L, "find", dict_find);
        regFunction(L, "load", dict_load);
        regFunction(L, "save", dict_save);
        regFunction(L, "__gc", dict_gc);
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);
        lua_settable(L, -3);
        lua_pushstring(L, "__metatable");
        lua_pushstring(L, "access denied");
        lua_settable(L, -3);
        lua_pop(L, 1);
    }
    Dictonary* nd = new Dictonary();
    luaT_pushobject(L, nd, get_dict(L));
    return 1;
}
