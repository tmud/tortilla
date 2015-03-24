#include "stdafx.h"
#include "pluginsView.h"
#include "plugin.h"
extern Plugin* _cp;

bool PluginsView::render()
{
    if (m_render_error || !m_render)
        return false;

    m_plugin->setRenderState(true);
    Plugin *old = _cp;
    _cp = m_plugin;
    bool render_ok = m_render->render();
    _cp = old;
    if (!render_ok)
    {
        m_render_error = true;
        m_plugin->setErrorState();
    }
    m_plugin->setRenderState(false);
    return render_ok;
}

int PluginsView::reg_pview_render(lua_State* L)
{
    if (!lua_isfunction(L, -1))
        return 0;
    lua_getglobal(L, "pvrender");
    if (!lua_istable(L, -1))
    {
        if (!lua_isnil(L, -1)) {
            lua_pop(L, 1);
            return 0;
        }
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "pvrender");
    }
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
