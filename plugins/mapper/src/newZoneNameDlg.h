#pragma once

class NewZoneNameDlg :  public CDialogImpl<NewZoneNameDlg>
{
    CEdit m_name;
    tstring name;
public:
   enum { IDD = IDD_NEWZONE };
   NewZoneNameDlg() {}
   const tstring& getName() const {
       return name;
   }
private:
    BEGIN_MSG_MAP(NewZoneNameDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_name.Attach(GetDlgItem(IDC_EDIT_ZONENAME));
        CenterWindow();
        return 0;
    }    
    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        int len = m_name.GetWindowTextLength()+1;
        std::vector<tchar> buffer(len);
        m_name.GetWindowText(&buffer.at(0), len);
        name.assign(&buffer.at(0));
        EndDialog(wID);
        return 0;
    }
};