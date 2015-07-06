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

class TempDC 
{
public:
    TempDC() : m_old_bitmap(NULL) {}
    ~TempDC() { clear(); }
    void create(HDC dc, SIZE sz)
    {
        clear();
        m_temp_dc.CreateCompatibleDC(dc);
        m_temp_bitmap.CreateCompatibleBitmap(dc, sz.cx, sz.cy);
        m_old_bitmap = m_temp_dc.SelectBitmap(m_temp_bitmap);
    }
    void destroy() { clear(); }
    operator HDC() const { return m_temp_dc; }

private:
    void clear() {
        if (m_temp_bitmap != NULL) {
        m_temp_dc.SelectBitmap(m_old_bitmap);
        m_temp_bitmap.DeleteObject();
        m_temp_dc.DeleteDC();
        }
        m_temp_bitmap= NULL;
    }
    CDC m_temp_dc;
    CBitmap m_temp_bitmap;
    HBITMAP m_old_bitmap;
};

class PopupWindow : public CWindowImpl<PopupWindow>
{
    CFont *m_font;
    std::wstring m_text;
    CSize m_dc_size;
    TempDC m_src_dc;

    Animation m_animation;
    MoveAnimation m_move_animation;
    int  m_animation_state;

    int wait_timer;
    Ticker m_ticker;
    float alpha;
    float m_move_dx, m_move_dy;

public:
    enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT, ANIMATION_MOVE };
    enum { ANIMATION_FINISHED = 0, MOVEANIMATION_FINISHED, STARTANIMATION_FINISHED, CLICK_EVENT };
    DECLARE_WND_CLASS(NULL)
    PopupWindow() : m_font(NULL),
        m_animation_state(ANIMATION_NONE),
        wait_timer(0), alpha(0), m_move_dx(0), m_move_dy(0)
    {
    }
    ~PopupWindow() {
        if (IsWindow()) DestroyWindow(); 
    }

    bool create(CFont *font)
    {
        m_font = font;
        Create(GetDesktopWindow(), CWindow::rcDefault, NULL, WS_POPUP, WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_LAYERED/*|WS_EX_NOPARENTNOTIFY*/);
        return (IsWindow()) ? true : false;    
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
    int  getAnimationState() const { return m_animation_state; }
    const Animation& getAnimation() const { return m_animation; }
    const MoveAnimation& getMoveAnimation() const { return m_move_animation; }
    void startAnimation(const Animation& a);
    void startMoveAnimation(const MoveAnimation& a);
    void onTick();

private:
    void onTimer();
    void fillDC();
    void setState(int newstate);
    void calcDCSize();
    void setAlpha(float a);
    void onClickButton();
    void sendNotify(int state);
private:
    BEGIN_MSG_MAP(PopupWindow)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
    END_MSG_MAP()
    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&) { onClickButton(); return 0;  }
};
