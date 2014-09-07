#pragma once

class ProfileDirHelper
{
public:
    bool makeDir(const tstring& config, const tstring& dir);
    bool makeDirEx(const tstring& config, const tstring& dir, const tstring& filename);
private:
    bool select(const tstring& dir);
};

class ProfilesListHelper
{
public:
    ProfilesListHelper();
    std::vector<tstring> profiles;
};

class ProfilePath
{
public:
    ProfilePath(const tstring& profile, const tstring& file);    
    operator const tchar*() { return m_path.c_str(); }
private:
    tstring m_path;
};

class ProfilePluginPath
{
public:
    ProfilePluginPath(const tstring& profile, const tstring& plugin, const tstring& file);
    operator const tchar*() { return m_path.c_str(); }
private:
    tstring m_path;
};
