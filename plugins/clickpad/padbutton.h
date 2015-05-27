#pragma once

class PadButton : public CBitmapButtonImpl<PadButton>
{
    tstring m_text;
    tstring m_command;
    UINT m_click_msg;
    WPARAM m_click_param;
    bool m_pushed;
    bool m_selected;
    static const int bufferlen = 32;
    static WCHAR buffer[bufferlen];    
    tstring m_image_fpath;
    int m_image_index;
    Image* m_image;

public:
    PadButton(UINT msg, WPARAM param) :
        CBitmapButtonImpl<PadButton>(BMPBTN_AUTOSIZE | BMPBTN_AUTO3D_SINGLE, NULL/*hImageList*/),
        m_click_msg(msg), m_click_param(param), m_pushed(false), m_selected(false), m_image_index(-1), m_image(NULL)
    {
    }

    void getText(tstring *text)
    {
        text->assign(m_text);
    }

    void setText(const tstring& text)
    {
        if (text == m_text)
            return;
        m_text.assign(text);
        Invalidate();
    }

    void getCommand(tstring *cmd)
    {
        cmd->assign(m_command);
    }

    void setCommand(const tstring& cmd)
    {
        m_command = cmd;    
    }

    bool isEmptyButton() const 
    {
        return (m_command.empty()) ? true : false;
    }

    void setSelected(bool selected)
    {
        m_selected = selected;
        Invalidate();
    }

    void setImage(const tstring& fpath, int index)
    {
        m_image_fpath = fpath;
        m_image_index = index;
       // m_image = m_images.loadImage(fpath);
    }

    void getImage(tstring* fpath, int *index )
    {
        fpath->assign(m_image_fpath);
        *index = m_image_index;
    }

private:
    BEGIN_MSG_MAP(PadButton)
      MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONUP, OnClickUp)
      CHAIN_MSG_MAP(CBitmapButtonImpl<PadButton>)
    END_MSG_MAP()

    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        m_pushed = true;
        ::SendMessage(GetParent(), m_click_msg, m_click_param, 0);
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnClickUp(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        m_pushed = false;
        ::SendMessage(GetParent(), m_click_msg, m_click_param, 1);
        bHandled = FALSE;
        return 0;
    }

    // override of CBitmapButtonImpl DoPaint(). Adds fillrect
    void DoPaint(CDCHandle dc)
    {
        // added by SoftGee to resolve image artifacts
        RECT rc;
        GetClientRect(&rc);
        UINT state = DFCS_BUTTONPUSH;
        if (m_pushed || m_selected) state |= DFCS_PUSHED;
        dc.DrawFrameControl(&rc,DFC_BUTTON, state);

        if (!m_text.empty())
        {
            int len = m_text.length(); 
            if (len > bufferlen) len = bufferlen;
            wcsncpy(buffer, m_text.c_str(), len);
            rc.left+=2; rc.right-=2;
            dc.SetBkMode(TRANSPARENT);
            dc.DrawTextEx(buffer, len, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        }

        /*if (m_selected)
        {
            COLORREF color = RGB(255,0,0);
            dc.Draw3dRect(&rc, color, color);
        }*/


        // call ancestor DoPaint() method
        //CBitmapButtonImpl<PadButton>::DoPaint(dc);
    }
};
