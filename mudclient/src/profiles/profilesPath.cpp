#include "stdafx.h"
#include "profilesPath.h"
#include "zip.h"

#ifdef _DEBUG
#pragma comment(lib, "libzipd.lib")
#else
#pragma comment(lib, "libzip.lib")
#endif

ProfilesInZipHelper::ProfilesInZipHelper(const char* file)
{
    struct zip_t *zip = zip_open(file, 0, 'r');
    if (!zip)
        return;
    int elements = zip_total_entries(zip);
    for (int i = 0; i < elements; ++i)
    {
        zip_entry_openbyindex(zip, i);
        if (!zip_entry_isdir(zip))
        {
            zip_entry_close(zip);
            continue;
        }
        tstring dir(TU2W(zip_entry_name(zip)));
        zip_entry_close(zip);
        if (dir.empty())
        {
            assert(false);
            continue;
        }
        int last = dir.size() - 1;
        if (dir[last] != L'/')
        {
            assert(false);
            continue;
        }
        dir = dir.substr(0, last);
        if (dir.find(L'/') != tstring::npos)
            continue;
        dirs.push_back(dir);
    }
    zip_close(zip);
}

ProfilesDirsListHelper::ProfilesDirsListHelper(const tstring& dir)
{
    WIN32_FIND_DATA fd;
    memset(&fd, 0, sizeof(WIN32_FIND_DATA));
    tstring mask(dir); mask.append(L"\\*.*");
    HANDLE file = FindFirstFile(mask.c_str(), &fd);
    if (file != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L".."))
                    dirs.push_back(fd.cFileName);
            }
        } while (::FindNextFile(file, &fd));
        ::FindClose(file);
    }
}

bool ProfileDirHelper::makeDir(const tstring& config, const tstring& dir)
{
    return makeDirEx(config, dir, L"");
}

bool ProfileDirHelper::makeDirEx(const tstring& config, const tstring& dir, const tstring& filename)
{
    if (config.empty() || dir.empty())
        return false;

    tstring d(L"gamedata");
    ChangeDir cd;
    if (!cd.changeDir(d))
    {
        CreateDirectory(d.c_str(), NULL);
        if (!cd.changeDir(d))
            return false;
    }
    if (!select(config) || !select(dir))
        return false;

    if (!filename.empty())
    {
        Tokenizer tk(filename.c_str(), L"\\/");
        for (int i = 0, e = tk.size() - 1; i < e; ++i)
        {
            if (!select(tk[i]))
                return false;
        }
    }
    return true;
}

bool ProfileDirHelper::select(const tstring& dir)
{
    const wchar_t *dpath = dir.c_str();
    if (!SetCurrentDirectory(dpath))
    {
        if (!CreateDirectory(dpath, NULL))
            return false;
        if (!SetCurrentDirectory(dpath))
            return false;
    }
    return true;
}

ProfilePath::ProfilePath(const tstring& profile, const tstring& file)
{
    m_path.assign(L"gamedata\\");
    m_path.append(profile);
    m_path.append(L"\\");
    m_path.append(file);
    tstring_replace(&m_path, L"/", L"\\");
}

ProfilePluginPath::ProfilePluginPath(const tstring& profile, const tstring& plugin, const tstring& file)
{
    m_path.assign(L"gamedata\\");
    m_path.append(profile);
    m_path.append(L"\\");
    m_path.append(plugin);
    m_path.append(L"\\");
    m_path.append(file);
    tstring_replace(&m_path, L"/", L"\\");
}

GlobalProfilePath::GlobalProfilePath()
{
    tchar szPath[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath);
    m_path.assign(szPath);
    int last = m_path.length() - 1;
    if (m_path.at(last) != L'\\')
        m_path.append(L"\\");
    m_path.append(L"tortilla.xml");
}