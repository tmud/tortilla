#pragma once
#include "resource.h"

LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
class SettingsDlg : public CDialogImpl<SettingsDlg>
{
    HHOOK m_hHook;
    //CEdit m_edit_columns;
    //CEdit m_edit_rows;
    //CEdit m_edit_bsize;

    CComboBox m_rows, m_columns, m_bsize;    

public:
    enum { IDD = IDD_SETTINGS };
    LRESULT HookGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
    
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

        wchar_t buffer[16];
        for (int i=1; i<=5; ++i)
            m_rows.AddString(_itow(i, buffer, 10));
        for (int i=1; i<=10; ++i)
            m_columns.AddString(_itow(i, buffer, 10));
        m_bsize.AddString(L"32x32");
        m_bsize.AddString(L"40x40");
        m_bsize.AddString(L"48x48");

        //m_edit_columns.Attach(GetDlgItem(IDC_EDIT_COLUMNS));
        //m_edit_rows.Attach(GetDlgItem(IDC_EDIT_ROWS));
        //m_edit_bsize.Attach(GetDlgItem(IDC_EDIT_BSIZE));
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
};
