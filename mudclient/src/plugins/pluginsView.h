#pragma once
#include "pluginsViewRender.h"

class Plugin;
class PluginsView : public CWindowImpl<PluginsView>
{
    CWindow m_child_window;
    Plugin *m_plugin;
    PluginsViewRender *m_render;
    bool m_render_error;

public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    PluginsView(Plugin *p) : m_child_window(NULL), m_plugin(p), m_render(NULL), m_render_error(false) {}
    ~PluginsView() { delete m_render; }
    Plugin* getPlugin() const { return m_plugin; }

    void attachChild(HWND wnd)
    {
        m_child_window.Attach(wnd);
        m_child_window.SetParent(m_hWnd);
        RECT rc; GetClientRect(&rc);
        m_child_window.MoveWindow(&rc);
    }

    PluginsViewRender* setRender(lua_State *L)
    {
        if (m_render)
            { delete m_render; m_render = NULL; }
        int render_id = reg_pview_render(L);
        if (render_id > 0)
            m_render = new PluginsViewRender(L, render_id, m_hWnd);
        return m_render;
    }

    bool isChildAttached() const
    {
        return (m_child_window.IsWindow()) ? true : false;
    }

    bool isRenderErrorState() const
    {
        return m_render_error;
    }

    void resetRenderErrorState()
    {
        m_render_error = false;
    }

private:
    BEGIN_MSG_MAP(PluginsView)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    END_MSG_MAP()
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&bHandled )
    {
        if (!m_child_window.IsWindow())
        {
            if (!render())
            {
                CPaintDC dc(m_hWnd);
                RECT rc; GetClientRect(&rc);
                dc.FillSolidRect(&rc, GetSysColor(COLOR_BACKGROUND));
            }
            return 0;
        }
        bHandled = FALSE;
        return 0;
    }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&) 
    {
        if (m_child_window.IsWindow())
        {
            RECT rc; GetClientRect(&rc);
            m_child_window.MoveWindow(&rc);
        }
        return 0; 
    }
    LRESULT OnEraseBackground(UINT, WPARAM, LPARAM, BOOL&)
    {
        return 1; // handled, no background painting needed
    }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_hWnd = NULL;
        return 0;
    }
private:
    int  reg_pview_render(lua_State* L);
    bool render();
};
