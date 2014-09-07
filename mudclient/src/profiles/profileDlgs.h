#pragma once
#include "profileHelpers.h"

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

   void getProfiles(tstring *group, tstring *name)
   {
       group->assign(m_profile_group);
       name->assign(m_profile_name);
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
        if (groups.init())
        {
            for (int i=0,e=groups.getCount(); i<e; ++i)
            {
                tstring gname;
                groups.getName(i, &gname);
                ProfilesList plist;
                plist.init(gname);
                if (plist.getCount() == 0)
                    continue;
                m_groups_name.push_back(gname);
                m_groups_list.AddString(gname.c_str());
            }

            int last = groups.getLast();
            m_groups_list.SetCurSel(last);
            m_group_idx = last;
            updateProfilesList();
        }
        m_ok.EnableWindow(FALSE);
        CenterWindow(GetParent());
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
        ProfilesList plist;
        plist.init(gname);
        for (int i=0,e=plist.getCount(); i<e; ++i)
        {
            tstring name;
            plist.getName(i, &name);
            m_list.AddString(name.c_str());
        }
    }
};

struct NewWorldDlgData
{
    tstring name;
    tstring profile;
    tstring from_name;
    tstring from_profile;
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
    NewWorldDlgData m_nwdata;

public:
    enum { IDD = IDD_NEW_WORLD };
    NewWorldDlg() : m_group_idx(-1) {}

    void getData(NewWorldDlgData* data)
    {
        *data = m_nwdata;
    }

private:
    BEGIN_MSG_MAP(NewWorldDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_HANDLER(IDC_LIST_PROFILE_GROUP, LBN_SELCHANGE, OnGroupChanged)
        COMMAND_HANDLER(IDC_LIST_PROFILE, LBN_SELCHANGE, OnProfileChanged)
        COMMAND_HANDLER(IDC_EDIT_NEWWORLD_NAME, EN_CHANGE, OnNameChanged)
        COMMAND_HANDLER(IDC_EDIT_NEWWORLD_PROFILE, EN_CHANGE, OnNameChanged)
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
        if (groups.init())
        {
            for (int i = 0, e = groups.getCount(); i<e; ++i)
            {
                tstring gname;
                groups.getName(i, &gname);
                ProfilesList plist;
                plist.init(gname);
                if (plist.getCount() == 0)
                    continue;
                m_groups_name.push_back(gname);
                m_groups_list.AddString(gname.c_str());
            }                        
            updateProfilesList();
        }
        m_ok.EnableWindow(FALSE);
        CenterWindow(GetParent());
        return 0;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        getWindowText(m_newworld_name, &m_nwdata.name);
        getWindowText(m_newworld_profile, &m_nwdata.profile);
        int group = m_groups_list.GetCurSel();
        if (group != 0)
        {
            m_nwdata.from_name = m_groups_name[group - 1];
            int idx = m_list.GetCurSel();
            int len = m_list.GetTextLen(idx) + 1;
            MemoryBuffer g(len * sizeof(WCHAR));
            WCHAR *buffer = (WCHAR*)g.getData();
            m_list.GetText(idx, buffer);
            m_nwdata.from_profile = buffer;
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
        ProfilesList plist;
        plist.init(gname);
        for (int i = 0, e = plist.getCount(); i<e; ++i)
        {
            tstring name;
            plist.getName(i, &name);
            m_list.AddString(name.c_str());
        }
    }

    void updateOK()
    {
        BOOL ok_status = FALSE;        
        tstring name, profile;
        getWindowText(m_newworld_name, &name);
        getWindowText(m_newworld_profile, &profile);
        if (!name.empty() && !profile.empty() && !IsExistIncorrectSymbols(name) && !IsExistIncorrectSymbols(profile))
        {
            bool confilcted_name = false;
            for (int i = 0, e = m_groups_name.size(); i < e; ++i)
            {
                if (m_groups_name[i] == name) { confilcted_name = true; break; }
            }
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
};

class NewProfileDlg : public CDialogImpl<NewProfileDlg>
{
    CListBox m_list;
    CEdit m_name;
    ProfilesList m_plist;
    CButton m_ok;
    tstring m_profile_source;
    tstring m_profile_name;

public:
   enum { IDD = IDD_NEW_PROFILE };

   void loadProfiles(const tstring& group)
   {
        m_plist.init(group);
   }

   void getProfiles(tstring *source, tstring *name)
   {
       source->assign(m_profile_source);
       name->assign(m_profile_name);
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
        
        tstring text;
        loadString(IDS_NEW_PROFILE, &text);
        m_list.AddString(text.c_str());
        m_list.SetCurSel(0);

        for (int i=0,e=m_plist.getCount(); i<e; ++i)
        {
            tstring profile;
            m_plist.getName(i, &profile);
            m_list.AddString(profile.c_str());            
        }
        m_ok.EnableWindow(FALSE);
        CenterWindow(GetParent());
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
            for (int i=0,e=m_plist.getCount(); i<e; ++i)
            {
                tstring profile;
                m_plist.getName(i, &profile);
                if (profile == text)
                    { enable_ok = FALSE; break; }
            }
            if (enable_ok)
            {
                if (IsExistIncorrectSymbols(text))
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
            m_plist.getName(src_index-1, &m_profile_source);
        EndDialog(IDOK);
        return 0;
    }
    
    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}
};
