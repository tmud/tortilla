#pragma once
#include "profileHelpers.h"
#include "profilesPath.h"

struct ProfileData
{
    ProfileData() : create_link(false), create_new(false), copy_from_src(false) {}
    Profile profile;
    Profile src;
    bool create_link;
    bool create_new;
    bool copy_from_src;
};

class SelectProfileDlg : public CDialogImpl<SelectProfileDlg>
{
    Profile m_current;
    ProfilesList m_plist;
    ProfileData m_state;

    CComboBox m_groups_list;
    CListBox m_profiles_list;
    CButton m_ok;
    CButton m_create_link;
    CButton m_create_new;
    CButton m_copy_current;
    CEdit m_group_name;
    CEdit m_profile_name;
    CStatic m_label_group_name;
    CStatic m_label_profile_name;
    bool m_create_new_manually;
    std::vector<tstring> m_groups;
    std::vector<tstring> m_profiles;
    bool m_update_edits_mode;
public:
   SelectProfileDlg(const Profile& current) : m_current(current), m_create_new_manually(false),
       m_update_edits_mode(false) {}
   enum { IDD = IDD_PROFILES };
   const ProfileData& getProfile() const
   {
       return m_state;
   }
private:
    BEGIN_MSG_MAP(SelectProfileDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)        
        COMMAND_ID_HANDLER(IDC_CHECK_CREATE_NEW_PROFILE, OnCheckNewProfile)
        COMMAND_HANDLER(IDC_COMBO_GROUPS, CBN_SELCHANGE, OnGroupItemChanged)
        COMMAND_HANDLER(IDC_LIST_PROFILE, LBN_SELCHANGE, OnProfileItemChanged)
        COMMAND_HANDLER(IDC_LIST_PROFILE, LBN_DBLCLK, OnProfileItemSelect)
        COMMAND_HANDLER(IDC_EDIT_NEWPROFILE_GROUP, EN_CHANGE, OnNameChanged)
        COMMAND_HANDLER(IDC_EDIT_NEWPROFILE_NAME, EN_CHANGE, OnNameChanged)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        std::vector<tstring> templates;
        ChangeDir cd;
        if (cd.changeDir(L"resources"))
        {
            ProfilesDirsListHelper ph(L"profiles");
            for (int i = 0, e = ph.dirs.size(); i < e; ++i)
            {
                const tstring& d = ph.dirs[i];
                if (d == default_profile_folder) continue;
                templates.push_back(d);
            }
        }
        cd.restoreDir();
        m_groups_list.Attach(GetDlgItem(IDC_COMBO_GROUPS));
        m_profiles_list.Attach(GetDlgItem(IDC_LIST_PROFILE));
        m_ok.Attach(GetDlgItem(IDOK));
        m_create_link.Attach(GetDlgItem(IDC_CHECK_CREATELINK));
        m_create_new.Attach(GetDlgItem(IDC_CHECK_CREATE_NEW_PROFILE));
        m_copy_current.Attach(GetDlgItem(IDC_CHECK_COPY_CURRENT_PROFILE));
        m_group_name.Attach(GetDlgItem(IDC_EDIT_NEWPROFILE_GROUP));
        m_profile_name.Attach(GetDlgItem(IDC_EDIT_NEWPROFILE_NAME));
        m_label_group_name.Attach(GetDlgItem(IDC_STATIC_GROUP_NAME));
        m_label_profile_name.Attach(GetDlgItem(IDC_STATIC_PROFILE_NAME));

