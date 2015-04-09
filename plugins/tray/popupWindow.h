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
    POINT start_pos;
    POINT end_pos;
    SIZE  window_size;
    float speed;
    int   timer_msec;
    int   wait_sec;
    COLORREF bkgnd_color;
    COLORREF text_color;
};

class PopupWindow : public CWindowImpl<PopupWindow>
{
    CFont *m_font;
    std::wstring m_text;
    SIZE m_dc_size;
    Animation m_animation;
    int  m_animation_state;
    enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT };   
    float pos_x, pos_y;
    float dx, dy;
    int wait_timer;
    Ticker m_ticker;

public:
    DECLARE_WND_CLASS(NULL)
    PopupWindow(CFont *font) : m_font(font), m_animation_state(ANIMATION_NONE), wait_timer(0), pos_x(0), pos_y(0), dx(0), dy(0)
    {
        m_dc_size.cx = 0;
        m_dc_size.cy = 0;
    }
    void setText(const u8string& text)
    {
        m_text.assign( TU2W(text.c_str()) );
        calcDCSize();
    }
    const SIZE& getSize() const
    {
        CSize sz(m_dc_size);
        sz += CSize(8,8);
        return sz;
    }
    bool isAnimated()
    {
        return (m_animation_state==ANIMATION_NONE) ? false : true;
    }
    void startAnimation(const Animation& a);

private:
    void onCreate();
    void onTimer();
    void onPaint(HDC dc);
    void setState(int newstate);
    void calcDCSize();
    void initAnimationVals();
    void moveWindow();

private:
    BEGIN_MSG_MAP(PopupWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
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
};
