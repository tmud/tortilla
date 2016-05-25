#pragma once
#include "profileHelpers.h"
#include "profilesPath.h"

class LoadProfileDlg : public CDialogImpl<LoadProfileDlg>
{
   CListBox m_groups_list;
   CListBox m_list;
   std::vector<tstring> m_groups_name;
   CButton m_ok;
   int m_group_idx;
   tstring m_profile_group;
   tstring m_profile_name;

public:
   enum { IDD = IDD_LOAD_PROFILE };
   LoadProfileDlg() : m_group_idx(-1) {}

   void getProfile(Profile *profile)
   {
       profile->group.assign(m_profile_group);
       profile->name.assign(m_profile_name);
   }

private:
    BEGIN_MSG_MAP(LoadProfileDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_HANDLER(IDC_LIST_PROFILE_GROUP, LBN_SELCHANGE, OnGroupChanged)
        COMMAND_HANDLER(IDC_LIST_PROFILE, LBN_SELCHANGE, OnProfileChanged)
        COMMAND_HANDLER(IDC_LIST_PROFILE, LBN_DBLCLK, OnProfileDblClick)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_groups_list.Attach(GetDlgItem(IDC_LIST_PROFILE_GROUP));
        m_list.Attach(GetDlgItem(IDC_LIST_PROFILE));
        m_ok.Attach(GetDlgItem(IDOK));

        ProfilesGroupList groups;
        groups.init();
        {
            for (int i=0,e=groups.getCount(); i<e; ++i)
            {
                tstring gname;
                groups.getName(i, &gname);
                ProfilesList plist(gname);
                if (plist.profiles.empty())
                    continue;
                m_groups_name.push_back(gname);
                m_groups_list.AddString(gname.c_str());
            }

            int last = groups.getLast();
            m_groups_list.SetCurSel(last);
            m_group_idx = last;
            updateProfilesList();
        }
        CenterWindow(GetParent());
        m_ok.EnableWindow(FALSE);
        m_groups_list.SetFocus();
        return 0;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
	{
        int idx = m_groups_list.GetCurSel();
        int len = m_groups_list.GetTextLen(idx) + 1;
        MemoryBuffer g(len * sizeof(WCHAR));
        WCHAR *buffer = (WCHAR*)g.getData();
        m_groups_list.GetText(idx, buffer);
        m_profile_group.assign(buffer);

        idx = m_list.GetCurSel();
        if (idx != -1)
        {
            len = m_list.GetTextLen(idx) + 1;
            g.alloc(len * sizeof(WCHAR));
            buffer = (WCHAR*)g.getData();
            m_list.GetText(idx, buffer);
            m_profile_name.assign(buffer);
        }
        else
        {
            return 0;
        }

        EndDialog(IDOK);
		return 0;
	}

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}

    LRESULT OnGroupChanged(WORD, WORD, HWND, BOOL&)
    {
        int idx = m_groups_list.GetCurSel();
        if (idx != m_group_idx)
        {
            m_ok.EnableWindow(FALSE);
            m_group_idx = idx;
            updateProfilesList();
        }
        return 0;
    }

    LRESULT OnProfileChanged(WORD, WORD, HWND, BOOL&)
    {
        m_ok.EnableWindow(TRUE);
        return 0;
    }

    LRESULT OnProfileDblClick(WORD, WORD, HWND, BOOL&)
    {
        BOOL outside = FALSE;
        POINT pt; GetCursorPos(&pt);
        m_list.ScreenToClient(&pt);
        UINT item = m_list.ItemFromPoint(pt, outside);
        if (!outside)
            OnOk(0, 0, 0, outside);
        return 0;
    }

    void updateProfilesList()
    {
        m_list.ResetContent();
        int sel = m_groups_list.GetCurSel();
        if (sel == -1)
            return;

        tstring gname = m_groups_name[sel];
        ProfilesList plist(gname);
        for (int i=0,e=plist.profiles.size(); i<e; ++i)
            m_list.AddString(plist.profiles[i].c_str());
    }
};

struct CopyProfileData
{
    Profile dst;
    Profile src;
};

