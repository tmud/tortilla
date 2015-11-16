#include "stdafx.h"
#include "padbutton.h"

WCHAR PadButton::buffer[bufferlen];


void PadButton::onPaint()
{
    CRect rc; 
    GetClientRect(&rc);
    CPaintDC pdc(m_hWnd);
    CMemoryDC dc(pdc, rc);

    dc.FillSolidRect(&rc, m_background_color);
    //rc.DeflateRect(1, 1);
 
    if (m_image && !m_image->empty())
    {
       if (m_pushed || m_selected)
           m_image->renderpushed(dc, 0, 0, rc.right, rc.bottom );
       else
           m_image->render(dc, 0, 0, rc.right, rc.bottom);
    } else
    {
        POINT pt = {1,1};
        if (m_pushed || m_selected) { 
          rc.left += 1; rc.top += 1;
        }        
        dc.FillSolidRect(&rc, GetSysColor(COLOR_BTNFACE));
        if (!m_text.empty())
        {
            HFONT old_font = NULL;
            if (m_font)
                old_font = dc.SelectFont(m_font);
            int len = m_text.length();
            if (len > bufferlen) len = bufferlen;
            wcsncpy(buffer, m_text.c_str(), len);
            rc.left += 2; rc.right -= 2;
            if (m_pushed || m_selected)
                rc.left += 1; rc.top += 1;
            dc.SetBkMode(TRANSPARENT);
            dc.DrawTextEx(buffer, len, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            if (old_font)
                dc.SelectFont(old_font);
        }
    }
}
