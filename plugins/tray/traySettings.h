#pragma once
#include "resource.h"

class HighlightSelectColor : public CWindowImpl < HighlightSelectColor, CButton, CControlWinTraits >
{
    COLORREF m_color;
    CBrush m_backgroundBrush;
    CBrush m_disabledStateBrush;
    COLORREF m_crShadow;
    COLORREF m_crDarkShadow;
    COLORREF m_crHiLight;
    UINT m_msg;

public:
    HighlightSelectColor(COLORREF color, UINT msg) : m_color(color), m_msg(msg) {}
    DECLARE_WND_SUPERCLASS(NULL, CButton::GetWndClassName())

    void setColor(COLORREF color)
    {
        m_color = color;
        initBackground();
        Invalidate();
    }

    COLORREF getColor() const
    {
        return m_color;
    }

private:
    BEGIN_MSG_MAP(HighlightSelectColor)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouseClick)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseClick)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        initBackground();
        SetButtonStyle(BS_OWNERDRAW);
        m_crShadow = GetSysColor(COLOR_3DSHADOW);
        m_crDarkShadow = GetSysColor(COLOR_3DDKSHADOW);
        m_crHiLight = GetSysColor(COLOR_3DHILIGHT);
        m_disabledStateBrush.CreateSysColorBrush(COLOR_BTNFACE);
        return 0;
    }

    LRESULT OnDrawItem(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
    {
        DRAWITEMSTRUCT* lpDIS = (DRAWITEMSTRUCT*)lParam;
        const RECT &rc = lpDIS->rcItem;
        CDCHandle dc = lpDIS->hDC;
        if (!IsWindowEnabled())
        {
            FillRect(dc, &rc, m_disabledStateBrush);
            dc.Draw3dRect(&rc, m_crHiLight, m_crDarkShadow);
            return 0;
        }
        dc.Rectangle(&rc);
        /*bool bSelected = (ODS_SELECTED & lpDIS->itemState) == ODS_SELECTED;
        if (bSelected)
            dc.Draw3dRect(&rc, m_crDarkShadow, m_crHiLight);
        else
            dc.Draw3dRect(&rc, m_crHiLight, m_crDarkShadow);*/
        CRect rc2(rc); rc2.DeflateRect(2, 2);
        FillRect(dc, &rc2, m_backgroundBrush);
        return 0;
    }

    LRESULT OnMouseClick(UINT uMsg, WPARAM, LPARAM, BOOL&)
    {
        ::SendMessage(GetParent(), m_msg, 0, 0);
        return 0;
    }

    void initBackground()
    {
        if (!m_backgroundBrush.IsNull())
            m_backgroundBrush.DeleteObject();
        m_backgroundBrush = CreateSolidBrush(m_color);
    }
};

struct TraySettings
{
    TraySettings() : timeout(5), interval(15), text(0), background(0), syscolor(true), showactive(false) {}
    int timeout;
    int interval;
    COLORREF text;
    COLORREF background;
    bool syscolor;
    bool showactive;
};

class TraySettingsDlg :  public CDialogImpl < TraySettingsDlg >
{
public:
    enum { IDD = IDD_SETTINGS };
    TraySettingsDlg() :  m_textColor(0, WM_USER+2), m_bkgColor(0, WM_USER+1) {}
    TraySettings settings;
private:
    CEdit m_timeout;
    CEdit m_interval;
    HighlightSelectColor m_textColor;
    HighlightSelectColor m_bkgColor;
    CButton m_syscolors;
    CButton m_showactive;
    wchar_t m_buffer[32];

private:
    BEGIN_MSG_MAP(TraySettingsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_USER+2, OnTextColor)
        MESSAGE_HANDLER(WM_USER+1, OnBkgndColor)
        //MESSAGE_HANDLER(WM_USER, OnTest)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_HANDLER(IDC_EDIT_TIMEOUT, EN_KILLFOCUS, OnTimeout)
        COMMAND_HANDLER(IDC_EDIT_INTERVAL, EN_KILLFOCUS, OnInterval)
        COMMAND_ID_HANDLER(IDC_CHECK_SYSCOLORS, OnSyscolors)
        COMMAND_ID_HANDLER(IDC_CHECK_SHOWACTIVE, OnShowActive)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    const wchar_t* i2a(int val)
    {
        _itow(val, m_buffer, 10);
        return m_buffer;
    }

