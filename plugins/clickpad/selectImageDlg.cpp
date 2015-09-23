#include "stdafx.h"
#include "clickpad.h"
#include "selectImageDlg.h"

void error(const tchar* msg, const tchar* p1 = NULL)
{
    tstring errtext(msg);
    if (p1) errtext.append(p1);
    TW2U text(errtext.c_str());
    luaT_log(getLuaState(), text);
}

ImageCollection::~ImageCollection()
{
    for (int i=0,e=m_files.size();i<e;++i)
        delete m_files[i].image;
}

int ImageCollection::getImagesCount() const
{
    return m_files.size();
}

const ImageCollection::imdata& ImageCollection::getImage(int index) const
{
    assert(index >= 0 && index < getImagesCount());
    return m_files[index];
}

void ImageCollection::scanImages()
{
    u8string sdir;
    base::getResource(getLuaState(), "", &sdir);
    tstring dir(TU2W(sdir.c_str()));
    if (!(GetFileAttributes(dir.c_str()) & FILE_ATTRIBUTE_DIRECTORY))
        return error(L"Невозможно прочитать каталог с иконками: ", dir.c_str());

    // 1. get current files list
    std::vector<imdata> current_files;
    tchar current_path[MAX_PATH + 1];
    GetCurrentDirectory(MAX_PATH, current_path);
    std::vector<tstring> dirs;
    dirs.push_back(dir);
    for (int index=0; index != dirs.size(); ++index)
    {
        tstring fullpath(dirs[index]);
        if (!SetCurrentDirectory(fullpath.c_str()))
            continue;
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(L"*.*", &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do
            {
                tstring filename(fd.cFileName);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (filename == L"." || filename == L"..")
                        continue;
                    tstring newdir(fullpath);
                    newdir.append(filename);
                    newdir.append(L"\\");
                    dirs.insert(dirs.begin()+index+1, newdir);
                }
                else
                {
                    size_t e = filename.find_last_of(L'.');
                    if (e == -1) continue;
                    tstring ext(filename.substr(e + 1));
                    bool ok = (ext == L"png" || ext == L"bmp" || ext == L"gif" || ext == L"ico" || ext == L"jpg") ? true : false;
                    if (!ok)
                        continue;
                    imdata f;
                    f.name = filename;
                    tstring &p = f.file_path;
                    p.append(fullpath);
                    p.append(filename);
                    f.image_size = 0;
                    size_t end = fullpath.find_last_of(L'\\');
                    if (end != -1)
                    {
                        size_t from = fullpath.find_last_of(L'\\', end-1);
                        if (from != -1) {
                        tstring lastdir(fullpath.substr(from+1, end-from-1));
                        bool only_numbers = (wcsspn(lastdir.c_str(), L"0123456789") != lastdir.length()) ? false : true;
                        int size = (only_numbers) ? _wtoi(lastdir.c_str()) : 0;
                        if (size > 0)
                            f.image_size = size;
                        }
                    }
                    current_files.push_back(f);
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
    }
    SetCurrentDirectory(current_path);

    // check diffs
    std::vector<imdata> new_files;
    for (int i=0,e=m_files.size(); i<e; ++i)
    {
        bool exist = false;
        for (int j=0,je=current_files.size();j<je;++j) {
            if (m_files[i].file_path == current_files[j].file_path) { exist = true; break; }
        }
        if (!exist) { delete m_files[i].image; }
        else { new_files.push_back(m_files[i]); }    
    }
    m_files.clear();
    m_files.swap(new_files);

    for (int j=0,je=current_files.size();j<je;++j)
    {
        bool exist = false;
        for (int i=0,e=m_files.size(); i<e; ++i) {
            if (current_files[j].file_path == m_files[i].file_path) { exist = true; break;}
        }
        if (!exist) 
        {
            const tchar* filepath = current_files[j].file_path.c_str();
            Image *img = new Image();
            if (!img->load(filepath, 0))
            {
                delete img; img = NULL;
                error(L"Невозможно прочитать файл: ", filepath);
            }
            else
            {
                current_files[j].image = img;
                new_files.push_back(current_files[j]);
            }
        }
    }
    m_files.insert(m_files.end(), new_files.begin(), new_files.end());


}
