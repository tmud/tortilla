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

struct MoveAnimation
{
     POINT pos;
     float speed;
};

class PopupWindow : public CWindowImpl<PopupWindow>
{
    CFont *m_font;
    std::wstring m_text;
    CSize m_dc_size;

    Animation m_animation;
    MoveAnimation m_move_animation;
    int  m_animation_state;

    int wait_timer;
    Ticker m_ticker;
    float alpha;
    float m_move_dx, m_move_dy;

public:
    enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT, ANIMATION_MOVE };    
    DECLARE_WND_CLASS(NULL)
    PopupWindow(CFont *font) : m_font(font),
        m_animation_state(ANIMATION_NONE),
        wait_timer(0), alpha(0), m_move_dx(0), m_move_dy(0)
    {
    }
    void setText(const u8string& text)
    {
        m_text.assign( TU2W(text.c_str()) );
        calcDCSize();
    }
    const SIZE& getSize() const
    {
        CSize sz(m_dc_size);
        sz += CSize(18,14);
        return sz;
    }

    bool isAnimated() const {  return (m_animation_state==ANIMATION_NONE) ? false : true; }
    int  getAnimationType() const { return m_animation_state; }
    const Animation& getAnimation() const { return m_animation; }
    const MoveAnimation& getMoveAnimation() const { return m_move_animation; }
    void startAnimation(const Animation& a);
    void startMoveAnimation(const MoveAnimation& a);

private:
    void onCreate();
    void onTimer();
    void onPaint(HDC dc);
    void setState(int newstate);
    void calcDCSize();
    void setAlpha(float a);
    void onClickButton();
    void sendNotify();
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
