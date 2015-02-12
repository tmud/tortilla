#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsViewRender.h"

PluginsViewRender::PluginsViewRender(lua_State *pL, int index, HWND wnd) : L(pL), m_render_func_index(index), m_wnd(wnd),
m_inside_render(false), m_bkg_color(0), m_width(0), m_height(0)
{
    assert(L && index > 0);
}

void PluginsViewRender::render()
{
    CPaintDC dc(m_wnd);
    m_dc = dc;

    RECT rc; m_wnd.GetClientRect(&rc);
    m_width = rc.right; m_height = rc.bottom;
    m_dc.FillSolidRect(&rc, m_bkg_color);

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
    m_inside_render = true;
    lua_pcall(L, 1, 0, -2);
    m_inside_render = false;
}

void PluginsViewRender::setBackground(COLORREF color)
{
    m_bkg_color = color;
    m_wnd.Invalidate();
}

int PluginsViewRender::width()
{
    if (m_inside_render)
        return m_width;
    RECT rc; m_wnd.GetClientRect(&rc);
    return rc.right;
}

int PluginsViewRender::height()
{
    if (m_inside_render)
        return m_height;
    RECT rc; m_wnd.GetClientRect(&rc);
    return rc.bottom;
}

int render_setbackground(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_RENDER, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        COLORREF color = RGB(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
        r->setBackground(color);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        COLORREF color = lua_tounsigned(L, 2);
        r->setBackground(color);
        return 0;
    }
    return pluginInvArgs(L, "render.setbackground");
}

int render_width(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        lua_pushinteger(L, r->width());
        return 1;
    }
    return pluginInvArgs(L, "render.width");
}

int render_height(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        lua_pushinteger(L, r->width());
        return 1;
    }
    return pluginInvArgs(L, "render.height");
}

void reg_mt_render(lua_State *L)
{
    luaL_newmetatable(L, "render");
    regFunction(L, "setbackground", render_setbackground);
    regFunction(L, "width", render_width);
    regFunction(L, "height", render_height);
    regIndexMt(L);
    lua_pop(L, 1);
}
