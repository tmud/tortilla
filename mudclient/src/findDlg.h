#pragma once

class FindDlg : public CDialogImpl<FindDlg>
{
public:
   enum { IDD = IDD_FIND };
private:
   BEGIN_MSG_MAP(FindDlg)
     MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
   {
       return 0;
   }

};
