﻿#include "stdafx.h"
#include "popupWindow.h"

 bool PopupWindow::create(CFont *font)
{
     m_font = font;
     Create(GetDesktopWindow(), CWindow::rcDefault, NULL, WS_POPUP, WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_NOACTIVATE);
     return (IsWindow()) ? true : false;
}

void PopupWindow::setText(const Msg& msg, int timeout)
{
    const std::wstring& text = msg.text;
    m_original_text = text;

    Animation &a = m_animation;
    a.speed = 0.4f;
    a.move_speed = 0.004f;
    a.wait_sec = timeout;
    a.bkgnd_color = msg.bkgndcolor;
    a.text_color = msg.textcolor;

    m_text.resize(1);
    m_text[0].assign(text);
    SIZE dc_size = calcDCSize();
    int dx = dc_size.cx;
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
            for (; len > 0; --len)
                parts.push_back(L" ");
        }

        int k = 0;
        for (int i = 0; i < count - 1; ++i)
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
        for (size_t i = k; i < parts.size(); ++i)
            str.append(parts[i]);
        trimleft(&str);
        if (str.empty())
            m_text.pop_back();
        else {
            int last = count - 1;
            m_text[last] = str;
        }
        dc_size = calcDCSize();
    }
    a.pos.w = dc_size.cx + 12;
    a.pos.h = dc_size.cy + 8;
    a.pos.x = a.pos.y = 0;
    wait_timer = 0;
}

void PopupWindow::startAnimation(const SharingWindow& startpos)
{
    m_animation.pos = startpos;
    m_destination = startpos;

    fillSrcDC();
    const Animation &a = m_animation;
    const SharingWindow &sw = a.pos;
    MoveWindow(sw.x, sw.y, sw.w, sw.h);

    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = 0;

    CWindow dw(GetDesktopWindow());
    CDC dstdc(dw.GetDC());
    POINT dstpt = {sw.x, sw.y};
    SIZE sz =  { sw.w, sw.h };
    POINT srcpt = { 0, 0 };
    UpdateLayeredWindow(m_hWnd, (HDC)dstdc, &dstpt, &sz, m_src_dc, &srcpt, 0, &blend, ULW_ALPHA);
    ShowWindow(SW_SHOWNA);
    m_ticker.sync();
    m_animation.state =  Animation::ANIMATION_FADE_UP;
}

bool PopupWindow::canMove() const
{
    return (m_animation.state == Animation::ANIMATION_WAIT);
}

bool PopupWindow::isAnimated() const
{
    return (m_animation.state != Animation::ANIMATION_NONE);
}

void PopupWindow::wait()
{
    if (m_animation.state == Animation::ANIMATION_MOVE)
        m_animation.state = Animation::ANIMATION_WAIT;
}

void PopupWindow::moveTo(const SharingWindow& pos)
{
    const SharingWindow &p = m_animation.pos;
    m_move_dx = static_cast<float>(pos.x - p.x);
    m_move_dy = static_cast<float>(pos.y - p.y);
    m_destination = pos;
    m_animation.state = Animation::ANIMATION_MOVE;
}

