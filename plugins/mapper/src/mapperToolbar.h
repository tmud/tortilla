#pragma once

class MapperToolbar : public CDialogImpl<MapperToolbar>
{
    HWND m_controlWindow;
    UINT m_controlMessage;
public:  
    enum { IDD = IDD_MAPPER_TOOLBAR };
    BEGIN_MSG_MAP(MapperToolbar)
		MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
        COMMAND_ID_HANDLER(IDC_BUTTON_LEVEL_DOWN, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_LEVEL_UP, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_LEVEL0, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_SAVEZONES, OnDebugButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_LOADZONES, OnDebugButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_CLEARZONES, OnDebugButton)
    END_MSG_MAP()
    MapperToolbar() : m_controlWindow(0), m_controlMessage(0) {}
    void setControlWindow(HWND wnd, UINT msg) {
        m_controlWindow = wnd;
        m_controlMessage = msg;
    }
private:
    void hide(UINT id) { GetDlgItem(id).ShowWindow(SW_HIDE); }
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{   
#ifndef _DEBUG
        hide(IDC_BUTTON_SAVEZONES);
        hide(IDC_BUTTON_LOADZONES);
        hide(IDC_BUTTON_CLEARZONES);
#endif
        return 0;
	}
    LRESULT OnButton(WORD, WORD id, HWND, BOOL&)
    {
        if (::IsWindow(m_controlWindow))
            ::PostMessage(m_controlWindow, m_controlMessage, id, 0 );
        return 0;
    }
    LRESULT OnDebugButton(WORD, WORD id, HWND, BOOL&)
    {
#ifdef _DEBUG
        if (::IsWindow(m_controlWindow))
            ::PostMessage(m_controlWindow, m_controlMessage, id, 0 );
#endif
        return 0;
    }
};

class ToolbarViewContainer : public CWindowImpl<ToolbarViewContainer>
{
    int m_size;
    CWindow m_first, m_second;
public:
    ToolbarViewContainer() : m_size(0) {}
    void attach(int size, HWND first, HWND second) {
        m_size = size;
        m_first.Attach(first);
        m_second.Attach(second);
        Invalidate();
    }
private:
    BEGIN_MSG_MAP(ToolbarViewContainer)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    void onSize() {
        RECT rc; GetClientRect(&rc);
        int bottom = rc.bottom;
        rc.bottom = m_size;
        if (m_first.IsWindow())
            m_first.MoveWindow(&rc);
        rc.top = rc.bottom;
        rc.bottom = bottom;
        if (m_second.IsWindow())
           m_second.MoveWindow(&rc);
    }
};
