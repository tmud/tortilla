#pragma once

class SelectImageDlg : public CWindowImpl<SelectImageDlg>
{
public:
    SelectImageDlg() {}
    enum { IDD = IDD_SETTINGS };

private:
    BEGIN_MSG_MAP(SelectImageDlg)
        //MESSAGE_HANDLER(WM_INITDIALOG, OnInitDlg)
        //MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)       
    END_MSG_MAP()
    
};
