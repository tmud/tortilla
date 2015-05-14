#pragma once
#include "resource.h"
#include "padbutton.h"

LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
class ClickpadSettings;

class ButtonSizeTranslator
{
public:
    int  getCount() const { return 3; }
    void getLabel(int index, tstring *label) {
        switch(index) {  
            case 0: label->assign(L"48x48"); break;
            case 1: label->assign(L"64x64"); break;
            case 2: label->assign(L"80x80"); break;
            default: assert(false); break;
        }
    }
    int getSize(int index) const {
          switch(index) {  
            case 0: return 48;
            case 1: return 64;
            case 2: return 80;
            default: assert(false); break;
        }
        return 64;
    }
    bool checkSize(int size) const {
        if (size != 48 && size != 64 && size != 80)
            return false;
        return true;
    }
    int getDefaultSize() const { return 64; }
    int getIndex(int size) const {
        switch(size) {
            case 48: return 0;
            case 64: return 1;
            case 80: return 2;
            default: assert(false); break;
        }
        return 1;
    }
};

class SettingsDlg : public CDialogImpl<SettingsDlg>
{
    HHOOK m_hHook;
    CEdit m_edit_text;
    CEdit m_edit_command;
    CComboBox m_rows, m_columns, m_bsize;
    ClickpadSettings *m_settings;
    PadButton* m_editable_button;
    CButton m_load_hotkey, m_del_button;
    bool m_load_hotkey_mode;

public:
    SettingsDlg() : m_editable_button(NULL), m_load_hotkey_mode(false) {}
    enum { IDD = IDD_SETTINGS };
    LRESULT HookGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
    void editButton(PadButton *button);
    void setSettings(ClickpadSettings *settings);
    bool canCloseSettingsDlg() const { return !m_load_hotkey_mode; }
    
private:
    BEGIN_MSG_MAP(SettingsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDlg)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)
        COMMAND_HANDLER(IDC_COMBO_ROWS, CBN_SELCHANGE, OnRowsChanged);
        COMMAND_HANDLER(IDC_COMBO_COLUMNS, CBN_SELCHANGE, OnColumnsChanged);
        COMMAND_HANDLER(IDC_COMBO_BUTTON_SIZE, CBN_SELCHANGE, OnBSizeChanged);
        COMMAND_HANDLER(IDC_EDIT_TEXT, EN_CHANGE, OnTextChanged)
        COMMAND_HANDLER(IDC_EDIT_COMMAND, EN_CHANGE, OnCommandChanged)
        COMMAND_ID_HANDLER(IDC_BUTTON_EXIT, OnButtonExit)
        COMMAND_ID_HANDLER(IDC_BUTTON_DELBUTTON, OnDelButton)
        COMMAND_ID_HANDLER(IDC_BUTTON_LOADHOTKEY, OnHotkeyButton)
    END_MSG_MAP()

    LRESULT OnInitDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_hHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc,  NULL, GetCurrentThreadId() );
        m_rows.Attach(GetDlgItem(IDC_COMBO_ROWS));
        m_columns.Attach(GetDlgItem(IDC_COMBO_COLUMNS));
        m_bsize.Attach(GetDlgItem(IDC_COMBO_BUTTON_SIZE));
        m_edit_text.Attach(GetDlgItem(IDC_EDIT_TEXT));
        m_edit_command.Attach(GetDlgItem(IDC_EDIT_COMMAND));
        m_load_hotkey.Attach(GetDlgItem(IDC_BUTTON_LOADHOTKEY));
        m_del_button.Attach(GetDlgItem(IDC_BUTTON_DELBUTTON));

        wchar_t buffer[16];
        for (int i=1; i<=5; ++i)
            m_rows.AddString(_itow(i, buffer, 10));
        for (int i=1; i<=10; ++i)
            m_columns.AddString(_itow(i, buffer, 10));
        ButtonSizeTranslator bt;
        for (int i=0; i<bt.getCount(); ++i)
        {
            tstring label;
            bt.getLabel(i, &label);
            m_bsize.AddString(label.c_str());            
        }
        return 0;
    }

    LRESULT OnDestroyDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        UnhookWindowsHookEx(m_hHook);
        return 0;
    }

    LRESULT OnRowsChanged(WORD, WORD, HWND, BOOL&);
    LRESULT OnColumnsChanged(WORD, WORD, HWND, BOOL&);   
    LRESULT OnBSizeChanged(WORD, WORD, HWND, BOOL&);
    LRESULT OnTextChanged(WORD, WORD, HWND, BOOL&);
    LRESULT OnCommandChanged(WORD, WORD, HWND, BOOL&);
    LRESULT OnButtonExit(WORD, WORD, HWND, BOOL&);
    LRESULT OnDelButton(WORD, WORD, HWND, BOOL&);
    LRESULT OnHotkeyButton(WORD, WORD, HWND, BOOL&);

    void resetEditable();
    void setEditableState(bool state);
};
