#pragma once

class SettingsDlg : public CDialogImpl<SettingsDlg>
{    
public:
    enum { IDD = IDD_SETTINGS };
    
private:
    BEGIN_MSG_MAP(SettingsDlg)
        //MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        CenterWindow(GetParent());
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        EndDialog(wID);
        return 0;
    }
};