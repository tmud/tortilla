#pragma once

#include "clickpadImage.h"

class PadButton : public CBitmapButtonImpl<PadButton>
{
    tstring m_text;
    tstring m_command;
    UINT m_click_msg;
    WPARAM m_click_param;
    bool m_pushed;
    bool m_selected;
    bool m_template;
    static const int bufferlen = 32;
    static WCHAR buffer[bufferlen];    
    ClickpadImage* m_image;

public:
    PadButton(UINT msg, WPARAM param) :
        CBitmapButtonImpl<PadButton>(BMPBTN_AUTOSIZE | BMPBTN_AUTO3D_SINGLE, NULL/*hImageList*/),
        m_click_msg(msg), m_click_param(param), m_pushed(false), m_selected(false), m_template(false), m_image(NULL)
    {
    }
    ~PadButton() { 
        delete m_image;
    }

    void getText(tstring *text) const
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

    void getCommand(tstring *cmd) const
    {
        cmd->assign(m_command);
    }

    void setCommand(const tstring& cmd)
    {
        m_command = cmd;    
    }

    bool isEmptyButton() const 
    {
        if (m_text.empty() && m_command.empty()) 
        {
           if (!m_image || m_image->empty())
                return true;
        }
        return false;
    }

    void clear()
    {
        m_text.clear();
        m_command.clear();
        delete m_image;
        m_image = NULL;    
    }

    void setSelected(bool selected)
    {
        m_selected = selected;
        Invalidate();
    }

    void setImage(ClickpadImage *image)
    {
       if (m_image)
           delete m_image;
       m_image = image;
       Invalidate(FALSE);
    }

    ClickpadImage * getImage() const
    {
        return m_image;
    }

    void setTemplate(bool template_flag)
    {
        m_template = template_flag;
    }

    bool getTemplate() const 
    {
        return m_template;
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
        RECT rc;
        GetClientRect(&rc);
        UINT state = DFCS_BUTTONPUSH;
        if (m_pushed || m_selected) 
            state |= DFCS_PUSHED;
        dc.DrawFrameControl(&rc,DFC_BUTTON, state);

        if (m_image && !m_image->empty())
        {
            int x = (rc.right - m_image->width()) / 2;
            int y = (rc.bottom - m_image->height()) / 2;
            if (m_pushed)
                 m_image->renderpushed(dc, x, y);
            else
                m_image->render(dc, x, y);
        }

        if (!m_text.empty())
        {
            int len = m_text.length(); 
            if (len > bufferlen) len = bufferlen;
            wcsncpy(buffer, m_text.c_str(), len);
            rc.left+=2; rc.right-=2;
            dc.SetBkMode(TRANSPARENT);
            dc.DrawTextEx(buffer, len, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        }
    }
};
