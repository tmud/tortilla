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

class filewriter {
    HANDLE file;
    static filewriter* callback;
public:
    filewriter() : file(INVALID_HANDLE_VALUE) {
        assert(callback == nullptr);
        callback = this;
    }
    ~filewriter() {
        callback = nullptr;
        if (file != INVALID_HANDLE_VALUE)
            CloseHandle(file);
    }
    bool init(const tstring filename) {
        file = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
            return false;
        return true;
    }
    size_t on_extract(void *arg, unsigned long long offset, const void *data, size_t size)
    {
        if (file == INVALID_HANDLE_VALUE)
            return 0;
        DWORD written = 0;
        if (!WriteFile(file, data, (DWORD)size, &written, NULL))
            return 0;
        return written;
    }

    static size_t clbk_on_extract(void *arg, unsigned long long offset, const void *data, size_t size)
    {
        if (callback)
            return callback->on_extract(arg, offset, data, size);
        return 0;
    }
};
filewriter* filewriter::callback = nullptr;

bool CopyProfileFromZipHelper::copyProfile(const char* file, const tstring& profile, const tstring& targetdir)
{
    struct zip_t *zip = zip_open(file, 0, 'r');
    if (!zip)
        return false;
    bool result = false;
    std::vector<int> files;
    int elements = zip_total_entries(zip);
    for (int i = 0; i < elements; ++i)
    {
        zip_entry_openbyindex(zip, i);
        if (!zip_entry_isdir(zip))
        {
            tstring file(TU2W(zip_entry_name(zip)));
            if (file.find(profile) == 0)
                files.push_back(i);
        }
        zip_entry_close(zip);
    }
    if (!files.empty()) 
    {
        result = true;
        for (int i : files)
        {
            zip_entry_openbyindex(zip, i);
            tstring file(TU2W(zip_entry_name(zip)));

            ProfileDirHelper dh;
            if (!dh.makeDir(targetdir, file))
            {
                return false;
            }
            filewriter fw;
            if (!fw.init(file))
            {
                result = false;
                break;
            }
            if (!zip_entry_extract(zip, &filewriter::clbk_on_extract, 0))
            {
                // success
                zip_entry_close(zip);

            }
            else
            {
                zip_entry_close(zip);
                result = false;
                break;
            }
        }
    }
    zip_close(zip);
    return result;
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