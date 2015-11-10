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
    luaT_fun_table ft("_pvrender");
    return ft.pushFunction(L);
}
