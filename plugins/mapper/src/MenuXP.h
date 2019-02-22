//  loaded from http://www.codeproject.com/Articles/1493/A-Powerfull-Ownerdraw-Menu
//  Modified by me
#pragma once

//The ownerdraw data
class CMenuXP;
class CMenuXPItem
{
public:
	DWORD		m_dwMagicNum;	//A magic number to distingush our data
	DWORD		m_dwID;			//Menu ID
	bool		m_bSeparator;	//Separator
	bool		m_bButtonOnly;	//Button only style item
    std::wstring m_strText;		//Menu item text
	HICON		m_hIcon;		//Menu icon
	int			m_nSize;		//Height of the item (Width of the sidebar if m_bSideBar is true)
    CMenuXP    *m_pSubmenu;     //Submenu

public:
	CMenuXPItem() 
	{
		m_dwMagicNum = 0x0505a0a0;
		m_dwID = 0;
		m_bSeparator = false;
		m_bButtonOnly = false;
		m_hIcon = NULL;
		m_nSize = 16;
        m_pSubmenu = NULL;
	};

	virtual ~CMenuXPItem()
	{
		if (m_hIcon)
			::DestroyIcon(m_hIcon);
	}
	BOOL IsMyData() const { return (m_dwMagicNum == 0x0505a0a0) ? TRUE : FALSE; }
    BOOL IsImage() const { return (m_hIcon) ? TRUE : FALSE; }
};
//------------------------------------------------------------------------------------------
//	For convenient, derive some class from CMenuXPItem, 
//	and do the initialization in constructor

class CMenuXPText : public CMenuXPItem	//Normal item with text and an optional icon
{
public:
    CMenuXPText(DWORD dwID, LPCTSTR strText, HICON icon = NULL)
	{
		m_dwID = dwID;
		m_strText = strText;
        m_hIcon = icon;
	}
};

class CMenuXPButton : public CMenuXPItem    //A button only item
{
public:
	CMenuXPButton(DWORD dwID, HICON icon)
	{
		m_dwID = dwID;
		m_bButtonOnly = true;
        m_hIcon = icon;
	}
};
//------------------------------------------------------------------------------------------
// Class CMenuXP, an ownerdraw menu
class CMenuXP : public CMenu  
{
public:
	CMenuXP();
	virtual ~CMenuXP();

	typedef	enum
	{
		STYLE_OFFICE,		//Draw a float button around the icon
		STYLE_STARTMENU,	//show selected bar below the icon
		STYLE_XP			//use different color for the icon area
	} MENUSTYLE;

	//Below is the functions to build the menu
    BOOL	AppendODMenu(CMenuXPItem *pItem, UINT nFlags = 0, ACCEL *pAccel = 0);
    BOOL	AppendODPopup(CMenuXP *pPopup, CMenuXPItem *pItem, UINT nFlags = 0);
	BOOL	AppendSeparator(void);		
	void	Break(void);	//change a column(the next item added will be in a new column)
	void	BreakBar(void);	//change a column with a break line(same as Break, except that a break line is drawn between two columns)
    void    EnableItem(DWORD dwID);
    void    DisableItem(DWORD dwID);
    void    SetItemState(DWORD dwID, bool state);

protected:
	CFont		m_fontMenu;	
	COLORREF	m_clrBackGround;	//Background color
	COLORREF	m_clrSelectedBar;	//selected bar color
	COLORREF	m_clrText;			//Text color
	COLORREF	m_clrSelectedText;	//selected text color
	COLORREF	m_clrDisabledText;	//disabled text color
	COLORREF	m_clrSideBarStart;	//Start color of the gradient sidebar
	COLORREF	m_clrSideBarEnd;	//end color of the gradient sidebar
	COLORREF	m_clrIconArea;		//Background color of the button(icon) area
	BOOL		m_bBreak;		//if true, next item inserted into the menu will be added with the sytle MF_MENUBREAK
	BOOL		m_bBreakBar;	//if true, next item inserted into the menu will be added with the sytle MF_MENUBARBREAK
	HBITMAP		m_hBitmap;		//Background bitmap
	CDC			m_memDC;		//Memory dc holding the background bitmap
	MENUSTYLE	m_Style;	    //menu style(currently support office or startmenu style)

public:	//User these functions to change the default attribute of the menu
	void	SetBackColor(COLORREF clr) { m_clrBackGround = clr; }
	void	SetSelectedBarColor(COLORREF clr) { m_clrSelectedBar = clr; }
	void	SetTextColor(COLORREF clr) { m_clrText = clr; }
	void	SetSelectedTextColor(COLORREF clr) { m_clrSelectedText = clr; }
	void	SetDisabledTextColor(COLORREF clr) { m_clrDisabledText = clr; }
	void	SetSideBarStartColor(COLORREF clr) { m_clrSideBarStart = clr; }
	void	SetSideBarEndColor(COLORREF clr) { m_clrSideBarEnd = clr; }
	void	SetIconAreaColor(COLORREF clr) { m_clrIconArea = clr; }
	void	SetBackBitmap(HBITMAP hBmp);
	void	SetMenuStyle(MENUSTYLE	style) { m_Style = style; }
	BOOL	SetMenuFont(LOGFONT	lgfont);

public:
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
	static LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);

protected:
	virtual void DrawBackGround(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled);
	virtual void DrawButton(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled, BOOL bChecked);
	virtual void DrawIcon(CDC *pDC, CRect rect, HICON hIcon, BOOL bSelected, BOOL bDisabled, BOOL bChecked);
	virtual void DrawText(CDC *pDC, CRect rect, const std::wstring& strText, BOOL bSelected, BOOL bDisabled, BOOL bBold);
	virtual void DrawCheckMark(CDC *pDC, CRect rect, BOOL bSelected);
    virtual void DrawMenuText(CDC& dc, CRect rc, const std::wstring& text, COLORREF color);
	virtual void DrawIconArea(CDC *pDC, CRect rect, BOOL bSelected, BOOL bDisabled, BOOL bChecked);	

private:
    void Clear();	//Clean all memory and handles
	void DrawEmbossed(CDC *pDC, HICON hIcon, CRect rect, BOOL bColor = FALSE, BOOL bShadow = FALSE);
	void FillRect(CDC *pDC, const CRect& rc, COLORREF color);
};
