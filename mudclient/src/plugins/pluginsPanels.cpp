#include "stdafx.h"
#include "pluginsPanels.h"
#include "pluginsApi.h"

int createpanel(lua_State *L)
{



    return 0;
}

int pn_create(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
    {
    }
    return 0;

}

void reg_mt_panels(lua_State *L)
{ 
    return; // todo

    lua_register(L, "createWindow", createpanel);

    luaL_newmetatable(L, "panel");
    regFunction(L, "create", pn_create);
   
    regIndexMt(L);
    lua_pop(L, 1);
}
