#pragma once

#include "clickpadImage.h"

class PadButton : public CWindowImpl<PadButton>
{
    std::wstring m_text;
    std::wstring m_command;
    std::wstring m_tooltip;
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
    bool m_mouseleave;
    CToolTipCtrl m_tooltipCtrl;

public:
    PadButton(UINT msg, WPARAM param) : m_click_msg(msg), m_click_param(param), m_pushed(false), m_selected(false), m_template(false),
        m_image(NULL),m_background_color(0), m_font(0), m_mouseleave(false) {}
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
    void getTooltip(std::wstring *ttip) const { ttip->assign(m_tooltip); }
    void setTooltip(const std::wstring& ttip) { m_tooltip = ttip; }

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
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
      MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
      MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
      MESSAGE_HANDLER(WM_LBUTTONUP, OnClickUp)
      MESSAGE_HANDLER(WM_PAINT, OnPaint)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        /*RECT r;
        r.left = r.top = 0;
        r.right = 200; r.bottom = 48;
        m_tooltipCtrl.Create(m_hWnd, r, L"TEST", TTS_ALWAYSTIP);
        m_tooltipCtrl.Activate(TRUE);
        m_tooltipCtrl.AddTool()
        /*m_tooltipCtrl.SetDelayTime(TTDT_AUTOPOP, -1);
        m_tooltipCtrl.SetDelayTime(TTDT_INITIAL, 0);
        m_tooltipCtrl.SetDelayTime(TTDT_RESHOW, 0);*/
        return 0;
    }

    LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL&)
    {
        /*if (!m_mouseleave)
        {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = m_hWnd;
            TrackMouseEvent(&tme);
            m_mouseleave = true;
           m_tooltipCtrl.Pop();
        }*/
        return 0;
    }

    LRESULT OnMouseLeave(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_mouseleave = false;
        return 0;
    }

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
