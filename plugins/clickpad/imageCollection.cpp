#include "stdafx.h"
#include "imageCollection.h"
#include "settingsDlg.h"

void error(const wchar_t* msg, const wchar_t* p1 = NULL)
{
    std::wstring errtext(msg);
    if (p1) errtext.append(p1);
    base::log(getLuaState(), errtext.c_str());
}

ImageCollection::ImageCollection()
{
}

ImageCollection::~ImageCollection()
{
    for (int i = 0, e = m_files.size(); i < e; ++i)
        delete m_files[i].image;
}

int ImageCollection::getImagesCount() const
{
    return m_files.size();
}

const BigImageData& ImageCollection::getImage(int index) const
{
    assert(index >= 0 && index < getImagesCount());
    return m_files[index];
}

void ImageCollection::scanImages()
{
    std::wstring dir;
    getImagesDir(&dir);
    if (!(GetFileAttributes(dir.c_str()) & FILE_ATTRIBUTE_DIRECTORY))
        return error(L"Невозможно прочитать каталог с иконками: ", dir.c_str());

    int dir_len = dir.length();

    // 1. get current files list
    std::vector<BigImageData> current_files;
    wchar_t current_path[MAX_PATH + 1];
    GetCurrentDirectory(MAX_PATH, current_path);
    std::vector<std::wstring> dirs;
    dirs.push_back(dir);
    for (int index = 0; index != dirs.size(); ++index)
    {
        std::wstring fullpath(dirs[index]);
        if (!SetCurrentDirectory(fullpath.c_str()))
            continue;
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(L"*.*", &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do
            {
                std::wstring filename(fd.cFileName);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (filename == L"." || filename == L"..")
                        continue;
                    std::wstring newdir(fullpath);
                    newdir.append(filename);
                    newdir.append(L"\\");
                    dirs.insert(dirs.begin() + index + 1, newdir);
                }
                else
                {
                    size_t e = filename.find_last_of(L'.');
                    if (e == -1) continue;
                    std::wstring ext(filename.substr(e + 1));
                    bool ok = (ext == L"png" || ext == L"bmp" || ext == L"gif" || ext == L"ico" || ext == L"jpg") ? true : false;
                    if (!ok)
                        continue;
                    BigImageData f;
                    f.name = filename;
                    std::wstring &p = f.file_path;
                    p.append(fullpath);
                    p.append(filename);
                    p = p.substr(dir_len);
                    f.image_size = 0;
                    size_t end = fullpath.find_last_of(L'\\');
                    if (end != -1)
                    {
                        size_t from = fullpath.find_last_of(L'\\', end - 1);
                        if (from != -1) {
                            std::wstring lastdir(fullpath.substr(from + 1, end - from - 1));
                            int pos = wcsspn(lastdir.c_str(), L"0123456789-");
                            if (pos == lastdir.length())
                            {
                                std::wstring border;
                                size_t defis = lastdir.find_last_of(L'-');
                                if (defis != -1) {
                                    border.assign(lastdir.substr(defis+1));
                                    lastdir = lastdir.substr(0,defis);
                                }
                                int size = 0; int border_size = 0;
                                s2i(lastdir, &size);
                                if (!border.empty()) {
                                    s2i(border, &border_size);
                                    if (border_size < 0 || border_size > 8)
                                        size = 0;
                                    else
                                        f.image_border = border_size;
                                }                                                                
                                if (size > 0)
                                    f.image_size = size;
                            }
                        }
                    }
                    current_files.push_back(f);
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
        SetCurrentDirectory(current_path);
    }

    // check diffs
    std::vector<BigImageData> new_files;
    for (int i = 0, e = m_files.size(); i < e; ++i)
    {
        bool exist = false;
        for (int j = 0, je = current_files.size(); j < je; ++j) {
            if (m_files[i].file_path == current_files[j].file_path) { exist = true; break; }
        }
        if (!exist) { delete m_files[i].image; }
        else { new_files.push_back(m_files[i]); }
    }
    m_files.clear();
    m_files.swap(new_files);

    for (int j = 0, je = current_files.size(); j < je; ++j)
    {
        bool exist = false;
        for (int i = 0, e = m_files.size(); i < e; ++i) {
            if (current_files[j].file_path == m_files[i].file_path) { exist = true; break; }
        }
        if (!exist)
        {
            std::wstring filepath(dir);
            filepath.append( current_files[j].file_path );
            Image *img = new Image();
            if (!img->load(filepath.c_str(), 0))
            {
                delete img; img = NULL;
                error(L"Невозможно прочитать файл: ", filepath.c_str());
            }
            else
            {
                ButtonSizeTranslator bst;
                int min_size = 8;
                int max_size = bst.getSize(bst.getCount()-1);
                int s = current_files[j].image_size;
                if (s == 0)
                {
                    int w = img->width(); int h = img->height();
                    if (w < min_size || h < min_size || w > max_size || h > max_size)
                      {  delete img; img = NULL; }
                }
                else
                {
                    if (s < min_size || s > max_size)
                      {  delete img; img = NULL; }
                }
                if (img) {
                  current_files[j].image = img;
                  new_files.push_back(current_files[j]);
                }
            }
        }
    }
    m_files.insert(m_files.end(), new_files.begin(), new_files.end());
}

ClickpadImage* ImageCollection::load(const std::wstring& params)
{
    if (params.empty())
        return NULL;
    const wchar_t* p = params.c_str();
    const wchar_t *p1 = wcschr(p, L',');
    if (!p1) return false;
    const wchar_t *p2 = wcschr(p1 + 1, L',');
    if (!p2) return false;
    std::wstring ax(p, p1 - p);
    std::wstring ay(p1 + 1, p2 - p1 - 1);
    std::wstring path(p2 + 1);

    int x = 0; int y = 0;
    if (!s2i(ax, &x) || !s2i(ay, &y))
        return NULL;
    return load(path, x, y);
}

ClickpadImage* ImageCollection::load(const std::wstring& path, int x, int y)
{
    int index = -1;
    for (int i=0,e=m_files.size();i<e;++i)
    {
        if (m_files[i].file_path == path) {
            index = i; break;
        }   
    }
    if (index == -1)
        return NULL;

    const BigImageData& id = getImage(index);
    int size = id.image_size;
    int px = x * (size+id.image_border);
    int py = y * (size+id.image_border);
    int w = (size == 0) ? id.image->width() : size;
    int h = (size == 0) ? id.image->height() : size;

    Image *cut = new Image();
    if (!cut->cut(*id.image, px, py, w, h))
    {
        delete cut; 
        return NULL;
    }

    ClickpadImage *clickpad = new ClickpadImage();
    clickpad->create(cut, path, x, y);
    return clickpad;
}

void ImageCollection::save(ClickpadImage* image, std::wstring* params)
{
    if (!image) { params->clear(); return; }
    const ClickpadImageParams& p = image->params();    
    wchar_t buffer[32];
    wsprintf(buffer, L"%d,%d,", p.atlas_x, p.atlas_y);
    params->assign(buffer);
    params->append(p.atlas_filename);
}
