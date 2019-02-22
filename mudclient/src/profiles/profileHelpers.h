#pragma once
#include "propertiesPages/propertiesData.h"

class ProfilesGroupList
{
    std::vector<tstring> m_groups_list;
    int m_last_accessed;
public:
    ProfilesGroupList();
    void init();
    int  getCount() const;
    int  getLast() const;
    void getName(int index, tstring* name) const;
private:
    bool getFileTime(const tstring& file, FILETIME *ft);
};

class ProfilesList
{
public:
    ProfilesList();
    ProfilesList(const tstring& group);
    void init(const tstring& group);
    std::vector<tstring> profiles;
};

class FilesList
{
public:
    FilesList(const tstring& dir);
    std::vector<tstring> files;
    std::vector<tstring> dirs;
};

class NewProfileHelper
{
    typedef std::pair<tstring,int> group_status;
    std::vector<group_status> m_groups;
    bool m_first_startup;
    Profile m_created_profile;
public:
    NewProfileHelper();
    bool createFromResources(const ProfilesGroupList& groups);
    bool copy(const Profile& src, const Profile& dst);
    bool isFirstStartUp() const { return m_first_startup; }
    const Profile& getProfile() const { return m_created_profile; }

private:
    bool createSettingsFile(const Profile& profile);
    bool createEmptyProfile(const Profile& profile);
    bool createFromResource(const Profile& src, const Profile& dst);
};