class NewWorldDlg : public CDialogImpl<NewWorldDlg>
{
    CListBox m_groups_list;
    CListBox m_list;
    CEdit m_newworld_name;
    CEdit m_newworld_profile;
    CButton m_ok;
    std::vector<tstring> m_groups_name;
    int m_group_idx;
    CopyProfileData m_data;
    std::vector<tstring> m_templates;
    bool m_group_name_changed;

public:
    enum { IDD = IDD_NEW_WORLD };
    NewWorldDlg() : m_group_idx(-1), m_group_name_changed(false)
    {
        readTemplates();
    }
    void getData(CopyProfileData* data)
    {
        *data = m_data;
    }
private:
    BEGIN_MSG_MAP(NewWorldDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_HANDLER(IDC_LIST_PROFILE_GROUP, LBN_SELCHANGE, OnGroupChanged)
        COMMAND_HANDLER(IDC_LIST_PROFILE, LBN_SELCHANGE, OnProfileChanged)
        COMMAND_HANDLER(IDC_EDIT_NEWWORLD_NAME, EN_CHANGE, OnNameChanged)
        COMMAND_HANDLER(IDC_EDIT_NEWWORLD_PROFILE, EN_CHANGE, OnProfileChanged)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_groups_list.Attach(GetDlgItem(IDC_LIST_PROFILE_GROUP));
        m_list.Attach(GetDlgItem(IDC_LIST_PROFILE));
        m_newworld_name.Attach(GetDlgItem(IDC_EDIT_NEWWORLD_NAME));
        m_newworld_profile.Attach(GetDlgItem(IDC_EDIT_NEWWORLD_PROFILE));
        m_ok.Attach(GetDlgItem(IDOK));

        m_groups_list.AddString(L"Создать пустой мир и профиль");
        m_groups_list.SetCurSel(0);
        m_group_idx = 0;

        ProfilesGroupList groups;
        groups.init();
        {
            for (int i = 0, e = groups.getCount(); i<e; ++i)
            {
                tstring gname;
                groups.getName(i, &gname);
                ProfilesList plist(gname);
                if (plist.profiles.empty())
                    continue;
                m_groups_name.push_back(gname);
                m_groups_list.AddString(gname.c_str());
            }
            for (int i = 0, e = m_templates.size(); i<e; ++i)
            {
                const tstring& t = m_templates[i];
                if (std::find(m_groups_name.begin(), m_groups_name.end(), t) == m_groups_name.end())
                {
                    m_groups_name.push_back(t);
                    m_groups_list.AddString(t.c_str());
                }
            }
            updateProfilesList();
        }

        CenterWindow(GetParent());
        m_ok.EnableWindow(FALSE);
        m_groups_list.SetFocus();
        return 0;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        getWindowText(m_newworld_name, &m_data.dst.group);
        getWindowText(m_newworld_profile, &m_data.dst.name);
        int group = m_groups_list.GetCurSel();
        if (group != 0)
        {
            int name_index = m_list.GetCurSel();
            m_data.src.group = m_groups_name[group - 1];
            const tstring &g  = m_data.src.group;
            bool with_default = (std::find(m_templates.begin(), m_templates.end(), g) != m_templates.end()) ? true : false;
            int idx = m_list.GetCurSel();
            int len = m_list.GetTextLen(idx) + 1;
            MemoryBuffer gn(len * sizeof(tchar));
            tchar *buffer = (tchar*)gn.getData();
            m_list.GetText(idx, buffer);
            m_data.src.name = buffer;
            if (with_default && idx == 0)
            {
                m_data.src.name.clear();
            }
        }
        EndDialog(IDOK);
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        EndDialog(wID);
        return 0;
    }

    LRESULT OnGroupChanged(WORD, WORD, HWND, BOOL&)
    {
        if (!m_group_name_changed)
        {
            tstring group;
            int sel = m_groups_list.GetCurSel();
            if (sel != -1 && sel != 0)
                m_newworld_name.SetWindowTextW(m_groups_name[sel-1].c_str());
            else
                m_newworld_name.SetWindowTextW(L"");
            m_group_name_changed = false;
        }

        int idx = m_groups_list.GetCurSel();
        if (idx != m_group_idx)
        {
            m_ok.EnableWindow(FALSE);
            m_group_idx = idx;
            updateProfilesList();
        }
        updateOK();
        return 0;
    }

    LRESULT OnProfileChanged(WORD, WORD, HWND, BOOL&)
    {
        updateOK(); 
        return 0;
    }

    LRESULT OnNameChanged(WORD, WORD, HWND, BOOL&)
    {
        m_group_name_changed = true;
        updateOK();
        return 0;
    }

    void updateProfilesList()
    {
        m_list.ResetContent();
        int sel = m_groups_list.GetCurSel();
        if (sel == -1 || sel == 0)
            return;
        tstring gname = m_groups_name[sel-1];
        if (std::find(m_templates.begin(), m_templates.end(), gname) != m_templates.end())
            m_list.AddString(L"- по умолчанию -");
        ProfilesList plist(gname);
        for (int i = 0, e = plist.profiles.size(); i<e; ++i)
        {
            const tstring &p = plist.profiles[i];
            m_list.AddString(p.c_str());
        }
    }

