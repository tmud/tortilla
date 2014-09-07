#pragma once

class ProfilesGroupList
{
    std::vector<tstring> m_groups_list;
    int m_last_accessed;

public:  
    ProfilesGroupList();
    bool init();
    int getCount() const;
    void getName(int index, tstring* name) const;
    int getLast() const;

private:
    bool getFileTime(const tstring& file, FILETIME *ft);
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


