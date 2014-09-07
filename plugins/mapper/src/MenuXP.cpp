#include "stdafx.h"
#include "MenuXP.h"

// constants used for drawing
const DWORD CXGAP = 0;			   // num pixels between button and text
const DWORD CXTEXTMARGIN = 2;      // num pixels after hilite to start text
const DWORD CXBUTTONMARGIN = 2;	   // num pixels wider button is than bitmap
const DWORD CYBUTTONMARGIN = 2;	   // ditto for height

// DrawText flags
const DWORD DT_MYSTANDARD = DT_SINGLELINE | DT_LEFT | DT_VCENTER;

class CMenuXPSeparator : public CMenuXPItem
{
public:
    CMenuXPSeparator() {  m_bSeparator = true; }
};

CMenuXP::CMenuXP()
{
	//initialize menu font with the default
	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
	m_fontMenu.CreateFontIndirect(&info.lfMenuFont);

	//initialize colors with system default
	m_clrBackGround = ::GetSysColor(COLOR_MENU);
	m_clrSelectedBar = ::GetSysColor(COLOR_HIGHLIGHT);
	m_clrSelectedText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_clrText = ::GetSysColor(COLOR_MENUTEXT);
	m_clrDisabledText = ::GetSysColor(COLOR_GRAYTEXT);
	m_clrIconArea = m_clrBackGround;

	//initialize sidebar colors
	m_clrSideBarStart = RGB(0, 0, 192);
	m_clrSideBarEnd = RGB(0, 0, 0);

	//the default sytle is office style
	m_Style = STYLE_OFFICE;

	m_bBreak = false;
	m_bBreakBar = false;
	m_hBitmap = NULL;
}

CMenuXP::~CMenuXP()
{
	Clear();
}

void CMenuXP::MeasureItem( LPMEASUREITEMSTRUCT lpms )
{
	if (lpms->CtlType != ODT_MENU)
		return;

	CMenuXPItem	*pItem = (CMenuXPItem *)lpms->itemData;    
    //ATLTRACE("pItem: 0x%x",(DWORD)pItem);	//This line prevent boundschecker from issue a resource leak

	if (!pItem || !pItem->IsMyData())
		return;

	if (pItem->m_bSeparator)
	{
		// separator: use half system height and zero width
		lpms->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK)>>1;
		lpms->itemWidth  = 0;
	}
	else
	{
		//calculate the size needed to draw the text: use DrawText with DT_CALCRECT

		CWindowDC dc(NULL);	// screen DC--I won't actually draw on it
		CRect rcText(0,0,0,0);

		//Calculate the size with bold font, for default item to be correct
		CFont	fontBold;
		LOGFONT	logFont;
		m_fontMenu.GetLogFont(&logFont);
		logFont.lfWeight = FW_BOLD;
		fontBold.CreateFontIndirect(&logFont);
		
		HFONT oldFont = dc.SelectFont(fontBold);
//		oldFont = dc.SelectFont(m_fontMenu);
		dc.DrawText(pItem->m_strText.c_str(), pItem->m_strText.length(), &rcText, DT_MYSTANDARD|DT_CALCRECT);
        dc.SelectFont(oldFont);

		// the height of the item should be the maximun of the text and the button
        int size0 = pItem->m_nSize + (CYBUTTONMARGIN << 1);
		lpms->itemHeight = max(rcText.Height(), size0);

		if (pItem->m_bButtonOnly)
		{	//for button only style, we set the item's width to be the same as its height
			lpms->itemWidth = lpms->itemHeight;
		}
		else
		{
			// width is width of text plus a bunch of stuff
			int cx = rcText.Width();	// text width 
			cx += CXTEXTMARGIN<<1;		// L/R margin for readability
			cx += CXGAP;					// space between button and menu text
			cx += (pItem->m_nSize + CYBUTTONMARGIN * 2) <<1;		// button width (L=button; R=empty margin)
			lpms->itemWidth = cx;		// done deal
		}
	}
	
	// whatever value I return in lpms->itemWidth, Windows will add the
	// width of a menu checkmark, so I must subtract to defeat Windows. Argh.
	lpms->itemWidth -= GetSystemMetrics(SM_CXMENUCHECK)-1;
	//ATLTRACE("MeasureItem: ID(%d), Width(%d), Height(%d)\n", lpms->itemID, lpms->itemWidth, lpms->itemHeight);
}

