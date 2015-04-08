#include "stdafx.h"
#include "popupWindow.h"

void PopupWindow::onCreate()
{
    calcDCSize();
}

void PopupWindow::calcDCSize()
{
    CDC dc(GetDC());
    HFONT oldfont = dc.SelectFont(*m_font);
    GetTextExtentPoint32(dc, m_text.c_str(), m_text.length(), &m_dc_size);
    dc.SelectFont(oldfont);
}
