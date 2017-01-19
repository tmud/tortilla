#pragma once
#include "common.h"
#include "sharingData.h"


struct Animation
{
    //enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT, ANIMATION_MOVE };
    //enum { ANIMATION_FINISHED = 0, MOVEANIMATION_FINISHED, STARTANIMATION_FINISHED, CLICK_EVENT };
    enum AnimationState { ANIMATION_NONE, ANIMATION_FADE_UP, ANIMATION_FADE_DOWN, ANIMATION_MOVE };    
    Animation() : state(ANIMATION_NONE), speed(0), wait_sec(0), bkgnd_color(0), text_color(0),
        notify_wnd(0), notify_msg(0), notify_param(0) {}
    SharingWindow pos;
    AnimationState state;
    float speed;
    int   wait_sec;
    COLORREF bkgnd_color;
    COLORREF text_color;
    HWND notify_wnd;
    UINT notify_msg;
    WPARAM notify_param;
};

class PopupWindow : public CWindowImpl<PopupWindow>
{
    Ticker m_ticker;    
    std::wstring m_original_text;
    std::vector<std::wstring> m_text;
    CFont *m_font;
    CSize m_dc_size;
    TempDC m_src_dc;
    Animation m_animation;
    int wait_timer;
    float alpha, m_move_dx, m_move_dy;    
public:
    DECLARE_WND_CLASS(NULL)
    //enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT, ANIMATION_MOVE };
    //enum { ANIMATION_FINISHED = 0, MOVEANIMATION_FINISHED, STARTANIMATION_FINISHED, CLICK_EVENT };

    PopupWindow() : m_font(NULL),
        wait_timer(0), alpha(0), m_move_dx(0), m_move_dy(0)
    {
    }
    ~PopupWindow() {
        if (IsWindow()) DestroyWindow(); 
    }

    bool create(CFont *font)
    {
        m_font = font;
        Create(GetDesktopWindow(), CWindow::rcDefault, NULL, WS_POPUP, WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_NOACTIVATE);
        return (IsWindow()) ? true : false;
    }

    void setText(const std::wstring& text)
    {
        if (m_original_text == text)
            return;
        m_original_text = text;
        m_text.resize(1);
        m_text[0].assign(text);
        calcDCSize();
        int dx = m_dc_size.cx;
        int wx = GetSystemMetrics(SM_CXSCREEN);
        int p = (dx * 100) / wx;
        if (p > 40)
        {
            int count = (p / 40) + 1;
            size_t perline = text.size() / count;
            m_text.resize(count);

            std::vector<std::wstring> parts;
            const wchar_t *p = text.c_str();
            const wchar_t *e = p + wcslen(p);
            while (p < e)
            {
                size_t len = wcscspn(p, L" ");
                parts.push_back(std::wstring(p, len));
                p = p + len;
                len = wcsspn(p, L" ");
                p = p + len;
                for (;len > 0;--len)
                    parts.push_back(L" ");
            }

            int k = 0;
            for (int i=0; i<count-1; ++i) 
            {
                std::wstring str;
                while (str.length() < perline)
                {
                    str.append(parts[k++]);
                }
                trimleft(&str);
                m_text[i] = str;
            }
            std::wstring str;
            for (size_t i=k; i < parts.size(); ++i)
                str.append(parts[i]);
            trimleft(&str);
            if (str.empty())
                m_text.pop_back();
            else {
                int last = count-1;
                m_text[last] = str;
            }
            calcDCSize();
        }
    }
    const SIZE& getSize() const
    {
        CSize sz(m_dc_size);
        sz += CSize(16,12);
        return sz;
    }

    /*bool isAnimated() const {  return (m_animation_state==ANIMATION_NONE) ? false : true; }
    int  getAnimationState() const { return m_animation_state; }
    const Animation& getAnimation() const { return m_animation; }
    const MoveAnimation& getMoveAnimation() const { return m_move_animation; }*/
    void startAnimation(const Animation& a);
    //void startMoveAnimation(const MoveAnimation& a);
    void onTick();

private:
    void onTimer();
    void fillSrcDC();
    void setState(int newstate);
    void calcDCSize();
    void setAlpha(float a);
    void onClickButton();
    void sendNotify(int state);
    void trimleft(std::wstring* s)
    {
        size_t pos = wcsspn(s->c_str(), L" ");
        if (pos != 0)
             s->assign(s->substr(pos));
    }
private:
    BEGIN_MSG_MAP(PopupWindow)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
    END_MSG_MAP()
    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&) { onClickButton(); return 0;  }
};