void CMenuXP::DrawItem( LPDRAWITEMSTRUCT lpds )
{
	assert(lpds);
	if (lpds->CtlType != ODT_MENU)
		return; // not handled by me
	CMenuXPItem * pItem = (CMenuXPItem *)lpds->itemData;
	if (!pItem)
		return;
	
	assert(lpds->itemAction != ODA_FOCUS);
	assert(lpds->hDC);
	CDC dc;
	dc.Attach(lpds->hDC);

	//get the drawing area
	CRect rcItem = lpds->rcItem;

	//ATLTRACE("DrawItem: ID(%d), Widht(%d),  Height(%d)\n", lpds->itemID, rcItem.Width(), rcItem.Height());

	if (pItem->m_bSeparator) 
	{
		//draw background first
		DrawBackGround(&dc, rcItem, FALSE, FALSE);
		// draw the background
		CRect rc = rcItem;								// copy rect
		rc.top += rc.Height()>>1;						// vertical center
		dc.DrawEdge(&rc, EDGE_ETCHED, BF_TOP);		// draw separator line
		
		// in XP mode, fill the icon area with the iconarea color
		if (m_Style == STYLE_XP)
		{
			CRect rcArea(rcItem.TopLeft(),
				CSize(pItem->m_nSize + (CYBUTTONMARGIN<<1), 
				pItem->m_nSize + (CYBUTTONMARGIN<<1)));
			DrawIconArea(&dc, rcArea, FALSE, FALSE, FALSE);
		}
	} 
	else
	{
		BOOL bDisabled = lpds->itemState & ODS_GRAYED;
		BOOL bSelected = lpds->itemState & ODS_SELECTED;
		BOOL bChecked  = lpds->itemState & ODS_CHECKED;

		//draw the background first
		DrawBackGround(&dc, rcItem, bSelected, bDisabled);
		
		//Draw the icon area for XP style
		if (m_Style == STYLE_XP)
		{
			CRect rcArea(rcItem.TopLeft(), CSize(rcItem.Height(), rcItem.Height()));
			DrawIconArea(&dc, rcArea, bSelected, bDisabled, bChecked);
		}

		//draw the button, not the icon
		CRect rcButton(rcItem.TopLeft(), CSize(rcItem.Height(), rcItem.Height()));
		if (pItem->m_bButtonOnly)
			rcButton = rcItem;
		if (/*pItem->IsImage() || */bChecked)
		{
			DrawButton(&dc, rcButton, bSelected, bDisabled, bChecked);
		}

		//draw the icon actually
        if (pItem->IsImage())
		{
            DrawBackGround(&dc, rcButton, bSelected, bDisabled);
			CRect rcIcon = rcButton;
			rcIcon.DeflateRect(2, 2);
			DrawIcon(&dc, rcIcon, pItem->m_hIcon, bSelected, bDisabled, bChecked);
		}
		else if (bChecked)	
		{
			//draw the check mark
			CRect	rcCheck = rcButton;
			rcCheck.DeflateRect(2, 2);
			DrawCheckMark(&dc, rcCheck, bSelected);
		}

		//draw text finally
		if (!pItem->m_bButtonOnly)
		{
			CRect rcText = rcItem;				 // start w/whole item
			rcText.left += rcButton.Width() + CXGAP + CXTEXTMARGIN; // left margin
			rcText.right -= pItem->m_nSize;				 // right margin
			DrawText(&dc, rcText, pItem->m_strText.c_str(), bSelected, bDisabled, lpds->itemState&ODS_DEFAULT ? 1 : 0);
		}
	}
	dc.Detach();
}