    int a2i(const wchar_t* val)
    {
        return _wtoi(val);
    }

    void getWindowText(HWND handle, tstring *string)
    {
        int text_len = ::GetWindowTextLength(handle);
        wchar_t *tmp = new wchar_t[text_len + 2];
        ::GetWindowText(handle, tmp, text_len + 1);
        string->assign(tmp);
        delete []tmp;
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_timeout.Attach(GetDlgItem(IDC_EDIT_TIMEOUT));
        m_timeout.SetWindowText(i2a(settings.timeout));
        m_timeout.SetLimitText(4);

        m_interval.Attach(GetDlgItem(IDC_EDIT_INTERVAL));
        m_interval.SetWindowText(i2a(settings.interval));
        m_interval.SetLimitText(3);

        RECT rc;
        CStatic tc(GetDlgItem(IDC_STATIC_TEXTCOLOR));
        tc.GetWindowRect(&rc);
        tc.ShowWindow(SW_HIDE);
        ScreenToClient(&rc);
        m_textColor.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP);
        m_textColor.setColor(settings.text);

        CStatic bc(GetDlgItem(IDC_STATIC_BKGCOLOR));
        bc.GetWindowRect(&rc);
        bc.ShowWindow(SW_HIDE);
        ScreenToClient(&rc);
        m_bkgColor.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP);
        m_bkgColor.setColor(settings.background);

        m_syscolors.Attach(GetDlgItem(IDC_CHECK_SYSCOLORS));
        if (settings.syscolor)
            m_syscolors.SetCheck(BST_CHECKED);

        m_showactive.Attach(GetDlgItem(IDC_CHECK_SHOWACTIVE));
        if (settings.showactive)
            m_showactive.SetCheck(BST_CHECKED);

        /*m_timeout.SetFocus();
        int pos = m_timeout.GetWindowTextLength();
        m_timeout.SetSel(pos, pos);*/
        ::SetFocus(GetDlgItem(IDOK));
        CenterWindow(GetParent());
        return 0;
    }

    LRESULT OnTimeout(WORD, WORD wID, HWND, BOOL&)
    {
        tstring timeout;
        getWindowText(m_timeout, &timeout);
        int val = !timeout.empty() ? a2i(timeout.c_str()) : 0;
        if (val < 1) { val = 5; m_timeout.SetWindowText(i2a(val)); }
        else if (val > MAX_TIMEOUT) { val = MAX_TIMEOUT; m_timeout.SetWindowText(i2a(val)); }
        settings.timeout = val;
        return 0;
    }

    LRESULT OnInterval(WORD, WORD wID, HWND, BOOL&)
    {
        tstring interval;
        getWindowText(m_interval, &interval);
        int val = !interval.empty() ? a2i(interval.c_str()) : 0;
        if (val < 5) { val = 5; m_interval.SetWindowText(i2a(val)); }
        else if (val > MAX_INTERVAL) { val = MAX_INTERVAL; m_interval.SetWindowText(i2a(val)); }
        settings.interval = val;
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        EndDialog(wID);
        return 0;
    }

    LRESULT OnTextColor(UINT, WPARAM, LPARAM, BOOL&)
    {
        CColorDialog dlg(settings.text, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            settings.text = dlg.GetColor();
            m_textColor.setColor(settings.text);
        }
        return 0;
    }

    LRESULT OnBkgndColor(UINT, WPARAM, LPARAM, BOOL&)
    {
        CColorDialog dlg(settings.background, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            settings.background = dlg.GetColor();
            m_bkgColor.setColor(settings.background);
        }
        return 0;
    }

    LRESULT OnSyscolors(WORD, WORD, HWND, BOOL&)
    {
        settings.syscolor = (m_syscolors.GetCheck()==BST_CHECKED) ? true : false;
        return 0;
    }

    LRESULT OnShowActive(WORD, WORD, HWND, BOOL&)
    {
        settings.showactive = (m_showactive.GetCheck()==BST_CHECKED) ? true : false;
        return 0;
    }

    LRESULT OnTest(UINT, WPARAM, LPARAM, BOOL&)
    {
        OutputDebugString(L"WM_USER ???\r\n");
        return 0;
    }
};