        std::vector<tstring> vgroups;
        ProfilesGroupList groups;
        groups.init();
        {
            int current_group = -1;
            for (int i = 0, e = groups.getCount(); i < e; ++i)
            {
                tstring gname;
                groups.getName(i, &gname);
                ProfilesList plist(gname);
                if (plist.profiles.empty())
                    continue;
                vgroups.push_back(gname);
                m_groups_list.AddString(gname.c_str());
                m_groups.push_back(gname);
                if (gname == m_current.group)
                    current_group = m_groups_list.GetCount() - 1;
            }
            for (int i = 0, e = templates.size(); i < e; ++i)
            {
                const tstring& t = templates[i];
                if (std::find(vgroups.begin(), vgroups.end(), t) == vgroups.end())
                {
                    vgroups.push_back(t);
                    m_groups_list.AddString(t.c_str());
                    m_groups.push_back(t);
                    if (t == m_current.group)
                        current_group = m_groups_list.GetCount() - 1;
                }
            }
            m_groups_list.SetCurSel(current_group);
            updateProfilesList();            
        }
        SetNewProfileGroupStatus(FALSE);
        updateOk();
        CenterWindow(GetParent());
        SetFocus();
        return 0;
    }

    void SetGroupEdit(const tstring& group)
    {
        m_update_edits_mode = true;
        m_group_name.SetWindowText(group.c_str());
        m_update_edits_mode = false;
    }

    void SetProfileEdit(const tstring& profile)
    {
        m_update_edits_mode = true;
        m_profile_name.SetWindowText(profile.c_str());
        m_update_edits_mode = false;
    }

    void SetNewProfileGroupStatus(BOOL status)
    {
        if (!status)
        {
            SetGroupEdit(L"");
            SetProfileEdit(L"");
        }
        m_group_name.EnableWindow(status);           
        m_profile_name.EnableWindow(status);
        m_label_group_name.EnableWindow(status);
        m_label_profile_name.EnableWindow(status);
    }

    LRESULT OnCheckNewProfile(WORD, WORD, HWND, BOOL&)
    {        
        BOOL status = m_create_new.GetCheck() ? TRUE : FALSE;
        SetNewProfileGroupStatus(status);
        m_create_new_manually = (status) ? true : false;        
        updateOk();
        return 0;
    }

    LRESULT OnGroupItemChanged(WORD, WORD, HWND, BOOL&)
    {
        updateProfilesList();
        updateOk();
        return 0;
    }

    LRESULT OnProfileItemChanged(WORD, WORD, HWND, BOOL&)
    {
        updateOk();
        return 0;
    }

    LRESULT OnProfileItemSelect(WORD, WORD, HWND, BOOL&)
    {
        bool newprofile = (m_create_new.GetCheck() == TBSTATE_CHECKED);
        if (!newprofile && m_profiles_list.GetCurSel() != -1)
        {
            BOOL dummy = FALSE;
            OnOk(0, 0, 0, dummy);
        }
        return 0;
    }

    LRESULT OnNameChanged(WORD, WORD, HWND, BOOL&)
    {
        if (!m_update_edits_mode)
            updateOk();
        return 0;
    }

    void updateOk()
    {
        if (m_profiles_list.GetCount() == 0)
        {
            m_create_new.SetCheck(BST_CHECKED);
            SetNewProfileGroupStatus(TRUE);
            updateGroupEdit();
            if (m_profile_name.GetWindowTextLength() == 0)
                SetProfileEdit(L"player");
            updateCopyCurrent();
            m_ok.EnableWindow(TRUE);
            return;
        }
        if (!m_create_new_manually)
        {
            m_create_new.SetCheck(BST_UNCHECKED);
            SetNewProfileGroupStatus(FALSE);
        }
        updateCopyCurrent();
        BOOL state = FALSE;
        int item = m_profiles_list.GetCurSel();
        bool newprofile = (m_create_new.GetCheck() == TBSTATE_CHECKED);
        if (newprofile)
            updateGroupEdit();
        if (item == -1)
        {
            if (newprofile)
            {
                if (m_group_name.GetWindowTextLength() > 0 &&
                    m_profile_name.GetWindowTextLength() > 0)
                {
                    state = checkProfileEdit();
                }
            }
        }
        else
        {
            state = TRUE;
            if (newprofile)
            {
                state = FALSE;
                if (m_group_name.GetWindowTextLength() > 0 &&
                    m_profile_name.GetWindowTextLength() > 0)
                {
                    state = checkProfileEdit();
                }
            }
        }
        m_ok.EnableWindow(state);
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        m_state.create_link = (m_create_link.GetCheck() == TBSTATE_CHECKED);
        m_state.src.name.clear();
        m_state.src.group.clear();
        m_state.copy_from_src = false;
        m_state.create_new = false;
        bool newprofile = (m_create_new.GetCheck() == TBSTATE_CHECKED);
        if (!newprofile)
        {
            getCurrentGroup(&m_state.profile.group);
            getCurrentProfile(&m_state.profile.name);
        }
        else
        {
            m_state.create_new = true;
            m_state.copy_from_src = (m_copy_current.GetCheck() == TBSTATE_CHECKED);
            getGroupEditText(&m_state.profile.group);
            getProfileEditText(&m_state.profile.name);
            if (m_state.copy_from_src)
            {
                getCurrentGroup(&m_state.src.group);
                getCurrentProfile(&m_state.src.name);
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

    void updateCopyCurrent()
    {
        bool newprofile = (m_create_new.GetCheck() == TBSTATE_CHECKED);
        if (!newprofile)
        {
            m_copy_current.SetState(BST_UNCHECKED);
            m_copy_current.EnableWindow(FALSE);
            return;
        }
        BOOL state = FALSE;
        int sel = m_profiles_list.GetCurSel();
        if (sel != -1)
            state = TRUE;
        m_copy_current.EnableWindow(state);
    }
    
    void updateProfilesList()
    {
        m_profiles_list.ResetContent();
        m_profiles.clear();
        tstring gname;
        getCurrentGroup(&gname);
        ProfilesList plist(gname);
        for (int i = 0, e = plist.profiles.size(); i < e; ++i)
        {
            m_profiles_list.AddString(plist.profiles[i].c_str());
            m_profiles.push_back(plist.profiles[i]);
        }
    }

    void updateGroupEdit()
    {
        tstring gname_edit;
        getGroupEditText(&gname_edit);
        tstring gname;
        getCurrentGroup(&gname);
        if (std::find(m_groups.begin(), m_groups.end(), gname_edit) != m_groups.end())
        {
            SetGroupEdit(gname);
            return;
        }
        if (gname_edit.empty())
            SetGroupEdit(gname.c_str());
    }

    void getGroupEditText(tstring* group)
    {
        int len = m_group_name.GetWindowTextLength() + 1;
        tchar *buffer = new tchar[len + 1];
        m_group_name.GetWindowText(buffer, len);
        group->assign(buffer);
        delete[]buffer;
    }

    BOOL checkProfileEdit()
    {
        tstring profile_edit;
        getProfileEditText(&profile_edit);
        if (profile_edit.empty())
            return FALSE;
        if (std::find(m_profiles.begin(), m_profiles.end(), profile_edit) != m_profiles.end())
        {
            return FALSE;
        }
        return TRUE;
    }

    void getProfileEditText(tstring* profile)
    {
        int len = m_profile_name.GetWindowTextLength() + 1;
        tchar *buffer = new tchar[len + 1];
        m_profile_name.GetWindowText(buffer, len);
        profile->assign(buffer);
        delete[]buffer;
    }

    void getCurrentGroup(tstring *group)
    {
        assert(group);
        int sel = m_groups_list.GetCurSel();
        if (sel == -1) {
            group->clear();
            return;
        }
        int len = m_groups_list.GetLBTextLen(sel);
        tchar *buffer = new tchar[len + 1];
        m_groups_list.GetLBText(sel, buffer);
        group->assign(buffer);
        delete[]buffer;
    }

    void getCurrentProfile(tstring* profile)
    {
        assert(profile);
        int sel = m_profiles_list.GetCurSel();
        if (sel == -1) {
            profile->clear();
            return;
        }
        int len = m_profiles_list.GetTextLen(sel);
        tchar *buffer = new tchar[len + 1];
        m_profiles_list.GetText(sel, buffer);
        profile->assign(buffer);
        delete[]buffer;
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
