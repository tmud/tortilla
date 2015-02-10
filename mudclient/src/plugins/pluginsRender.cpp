#include "stdafx.h"
#include "pluginsPanels.h"
#include "pluginsApi.h"
#include "pluginSupport.h"
#include "pluginsView.h"
#include "MainFrm.h"

int render_setbkgnd(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_RENDER, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);        
        COLORREF color = RGB(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
        v->setBackground(color);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TNUMBER))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        COLORREF color = lua_tounsigned(L, 2);
        v->setBackground(color);
        return 0;
    }
    return pluginInvArgs(L, "render.setbackground");
}

void reg_mt_render(lua_State *L)
{
    luaL_newmetatable(L, "render");
    regFunction(L, "setbackground", render_setbkgnd);
    regIndexMt(L);
    lua_pop(L, 1);
}
