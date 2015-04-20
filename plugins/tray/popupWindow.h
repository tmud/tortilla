#pragma once

class Ticker
{
    DWORD m_ticker;
public:
    Ticker() { sync(); }
    void sync() { m_ticker = GetTickCount(); }
    DWORD getDiff() const
    {
        DWORD diff = -1;
        DWORD tick = GetTickCount();
        if (tick >= m_ticker)
            diff = tick - m_ticker;
        else
        {   // overflow 49.7 days (MSDN GetTickCount)
            diff = diff - m_ticker;
            diff = diff + tick + 1;
        }
        return diff;
    }
};

struct Animation
{
    POINT pos;
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
    CFont *m_font;
    std::wstring m_text;
    SIZE m_dc_size;

    enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT, ANIMATION_MOVE };
    Animation m_animation;
    int  m_animation_state;
    int  m_move_animation_state;
    POINT m_moveanimation;

    int wait_timer;
    Ticker m_ticker;
    float alpha;

public:
    DECLARE_WND_CLASS(NULL)
    PopupWindow(CFont *font) : m_font(font),
        m_animation_state(ANIMATION_NONE), m_move_animation_state(ANIMATION_NONE),
        wait_timer(0), alpha(0)
    {
        m_dc_size.cx = 0;
        m_dc_size.cy = 0;
        m_moveanimation.x = 0;
        m_moveanimation.y = 0;
    }
    void setText(const u8string& text)
    {
        m_text.assign( TU2W(text.c_str()) );
        calcDCSize();
    }
    const SIZE& getSize() const
    {
        CSize sz(m_dc_size);
        sz += CSize(18,16);
        return sz;
    }
    bool isAnimated()
    {
        return (m_animation_state==ANIMATION_NONE) ? false : true;
    }
    void startAnimation(const Animation& a);
    void startMoveAnimation(POINT newpos);

private:
    void onCreate();
    void onTimer();
    void onPaint(HDC dc);
    void setState(int newstate);
    void calcDCSize();
    void setAlpha(float a);
    void onClickButton();

private:
    BEGIN_MSG_MAP(PopupWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&) { onTimer(); return 0; }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&) { return 1;  }
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&) 
    {
        CPaintDC dc(m_hWnd);
        onPaint(dc);
        return 0; 
    }
    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&) { onClickButton(); return 0;  }
};
