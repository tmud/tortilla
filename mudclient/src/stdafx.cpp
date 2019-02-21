// stdafx.cpp : source file that includes just the standard includes
//	mudclient.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#if (_ATL_VER < 0x0700)
#include <atlimpl.cpp>
#endif //(_ATL_VER < 0x0700)

// Handler prototypes (uncomment arguments if needed):
// LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
// LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
// LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

/*
MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtrlColor)
MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtrlColor)

LRESULT OnCtrlColor(UINT, WPARAM wParam, LPARAM, BOOL&)
    { // set background mode and text color
      SetBkMode((HDC)wParam, TRANSPARENT); // transparent background
      SetTextColor((HDC)wParam, RGB(255, 255, 255)); // white text
      return (LRESULT)GetStockObject(BLACK_BRUSH); }
    };
*/
