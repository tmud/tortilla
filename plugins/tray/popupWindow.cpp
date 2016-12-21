#include "stdafx.h"
#include "popupWindow.h"

void PopupWindow::onTick()
{
    DWORD dt = m_ticker.getDiff();
    m_ticker.sync();
    if (m_animation_state == ANIMATION_MOVE)
    {
        const SharingWindow &curpos = m_animation.pos;
        const POINT& target = m_move_animation.pos;

        float x = static_cast<float>(curpos.x);
        float y = static_cast<float>(curpos.y);

        float dx = m_move_dx * m_move_animation.speed * dt;
        float dy = m_move_dy * m_move_animation.speed * dt;

        POINT p = { 0, 0 };

        float ax = abs(dx); float ay = abs(dy);
        if (ax > 6 || ay > 6  || (ax > 0 && ax < 1) || (ay > 0 && ay < 1))
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
        SIZE sz = getSize();
        RECT pos = { p.x, p.y, p.x+sz.cx, p.y+sz.cy };
        MoveWindow(&pos);
        if (stop)
        {
            setState(ANIMATION_WAIT);
            sendNotify(MOVEANIMATION_FINISHED);
        }
    }

    if (m_animation_state == ANIMATION_NONE)
    {
        assert(false);
        return;
    }

    if (m_animation_state == ANIMATION_WAIT)
    {
        wait_timer += dt;
        int end_timer = m_animation.wait_sec * 1000;
        if (wait_timer >= end_timer)
            setState(ANIMATION_TOSTART);
        return;
    }
    float da = static_cast<float>(dt) * m_animation.speed;
    if (m_animation_state == ANIMATION_TOEND)
    {
        alpha = min(alpha+da, 255.0f);
        setAlpha(alpha);
        if (alpha == 255.0f)
        {
            setState(ANIMATION_WAIT);
            sendNotify(STARTANIMATION_FINISHED);
        }
    }
    if (m_animation_state == ANIMATION_TOSTART)
    {
        alpha = max(alpha-da, 0.0f);
        setAlpha(alpha);
        if (alpha == 0.0f)
            setState(ANIMATION_NONE);
    }
}

void PopupWindow::startAnimation(const Animation& a)
{
    m_animation = a;
    setState(ANIMATION_TOEND);
}

void PopupWindow::startMoveAnimation(const MoveAnimation& a)
{
    const SharingWindow &p = m_animation.pos;
    m_move_dx = static_cast<float>(a.pos.x - p.x);
    m_move_dy = static_cast<float>(a.pos.y - p.y);
    m_move_animation = a;
    setState(ANIMATION_MOVE);
}

void PopupWindow::setState(int newstate)
{
    const Animation &a = m_animation;
    m_animation_state = newstate;

    switch(m_animation_state)
    {
    case ANIMATION_TOEND:
    {
        fillSrcDC();

        BLENDFUNCTION blend;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.AlphaFormat = 0;
        blend.SourceConstantAlpha = 0;

        CWindow dw(GetDesktopWindow());
        CDC dstdc(dw.GetDC());
        POINT dstpt = {a.pos.x, a.pos.y};
        SIZE sz = getSize();
        POINT srcpt = {0,0};
        UpdateLayeredWindow(m_hWnd, (HDC)dstdc, &dstpt, &sz, m_src_dc, &srcpt, 0, &blend, ULW_ALPHA);
        ShowWindow(SW_SHOWNA);
    }
    break;
    case ANIMATION_NONE:

        ShowWindow(SW_HIDE);
        m_src_dc.destroy();
        wait_timer = 0;
        alpha = 0;
        sendNotify(ANIMATION_FINISHED);
    break;
    }
    m_ticker.sync();
}

void PopupWindow::calcDCSize()
{
    assert(m_font);
    CDC dc(GetDC());
    HFONT oldfont = dc.SelectFont(*m_font);
    CSize size(0, 0);
    m_dc_size = size;
    for (int i = 0, e=m_text.size(); i<e; ++i)
    {
        const std::wstring& t = m_text[i];
        if (t.empty()) continue;
        GetTextExtentPoint32(dc, t.c_str(), t.length(), &size);
        if (size.cx > m_dc_size.cx)
            m_dc_size.cx = size.cx;
    }
    m_dc_size.cy = m_text.size() * size.cy;
    dc.SelectFont(oldfont);
}

void PopupWindow::fillSrcDC()
{
    assert(m_font);
    SIZE sz = getSize();
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
    if (count > 0)
    {
        rc.left = 4; rc.right-=4;
        int dy = (m_dc_size.cy / count);
        rc.top = (sz.cy - m_dc_size.cy) / 2;
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
    setState(ANIMATION_TOSTART);
    sendNotify(CLICK_EVENT);
}

void PopupWindow::sendNotify(int state)
{
     if (m_animation.notify_wnd)
            ::PostMessage(m_animation.notify_wnd, m_animation.notify_msg, m_animation.notify_param, state);
}