    void updateOK()
    {
        BOOL ok_status = FALSE;
        tstring name, profile;
        getWindowText(m_newworld_name, &name);
        getWindowText(m_newworld_profile, &profile);
        if (!name.empty() && !profile.empty() && isOnlyFilnameSymbols(name) && isOnlyFilnameSymbols(profile))
        {
            bool confilcted_name = false;
            /*for (int i = 0, e = m_profiles_name.size(); i < e; ++i)
            {
                if (m_profiles_name[i] == profile) { confilcted_name = true; break; }
            }*/
            if (!confilcted_name)
            {
                int group = m_groups_list.GetCurSel();
                if (group == 0) ok_status = TRUE;
                else
                {
                    int profile = m_list.GetCurSel();
                    if (profile != -1) ok_status = TRUE;
                }
            }
        }
        m_ok.EnableWindow(ok_status);
    }

    void readTemplates()
    {
        ChangeDir cd;
        if (cd.changeDir(L"resources"))
        {
            ProfilesDirsListHelper ph(L"profiles");
            for (int i=0,e=ph.dirs.size();i<e;++i)
            {
                const tstring& d = ph.dirs[i];
                if (d == default_profile_folder) continue;
                m_templates.push_back(d);
            }
        }
    }
};

class NewProfileDlg : public CDialogImpl<NewProfileDlg>
{
    CListBox m_list;
    CEdit m_name;
    ProfilesList m_plist;
    CButton m_ok;
    tstring m_group;
    tstring m_profile_source;
    tstring m_profile_name;

public:
   enum { IDD = IDD_NEW_PROFILE };

   void loadProfiles(const tstring& group)
   {
        m_group = group;
        m_plist.init(group);
   }

   void getProfile(CopyProfileData *profile)
   {
       profile->src.group = m_group;
       profile->src.name = m_profile_source;
       profile->dst.group = m_group;
       profile->dst.name = m_profile_name;
   }

private:
    BEGIN_MSG_MAP(NewProfileDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_HANDLER(IDC_EDIT_PROFILE_NEW, EN_CHANGE, OnNameChanged)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_list.Attach(GetDlgItem(IDC_LIST_PROFILE_NEW));
        m_name.Attach(GetDlgItem(IDC_EDIT_PROFILE_NEW));
        m_ok.Attach(GetDlgItem(IDOK));

        tstring text(L"Создать пустой профиль");
        m_list.AddString(text.c_str());
        m_list.SetCurSel(0);

        for (int i=0,e=m_plist.profiles.size(); i<e; ++i)
            m_list.AddString(m_plist.profiles[i].c_str());

        CenterWindow(GetParent());
        m_ok.EnableWindow(FALSE);
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnNameChanged(WORD, WORD, HWND, BOOL&)
    {
        tstring text;
        getWindowText(m_name, &text);
        BOOL enable_ok = FALSE;
        if (!text.empty())
        {
            enable_ok = TRUE;
            for (int i=0,e=m_plist.profiles.size(); i<e; ++i)
            {
                if (m_plist.profiles[i] == text)
                    { enable_ok = FALSE; break; }
            }
            if (enable_ok)
            {
                if (!isOnlyFilnameSymbols(text))
                    enable_ok = FALSE;
            }
        }
        m_ok.EnableWindow(enable_ok);
        return 0;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        getWindowText(m_name, &m_profile_name);
        int src_index = m_list.GetCurSel();
        if (src_index != 0)
            m_profile_source = m_plist.profiles[src_index-1];
        EndDialog(IDOK);
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}
};

class CStartupWorldDlg : public CDialogImpl<CStartupWorldDlg>
{
    CListBox m_list;
    CButton  m_show_about;
    CButton  m_ok;
    CEdit    m_edit_profile_folder, m_edit_profile_name;
    std::vector<tstring> m_data;
    int  m_selected_item;
    bool m_show_help;
    tstring m_profile_name;
    tstring m_profile_group;
    tstring m_source_name;
    tstring m_sorce_group;
    bool m_folder_changed;

public:
    CStartupWorldDlg() : m_selected_item(-1), m_show_help(true), m_folder_changed(false) {}
    enum { IDD = IDD_STARTUP_WORLD };
    void setList(const std::vector<tstring>& list)
    {
        m_data.assign(list.begin(), list.end());
    }
    //int getItem() const { return m_selected_item; }
    bool getHelpState() const { return m_show_help; }
    void getProfile(Profile *p) const
    {
        p->group.assign(m_profile_group);
        p->name.assign(m_profile_name);
    }
    void getSourceProfile(Profile *p) const
    {
        p->name.assign(m_source_name);
        p->group.assign(m_sorce_group);
    }
private:
    BEGIN_MSG_MAP(CStartupWorldDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_USER+1, OnFocus)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_HANDLER(IDC_EDIT_STARTUP_FOLDER, EN_CHANGE, OnFolderChanged)
        COMMAND_HANDLER(IDC_EDIT_STARTUP_PROFILE, EN_CHANGE, OnNameChanged)
        COMMAND_HANDLER(IDC_LIST_STARTUP_WORLD, LBN_SELCHANGE, OnListItemChanged)
    END_MSG_MAP()

