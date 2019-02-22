#include "stdafx.h"
#include "pluginsApi.h"

int luapcre_create(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        Pcre *p = new Pcre();
        if (p->init(luaT_towstring(L, 1)))
            luaT_pushobject(L, p, LUAT_PCRE);
        else
        {
            delete p;
            return pluginInvArgsValues(L, L"createPcre");
        }
        return 1;
    }
    return pluginInvArgs(L, L"createPcre");
}

int luapcre_find(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_PCRE, LUA_TSTRING))
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        bool result = p->find(luaT_towstring(L, 2));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"pcre:find");
}

int luapcre_findall(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_PCRE, LUA_TSTRING))
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        bool result = p->findall(luaT_towstring(L, 2));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"pcre:findall");
}

int luapcre_size(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_PCRE))
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        lua_pushinteger(L, p->size());
        return 1;
    }
    return pluginInvArgs(L, L"pcre:size");
}

int luapcre_first(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_PCRE, LUA_TNUMBER))
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        int first = p->first(lua_tointeger(L, 2));
        if (first >= 0)
        {
            lua_pushinteger(L, first+1);
            return 1;
        }
        return pluginInvArgsValues(L, L"pcre:first");
    }
    return pluginInvArgs(L, L"pcre:first");
}

int luapcre_last(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_PCRE, LUA_TNUMBER))
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        int last = p->last(lua_tointeger(L, 2));
        if (last >= 0)
        {
            lua_pushinteger(L, last+1);
            return 1;
        }
        return pluginInvArgsValues(L, L"pcre:last");
    }
    return pluginInvArgs(L, L"pcre:last");
}

int luapcre_get(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_PCRE, LUA_TNUMBER))
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        tstring str;
        if (p->get(lua_tointeger(L, 2), &str))
        {
            luaT_pushwstring(L, str.c_str());
            return 1;
        }
        return pluginInvArgsValues(L, L"pcre:get");
    }
    return pluginInvArgs(L, L"pcre:get");
}

int luapcre_regexp(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_PCRE))
    {
        Pcre *p = (Pcre*)luaT_toobject(L, 1);
        tstring str;
        p->getRegExp(&str);
        luaT_pushwstring(L, str.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"pcre:regexp");
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

int luapcre_towatch(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_PCRE))
    {
        lua_newtable(L);
        Pcre *p = (Pcre *)luaT_toobject(L, 1);
        int size = p->size();
        tstring rgxp;
        p->getRegExp(&rgxp);
        lua_pushstring(L, "key");
        luaT_pushwstring(L, rgxp.c_str());
        lua_settable(L, -3);
        for (int i=0; i<size; ++i)
        {
            tstring str;
            p->get(i, &str);
            lua_pushinteger(L, i);
            luaT_pushwstring(L, str.c_str());
            lua_settable(L, -3);
        }
        return 1;
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
    regFunction(L, "regexp", luapcre_regexp);
    regFunction(L, "__gc", luapcre_gc);
    regFunction(L, "__towatch", luapcre_towatch);
    regIndexMt(L);
    lua_pop(L, 1);
}