//draw background
void CMenuXP::DrawBackGround(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled)
{
	if (m_hBitmap && (!bSelected || bDisabled))
	{
		pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), m_memDC, 0, rect.top, SRCCOPY);
	}
	else if (bSelected)
	{
		FillRect(pDC, rect, bDisabled? ((m_Style==STYLE_XP)?m_clrBackGround:m_clrSelectedBar) : m_clrSelectedBar);
	}
	else
	{
		FillRect(pDC, rect, m_clrBackGround);
	}

	//in XP mode, draw a line rectangle around
	if (m_Style == STYLE_XP && bSelected && !bDisabled)
	{
		HBRUSH oldBrush = pDC->SelectStockBrush(HOLLOW_BRUSH);
		HPEN oldPen = pDC->SelectStockPen(BLACK_PEN);
		pDC->Rectangle(rect);
		pDC->SelectBrush(oldBrush);
		pDC->SelectPen(oldPen);
	}
}

//draw the icon button, the icon is not included
void CMenuXP::DrawButton(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled, BOOL bChecked)
{
	if (m_Style == STYLE_OFFICE)
	{
		// normal: fill BG depending on state
		if (bChecked && !bSelected)
		{
			FillRect(pDC, rect, GetSysColor(COLOR_3DHILIGHT));
		}
		else
			FillRect(pDC, rect, m_clrBackGround);
	
		// draw pushed-in or popped-out edge
		if (!bDisabled && (bSelected || bChecked) )
		{
			pDC->DrawEdge(rect, bChecked ? BDR_SUNKENOUTER : BDR_RAISEDINNER,
				BF_RECT);
		}
	}
	else if (m_Style == STYLE_XP && !bSelected)
	{
		if (bChecked && !bDisabled)
		{
			DrawBackGround(pDC, rect, TRUE, FALSE);
		}
	}	
}

//draw the icon area, the icon is not included, only in XP style
void CMenuXP::DrawIconArea(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled, BOOL bChecked)
{
	if (m_Style != STYLE_XP)
		return;

	// normal: fill BG depending on state
	if (!bSelected || bDisabled)
	{
		FillRect(pDC, rect, m_clrIconArea);
	}
}

//draw the icon
void CMenuXP::DrawIcon(CDC *pDC, CRect rect, HICON hIcon, BOOL bSelected, BOOL bDisabled, BOOL bChecked)
{
	if (bDisabled)
	{
		DrawEmbossed(pDC, hIcon, rect);
	}
	else
	{
		if(m_Style==STYLE_XP && bSelected && !bChecked)
		{
			DrawEmbossed(pDC, hIcon, rect, FALSE, TRUE);
			rect.OffsetRect(-1,-1);
		}
		::DrawIconEx(pDC->m_hDC, rect.left, rect.top, hIcon,
			rect.Width(), rect.Height(), 0, NULL,
			DI_NORMAL);
/*		::DrawIconEx(pDC->m_hDC, rect.left, rect.top, hIcon,
			rect.Width(), rect.Height(), 0, NULL,
			DI_NORMAL);
*/	}
}

