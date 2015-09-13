#pragma once

class ProfilesGroupList
{
    std::vector<tstring> m_groups_list;
    int m_last_accessed;
    bool m_first_startup;
public:
    ProfilesGroupList();
    bool init();
    int  getCount() const;
    int  getLast() const;
    void getName(int index, tstring* name) const;
    bool isFirstStartUp() const { return m_first_startup; }

private:
    bool getFileTime(const tstring& file, FILETIME *ft);
    bool initDefaultProfile();
    bool copyProfileFile(const tstring& group, const tstring &srcfile, const tstring& profile);
    bool createEmptyProfileFile(const tstring& group, const tstring& profile);
    bool createSettingsFile(const tstring& group, const tstring& profile);
};

class ProfilesList
{
    std::vector<tstring> m_profiles_list;
public:
    void init(const tstring& group);
    int  getCount() const;
    void getName(int index, tstring* name) const;
};

