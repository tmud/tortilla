#pragma once
#include "pluginsViewRender.h"
#include "pluginsMouseHandler.h"

class Plugin;
class PluginsView : public CWindowImpl<PluginsView>
{
    CWindow m_child_window;
    Plugin *m_plugin;
    PluginsMouseHandler *m_mouse;
    PluginsViewRender *m_render;    
    bool m_render_error;
    bool m_track_mouse;
public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    PluginsView(Plugin *p) : m_child_window(NULL), m_plugin(p), m_mouse(NULL), m_render(NULL),
        m_render_error(false), m_track_mouse(false) {}
    ~PluginsView() { delete m_mouse; delete m_render; }
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
        if (lua_isfunction(L, -1))
            m_render = new PluginsViewRender(L, m_hWnd);
        return m_render;
    }

    void setMouseHandler(lua_State *L)
    {
        if (m_mouse)
            { delete m_mouse; m_mouse = NULL; }
        if (lua_istable(L, -1))
            m_mouse = new PluginsMouseHandler(L);
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

    bool mouseWheelEvent(const POINT& pt, DWORD param);

private:
    BEGIN_MSG_MAP(PluginsView)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouse)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouse)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnMouse)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouse)
        MESSAGE_HANDLER(WM_RBUTTONDBLCLK, OnMouse)
        MESSAGE_HANDLER(WM_RBUTTONUP, OnMouse)
        MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMouse)
        MESSAGE_HANDLER(WM_MBUTTONDBLCLK, OnMouse)
        MESSAGE_HANDLER(WM_MBUTTONUP, OnMouse)
    END_MSG_MAP()

    bool validmouse() {
        if (!m_mouse) return false;
        if (m_child_window.IsWindow()) return false;
        return true;
    }
    bool mouseEvent(UINT msg, WPARAM wparam, LPARAM lparam);

    LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam, BOOL&bHandled )
    {
        if (validmouse())
        { 
            trackMouseLeave();
            if (mouseEvent(msg, wparam, lparam))
                return 0;
        }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam, BOOL&bHandled )
    {
        mouseLeave();
        if (validmouse())
        {
            if (mouseEvent(msg, wparam, lparam))
               return 0;
        }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnMouse(UINT msg, WPARAM wparam, LPARAM lparam, BOOL&bHandled )
    {
        if (validmouse() && mouseEvent(msg, wparam, lparam))
           return 0;
        bHandled = FALSE;
        return 0;
    }

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
    bool render();
    void trackMouseLeave()
    {
        if (m_track_mouse) return;
        m_track_mouse = true;
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        tme.dwHoverTime = 0;
        TrackMouseEvent(&tme);
    }
    void mouseLeave() 
    {
        m_track_mouse = false;
    }
};