//draw the check mark
void CMenuXP::DrawCheckMark(CDC *pDC, CRect rect, BOOL bSelected)
{
/*	//"#define OEMRESOURCE" must be in the begining of your stdafx.h
	//for the LoadOEMBitmap to work
#ifdef OEMRESOURCE
	CBitmap bmp;	//Check mark bitmap
	VERIFY(bmp.LoadOEMBitmap(OBM_CHECK));	

	// center bitmap in caller's rectangle
	BITMAP bm;
	bmp.GetBitmap(&bm);
	int cx = bm.bmWidth;
	int cy = bm.bmHeight;
	CRect rcDest = rect;
	CPoint p(0,0);
	CSize delta(CPoint((rect.Width() - cx)/2, (rect.Height() - cy)/2));
	if (rect.Width() > cx)
		rcDest = CRect(rect.TopLeft() + delta, CSize(cx, cy));
	else
		p -= delta;

	// select checkmark into memory DC
	CDC memdc;
	memdc.CreateCompatibleDC(pDC);
	CBitmap *pOldBmp = memdc.SelectObject(&bmp);

	COLORREF colorOld =
		pDC->SetBkColor(GetSysColor(bSelected ? COLOR_MENU : COLOR_3DLIGHT));
	pDC->BitBlt(rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(),
		&memdc, p.x, p.y, SRCCOPY);
	pDC->SetBkColor(colorOld);

	memdc.SelectObject(pOldBmp);
	bmp.DeleteObject();
#else
	CRect	rcDest = rect;
	pDC->DrawFrameControl(rcDest, DFC_MENU, DFCS_MENUCHECK);
#endif
*/
	//Draw it myself :(
	const int nCheckDots = 8;
	CPoint pt1, pt2, pt3;	//3 point of the checkmark
	pt1.x = 0;	// 5/18 of the rect width
	pt1.y = 3;	
	pt2.x = 2;
	pt2.y = 5;
	pt3.x = 7;
	pt3.y = 0;

	int xOff = (rect.Width()-nCheckDots)/2 + rect.left ;
	int yOff = (rect.Height()-nCheckDots)/2 + rect.top;
	pt1.Offset(xOff, yOff);
	pt2.Offset(xOff, yOff);
	pt3.Offset(xOff, yOff);

	CPen pen;
    pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN oldPen = pDC->SelectPen(pen);
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);
	pDC->LineTo(pt3);
	pt1.Offset(0, 1);
	pt2.Offset(0, 1);
	pt3.Offset(0, 1);
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);
	pDC->LineTo(pt3);
	pt1.Offset(0, 1);
	pt2.Offset(0, 1);
	pt3.Offset(0, 1);
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);
	pDC->LineTo(pt3);
    pDC->SelectPen(oldPen);
}

//Draw menu text
void CMenuXP::DrawText(CDC *pDC, CRect rect, const std::wstring& strText, BOOL bSelected, BOOL bDisabled, BOOL bBold)
{
	HFONT	oldFont;
	CFont	fontBold;
	if (bBold)
	{
		LOGFONT	logFont;
		m_fontMenu.GetLogFont(&logFont);
		logFont.lfWeight = FW_BOLD;
		fontBold.CreateFontIndirect(&logFont);
		oldFont = pDC->SelectFont(fontBold);
	}
	else
	{
        oldFont = pDC->SelectFont(m_fontMenu);
	}
	pDC->SetBkMode(TRANSPARENT);
	if (bDisabled && (!bSelected || m_Style == STYLE_XP))
	{
		DrawMenuText(*pDC, rect + CPoint(1, 1), strText, m_clrSelectedText);
	}
	if (bDisabled)
	{
		DrawMenuText(*pDC, rect, strText, m_clrDisabledText);
	}
	else
	{
		DrawMenuText(*pDC, rect, strText, bSelected? m_clrSelectedText : m_clrText);
	}
	pDC->SelectFont(oldFont);
}

//set menu font
BOOL CMenuXP::SetMenuFont(LOGFONT lgfont)
{
	m_fontMenu.DeleteObject();
	m_fontMenu.CreateFontIndirect(&lgfont);
    return m_fontMenu.m_hFont ? TRUE : FALSE;
}

//clear all memory and handles
void CMenuXP::Clear(void)
{
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}

    MENUITEMINFO info;
    memset(&info, 0, sizeof(MENUITEMINFO));
    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_DATA | MIIM_TYPE;
    UINT nCount = GetMenuItemCount();
    for (UINT i = 0; i<nCount; i++)
    {        
        GetMenuItemInfo(i, TRUE, &info);
        CMenuXPItem *pData = (CMenuXPItem *)info.dwItemData;
        CMenuXP *pSubmenu = NULL;
        if ((info.fType & MFT_OWNERDRAW) && pData && pData->IsMyData())
        {
            delete pData->m_pSubmenu;
            delete pData;
        }
    }
}

//draw embossed icon for the disabled item
const DWORD	MAGICROP = 0xb8074a;
const COLORREF CWHITE  = RGB(255,255,255);

