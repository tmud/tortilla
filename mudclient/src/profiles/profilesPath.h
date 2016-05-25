#pragma once

class ProfileDirHelper
{
public:
    bool makeDir(const tstring& config, const tstring& dir);
    bool makeDirEx(const tstring& config, const tstring& dir, const tstring& filename);
private:
    bool select(const tstring& dir);
};

class ProfilesDirsListHelper
{
public:
    ProfilesDirsListHelper(const tstring& dir);
    std::vector<tstring> dirs;
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
    operator const tchar*() const { return m_path.c_str(); }
private:
    tstring m_path;
};

class GlobalProfilePath
{
public:
    GlobalProfilePath();
    operator const tchar*() const { return m_path.c_str(); }
private:
    tstring m_path;
};