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
            //todo
        }

        //_wndMain.m_gameview.createPanel()


    }

    return pluginInvArgs(L, "createPanel");
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
    lua_register(L, "createPanel", createpanel);

    luaL_newmetatable(L, "panel");
    regFunction(L, "create", pn_create);
   
    regIndexMt(L);
    lua_pop(L, 1);
}
