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

    rc.DeflateRect(1, 1);
    
     /*UINT state = DFCS_BUTTONPUSH;
        if (m_pushed || m_selected) 
            state |= DFCS_PUSHED;
        dc.DrawFrameControl(&rc,DFC_BUTTON, state);*/
    /*if (m_pushed || m_selected)
       dc.DrawFrameControl(&rc,DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);*/
        
    if (m_image && !m_image->empty())
    {
       int x = 0; //(rc.right - m_image->width()) / 2;
       int y = 0; //(rc.bottom - m_image->height()) / 2;
       if (m_pushed || m_selected)
           m_image->renderpushed(dc, x, y, rc.right+1, rc.bottom+1 );
       else
           m_image->render(dc, x, y, rc.right+1, rc.bottom+1);
    } else {
        POINT pt = { 0, 0 };
        dc.RoundRect(&rc, pt);
    }

    /*if (!m_text.empty())
    {
        int len = m_text.length(); 
        if (len > bufferlen) len = bufferlen;
        wcsncpy(buffer, m_text.c_str(), len);
        rc.left+=2; rc.right-=2;
        dc.SetBkMode(TRANSPARENT);
        dc.DrawTextEx(buffer, len, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }*/
}
