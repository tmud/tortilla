#pragma once

class PasswordDlg : public CDialogImpl < PasswordDlg >
{
    tstring m_title;
    tstring m_password;
public:
    enum { IDD = IDD_PASSWORD };
    const tstring& getPassword() const { return m_password; }
    PasswordDlg(const tstring& title) : m_title(title) {}

private:
    BEGIN_MSG_MAP(PasswordDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        if (!m_title.empty())
            SetWindowText(m_title.c_str());
        CenterWindow(GetParent());
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        if (wID == IDOK)
            getWindowText(GetDlgItem(IDC_EDIT_PASSWORD), &m_password);
        EndDialog(wID);
        return 0;
    }
};
