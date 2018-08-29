#pragma once

class ModeDlg : public CDialogImpl<ModeDlg>, PropertyListCtrlHandler
{
    PropertiesData *m_pdata;
public:
    enum { IDD = IDD_MODE };
    ModeDlg(PropertiesData *pd) : m_pdata(pd) {}
private:
    BEGIN_MSG_MAP(ModeDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
      COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    void add(int id, int mode) 
    {
        CButton b(GetDlgItem(id));
        b.SetCheck(mode ? 1 : 0);
    }

    void set(int id, int& mode)
    {
        CButton b(GetDlgItem(id));
        int state = b.GetCheck();
        mode = (state) ? 1 : 0;
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
       PropertiesData::working_mode &m = m_pdata->mode;
       add(IDC_CHECK_C_ACTIONS, m.actions);
       add(IDC_CHECK_C_ALIASES, m.aliases);
       add(IDC_CHECK_C_SUBS, m.subs);
       add(IDC_CHECK_C_HOTKEYS, m.hotkeys);
       add(IDC_CHECK_C_HIGHLIGHTS, m.highlights);
       add(IDC_CHECK_C_ANTISUBS, m.antisubs);
       add(IDC_CHECK_C_GAGS, m.gags);
       add(IDC_CHECK_C_PLUGINS, m.plugins);
       CenterWindow(GetParent());
       return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
        if (wID == IDOK) {
            PropertiesData::working_mode &m = m_pdata->mode;
            set(IDC_CHECK_C_ACTIONS, m.actions);
            set(IDC_CHECK_C_ALIASES, m.aliases);
            set(IDC_CHECK_C_SUBS, m.subs);
            set(IDC_CHECK_C_HOTKEYS, m.hotkeys);
            set(IDC_CHECK_C_HIGHLIGHTS, m.highlights);
            set(IDC_CHECK_C_ANTISUBS, m.antisubs);
            set(IDC_CHECK_C_GAGS, m.gags);
            set(IDC_CHECK_C_PLUGINS, m.plugins);
        }
		EndDialog(wID);
		return 0;
	}
};
