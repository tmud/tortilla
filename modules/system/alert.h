#pragma once
#include "resource.h"

class AlertDlg :  public CDialogImpl<AlertDlg>
{     
    std::wstring m_text;
    CEdit m_edit;
public:
   enum { IDD = IDD_ALERT};
private:
   BEGIN_MSG_MAP(AlertDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
   END_MSG_MAP()

   void setText(const std::wstring& text)
   {
        m_text = text;
   }

   LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
   {
       m_edit.Attach(GetDlgItem(IDC_EDIT_TEXT));
       m_edit.SetWindowText(m_text.c_str());
       return 0;
   }

   LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
        DestroyWindow();
        m_hWnd = NULL;
		return 0;
	}
};
