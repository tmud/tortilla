#pragma once

struct Animation
{
    POINT start_pos;
    POINT pos;
    POINT end_pos;
    SIZE  window_size;
    float speed;
};

class PopupWindow : public CWindowImpl<PopupWindow>
{
    CFont *m_font;
    std::wstring m_text;
    SIZE m_dc_size;
    Animation m_animation;
    bool m_animated;

public:
    PopupWindow(const u8string& text, CFont *font) : m_font(font), m_animated(false)
    {
        m_text.assign( TU2W(text.c_str()) );
        m_dc_size.cx = 0;
        m_dc_size.cy = 0;            
    }
    const SIZE& getSize() const
    {
        CSize sz(m_dc_size);
        sz += CSize(4,4);
        return sz;
    }
    const Animation& getAnimation() const
    {
        return m_animation;
    }
    void startAnimation(const Animation& a)
    {
        m_animation = a;
        m_animated = true;
    }
    bool isAnimated()
    {
        return m_animated;
    }

private:
    void onCreate();
    void calcDCSize();

private:
    BEGIN_MSG_MAP(PopupWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
};
