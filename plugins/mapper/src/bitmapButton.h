#pragma once

class CToolbarButton : public CWindowImpl<CToolbarButton, CWindow>
{
public:
    DECLARE_WND_CLASS(NULL)

    CIcon m_icon;
    CSize m_icon_size;
    UINT m_id;

    CToolTipCtrl m_tip;
    LPTSTR m_lpstrToolTipText;
    CBrush m_backgroundBrush;

    unsigned m_fMouseOver : 1;
    unsigned m_fPressed : 1;
    unsigned m_fTracking : 1;
    unsigned m_fChecked : 1;

    CToolbarButton() :
        m_id(0),
        m_lpstrToolTipText(NULL),
        m_fMouseOver(0),
        m_fPressed(0),
        m_fTracking(0),
        m_fChecked(0)
    {
    }

    ~CToolbarButton()
    {
        delete[] m_lpstrToolTipText;
    }

    HWND Create(HWND parent, UINT id, RECT rc, HICON image, LPCTSTR tooltip, float dpi)
    {
        m_id = id;
        if (tooltip != NULL) {
            size_t len = _tcslen(tooltip);
            m_lpstrToolTipText = new TCHAR[len + 1];
            _tcscpy(m_lpstrToolTipText, tooltip);
        }
        m_backgroundBrush.CreateSysColorBrush(COLOR_BTNFACE);
        m_icon.Attach(image);
        ICONINFO info;
        m_icon.GetIconInfo(&info);
        CBitmapHandle bh(info.hbmColor);
        SIZE size = { 0, 0 };
        bh.GetSize(size);
        m_icon_size.cx = static_cast<int>(size.cx * dpi);
        m_icon_size.cy = static_cast<int>(size.cy * dpi);
        return CWindowImpl::Create(parent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    }

    void SetPushed(bool state)
    {
        m_fChecked = (state) ? 1 : 0;
        Invalidate();
    }

    // Message map and handlers
    BEGIN_MSG_MAP(CToolbarButton)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        // create a tool tip
        m_tip.Create(m_hWnd);
        ATLASSERT(m_tip.IsWindow());
        if (m_tip.IsWindow() && m_lpstrToolTipText != NULL)
        {
            m_tip.Activate(TRUE);
            m_tip.AddTool(m_hWnd, m_lpstrToolTipText);
        }
        bHandled = FALSE;
        return 1;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        if (m_tip.IsWindow())
        {
            m_tip.DestroyWindow();
            m_tip.m_hWnd = NULL;
        }
        bHandled = FALSE;
        return 1;
    }

    LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        MSG msg = { m_hWnd, uMsg, wParam, lParam };
        if (m_tip.IsWindow())
            m_tip.RelayEvent(&msg);
        bHandled = FALSE;
        return 1;
    }

    LRESULT OnEraseBackground(UINT, WPARAM, LPARAM, BOOL&)
    {
        return 1;   // no background needed
    }

    LRESULT OnPaint(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        RECT pos;
        GetClientRect(&pos);
        CPaintDC dc(m_hWnd);
        CMemoryDC mdc(dc, pos);
        mdc.FillRect(&pos, m_backgroundBrush);


        int iconx = (pos.right - m_icon_size.cx) / 2;
        int icony = (pos.bottom - m_icon_size.cy) / 2;

        if (m_fChecked == 1)
        {
            if (m_fPressed == 1)
            {
                mdc.DrawEdge(&pos, EDGE_SUNKEN, BF_RECT);
            }
            else
            {
                iconx++;
                icony++;
                mdc.DrawEdge(&pos, EDGE_SUNKEN, BF_RECT);
            }
        }
        else if (m_fMouseOver == 1)
        {
            if (m_fPressed == 1) 
            {
                iconx++;
                icony++;
                mdc.DrawEdge(&pos, EDGE_SUNKEN, BF_RECT);
            }
            else
                mdc.DrawEdge(&pos, EDGE_ETCHED, BF_RECT);
        }
        m_icon.DrawIcon(mdc, iconx, icony);
        return 0;
    }

    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
    {
        m_fPressed = 1;
        Invalidate();
        ::SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(m_id, 0), 0);
        return 0;
    }

    LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
    {
        m_fPressed = 0;
        Invalidate();
        return 0;
    }

    LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        if (!m_fMouseOver) 
        {
            m_fMouseOver = true;
            StartTrackMouseLeave();
            Invalidate();
        }
        bHandled = FALSE;
        return 1;
    }

    LRESULT OnMouseLeave(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_fMouseOver = 0;
        m_fPressed = 0;
        Invalidate();
        return 0;
    }

    BOOL StartTrackMouseLeave()
    {
        TRACKMOUSEEVENT tme = { 0 };
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        return _TrackMouseEvent(&tme);
    }
};