    void getSeletedGroup(tstring *group)
    {
        int idx = m_list.GetCurSel();
        int len = m_list.GetTextLen(idx) + 1;
        MemoryBuffer g(len * sizeof(tchar));
        tchar *buffer = (tchar*)g.getData();
        m_list.GetText(idx, buffer);
        group->assign(buffer);
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        CenterWindow(GetParent());
        m_list.Attach(GetDlgItem(IDC_LIST_STARTUP_WORLD));
        m_show_about.Attach(GetDlgItem(IDC_CHECK_OPEN_ABOUT));
        m_ok.Attach(GetDlgItem(IDOK));
        m_edit_profile_folder.Attach(GetDlgItem(IDC_EDIT_STARTUP_FOLDER));
        m_edit_profile_name.Attach(GetDlgItem(IDC_EDIT_STARTUP_PROFILE));
        m_edit_profile_name.SetWindowText(default_profile_name);
        m_list.AddString(L"Создать пустой профиль");
        for (int i=0,e=m_data.size();i<e;++i)
        {
            if (m_data[i]==default_profile_folder) continue;
            m_list.AddString(m_data[i].c_str());
        }
        m_show_about.SetCheck(BST_CHECKED);
        PostMessage(WM_USER+1, 0, 0);
        return TRUE;
    }

    LRESULT OnOk(WORD, WORD wID, HWND, BOOL&)
    {
        m_selected_item = m_list.GetCurSel();
        if (m_selected_item > 0) {
            getSeletedGroup(&m_sorce_group);
        } else {
            m_sorce_group = default_profile_folder;
        }
        m_source_name = default_profile_name;
        m_show_help = (m_show_about.GetCheck() == BST_CHECKED) ? true : false;
        getWindowText(m_edit_profile_name, &m_profile_name);
        getWindowText(m_edit_profile_folder, &m_profile_group);
        EndDialog(IDOK);
        return 0;
    }

    LRESULT OnCancel(WORD, WORD wID, HWND, BOOL&)
    {
        m_sorce_group = default_profile_folder;
        m_source_name = default_profile_name;
        m_profile_group = m_sorce_group;
        m_profile_name = m_source_name;
        EndDialog(IDCANCEL);
        return 0;
    }

    LRESULT OnFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_list.SetCurSel(0);
        m_list.SetFocus();
        if (!m_folder_changed)
        {
            m_edit_profile_folder.SetWindowText(default_profile_folder);
            m_folder_changed = false;
        }
        return 0;
    }
 
    LRESULT OnListItemChanged(WORD, WORD, HWND, BOOL&)
    {
        if (!m_folder_changed)
        {
            int idx = m_list.GetCurSel();
            if (idx == 0) {
               m_edit_profile_folder.SetWindowText(default_profile_folder);
               m_folder_changed = false;
               return 0;
            }
            tstring group;
            getSeletedGroup(&group);
            m_edit_profile_folder.SetWindowText(group.c_str());
            m_folder_changed = false;
        }
        return 0;
    }

    LRESULT OnFolderChanged(WORD, WORD, HWND, BOOL&)
    {
        m_folder_changed = true;
        tstring text;
        getWindowText(m_edit_profile_folder, &text);
        tstring_trim(&text);
        BOOL ok = (!text.empty() && isOnlyFilnameSymbols(text)) ? TRUE : FALSE;
        m_ok.EnableWindow(ok);
        return 0;
    }

    LRESULT OnNameChanged(WORD, WORD, HWND, BOOL&)
    {
        tstring text;
        getWindowText(m_edit_profile_name, &text);
        tstring_trim(&text);
        BOOL ok = (!text.empty() && isOnlyFilnameSymbols(text)) ? TRUE : FALSE;
        m_ok.EnableWindow(ok);
        return 0;
    }
};
