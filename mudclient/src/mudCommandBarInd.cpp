#include "stdafx.h"
#include "mudCommandBarInd.h"

//CFont MudCommandBarIndicator::m_font;
CPen MudCommandBarIndicator::m_pen1;
CPen MudCommandBarIndicator::m_pen2;
CBrush MudCommandBarIndicator::m_brush1;
CBrush MudCommandBarIndicator::m_brush2;

const COLORREF disable = RGB(255,0,0);
const COLORREF enable = RGB(0,255,0);

void MudCommandBarIndicator::onCreate()
{
    if (m_pen1.IsNull())
        m_pen1.CreatePen(PS_SOLID, 1, disable);
    if (m_pen2.IsNull())
        m_pen2.CreatePen(PS_SOLID, 1, enable);
    if (m_brush1.IsNull())
        m_brush1.CreateSolidBrush(disable);
    if (m_brush2.IsNull())
        m_brush2.CreateSolidBrush(enable);
}

void MudCommandBarIndicator::onPaint()
{
    if (m_font.IsNull())
        return;

    RECT pos;
    GetClientRect(&pos);
    CPaintDC dc(m_hWnd);
    CMemoryDC mdc(dc, pos);
    mdc.FillRect(&pos, GetSysColorBrush(COLOR_BTNFACE));
    mdc.SetBkMode(TRANSPARENT);
    mdc.SetTextColor(GetSysColor(COLOR_BTNTEXT));
    HFONT old = mdc.SelectFont(m_font);
    mdc.DrawTextEx(buffer, buffer_len, &pos, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    pos.left = width - height - 6;
    pos.right = pos.left + height;
    pos.top = (pos.bottom - height) / 2;
    pos.bottom = pos.top + height;
    mdc.SelectPen((m_status) ? m_pen2 : m_pen1);
    mdc.Ellipse(&pos);
    mdc.SelectBrush((m_status) ? m_brush2 : m_brush1);
    mdc.FloodFill(pos.left+height/2, pos.top+height/2, (m_status) ? enable : disable );
    mdc.SelectFont(old);
}

void MudCommandBarIndicator::calcWidth() 
{
    if (m_font.IsNull()) {
        width = 0;
        return;
    }
    CDC dc(GetDC());
    HFONT old = dc.SelectFont(m_font);
    SIZE sz = { 0, 0 };
    GetTextExtentPoint32(dc, buffer, buffer_len, &sz);
    dc.SelectFont(old);
    width = sz.cx + sz.cy + 8;
    height = sz.cy;
}

void MudCommandBarIndicator::createFont(const tstring& fontname) {

    if (!m_font.IsNull())
        m_font.DeleteObject();
    LOGFONT lf;
    lf.lfHeight = -MulDiv(10, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = FW_NORMAL;
    lf.lfItalic = 0;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH;
    wcscpy(lf.lfFaceName, fontname.c_str());
    m_font.CreateFontIndirect(&lf);
}