void CMenuXP::DrawEmbossed(CDC *pDC, HICON hIcon, CRect rect, BOOL bColor, BOOL bShadow)
{
	CDC	memdc;
	memdc.CreateCompatibleDC(pDC->m_hDC);
	int cx = rect.Width();
	int cy = rect.Height();

	// create mono or color bitmap
	CBitmap bm;
	if (bColor)
		bm.CreateCompatibleBitmap(pDC->m_hDC, cx, cy);
	else
		bm.CreateBitmap(cx, cy, 1, 1, NULL);

	// draw image into memory DC--fill BG white first
	HBITMAP oldBitmap = memdc.SelectBitmap(bm);
	//FillRect(&memdc, CRect(0, 0, cx, cy), m_clrBackGround);
	memdc.PatBlt(0, 0, cx, cy, WHITENESS);
	::DrawIconEx(memdc.m_hDC, 0, 0, hIcon, cx, cy, 1, NULL, DI_NORMAL);

	// This seems to be required. Why, I don't know. ???
	COLORREF colorOldBG = pDC->SetBkColor(CWHITE);

	// Draw using hilite offset by (1,1), then shadow
    CBrush brShadow; brShadow.CreateSysColorBrush(COLOR_3DSHADOW);
    CBrush brHilite; brHilite.CreateSysColorBrush(COLOR_3DHIGHLIGHT);
	HBRUSH oldBrush = pDC->SelectBrush(bShadow ? brShadow : brHilite);
	pDC->BitBlt(rect.left+1, rect.top+1, cx, cy, memdc, 0, 0, MAGICROP);
	pDC->SelectBrush(brShadow);
	pDC->BitBlt(rect.left, rect.top, cx, cy, memdc, 0, 0, MAGICROP);
	pDC->SelectBrush(oldBrush);
	pDC->SetBkColor(colorOldBG);	    // restore
	memdc.SelectBitmap(oldBitmap);
	bm.DeleteObject();
	brShadow.DeleteObject();
	brHilite.DeleteObject();
}

// Shorthand to fill a rectangle with a solid color.
void CMenuXP::FillRect(CDC *pDC, const CRect& rc, COLORREF color)
{
    CBrush brush; brush.CreateSolidBrush(color);
	HBRUSH oldBrush = pDC->SelectBrush(brush);
	pDC->PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);
	pDC->SelectBrush(oldBrush);
}

//static member for keyboard operation, you can used it in you parent window
//it work with shortcut key
LRESULT CMenuXP::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	UINT iCurrentItem = (UINT)-1;   // guaranteed higher than any command ID
	std::vector<UINT> arItemsMatched;		// items that match the character typed

	UINT nItem = pMenu->GetMenuItemCount();
	for (UINT i=0; i< nItem; i++) 
	{
		MENUITEMINFO	info;
		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		info.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
		::GetMenuItemInfo(*pMenu, i, TRUE, &info);

		CMenuXPItem	*pData = (CMenuXPItem *)info.dwItemData;
		if ((info.fType & MFT_OWNERDRAW) && pData && pData->IsMyData())
		{
			const std::wstring& text = pData->m_strText;
			int iAmpersand = text.find('&');
			if (iAmpersand >=0 && toupper(nChar)==toupper(text[iAmpersand+1]))
				arItemsMatched.push_back(i);
		}
		if (info.fState & MFS_HILITE)
			iCurrentItem = i; // note index of current item
	}
	

	// arItemsMatched now contains indexes of items that match the char typed.
	//
	//   * if none: beep
	//   * if one:  execute it
	//   * if more than one: hilite next
	//
	UINT nFound = arItemsMatched.size();
	if (nFound == 0)
		return 0;

	else if (nFound==1)
		return MAKELONG(arItemsMatched[0], MNC_EXECUTE);

	// more than one found--return 1st one past current selected item;
	UINT iSelect = 0;
	for (UINT i=0; i < nFound; i++) {
		if (arItemsMatched[i] > iCurrentItem) {
			iSelect = i;
			break;
		}
	}
	return MAKELONG(arItemsMatched[iSelect], MNC_SELECT);
}

