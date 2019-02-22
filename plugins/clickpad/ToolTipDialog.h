
#ifndef __TTDLG_H__
#define __TTDLG_H__

//
//		CToolTipDialog 
//
// Written by Alain Rist (ar@navpoch.com)
// Copyright (c) 2003 Alain Rist.
//
// This file is NOT a part of the Windows Template Library.
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the author written consent, and 
// providing that this notice and the author name is included. 
//
// Beware of bugs.
//

#pragma once

#ifndef __ATLCTRLS_H__
	#error ToolTipDialog.h requires atlctrls.h to be included first
#endif


#ifndef TTS_BALLOON
#define TTS_BALLOON 0
#endif

namespace WTL
{
template < class T ,class TT = CToolTipCtrl >
class CToolTipDialog
{
// Data declarations and members
public:
	TT& GetTT(){return m_TT;}
protected:
	TT m_TT;
	UINT m_uTTStyle;
	UINT m_uToolFlags;
// Construction
	CToolTipDialog( UINT uTTSTyle= TTS_NOPREFIX | TTS_ALWAYSTIP , UINT uToolFlags = TTF_IDISHWND | TTF_SUBCLASS ) 
		: m_TT( NULL ), m_uTTStyle( uTTSTyle ), 
		m_uToolFlags( uToolFlags | TTF_SUBCLASS )
		{}

	void TTInit()
		{
		T* pT= (T*)this;
		ATLASSERT( ::IsWindow( *pT ));
		m_TT.Create( *pT, NULL, NULL, m_uTTStyle );
		CToolInfo ToolInfo( pT->m_uToolFlags, *pT); // , 0, 0, MAKEINTRESOURCE(pT->IDD) );
		m_TT.AddTool( &ToolInfo );
		::EnumChildWindows( *pT, SetTool, (LPARAM)pT );
		TTSize( 0 );
		TTActivate( TRUE );
		}
// Operations
public:
	void TTActivate(BOOL bActivate)
		{ m_TT.Activate( bActivate ); }
	void TTSize( int nPixel )
		{ m_TT.SetMaxTipWidth( nPixel );}

	void TTSetTxt( HWND hTool, _U_STRINGorID text )
		{ m_TT.UpdateTipText( text, hTool);}
	void TTSetTxt( UINT idTool, _U_STRINGorID text )
		{ TTSetTxt( GetHWND( idTool ) , text);}

	BOOL TTAdd( HWND hTool )
		{ return SetTool( hTool, (LPARAM)(T*)this );}
	BOOL TTAdd( UINT idTool )
		{ return TTAdd( GetHWND( idTool ));}

	void TTRemove( HWND hTool )
		{ m_TT.DelTool( hTool );}
	void TTRemove( UINT idTool )
		{ TTRemove( GetHWND( idTool ));}
// Message map and handlers
	BEGIN_MSG_MAP(CToolTipDialog)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST,WM_MOUSELAST, OnMouse)
		//MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
		
	LRESULT OnInitDialog(UINT , WPARAM , LPARAM, BOOL& bHandled)
	{
		TTInit();
		bHandled = FALSE;
		return TRUE;
	}

	LRESULT OnMouse(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = (T*)this;
		bHandled = FALSE;
		if(m_TT.IsWindow())
			m_TT.RelayEvent((LPMSG)pT->GetCurrentMessage());
		return 0;
	}
// Implementation
private:
	HWND GetHWND( UINT idTool )
		{ return ::GetDlgItem( *(T*)this, idTool );}

	static BOOL CALLBACK SetTool( HWND hTool, LPARAM pDlg)
		{
		T* pT = (T*)pDlg;
		int idTool = ::GetWindowLong(hTool, GWL_ID);
		if ( idTool != IDC_STATIC )
			{
			CToolInfo ToolInfo( pT->m_uToolFlags, hTool, 0, 0, (LPTSTR)idTool );
			pT->m_TT.AddTool( &ToolInfo );
			}
		return TRUE;
		}
};
} // namespace WTL
#endif // __TTDLG_H__