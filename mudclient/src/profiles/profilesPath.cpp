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

class zipfilewriter
{
    HANDLE file;
    static zipfilewriter* callback;
public:
    zipfilewriter() : file(INVALID_HANDLE_VALUE)
    {
    }
    ~zipfilewriter()
    {
        callback = nullptr;
        if (file != INVALID_HANDLE_VALUE)
            CloseHandle(file);
    }
    bool init(const tstring& filename)
    {
        assert(callback == nullptr);
        callback = this;
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
zipfilewriter* zipfilewriter::callback = nullptr;

bool CopyProfileFromZipHelper::copyProfile(const char* zipfile, const Profile& src, const Profile& dst)
{
    struct zip_t *zip = zip_open(zipfile, 0, 'r');
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
            if (file.find(src.group) == 0)
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
            tstring dst_file(TU2W(zip_entry_name(zip)));
            dst_file = dst_file.substr(src.group.length() + 1);

            tstring filename(dst_file);
            size_t pos = dst_file.rfind(L"/");
            if (pos != tstring::npos)
                filename = dst_file.substr(pos + 1);
            tstring name(filename);
            size_t ext_pos = filename.rfind(L".");
            if (ext_pos != tstring::npos)
                name = filename.substr(0, ext_pos);
            bool name_changed = false;
            if (name == src.name) {
                name = dst.name; name_changed = true;
            }
            if (ext_pos != tstring::npos)
                name.append(filename.substr(ext_pos));
            filename = name;
            if (pos != tstring::npos)
            {
                filename.assign(dst_file.substr(0, pos + 1));
                filename.append(name);
            }
            ProfilePath pp(dst.group, filename);
            DWORD a = GetFileAttributes(pp);
            if (a != INVALID_FILE_ATTRIBUTES && !name_changed)
            {
                zip_entry_close(zip);
                continue;
            }

            size_t last = filename.find_last_of(L'/');
            if (last == tstring::npos)
            {
                result = false;
                break;
            }
            tstring dir(filename.substr(0, last));
            ProfileDirHelper dh;
            if (!dh.makeDir(dst.group, dir))
            {
                result = false;
                break;
            }
            tstring filepath(pp);
            zipfilewriter fw;
            if (!fw.init(filepath))
            {
                result = false;
                break;
            }
            bool success = (!zip_entry_extract(zip, &zipfilewriter::clbk_on_extract, 0));
            if (!success)
            {
                result = false;
                break;
            }
            zip_entry_close(zip);
        }
        if (!result)
            zip_entry_close(zip);
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