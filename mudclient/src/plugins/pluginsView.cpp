#include "stdafx.h"
#include "pluginsView.h"

void reg_pview_render_table(lua_State *L)
{
    lua_newtable(L);
    lua_setglobal(L, "pvrender");
}

int reg_pview_render(lua_State* L)
{
    if (!lua_isfunction(L, -1))
        return 0;
    lua_getglobal(L, "pvrender");
    if (!lua_istable(L, -1))
        return 0;
    lua_len(L, -1);    
    int index = lua_tointeger(L, -1) + 1;
    lua_pop(L, 1);
    lua_insert(L, -2);
    lua_pushinteger(L, index);
    lua_insert(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1);
    return index;
}

void PluginsViewRender::render(HDC dc, int width, int height)
{
    m_width = width;
    m_height = height;
    m_dc = dc;
    m_dc.FillSolidRect(0, 0, m_width, m_height, m_bkg_color);

    lua_getglobal(L, "pvrender");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return;
    }
    lua_pushinteger(L, m_render_func_index);
    lua_gettable(L, -2);
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return;
    }
    lua_insert(L, -2);
    lua_pop(L, 1);
    luaT_pushobject(L, this, LUAT_RENDER);
    lua_pcall(L, 1, 0, -2);
}

void PluginsViewRender::setBackground(COLORREF color)
{
    m_bkg_color = color;
}

void PluginsViewRender::clear(COLORREF color)
{
    m_dc.FillSolidRect(0, 0, m_width, m_height, color);
}
