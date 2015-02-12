#pragma once
#include "pluginsViewRender.h"

class PluginsView : public CWindowImpl<PluginsView>
{
    CWindow m_child_window;
    tstring m_plugin_name;
    PluginsViewRender *m_render;

public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    PluginsView(const tstring& plugin_name) : m_child_window(NULL), m_plugin_name(plugin_name), m_render(NULL) {}
    ~PluginsView() { delete m_render; }

    const tstring& getPluginName() const
    {
        return m_plugin_name;
    }

    void attachChild(HWND wnd)
    {
        m_child_window.Attach(wnd);
        m_child_window.SetParent(m_hWnd);
        RECT rc; GetClientRect(&rc);
        m_child_window.MoveWindow(&rc);
    }

    PluginsViewRender* setRender(lua_State *L)
    {
        delete m_render;
        int render_id = reg_pview_render(L);
        m_render = new PluginsViewRender(L, render_id, m_hWnd);
        return m_render;
    }

private:
    BEGIN_MSG_MAP(PluginsView)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    END_MSG_MAP()
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&bHandled )
    {
        if (!m_child_window.IsWindow())
        {
            if (m_render)
                m_render->render();
            else
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
private:
    int  reg_pview_render(lua_State* L);
};
