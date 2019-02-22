#pragma once

class HighlightSelectColor :  public CWindowImpl<HighlightSelectColor, CButton, CControlWinTraits>
{
    COLORREF m_color;
    CBrush m_backgroundBrush;
    CBrush m_disabledStateBrush;
    COLORREF m_crShadow;
	COLORREF m_crDarkShadow;
	COLORREF m_crHiLight;
    UINT m_msg;

public:
    HighlightSelectColor(COLORREF color, UINT msg) :
      m_color(color), m_msg(msg) {}
    DECLARE_WND_SUPERCLASS(NULL, CButton::GetWndClassName())	

    void setColor(COLORREF color)
    {
        m_color = color;
        initBackground();
        Invalidate();
    }

    COLORREF getColor() const
    {
        return m_color;
    }

private:
    BEGIN_MSG_MAP(HighlightSelectColor)
       MESSAGE_HANDLER(WM_CREATE, OnCreate)
       MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
       MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouseClick)
       MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseClick)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        initBackground();
        SetButtonStyle(BS_OWNERDRAW);
        m_crShadow = GetSysColor(COLOR_3DSHADOW);
        m_crDarkShadow = GetSysColor(COLOR_3DDKSHADOW);
        m_crHiLight = GetSysColor(COLOR_3DHILIGHT);
        m_disabledStateBrush.CreateSysColorBrush(COLOR_BTNFACE);
        return 0;
    }

    LRESULT OnDrawItem(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
    {
        DRAWITEMSTRUCT* lpDIS = (DRAWITEMSTRUCT*)lParam;
        const RECT &rc = lpDIS->rcItem;

	    int	nArrowWidth	= GetSystemMetrics(SM_CXHSCROLL);
	    bool bSelected = (ODS_SELECTED & lpDIS->itemState) == ODS_SELECTED;
	    CDCHandle dc = lpDIS->hDC;
        if (!IsWindowEnabled())
        {
            FillRect(dc, &rc, m_disabledStateBrush);
            dc.Draw3dRect(&rc, m_crHiLight, m_crDarkShadow);
            return 0;            
        }
        FillRect(dc, &rc, m_backgroundBrush);
        if (bSelected)
	    {
		    dc.Draw3dRect(&rc, m_crDarkShadow, m_crHiLight);
    	    CRect rcBorder(rc);
		    rcBorder.DeflateRect(1, 1);
		    dc.Draw3dRect(rcBorder, m_crShadow, m_color);
	    }
	    else	
	    {
		    dc.Draw3dRect(&rc, m_crHiLight, m_crDarkShadow);
		    CRect rcBorder(rc);
		    rcBorder.DeflateRect(1, 1);
		    dc.Draw3dRect(rcBorder, m_color, m_crShadow);
	    }        
        return 0;
    }

    LRESULT OnMouseClick(UINT uMsg, WPARAM, LPARAM, BOOL&)
    {           
        ::SendMessage(GetParent(), m_msg, 0, 0);
        return 0;
    }

    void initBackground()
    {
        if (!m_backgroundBrush.IsNull())
            m_backgroundBrush.DeleteObject();
        m_backgroundBrush = CreateSolidBrush(m_color);
    }
};


class HighlightsExampleWindow :  public CWindowImpl<HighlightsExampleWindow, CWindow>
{      
    PropertiesData *propData;    
    CBrush m_backgroundBrush;
    CFont  m_font;
    COLORREF m_textBackground;
    COLORREF m_textColor;
    bool m_italic;
    bool m_underlined;
    bool m_border;
    
public:
    HighlightsExampleWindow(PropertiesData *data) : propData(data), m_backgroundBrush(NULL),
        m_textBackground(RGB(0,0,0)), m_textColor(RGB(192,192,192)),
        m_italic(false), m_underlined(false), m_border(false) {}    
    
    void setTextColor(COLORREF color)
    {
        m_textColor = color;
        Invalidate();
    }

    void setBkgColor(COLORREF color)
    {
        m_textBackground = color;
        Invalidate();
    }
   
    void setUnderlined(bool underlined)
    {
        if (underlined == m_underlined) 
            return;
        m_underlined = underlined;
        initFont();
        Invalidate();
    }

    void setBorder(bool border)
    {
        if (border == m_border) 
            return;
        m_border = border;
        initFont();
        Invalidate();
    }

    void setItalic(bool italic)
    {
        if (italic == m_italic) 
            return;
        m_italic = italic;
        initFont();
        Invalidate();
    }

    void setAllParameters(COLORREF text, COLORREF background, bool underlined, bool border, bool italic)
    {
        m_textColor = text;
        m_textBackground = background;        
        m_underlined = underlined;
        m_border = border;
        m_italic = italic;
        initFont();
        Invalidate();
    }

    void updateProps()
    {
        COLORREF bkgnd = propData->bkgnd;
        if (m_backgroundBrush)
            m_backgroundBrush.DeleteObject();            
        m_backgroundBrush = CreateSolidBrush(bkgnd);
        initFont();
    }
 
private:
    BEGIN_MSG_MAP(ColorExampleWindow)
       MESSAGE_HANDLER(WM_CREATE, OnCreate)
       MESSAGE_HANDLER(WM_PAINT, OnPaint)
       MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBknd)          
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        updateProps();
        return 0;
    }

    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&)
    {        
        RECT pos;
        GetClientRect(&pos);
        CPaintDC dc(m_hWnd);
        dc.FillRect(&pos, m_backgroundBrush);

        HFONT oldfont=dc.SelectFont(m_font);
        SIZE size = { 0, 0 };
        tstring s(L"Пример текста");
        GetTextExtentPoint32(dc, s.c_str(), s.length(), &size);
        size.cx += 4;

        pos.left = (pos.right - size.cx) / 2;
        pos.right = pos.left + size.cx;
        pos.top = (pos.bottom - size.cy) / 2;
        pos.bottom = pos.top + size.cy;       
        
        dc.FillSolidRect(&pos, m_textBackground);
        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(m_textColor);     
        dc.DrawText(s.c_str(), -1, &pos, DT_CENTER|DT_SINGLELINE|DT_VCENTER);
        dc.SelectFont(oldfont);

        if (m_border)
        {
            CPen p;
            p.CreatePen(PS_SOLID, 1, m_textColor);
            HPEN oldpen = dc.SelectPen(p);
            MoveToEx(dc, pos.left, pos.top, NULL);
            LineTo(dc, pos.right-1, pos.top); LineTo(dc, pos.right-1, pos.bottom-1);
            LineTo(dc, pos.left, pos.bottom-1); LineTo(dc, pos.left, pos.top);                
            dc.SelectPen(oldpen);
        }
        return 0;
    }

    LRESULT OnEraseBknd(UINT, WPARAM, LPARAM, BOOL&)
    {
        return 1;
    }

private:
    void initFont()
    {
        LOGFONT logfont;
        propData->initLogFont(m_hWnd, &logfont);
        if (!m_font.IsNull())
            m_font.DeleteObject();
        if (m_underlined)
            logfont.lfUnderline = 1;
        if (m_italic)
        {
            if (logfont.lfItalic)
                logfont.lfItalic = 0;
            else
                logfont.lfItalic = 1;
        }
        m_font.CreateFontIndirect(&logfont);
    }
};

