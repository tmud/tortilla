#pragma once

#include "clickpadImage.h"

class PadButton : public CWindowImpl<PadButton>
{
    std::wstring m_text;
    std::wstring m_command;
    UINT m_click_msg;
    WPARAM m_click_param;
    bool m_pushed;
    bool m_selected;
    bool m_template;
    static const int bufferlen = 32;
    static WCHAR buffer[bufferlen];    
    ClickpadImage* m_image;
    COLORREF m_background_color;
    HFONT m_font;

public:
    PadButton(UINT msg, WPARAM param) : m_click_msg(msg), m_click_param(param), m_pushed(false), m_selected(false), m_template(false),
        m_image(NULL),m_background_color(0), m_font(0) {}
    ~PadButton() { delete m_image; }
    void getText(std::wstring *text) const { text->assign(m_text); }
    void setText(const std::wstring& text)
    {
        if (text == m_text)
            return;
        m_text.assign(text);
        Invalidate(FALSE);
    }
    void getCommand(std::wstring *cmd) const { cmd->assign(m_command); }
    void setCommand(const std::wstring& cmd) { m_command = cmd; }
    bool isEmptyButton() const {
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

    void setFont(HFONT font)
    {
        m_font = font;
        Invalidate(FALSE);
    }

    ClickpadImage * getImage() const { return m_image; }
    void setTemplate(bool template_flag) { m_template = template_flag; }
    bool getTemplate() const { return m_template; }
    void setBackgroundColor(COLORREF color) { m_background_color = color; }
private:
    BEGIN_MSG_MAP(PadButton)
      MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONUP, OnClickUp)
      MESSAGE_HANDLER(WM_PAINT, OnPaint)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    END_MSG_MAP()

    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&bHandled)
    { 
        m_pushed = true;
        ::SendMessage(GetParent(), m_click_msg, m_click_param, 0);
        Invalidate(FALSE);
        bHandled = FALSE;
        return 0; 
    }

    LRESULT OnClickUp(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        m_pushed = false;
        ::SendMessage(GetParent(), m_click_msg, m_click_param, 1);
        Invalidate(FALSE);
        bHandled = FALSE;
        return 0;
    }
    void onPaint();
    LRESULT OnEraseBackground(UINT, WPARAM, LPARAM, BOOL&) { return 1; }    
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&) { onPaint();  return 0; }
};
