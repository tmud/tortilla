#pragma once
#include "sharingData.h"

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
    SharingWindow pos;
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
    std::wstring m_original_text;
    std::vector<std::wstring> m_text;
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

    bool isAnimated() const {  return (m_animation_state==ANIMATION_NONE) ? false : true; }
    int  getAnimationState() const { return m_animation_state; }
    const Animation& getAnimation() const { return m_animation; }
    const MoveAnimation& getMoveAnimation() const { return m_move_animation; }
    void startAnimation(const Animation& a);
    void startMoveAnimation(const MoveAnimation& a);
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
