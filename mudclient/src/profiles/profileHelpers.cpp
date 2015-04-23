#include "stdafx.h"
#include "profileHelpers.h"
#include "profilesPath.h"

ProfilesGroupList::ProfilesGroupList() : m_last_accessed(-1)
{
}

bool ProfilesGroupList::init()
{
    ProfilesDirsListHelper ph;
    // find last changed config
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

    if (m_groups_list.empty())
        initEmptyGroupList(ph.dirs);

    tstring config(L"mudworld");
    if (!m_groups_list.empty())
        config.assign(m_groups_list[m_last_accessed]);
    else
    {
        m_groups_list.push_back(config);
        m_last_accessed = 0;
    }

    ProfileDirHelper dh;
    if (!dh.makeDir(config, L"profiles"))
        return false;
    return true;
}

int ProfilesGroupList::getCount() const
{
    return m_groups_list.size();
}

void ProfilesGroupList::getName(int index, tstring* name) const
{
    name->assign(m_groups_list[index]);
}

int ProfilesGroupList::getLast() const
{
    return m_last_accessed;
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

void ProfilesGroupList::initEmptyGroupList(const std::vector<tstring>& dirs)
{
   xml::node f("settings");
   xml::node n(f.createsubnode("profile"));
   for (int i = 0, e = dirs.size(); i < e; ++i)
   {
      const tstring& group = dirs[i];
      ProfilePath ph(group, L"settings.xml");
      ProfilesList pl;
      pl.init(group);
      if (pl.getCount())
      {
          tstring name;
          pl.getName(0, &name);
          n.settext(TW2U(name.c_str()));
          f.save(TW2U(ph));
          m_groups_list.push_back(group);
      }
      else
      {
          ProfilePath ph1(group, L"profiles\\player.txml");
          DWORD a = GetFileAttributes(ph1);
          if (a != INVALID_FILE_ATTRIBUTES && !(a&FILE_ATTRIBUTE_DIRECTORY))
          {
              ProfilePath ph2(group, L"profiles\\player.xml");
              if (CopyFile(ph1, ph2, FALSE))
              {
                  n.settext("player");
                  f.save(TW2U(ph));
                  m_groups_list.push_back(group);
              }
          }
      }
  }
  f.deletenode();
  if (m_groups_list.empty())
      return;
  m_last_accessed = 0;
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

bool IsExistIncorrectSymbols(const tstring& s)
{
    return (isExistSymbols(s, L"*?:/<>|\"\\")) ? true : false;
}
