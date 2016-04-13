#include "stdafx.h"

std::map<lua_State*, int> m_cmap_types;
typedef std::map<lua_State*, int>::iterator iterator;
int gettype(lua_State *L)
{
    iterator it = m_cmap_types.find(L);
    if (it == m_cmap_types.end())
        return -1;
    return it->second;
}
void regtype(lua_State *L, int type)
{
    m_cmap_types[L] = type;
}

int cmap_add(lua_State *L)
{
    return 0;
}

int cmap_find(lua_State *L)
{
    return 0;
}

int cmap_load(lua_State *L)
{
    return 0;
}

int cmap_save(lua_State *L)
{
    return 0;
}

int cmap_new(lua_State *L)
{
    if (lua_gettop(L) != 0)
    {
        luaT_push_args(L, "new");
        return lua_error(L);
    }

    if (gettype(L) == -1)
    {
        int type = luaT_regtype(L, "cmap");
        if (!type)
            return 0;
        regtype(L, type);
        luaL_newmetatable(L, "cmap");
        regFunction(L, "add", cmap_add);
        regFunction(L, "find", cmap_find);
        regFunction(L, "load", cmap_load);
        regFunction(L, "save", cmap_save);
        /*
        regFunction(L, "clear", declension_clear);
        regFunction(L, "compare", declension_compare);
        regFunction(L, "check", declension_check);
        regFunction(L, "__gc", declension_gc);*/
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);
        lua_settable(L, -3);
        lua_pushstring(L, "__metatable");
        lua_pushstring(L, "access denied");
        lua_settable(L, -3);
        lua_pop(L, 1);
    }
    //Dictonary* nd = new Dictonary();
    //luaT_pushobject(L, nd, gettype(L));
    return 1;
}
