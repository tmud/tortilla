#pragma once

class ProfilesGroupList
{
    std::vector<tstring> m_groups_list;
    int m_last_accessed;
    bool m_empty_group_list;
    bool m_first_startup;

public:
    ProfilesGroupList();
    bool init();
    int getCount() const;
    void getName(int index, tstring* name) const;
    int getLast() const;
    bool isEmptyGroupList() const { return m_empty_group_list; }
    bool isFirstStartUp() const { return m_first_startup; }

private:
    bool getFileTime(const tstring& file, FILETIME *ft);
    void initEmptyGroupList(const std::vector<tstring>& dirs);
};

class ProfilesList
{
    std::vector<tstring> m_profiles_list;
public:
    void init(const tstring& group);
    int  getCount() const;
    void getName(int index, tstring* name) const;
};

bool IsExistIncorrectSymbols(const tstring& s);
