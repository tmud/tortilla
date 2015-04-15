#include "stdafx.h"
#include "pluginsApi.h"

int luapcre_create(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        Pcre *p = new Pcre();
        if (p->init(lua_tostring(L, 1)))
            luaT_pushobject(L, p, LUAT_PCRE);
        else
        {
            delete p;
            lua_pushnil(L);
        }
        return 1;
    }
    return pluginInvArgs(L, "createPcre");
}

int luapcre_find(lua_State *L)
{
    if (luaT_check(L, 2), LUAT_PCRE, LUA_TSTRING)
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        bool result = p->find(lua_tostring(L, 2));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "pcre:find");
}

int luapcre_findall(lua_State *L)
{
    if (luaT_check(L, 2), LUAT_PCRE, LUA_TSTRING)
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        bool result = p->findall(lua_tostring(L, 2));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "pcre:findall");
}

int luapcre_size(lua_State *L)
{
    if (luaT_check(L, 1), LUAT_PCRE)
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        lua_pushinteger(L, p->size());
        return 1;
    }
    return pluginInvArgs(L, "pcre:size");
}

int luapcre_first(lua_State *L)
{
    if (luaT_check(L, 2), LUAT_PCRE, LUA_TNUMBER)
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        int first = p->first(lua_tointeger(L, 2));
        lua_pushinteger(L, first);
        return 1;
    }
    return pluginInvArgs(L, "pcre:first");
}

int luapcre_last(lua_State *L)
{
    if (luaT_check(L, 2), LUAT_PCRE, LUA_TNUMBER)
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        int last = p->last(lua_tointeger(L, 2));
        lua_pushinteger(L, last);
        return 1;
    }
    return pluginInvArgs(L, "pcre:last");
}

int luapcre_get(lua_State *L)
{
    if (luaT_check(L, 2), LUAT_PCRE, LUA_TNUMBER)
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        u8string str;
        p->get(lua_tointeger(L, 2), &str);
        lua_pushstring(L, str.c_str());
        return 1;
    }
    return pluginInvArgs(L, "pcre:get");
}

int luapcre_gc(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_PCRE))
    {
        Pcre *p = (Pcre *)luaT_toobject(L, 1);
        delete p;
    }
    return 0;
}

void reg_mt_pcre(lua_State *L)
{
    lua_register(L, "createPcre", luapcre_create);
    luaL_newmetatable(L, "pcre");
    regFunction(L, "find", luapcre_find);
    regFunction(L, "findall", luapcre_findall);
    regFunction(L, "size", luapcre_size);
    regFunction(L, "first", luapcre_first);
    regFunction(L, "last", luapcre_last);
    regFunction(L, "get", luapcre_get);
    regFunction(L, "__gc", luapcre_gc);
    regIndexMt(L);
    lua_pop(L, 1);
}
