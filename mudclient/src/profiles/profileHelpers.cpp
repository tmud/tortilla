#include "stdafx.h"
#include "profileHelpers.h"
#include "profilesPath.h"
#include "profileDlgs.h"
#include "AboutDlg.h"

ProfilesGroupList::ProfilesGroupList() : m_last_accessed(-1)
{
}

void ProfilesGroupList::init()
{
    // find last changed settings.xml
    ProfilesDirsListHelper ph(L"gamedata");
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
        }
        m_last_accessed = index;
    }
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
//---------------------------------------------------------------------------
ProfilesList::ProfilesList() {}
ProfilesList::ProfilesList(const tstring& group) { init(group); }
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
                profiles.push_back(file.substr(0,pos) );
            }
        } while (::FindNextFile(file, &fd));
        ::FindClose(file);
    }
}
//---------------------------------------------------------------------------
FilesList::FilesList(const tstring& dir)
{
    if (dir.empty())
        return;
    int last = dir.size() - 1;
    if (dir[last] == L'\\')
        dirs.push_back(dir.substr(0, last));
    else
        dirs.push_back(dir);
    ChangeDir cd;
    for (size_t i=0; i<dirs.size(); ++i)
    {
        tstring path(dirs[i]);
        if (!SetCurrentDirectory(path.c_str() ))
            break;
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(L"*.*", &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do {
              tstring name(fd.cFileName);
              tstring tmp(path);
              if (!tmp.empty()) tmp.append(L"\\");
              tmp.append(name);
              if ( (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
              {
                  if (name == L"." || name == L"..")
                     continue;
                  dirs.push_back(tmp);
              }
              else
              {
                  files.push_back(tmp);
              }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
        SetCurrentDirectory(cd.getCurrentDir().c_str());
    }
}
//---------------------------------------------------------------------------
NewProfileHelper::NewProfileHelper() : m_first_startup(false)
{
}

bool NewProfileHelper::createFromResources(const ProfilesGroupList& groups)
{
    for (int i=0,e=groups.getCount();i<e;++i) {
        tstring group;
        groups.getName(i, &group);
        m_groups.push_back(group_status(group, 0));
    }
    ChangeDir cd;
    if (cd.changeDir(L"resources"))
    {
        ProfilesDirsListHelper ph(L"profiles");
        for (int i=0,e=ph.dirs.size();i<e;++i)
        {
            const tstring& d = ph.dirs[i];
            int index = -1;
            for (int j=0,je=m_groups.size();j<je;++j) {
                if (m_groups[j].first == d) { index = j; break; }
            }
            if (index == -1) { m_groups.push_back(group_status(d, 1)); }
            else { m_groups[index].second = 2; }
        }
    }

    CStartupWorldDlg dlg;
    std::vector<tstring> list;
    std::for_each(m_groups.begin(),m_groups.end(),[&list](group_status &d) { list.push_back(d.first); } );
    dlg.setList(list);
    if (dlg.DoModal() != IDOK)
        return false;
    m_first_startup = dlg.getHelpState();
    Profile src, dst;
    dlg.getSourceProfile(&src);
    dlg.getProfile(&dst);
    cd.restoreDir();
    m_created_profile = dst;
    if (createFromResource(src, dst))
    {
        return createSettingsFile(dst);
    }
    return createEmptyProfile(dst);
}

bool NewProfileHelper::copy(const Profile& src, const Profile& dst)
{
    ProfilePath pp(src.group, L"");
    tstring p(pp);
    FilesList fl(p);
    tstring path(fl.dirs[0]);
    size_t path_len = path.length()+1;

    if (src.group != dst.group)
    {
        // создаем каталоги в папке назначения
        ProfileDirHelper dh;
        for (int i=1,e=fl.dirs.size();i<e;++i)
        {
            tstring srcdir(fl.dirs[i]);      
            srcdir = srcdir.substr(path_len);
            if (srcdir.empty())
                continue;
            if (!dh.makeDir(dst.group, srcdir))
                return false;
        }
    }
    // копируем файлы из исходника
    // меняем имя файла, если совпадает с src.name
    for (int i=0,e=fl.files.size();i<e;i++)
    {
        tstring dst_file(fl.files[i].substr(path_len));
        tstring filename(dst_file);
        size_t pos = dst_file.rfind(L"\\");
        if (pos != tstring::npos)
            filename = dst_file.substr(pos+1);
        tstring name(filename);
        size_t ext_pos = filename.rfind(L".");
        if (ext_pos != tstring::npos)
            name = filename.substr(0, ext_pos);
        bool name_changed = false;
        if (name == src.name)
            { name = dst.name; name_changed = true; }
        if (ext_pos != tstring::npos)
            name.append(filename.substr(ext_pos));
        filename = name;
        if (pos != tstring::npos)
        {
            filename.assign(dst_file.substr(0, pos+1));
            filename.append(name);
        }
        ProfilePath pp(dst.group, filename);
        DWORD a = GetFileAttributes(pp);
        if (a != INVALID_FILE_ATTRIBUTES && !name_changed)
            continue;
        if (!CopyFile(fl.files[i].c_str(), pp, FALSE))
            return false;
    }
    return true;
}

bool NewProfileHelper::createFromResource(const Profile& src, const Profile& dst)
{
    int index = -1;
    for (int i=0,e=m_groups.size();i<e;++i) {
       if (m_groups[i].first == src.group) { index = i; break; }
    }
    if (index == -1)
        return false;
    int type = m_groups[index].second;

    // список файлов и каталогов
    tstring path(L"resources\\profiles\\");
    path.append(src.group);
    FilesList fl(path);
    size_t path_len = path.length()+1;

    // создаем каталоги в папке назначения
    ProfileDirHelper dh;
    for (int i=1,e=fl.dirs.size();i<e;++i)
    {
        tstring srcdir(fl.dirs[i]);
        srcdir = srcdir.substr(path_len);
        if (srcdir.empty())
            continue;
        if (!dh.makeDir(dst.group, srcdir))
            return false;
    }

    // копируем файлы из исходника
    // меняем имя файла, если совпадает с src.name
    for (int i=0,e=fl.files.size();i<e;i++)
    {
        tstring dst_file(fl.files[i].substr(path_len));
        tstring filename(dst_file);
        size_t pos = dst_file.rfind(L"\\");
        if (pos != tstring::npos)
            filename = dst_file.substr(pos+1);
        tstring name(filename);
        size_t ext_pos = filename.rfind(L".");
        if (ext_pos != tstring::npos)
            name = filename.substr(0, ext_pos);
        bool name_changed = false;
        if (name == src.name)
            { name = dst.name; name_changed = true; }
        if (ext_pos != tstring::npos)
            name.append(filename.substr(ext_pos));
        filename = name;
        if (pos != tstring::npos)
        {
            filename.assign(dst_file.substr(0, pos+1));
            filename.append(name);
        }
        ProfilePath pp(dst.group, filename);
        DWORD a = GetFileAttributes(pp);
        if (a != INVALID_FILE_ATTRIBUTES && !name_changed)
            continue;
        if (!CopyFile(fl.files[i].c_str(), pp, FALSE))
            return false;
    }
    return true;
}

bool NewProfileHelper::createSettingsFile(const Profile& profile)
{
    xml::node f(L"settings");
    xml::node n(f.createsubnode(L"profile"));
    ProfilePath ph(profile.group, L"settings.xml");
    n.settext(profile.name.c_str());
    bool result = f.save(ph);
    f.deletenode();
    return result;
}

bool NewProfileHelper::createEmptyProfile(const Profile& profile)
{
    xml::node p(L"profile");
    tstring dst(L"profiles\\"); dst.append(profile.name); dst.append(L".xml");
    ProfilePath ph(profile.group, dst);
    ProfileDirHelper dh;
    if (!dh.makeDir(profile.group, L"profiles"))
        return false;
    DWORD a = GetFileAttributes(ph);
    bool result = true;
    if (a != INVALID_FILE_ATTRIBUTES && !(a&FILE_ATTRIBUTE_DIRECTORY)) {}
    else { result = p.save(ph); }
    p.deletenode();
    if (result)
        result = createSettingsFile(profile);
    return result;
}
