#pragma once

class PluginsView : public CWindowImpl<PluginsView>
{
    CWindow m_child_window;
    tstring m_plugin_name;
public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    PluginsView(const tstring& plugin_name) : m_child_window(NULL), m_plugin_name(plugin_name) {}
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
private:
    BEGIN_MSG_MAP(PluginsView)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&) 
    {
        if (m_child_window.IsWindow())
        {
            RECT rc; GetClientRect(&rc);
            m_child_window.MoveWindow(&rc);
        }
        return 0; 
    }
};
