#pragma once
#include "resource.h"

class SaveSoundDlg : public CDialogImpl<SaveSoundDlg>
{
public:
    SaveSoundDlg() {}
    enum { IDD = IDD_SAVE_SOUND };

private:
    BEGIN_MSG_MAP(SaveSoundDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        //COMMAND_ID_HANDLER(IDOK, OnOk)
        //COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {        
        CenterWindow(GetParent());
        return 0;
    }
};