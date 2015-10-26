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

int luaimage_cut(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_IMAGE, LUA_TTABLE))
    {
        ParametersReader pr(L);
        RECT rc;
        if (pr.getrect(&rc))
        {
            Image *img = (Image *)luaT_toobject(L, 1);           
            Image *new_img = new Image();
            if (new_img->cut(*img, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top))
            {
                luaT_pushobject(L, new_img, LUAT_IMAGE);
                return 1;
            }
            delete new_img;
            lua_pushnil(L);
            return 1;
        }
    }
    return pluginInvArgs(L, "image:cut");
}

void reg_mt_image(lua_State *L)
{
    luaL_newmetatable(L, "image");
    regFunction(L, "width", luaimage_width);
    regFunction(L, "height", luaimage_height);
    regFunction(L, "cut", luaimage_cut);
    regIndexMt(L);
    lua_pop(L, 1);
}
