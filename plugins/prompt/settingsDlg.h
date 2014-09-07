#pragma once

class SettingsDlg : public CDialogImpl<SettingsDlg>
{
    std::wstring m_regexp;
    CEdit m_redit;
public:
    enum { IDD = IDD_SETTINGS };
    SettingsDlg(const std::wstring& regexp) : m_regexp(regexp) {}
    const wchar_t* getRegexp() { return m_regexp.c_str(); }
    
private:
    BEGIN_MSG_MAP(SettingsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_redit.Attach(GetDlgItem(IDC_EDIT_PROMPT));
        m_redit.SetWindowText(m_regexp.c_str());
        CenterWindow();
        return 0;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        int text_len = m_redit.GetWindowTextLength();
        int buffer_len = text_len + 2;
        wchar_t *buffer = new wchar_t[buffer_len];
        ::GetWindowText(m_redit, buffer, buffer_len);
        m_regexp.assign(buffer);
        delete []buffer;
        EndDialog(IDOK);
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        EndDialog(wID);
        return 0;
    }
};