void PopupWindow::tick(int id)
{
    DWORD dt = m_ticker.getDiff();
    m_ticker.sync();

    const Animation::AnimationState& state = m_animation.state;
    if (state == Animation::ANIMATION_NONE)
        return;
    if (state == Animation::ANIMATION_FADE_UP)
    {
        float da = static_cast<float>(dt) * m_animation.speed;
        alpha = min(alpha+da, 255.0f);
        setAlpha(alpha);
        if (alpha == 255.0f)
            m_animation.state = Animation::ANIMATION_WAIT;            
        return;
    }
    if (state == Animation::ANIMATION_FADE_DOWN)
    {
        float da = static_cast<float>(dt) * m_animation.speed;
        alpha = max(alpha-da, 0.0f);
        setAlpha(alpha);
        if (alpha == 0.0f)
            m_animation.state =  Animation::ANIMATION_NONE;
        return;
    }

    bool move_animation = false;
    if (state == Animation::ANIMATION_WAIT || state == Animation::ANIMATION_MOVE)
    {
        /*char buffer[256];
        sprintf(buffer, "%d=%d\r\n", id, wait_timer);
        OutputDebugStringA(buffer);*/

        wait_timer += dt;
        int end_timer = m_animation.wait_sec * 1000;
        if (wait_timer >= end_timer)
        {
            m_animation.state =  Animation::ANIMATION_FADE_DOWN;
        }
        else 
        {
            if (state == Animation::ANIMATION_MOVE && (m_move_dx > 0 || m_move_dy > 0))
                move_animation = true;
        }
    }
    if (move_animation)
    {
        const SharingWindow &curpos = m_animation.pos;
        POINT target = { m_destination.x, m_destination.y };

        float x = static_cast<float>(curpos.x);
        float y = static_cast<float>(curpos.y);

        float dx = m_move_dx * m_animation.move_speed * dt;
        float dy = m_move_dy * m_animation.move_speed * dt;

        POINT p = { 0, 0 };

        float ax = abs(dx); float ay = abs(dy);
        if (ax > 18 || ay > 18  || (ax > 0 && ax < 1) || (ay > 0 && ay < 1))
        {
            p.x = target.x;
            p.y = target.y;
        }
        else
        {
            p.x = static_cast<LONG>(x+dx);
            p.y = static_cast<LONG>(y+dy);
        }

        bool stop = false;
        if ((m_move_dx < 0 && p.x <= target.x) || (m_move_dx > 0 && p.x >= target.x))
            { p.x = target.x; stop = true; }
        if ((m_move_dy < 0 && p.y <= target.y) || (m_move_dy > 0 && p.y >= target.y))
            { p.y = target.y; stop = true; }
        if (!stop && p.x == target.x && p.y == target.y)
            { stop = true; }
        SIZE sz = { m_animation.pos.w, m_animation.pos.h };
        RECT pos = { p.x, p.y, p.x+sz.cx, p.y+sz.cy };
        MoveWindow(&pos);
        SharingWindow &w = m_animation.pos;
        w.x = pos.left;
        w.y = pos.top;
        w.w = pos.right-pos.left;
        w.h = pos.bottom - pos.top;
        if (stop)
        {
            m_animation.state =  Animation::ANIMATION_WAIT;
        }
        return;
    }
}

SIZE PopupWindow::calcDCSize()
{
    assert(m_font);
    CDC dc(GetDC());
    HFONT oldfont = dc.SelectFont(*m_font);
    SIZE size = { 0, 0 };
    SIZE dc_size = size;
    for (int i = 0, e=m_text.size(); i<e; ++i)
    {
        const std::wstring& t = m_text[i];
        if (t.empty()) continue;
        GetTextExtentPoint32(dc, t.c_str(), t.length(), &size);
        if (size.cx > dc_size.cx)
            dc_size.cx = size.cx;
    }
    dc_size.cy = m_text.size() * size.cy;
    dc.SelectFont(oldfont);
    return dc_size;
}

void PopupWindow::fillSrcDC()
{
    assert(m_font);
    SIZE sz = { m_animation.pos.w, m_animation.pos.h };
    CDC m_wnd_dc(GetDC());
    m_src_dc.create(m_wnd_dc, sz);
    CDCHandle pdc(m_src_dc);
    RECT rc = { 0, 0, sz.cx, sz.cy};
    pdc.FillSolidRect(&rc, m_animation.bkgnd_color);
    HFONT old_font = pdc.SelectFont(*m_font);
    pdc.SetBkColor(m_animation.bkgnd_color);
    pdc.SetTextColor(m_animation.text_color);
    CPen p;
    p.CreatePen(PS_SOLID, 2, m_animation.text_color);
    HPEN old = pdc.SelectPen(p);
    pdc.MoveTo(rc.left+1, rc.top+1);
    pdc.LineTo(rc.right-1, rc.top+1);
    pdc.LineTo(rc.right-1, rc.bottom-1);
    pdc.LineTo(rc.left+1, rc.bottom-1);
    pdc.LineTo(rc.left+1, rc.top);
    pdc.SelectPen(old);

    int count = m_text.size();
    int dc_size_y = m_animation.pos.h;
    if (count > 0)
    {
        rc.left = 4; rc.right-=4;
        int dy = (dc_size_y / count);
        rc.top = (sz.cy - dc_size_y) / 2;
        rc.bottom = rc.top + dy;
        for (int i=0;i<count;++i)
        {
            const std::wstring &t = m_text[i];
            pdc.DrawText(t.c_str(), t.length(), &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            rc.top = rc.bottom;
            rc.bottom = rc.top + dy;
        }
    }
    pdc.SelectFont(old_font);
}

void PopupWindow::setAlpha(float a)
{
    BYTE va = static_cast<BYTE>(a);
    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = va;
    UpdateLayeredWindow(m_hWnd, NULL, NULL, NULL, NULL, NULL, 0, &blend, ULW_ALPHA);
    UpdateWindow();
}

void PopupWindow::onClickButton()
{
    m_animation.state = Animation::ANIMATION_FADE_DOWN;
}

void PopupWindow::trimleft(std::wstring* s)
{
    size_t pos = wcsspn(s->c_str(), L" ");
    if (pos != 0)
        s->assign(s->substr(pos));
}