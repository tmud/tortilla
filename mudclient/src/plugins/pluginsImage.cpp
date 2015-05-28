#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginSupport.h"

int luaimage_width(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_IMAGE))
    {
        Image *p = (Image *)luaT_toobject(L, 1);
        lua_pushinteger(L, p->width());
        return 1;
    }
    return pluginInvArgs(L, "image:width");
}

int luaimage_height(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_IMAGE))
    {
        Image *p = (Image *)luaT_toobject(L, 1);
        lua_pushinteger(L, p->height());
        return 1;
    }
    return pluginInvArgs(L, "image:height");
}

void reg_mt_image(lua_State *L)
{
    luaL_newmetatable(L, "image");
    regFunction(L, "width", luaimage_width);
    regFunction(L, "height", luaimage_height);
    regIndexMt(L);
    lua_pop(L, 1);
}
