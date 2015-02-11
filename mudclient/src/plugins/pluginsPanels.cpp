#include "stdafx.h"
#include "pluginsPanels.h"
#include "pluginsApi.h"
#include "pluginSupport.h"
#include "plugin.h"
#include "MainFrm.h"

extern CMainFrame _wndMain;
extern Plugin* _cp;

int createpanel(lua_State *L)
{
    PluginData &p = find_plugin();
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
    {
        PanelWindow w;
        w.size = lua_tointeger(L, 2);
        w.side = _wndMain.m_gameview.convertSideFromString(TU2W(lua_tostring(L, 1)));
        if (IsDocked(w.side))
        {
            PluginsView *window = _wndMain.m_gameview.createPanel(w, p.name);
            if (window)
                _cp->panels.push_back(window);
            luaT_pushobject(L, window, LUAT_PANEL);
            return 1;
        }
    }
    return pluginInvArgs(L, "createPanel");
}

int pn_attach(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_PANEL, LUA_TNUMBER))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        HWND child = (HWND)lua_tointeger(L, 2);
        v->attachChild(child);
        return 0;
    }
    return pluginInvArgs(L, "panel.attach");
}

int pn_setrender(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_PANEL, LUA_TFUNCTION))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        int id = reg_pview_render(L);
        v->setRender(L, id);
        return 0;
    }
    return pluginInvArgs(L, "panel.setrender");
}

int pn_update(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_PANEL))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        v->update();
        return 0;
    }
    return pluginInvArgs(L, "panel.update");
}

int pn_setbackground(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_PANEL, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsView *r = (PluginsView *)luaT_toobject(L, 1);
        COLORREF color = RGB(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
        r->setBackground(color);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_PANEL, LUA_TNUMBER))
    {
        PluginsView *r = (PluginsView *)luaT_toobject(L, 1);
        COLORREF color = lua_tounsigned(L, 2);
        r->setBackground(color);
        return 0;
    }
    return pluginInvArgs(L, "panel.setbackground");
}

void reg_mt_panels(lua_State *L)
{
    lua_register(L, "createPanel", createpanel);
    luaL_newmetatable(L, "panel");
    regFunction(L, "attach", pn_attach);
    regFunction(L, "setrender", pn_setrender);
    regFunction(L, "update", pn_update);
    regFunction(L, "setbackground", pn_setbackground);
    regIndexMt(L);
    lua_pop(L, 1);
}
