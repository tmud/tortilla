#pragma once
#include "resource.h"
#include "padbutton.h"

LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
class ClickpadSettings;
class SettingsDlg : public CDialogImpl<SettingsDlg>
{
    HHOOK m_hHook;
    CEdit m_edit_text;
    CEdit m_edit_command;
    CComboBox m_rows, m_columns, m_bsize;
    ClickpadSettings *m_settings;

public:
    enum { IDD = IDD_SETTINGS };
    LRESULT HookGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
    void editButton(PadButton *button);
    void setSettings(ClickpadSettings *settings);
    
private:
    BEGIN_MSG_MAP(SettingsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDlg)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)
        //MESSAGE_HANDLER(WM_SIZE, OnSize)
        //COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        //COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_hHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc,  NULL, GetCurrentThreadId() );
        m_rows.Attach(GetDlgItem(IDC_COMBO_ROWS));
        m_columns.Attach(GetDlgItem(IDC_COMBO_COLUMNS));
        m_bsize.Attach(GetDlgItem(IDC_COMBO_BUTTON_SIZE));
        m_edit_text.Attach(GetDlgItem(IDC_EDIT_TEXT));
        m_edit_command.Attach(GetDlgItem(IDC_EDIT_COMMAND));

        wchar_t buffer[16];
        for (int i=1; i<=5; ++i)
            m_rows.AddString(_itow(i, buffer, 10));
        for (int i=1; i<=10; ++i)
            m_columns.AddString(_itow(i, buffer, 10));
        m_bsize.AddString(L"48x48");
        m_bsize.AddString(L"64x64");
        m_bsize.AddString(L"80x80");
        return 0;
    }

    LRESULT OnDestroyDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        UnhookWindowsHookEx(m_hHook);
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        return 0;
    }

private:

};
