#ifndef __BEVELLINE_H__
#define __BEVELLINE_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CBevelLine - Gradient label control implementation
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2000.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
  #error BevelLine.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
  #error BevelLine.h requires atlctrls.h to be included first
#endif

#if (_WIN32_IE < 0x0400)
  #error BevelLine.h requires _WIN32_IE >= 0x0400
#endif


template< class T, class TBase = CWindow, class TWinTraits = CControlWinTraits >
class ATL_NO_VTABLE CBevelLineImpl : public CWindowImpl< T, TBase, TWinTraits >
{
public:
  bool m_bSunken;
  int m_iLineWidth;
  int m_iLineHeight;

  CBevelLineImpl() : 
    m_bSunken(true),
    m_iLineWidth(1),
    m_iLineHeight(1)
  { };

  ~CBevelLineImpl()
  {
  }

  // Operations

  BOOL SubclassWindow(HWND hWnd)
  {
    ATLASSERT(m_hWnd == NULL);
    ATLASSERT(::IsWindow(hWnd));
    BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
    if( bRet ) Init();
    return bRet;
  }

  // Implementation

  void Init()
  {
    ATLASSERT(::IsWindow(m_hWnd));

    // Check if we should paint a label
    TCHAR lpszBuffer[10] = { 0 };
    if(::GetClassName(m_hWnd, lpszBuffer, 8)) {
      if(::lstrcmpi(lpszBuffer, _T("static")) == 0) {
        ModifyStyle(0, SS_NOTIFY);  // we need this
        DWORD dwStyle = GetStyle() & 0x000000FF;
        if(dwStyle == SS_ICON || dwStyle == SS_BLACKRECT || dwStyle == SS_GRAYRECT || 
            dwStyle == SS_WHITERECT || dwStyle == SS_BLACKFRAME || dwStyle == SS_GRAYFRAME || 
            dwStyle == SS_WHITEFRAME || dwStyle == SS_OWNERDRAW || 
            dwStyle == SS_BITMAP || dwStyle == SS_ENHMETAFILE)
          ATLASSERT("Invalid static style for bevel control"==NULL);
      }
    }
  }

  // Message map and handlers

  BEGIN_MSG_MAP(CBevelLineImpl)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
  END_MSG_MAP()

  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    Init();
    return 0;
  }

  LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
  {
    T* pT = static_cast<T*>(this);
    if(wParam != NULL)
    {
      pT->DoPaint((HDC)wParam);
    }
    else
    {
      CPaintDC dc(m_hWnd);
      pT->DoPaint(dc.m_hDC);
    }
    return 0;
  }

  void DoPaint(CDCHandle dc)
  {
    RECT r;
    GetClientRect(&r); 
    DWORD hiCol = ::GetSysColor(!m_bSunken ? COLOR_3DHIGHLIGHT : COLOR_3DSHADOW); 
    DWORD loCol = ::GetSysColor(m_bSunken ? COLOR_3DHIGHLIGHT : COLOR_3DSHADOW); 
    if( r.bottom > r.right) { // vertical
      r.right /= 2; 
      r.left = r.right - m_iLineWidth; 
      r.right += m_iLineWidth; 
    } 
    else { // horizonzal 
      r.bottom /= 2; 
      r.top = r.bottom - m_iLineHeight; 
      r.bottom += m_iLineHeight; 
    }
    dc.Draw3dRect(&r,hiCol,loCol); 
  }
};


class CBevelLine : public CBevelLineImpl<CBevelLine>
{
public:
  DECLARE_WND_CLASS(_T("WTL_BevelLine"))
};


#endif //__BEVELLINE_H__
