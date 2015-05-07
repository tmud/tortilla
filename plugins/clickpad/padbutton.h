#pragma once

class PadButton : public CBitmapButtonImpl<PadButton>
{
    tstring m_command;
    UINT m_click_msg;
    WPARAM m_click_param;

public:
    PadButton(UINT msg, WPARAM param) :  m_click_msg(msg), m_click_param(param)
    {
    }

    void getText(tstring *text)
    {
        int len = GetWindowTextLength() + 1;
        wchar_t *buffer = new wchar_t[len];
        GetWindowText(buffer, len);
        text->assign(buffer);
        delete []buffer;    
    }

    void setText(const tstring& text)
    {
        SetWindowText(text.c_str());    
    }

    void getCommand(tstring *cmd)
    {
        cmd->assign(m_command);
    }

    void setCommand(const tstring& cmd)
    {
        m_command = cmd;    
    }

private:
    BEGIN_MSG_MAP(PadButton)
      MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONUP, OnClickUp)
    END_MSG_MAP()

    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        ::SendMessage(GetParent(), m_click_msg, m_click_param, 0);
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnClickUp(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        ::SendMessage(GetParent(), m_click_msg, m_click_param, 1);
        bHandled = FALSE;
        return 0;
    }
};

  /*  CButtons
   // override of CBitmapButtonImpl DoPaint(). Adds fillrect
    void DoPaint(CDCHandle dc)
    {
        // added by SoftGee to resolve image artifacts
        RECT rc;
        GetClientRect(&rc);
        dc.FillRect(&rc, (HBRUSH)(COLOR_BTNFACE + 1));

        // call ancestor DoPaint() method
        CBitmapButtonImpl<CBmpBtn>::DoPaint(dc);
    }*/

/*
class CBmpBtn : public CBitmapButtonImpl<CBmpBtn>
{
public:
    DECLARE_WND_SUPERCLASS(_T("WTL_BmpBtn"), GetWndClassName())

    // added border style (auto3d_single)
    CBmpBtn(DWORD dwExtendedStyle = BMPBTN_AUTOSIZE | BMPBTN_AUTO3D_SINGLE, HIMAGELIST hImageList = NULL) :
        CBitmapButtonImpl<CBmpBtn>(dwExtendedStyle, hImageList)
    { }

    BEGIN_MSG_MAP(CBmpBtn)
        CHAIN_MSG_MAP(CBitmapButtonImpl<CBmpBtn>)
    END_MSG_MAP()

    // override of CBitmapButtonImpl DoPaint(). Adds fillrect
    void DoPaint(CDCHandle dc)
    {
        // added by SoftGee to resolve image artifacts
        RECT rc;
        GetClientRect(&rc);
        dc.FillRect(&rc, (HBRUSH)(COLOR_BTNFACE + 1));

        // call ancestor DoPaint() method
        CBitmapButtonImpl<CBmpBtn>::DoPaint(dc);
    }
};
*/