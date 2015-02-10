#pragma once

class PluginsView : public CWindowImpl<PluginsView>
{
    CWindow m_child_window;
    tstring m_plugin_name;
    COLORREF m_background_color;
public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    PluginsView(const tstring& plugin_name) : m_child_window(NULL), m_plugin_name(plugin_name) 
    {
        m_background_color = 0;
    }
    ~PluginsView() {}

    const tstring& getPluginName() const
    {
        return m_plugin_name;
    }

    void attachChild(HWND wnd)
    {
        m_child_window.Attach(wnd);
        RECT rc; GetClientRect(&rc);
        m_child_window.MoveWindow(&rc);
    }

    void setBackground(COLORREF color)
    {
        m_background_color = color;
        Invalidate();
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
            RECT rc; GetClientRect(&rc);
            CPaintDC dc(m_hWnd);
            dc.FillSolidRect(&rc, m_background_color);
        }
        else
        {
            bHandled = FALSE;
        }
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
};