void CMenuXP::DrawMenuText(CDC& dc, CRect rc, const std::wstring& text, COLORREF color)
{
	std::wstring left = text;
    std::wstring right;

	int iTabPos = left.find(L'\t');
	if (iTabPos >= 0) 
    {
		right = left.substr(left.length() - iTabPos - 1);
		left  = left.substr(0, iTabPos);
	}
	dc.SetTextColor(color);
	dc.DrawText(left.c_str(), left.length(), &rc, DT_MYSTANDARD);
	if (iTabPos > 0)
        dc.DrawText(right.c_str(), right.length(), &rc, DT_MYSTANDARD | DT_RIGHT);
}

//add a normal menuitem, an accelerator key could be specified, and the accel text will
//be added automatically
BOOL CMenuXP::AppendODMenu(CMenuXPItem *pItem, UINT nFlags, ACCEL *pAccel)
{
	ATLASSERT(pItem);

	nFlags |= MF_OWNERDRAW;
	if (m_bBreak) 
		nFlags |= MF_MENUBREAK;
	if (m_bBreakBar)
		nFlags |= MF_MENUBARBREAK;
	m_bBreak = m_bBreakBar = FALSE;

	if (pAccel)
	{
	/*	CBCGKeyHelper	keyhelper(pAccel);
		CString	strAccel;
		keyhelper.Format(strAccel);
		if (strAccel.GetLength()>0)
		{
			pItem->m_strText += _T("\t");
			pItem->m_strText += strAccel;
		}*/
	}
	return AppendMenu(nFlags, pItem->m_dwID, (LPCTSTR)pItem);
}

//Add a separator line
BOOL CMenuXP::AppendSeparator(void)	
{
	m_bBreak = m_bBreakBar = FALSE;
    CMenuXPSeparator *pItem = new CMenuXPSeparator;    
    return AppendMenu(MF_OWNERDRAW | MF_SEPARATOR, (UINT)0, (LPCTSTR)pItem);
}

//add a popup menu
BOOL CMenuXP::AppendODPopup(CMenuXP *pPopup, CMenuXPItem *pItem, UINT nFlags)
{
	ATLASSERT(pPopup);
	ATLASSERT(pItem);
	nFlags |= MF_OWNERDRAW;
	nFlags |= MF_POPUP;
	if (m_bBreak) 
		nFlags |= MF_MENUBREAK;
	if (m_bBreakBar)
		nFlags |= MF_MENUBARBREAK;
	m_bBreak = m_bBreakBar = FALSE;
    pItem->m_pSubmenu = pPopup;
	return AppendMenu(nFlags, (UINT)pPopup->m_hMenu, (LPCTSTR)pItem);
}

//Change column, the next item added will be in the next column
void CMenuXP::Break(void)
{
	m_bBreak = TRUE;
}

//same as Break(), except that a break line will appear between the two columns
void CMenuXP::BreakBar(void)	
{
	m_bBreakBar = TRUE;
}

void CMenuXP::EnableItem(DWORD dwID)
{
    EnableMenuItem(dwID, MF_ENABLED | MF_BYCOMMAND);
}

void CMenuXP::DisableItem(DWORD dwID)
{
    EnableMenuItem(dwID, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
}

void CMenuXP::SetItemState(DWORD dwID, bool state)
{
    (state) ? EnableItem(dwID) : DisableItem(dwID);
}

//Set background bitmap, null to remove
void CMenuXP::SetBackBitmap(HBITMAP hBmp)
{
	if (hBmp == NULL && m_hBitmap)
	{
		::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
		m_memDC.DeleteDC();
		return;
	}
	m_hBitmap = hBmp;
	if (!m_memDC.m_hDC)
	{
		CWindowDC dc(NULL);
		m_memDC.CreateCompatibleDC(dc);
	}

	ATLASSERT(m_memDC.m_hDC);
	::SelectObject(m_memDC.m_hDC, m_hBitmap);
}
