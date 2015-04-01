#pragma once

class ButtonDlg : public CDialogImpl<ButtonDlg>
{    
public:
    enum { IDD = IDD_BUTTON };
    
private:
    BEGIN_MSG_MAP(ButtonDlg)
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
