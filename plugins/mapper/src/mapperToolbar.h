#pragma once
#include "bitmapButton.h"

class MapperToolbar : public CDialogImpl<MapperToolbar>
{
    HWND m_controlWindow;
    UINT m_controlMessage;
    CBitmapButtonEx m_down, m_up, m_level0, m_center;
    CImageList m_icons;
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
    END_MSG_MAP()
    MapperToolbar() : m_controlWindow(0), m_controlMessage(0) {}
    void setControlWindow(HWND wnd, UINT msg) {
        m_controlWindow = wnd;
        m_controlMessage = msg;
    }
    void setCenterMode(bool centerMode)
    {
        m_center.SetPushed(centerMode);
    }
private:
    void hide(UINT id) { GetDlgItem(id).ShowWindow(SW_HIDE); }
    void init(CBitmapButtonEx& b, int image, UINT id, const tstring& tooltip) { 
        b.SetImageList(m_icons);
        b.SetImages(image);
        b.SetBitmapButtonExtendedStyle(BMPBTN_AUTO3D_DOUBLE|BMPBTN_SHAREIMAGELISTS|BMPBTN_HOVER, 
            BMPBTN_AUTO3D_DOUBLE|BMPBTN_AUTOSIZE|BMPBTN_SHAREIMAGELISTS|BMPBTN_HOVER);
        b.SubclassWindow(GetDlgItem(id));
        b.SetToolTipText(tooltip.c_str());
    }
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{
        m_icons.Create(IDB_BITMAP_TOOLBAR, 24, 0, RGB(140,140,140));
        init(m_down, 1, IDC_BUTTON_LEVEL_DOWN, L"�� ������� ����");
        init(m_up, 0, IDC_BUTTON_LEVEL_UP, L"�� ������� �����");
        init(m_level0, 2, IDC_BUTTON_LEVEL0, L"������ ������� �������");
        init(m_center, 3, IDC_BUTTON_CENTER, L"���������� ������� �������");
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
