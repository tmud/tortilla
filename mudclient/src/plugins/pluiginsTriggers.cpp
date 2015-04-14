#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsTriggers.h"

int ptrigger_create(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TFUNCTION))
    {    
        return 0;
    }
    return pluginInvArgs(L, "createTrigger");
}

void reg_mt_trigger(lua_State *L)
{
    lua_register(L, "createTrigger", ptrigger_create);       
}
