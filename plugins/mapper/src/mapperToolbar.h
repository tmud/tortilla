#pragma once
#include "bitmapButton.h"

class MapperToolbar : public CDialogImpl<MapperToolbar>
{
    HWND m_controlWindow;
    UINT m_controlMessage;
    CToolbarButton m_down, m_up, m_level0, m_center, m_home;
    CImageList m_icons;
    float dpi;
public:
    enum { IDD = IDD_MAPPER_TOOLBAR };
    BEGIN_MSG_MAP(MapperToolbar)
		MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_ID_HANDLER(IDC_BUTTON_LEVEL_DOWN, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_LEVEL_UP, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_LEVEL0, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_CENTER, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_SAVEZONES, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_LOADZONES, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_CLEARZONES, OnButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_HOME, OnButton)
    END_MSG_MAP()
    MapperToolbar() : m_controlWindow(0), m_controlMessage(0), dpi(1.0f){}
    void setControlWindow(HWND wnd, UINT msg) 
    {
        m_controlWindow = wnd;
        m_controlMessage = msg;
    }
    void setDpi(float sdpi)
    {
        dpi = sdpi;
    }
    void setCenterMode(bool centerMode)
    {
        m_center.SetPushed(centerMode);
    }
private:
    void hide(UINT id) { GetDlgItem(id).ShowWindow(SW_HIDE); }
    void button(CToolbarButton& b, int image, UINT id, const tstring& tooltip)
    { 
        RECT rc;
        CWindow cb(GetDlgItem(id));
        cb.GetWindowRect(&rc);
        ScreenToClient(&rc);
        cb.ShowWindow(SW_HIDE);
        b.Create(m_hWnd, id, rc, m_icons.GetIcon(image), tooltip.c_str(), dpi);
    }
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{
        m_icons.Create(32, 32, ILC_COLOR24 | ILC_MASK, 0, 0);
        CBitmap icons;
        icons.LoadBitmap(IDB_BITMAP_TOOLBAR);
        m_icons.Add(icons, RGB(255, 255, 255));
        button(m_down, 0, IDC_BUTTON_LEVEL_DOWN, L"На уровень вниз");
        button(m_up, 1, IDC_BUTTON_LEVEL_UP, L"На уровень вверх");
        button(m_level0, 2, IDC_BUTTON_LEVEL0, L"Задать нулевой уровень");
        button(m_center, 3, IDC_BUTTON_CENTER, L"Центровать текущую позицию");
        button(m_home, 4, IDC_BUTTON_HOME, L"Вернуться на текущую позицию");
#ifndef _DEBUG
        //hide(IDC_BUTTON_SAVEZONES);
        //hide(IDC_BUTTON_LOADZONES);
        hide(IDC_BUTTON_CLEARZONES);
#endif
        return 0;
	}
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        m_down.DestroyWindow(); m_down.Detach();
        m_up.DestroyWindow(); m_up.Detach();
        m_level0.DestroyWindow(); m_level0.Detach();
        return 0;
    }
    LRESULT OnButton(WORD, WORD id, HWND, BOOL&)
    {
        if (::IsWindow(m_controlWindow))
            ::PostMessage(m_controlWindow, m_controlMessage, id, 0 );
        return 0;
    }
};

class ToolbarViewContainer : public CWindowImpl<ToolbarViewContainer>
{
    int m_size;
    CWindow m_first, m_second;
public:
    ToolbarViewContainer() : m_size(0) {}
    void attach(int size, HWND first, HWND second)
    {
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
    void onSize()
    {
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
