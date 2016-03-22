#include "stdafx.h"
#include "profileHelpers.h"
#include "profilesPath.h"
#include "AboutDlg.h"

ProfilesGroupList::ProfilesGroupList() : m_last_accessed(-1), m_first_startup(false)
{
}

bool ProfilesGroupList::init()
{
    std::vector<tstring> empty_dirs;

    // find last changed settings.xml
    ProfilesDirsListHelper ph;
    {
        FILETIME ft0; memset(&ft0, 0, sizeof(FILETIME));
        int index = -1;
        for (int i=0,e=ph.dirs.size(); i<e; ++i)
        {
            tstring f(ProfilePath(ph.dirs[i], L"settings.xml"));
            FILETIME ft;
            if (getFileTime(f, &ft))
            {
                m_groups_list.push_back(ph.dirs[i]);
                if (CompareFileTime(&ft0, &ft) == -1)
                {
                    ft0 = ft; 
                    index = m_groups_list.size()-1;
                }
            }
            else
            {
                empty_dirs.push_back(ph.dirs[i]);
            }
        }
        m_last_accessed = index;
    }

    for (int i = 0, e = empty_dirs.size(); i < e; ++i)
    {
       const tstring& group = empty_dirs[i];
       if (copyProfileFile(group, L"player.txml", L"player"))
           m_groups_list.push_back(group);
    }

    if (m_last_accessed == -1)
        return initDefaultProfile();

    return true;
}

int ProfilesGroupList::getCount() const
{
    return m_groups_list.size();
}

int ProfilesGroupList::getLast() const
{
    return m_last_accessed;
}

void ProfilesGroupList::getName(int index, tstring* name) const
{
    name->assign(m_groups_list[index]);
}

bool ProfilesGroupList::getFileTime(const tstring& file, FILETIME *ft)
{
    memset(ft, 0, sizeof(FILETIME));
    HANDLE hf = CreateFile(file.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, 
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE)
        return false;
    bool result = (GetFileTime(hf, NULL, NULL, ft)) ? true : false;
    CloseHandle(hf);
    return result;
}

bool ProfilesGroupList::initDefaultProfile()
{
    tstring profile(L"player");
    if (!m_groups_list.empty())
    {
        CStartupWorldDlg dlg;
        dlg.setList(m_groups_list);
        if (dlg.DoModal() == IDOK)      
        {
            m_last_accessed = dlg.getItem();
            m_first_startup = dlg.getHelpState();            
            dlg.getProfileName(&profile);
            if (m_last_accessed != -1) 
            {
                const tstring& group = m_groups_list[m_last_accessed];
                if (copyProfileFile(group, L"player.txml", profile))
                    return true;
                return createEmptyProfileFile(group, profile);
            }
        }
    }

    tstring group(L"mudworld");
    for (int i=0,e=m_groups_list.size(); i<e; ++i)
    {
         if (m_groups_list[i] == group)
            { m_last_accessed = i; break; }
    }

    if (m_last_accessed == -1)
    {
         m_groups_list.push_back(group);
         m_last_accessed = m_groups_list.size()-1;
    }
    ProfileDirHelper dh;
    if (!dh.makeDir(group, L"profiles"))
         return false;
    createEmptyProfileFile(group, profile);
    return true;
}

bool ProfilesGroupList::copyProfileFile(const tstring& group, const tstring &srcfile, const tstring& profile)
{
    bool result = false;
    tstring src(L"profiles\\"); src.append(srcfile);
    ProfilePath ph1(group, src);
    DWORD a = GetFileAttributes(ph1);
    if (a != INVALID_FILE_ATTRIBUTES && !(a&FILE_ATTRIBUTE_DIRECTORY))
    {
        tstring dst(L"profiles\\"); dst.append(profile); dst.append(L".xml");
        ProfilePath ph2(group, dst);
        DWORD b = GetFileAttributes(ph2);
        if (b != INVALID_FILE_ATTRIBUTES && !(b&FILE_ATTRIBUTE_DIRECTORY)) {
            result = true;
        } else {
            result = CopyFile(ph1, ph2, FALSE) ? true : false;
        }
        if (result)
            result = createSettingsFile(group, profile);
    }
    return result;
}

bool ProfilesGroupList::createEmptyProfileFile(const tstring& group, const tstring& profile)
{
    xml::node p(L"profile");
    tstring dst(L"profiles\\"); dst.append(profile); dst.append(L".xml");
    ProfilePath ph(group, dst);
    DWORD a = GetFileAttributes(ph);
    bool result = true;
    if (a != INVALID_FILE_ATTRIBUTES && !(a&FILE_ATTRIBUTE_DIRECTORY)) {}
    else { result = p.save(ph); }
    p.deletenode();
    if (result)
        result = createSettingsFile(group, profile);
    return result;
}

bool ProfilesGroupList::createSettingsFile(const tstring& group, const tstring& profile)
{
    xml::node f(L"settings");
    xml::node n(f.createsubnode(L"profile"));
    ProfilePath ph(group, L"settings.xml");
    n.settext(profile.c_str());
    bool result = f.save(ph);
    f.deletenode();
    return result;
}
//---------------------------------------------------------------------------
void ProfilesList::init(const tstring& group)
{
    ProfilePath ph(group, L"profiles\\*.xml");
    WIN32_FIND_DATA fd;
    memset(&fd, 0, sizeof(WIN32_FIND_DATA));
    HANDLE file = FindFirstFile(ph, &fd);
    if (file != INVALID_HANDLE_VALUE)
    {
        do {
            if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            {
                tstring file(fd.cFileName);
                int pos = file.rfind(L".");
                m_profiles_list.push_back(file.substr(0,pos) );
            }
        } while (::FindNextFile(file, &fd));
        ::FindClose(file);
    }
}

int ProfilesList::getCount() const
{
    return m_profiles_list.size();
}

void ProfilesList::getName(int index, tstring* name) const
{
    name->assign(m_profiles_list[index]);
}
