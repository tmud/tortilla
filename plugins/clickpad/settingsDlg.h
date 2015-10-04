#pragma once
#include "resource.h"
#include "padbutton.h"
#include "api/api.h"
#include "clickpad.h"

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

class ImageExampleWindow : public CWindowImpl<ImageExampleWindow>
{
    ClickpadImage* m_image;
    CPen m_border;
    CBrush m_bkgnd;
public:
    ImageExampleWindow() : m_image(NULL) {}
    void setImage(ClickpadImage *img) { m_image = img; Invalidate(FALSE); }
private:
    BEGIN_MSG_MAP(ImageExampleWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
        m_border.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNTEXT));
        m_bkgnd.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        return 0;
    }
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
        CPaintDC dc(m_hWnd);         
         HPEN old = dc.SelectPen(m_border);
         HBRUSH oldb = dc.SelectBrush(m_bkgnd);
         RECT rc; GetClientRect(&rc);
         dc.Rectangle(&rc);
         dc.SelectBrush(oldb);
         dc.SelectPen(old);
         if (m_image) {
            int x = (rc.right - m_image->width()) / 2;
            int y = (rc.bottom - m_image->height()) / 2;
            m_image->render(dc, x, y);
        }     
        return 0;
    }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&) {
        return 1;
    }
};

class SettingsDlg : public CDialogImpl<SettingsDlg>
{
    HHOOK m_hHook;
    CEdit m_edit_text;
    CEdit m_edit_command;
    CComboBox m_rows, m_columns, m_bsize;
    CButton m_del_hotkey, m_del_button;
    CListViewCtrl m_list;
    CStatic m_close_settings;
    CButton m_template_cmd;
    CButton m_button_icon;
    ImageExampleWindow m_image_example;

    struct image_file
    {
        image_file() : set_size(-1) {}
        tstring path;
        int set_size;
    };
    std::vector<image_file> m_image_files;
    ClickpadSettings *m_settings;
    PadButton* m_editable_button;

public:
    SettingsDlg() : m_editable_button(NULL)  {}
    enum { IDD = IDD_SETTINGS };
    LRESULT HookGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
    void editButton(PadButton *button);
    void setSettings(ClickpadSettings *settings);
    void setSettingsBlock(bool block);

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
        COMMAND_ID_HANDLER(IDC_BUTTON_DELHOTKEY, OnDelHotkey)
        COMMAND_ID_HANDLER(IDC_CHECK_TEMPLATE, OnTemplate)
        NOTIFY_HANDLER(IDC_LIST_HOTKEYS, LVN_ITEMCHANGED, OnListItemChanged)
        NOTIFY_HANDLER(IDC_LIST_HOTKEYS, NM_SETFOCUS, OnListItemChanged)
        NOTIFY_HANDLER(IDC_LIST_HOTKEYS, NM_KILLFOCUS, OnListKillFocus)
        COMMAND_ID_HANDLER(IDC_BUTTON_ICON, OnIconButton)
        MESSAGE_HANDLER(WM_USER, OnImageChanged)
    END_MSG_MAP()

    LRESULT OnInitDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_hHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc,  NULL, GetCurrentThreadId() );
        m_rows.Attach(GetDlgItem(IDC_COMBO_ROWS));
        m_columns.Attach(GetDlgItem(IDC_COMBO_COLUMNS));
        m_bsize.Attach(GetDlgItem(IDC_COMBO_BUTTON_SIZE));
        m_edit_text.Attach(GetDlgItem(IDC_EDIT_TEXT));
        m_edit_command.Attach(GetDlgItem(IDC_EDIT_COMMAND));
        m_del_hotkey.Attach(GetDlgItem(IDC_BUTTON_DELHOTKEY));
        m_del_button.Attach(GetDlgItem(IDC_BUTTON_DELBUTTON));
        m_list.Attach(GetDlgItem(IDC_LIST_HOTKEYS));
        m_close_settings.Attach(GetDlgItem(IDC_STATIC_CLOSE_SETTING));
        m_template_cmd.Attach(GetDlgItem(IDC_CHECK_TEMPLATE));
        m_button_icon.Attach(GetDlgItem(IDC_BUTTON_ICON));

        RECT rc;
        CWindow imagepos(GetDlgItem(IDC_STATIC_IMAGE));
        imagepos.GetWindowRect(&rc);
        ScreenToClient(&rc);
        imagepos.ShowWindow(SW_HIDE);
        m_image_example.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE);
        
        m_list.GetClientRect(&rc);
        float width_percent =  static_cast<float>(rc.right) / 100;
        m_list.InsertColumn(0, L"Hotkey", LVCFMT_LEFT, static_cast<int>(width_percent * 15));
        m_list.InsertColumn(1, L"Команда", LVCFMT_LEFT, static_cast<int>(width_percent * 60));
        m_list.InsertColumn(2, L"Группа", LVCFMT_LEFT, static_cast<int>(width_percent * 20));
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
        luaT_ActiveObjects hk(getLuaState(), "hotkeys");
        u8string key, value, group;
        int item = 0;
        for (int i=1,e=hk.size();i<=e;++i)
        {
            hk.select(i);
            if (hk.get(luaT_ActiveObjects::KEY, &key) &&
                hk.get(luaT_ActiveObjects::VALUE, &value) &&
                hk.get(luaT_ActiveObjects::GROUP, &group))
            {
                m_list.AddItem(item, 0, TU2W(key.c_str()));
                m_list.SetItem(item, 1, LVIF_TEXT, TU2W(value.c_str()), 0, 0, 0, NULL);
                m_list.SetItem(item, 2, LVIF_TEXT, TU2W(group.c_str()), 0, 0, 0, NULL);
                item++;
            }
        }

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
        m_del_hotkey.EnableWindow(FALSE);
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
    LRESULT OnDelHotkey(WORD, WORD, HWND, BOOL&);
    LRESULT OnTemplate(WORD, WORD, HWND, BOOL&);
    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&);
    LRESULT OnListKillFocus(int , LPNMHDR , BOOL&);
    LRESULT OnIconButton(WORD, WORD, HWND, BOOL&);

    void resetEditable();
    void setEditableState(bool state);
    void getListItemText(int item, int subitem, tstring* text);
    bool isSupportedExt(const wchar_t* file);
    void updateImage();
    LRESULT OnImageChanged(UINT, WPARAM, LPARAM, BOOL&) {     
        updateImage();
        return 0;
    }
};