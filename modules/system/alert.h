#pragma once
#include "resource.h"

class AlertDlgControl : public CDialogImpl<AlertDlgControl>
{
     CEdit m_edit;
     std::wstring m_text;
public:
    enum { IDD = IDD_ALERT };

private:   
   BEGIN_MSG_MAP(AlertDlgControl)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		COMMAND_ID_HANDLER(IDOK, OnClick)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
   {
       m_edit.Attach(GetDlgItem(IDC_EDIT_TEXT));
       m_edit.SetWindowText(m_text.c_str());
       return 0;
   }
   LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL&)
   {
       return 0;
   }
    LRESULT OnKeyDown(UINT, WPARAM, LPARAM, BOOL&)
   {
       return 0;
   }

	LRESULT OnClick(WORD, WORD, HWND, BOOL&)
	{
	
		return 0;
	}
};


class AlertDlg :  public CWindowImpl<AlertDlg>
{     
    AlertDlgControl m_dlg;
private:
   BEGIN_MSG_MAP(AlertDlg)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)        
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   END_MSG_MAP()

   void setText(const std::wstring& text)
   {
        //m_text = text;
   }

   LRESULT OnKeyDown(UINT, WPARAM, LPARAM, BOOL&)
   {
	   return 0;
   }

   LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
   {             
       m_dlg.Create(m_hWnd);
       RECT rc;
       m_dlg.GetClientRect(&rc);
       int dy = GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYBORDER);
       int dx = 2*GetSystemMetrics(SM_CXBORDER);
       rc.right += dx;
       rc.bottom += dy;
       MoveWindow(&rc);
       //SubclassWindow(m_dlg);
       return 0;
   }

   LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL&)
   {
       return 0;
   }

   LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
        DestroyWindow();
        m_hWnd = NULL;
		return 0;
	}
};
