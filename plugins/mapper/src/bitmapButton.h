#pragma once
#pragma once

class CBitmapButtonEx : public CBitmapButtonImpl<CBitmapButtonEx>
{
    bool m_pushed;
public:
	DECLARE_WND_SUPERCLASS(_T("WTLBitmapButton"), GetWndClassName())
	// added border style (auto3d_single)
	CBitmapButtonEx(DWORD dwExtendedStyle = BMPBTN_AUTOSIZE | BMPBTN_AUTO3D_SINGLE) : 
		CBitmapButtonImpl<CBitmapButtonEx>(dwExtendedStyle, NULL), m_pushed(false)
	{ }
	BEGIN_MSG_MAP(CBitmapButtonEx)
		CHAIN_MSG_MAP(CBitmapButtonImpl<CBitmapButtonEx>)
	END_MSG_MAP()

    void SetPushed(bool state)
    {
        m_fPressed = (state) ? 1 : 0;
    }

	// override of CBitmapButtonImpl DoPaint(). Adds fillrect
	void DoPaint(CDCHandle dc)
	{
		// added to resolve image artifacts
		RECT rc;
		GetClientRect(&rc);
		dc.FillRect(&rc, (HBRUSH)(COLOR_BTNFACE+1));
		
        // added to remove focus rect
		unsigned state = m_fFocus;
        m_fFocus = 0;
        /*if (m_pushed)
            m_fPressed = 1;*/
		CBitmapButtonImpl<CBitmapButtonEx>::DoPaint(dc);
        m_fFocus = state;
       // if (m_pushed)
         //   m_fPressed = 0;
	}
};
