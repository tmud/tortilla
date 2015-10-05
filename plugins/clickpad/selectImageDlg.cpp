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
            if (current_files[j].file_path == m_files[i].file_path) { exist = true; break; }
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

void SelectImage::setImage(Image* image, int size, const tstring& image_filepath)
{
    if (!image || image->width() == 0 || image->height() == 0 || size < 0) {
        m_pimg = NULL; m_size = 0; 
        Invalidate(FALSE);
        return;
    }
    m_filepath = image_filepath;
    m_pimg = image;
    m_size = size;
    m_width = m_pimg->width();
    m_height = m_pimg->height();
    m_wcount = (m_size > 0) ? m_width / m_size : 1;
    m_hcount = (m_size > 0) ? m_height/ m_size : 1;
    updateScrollsbars();
    Invalidate(FALSE);
}

ClickpadImage* SelectImage::createImageFromSelected()
{
    if (m_selected_x == -1)
        return NULL;
    int x = m_selected_x * m_size;
    int y = m_selected_y * m_size;
    int w = (m_size == 0) ? m_width : m_size;
    int h = (m_size == 0) ? m_height : m_size;

    Image *cut = new Image();
    if (!cut->cut(*m_pimg, x, y, w, h))
        { delete cut; return NULL; }
    
    int path_len = m_root_dir.length();
    tstring fpath(m_filepath.substr(path_len));

    ClickpadImage *new_image = new ClickpadImage();
    new_image->create(cut, fpath, m_selected_x, m_selected_y);
    return new_image;
}

int SelectImage::updateBar(int bar, int window_size, int image_size)
{
    if (window_size >= image_size)
    {
        SetScrollRange(bar, 0, 0);
        SetScrollPos(bar, 0);
        return 0;
    }
    int pos = GetScrollPos(bar);
    int cur_min = 0; int cur_max = 0;
    GetScrollRange(bar, &cur_min, &cur_max);
    int max_sb = image_size - window_size;
    SetScrollRange(bar, 0, max_sb, FALSE);
    if (cur_max > 0) {
        pos = (pos * max_sb) / cur_max;
    }
    else { pos = 0; }
    SetScrollPos(bar, pos);
    return pos;
}

void SelectImage::updateScrollsbars()
{
    RECT rc; GetClientRect(&rc);
    int window_width = rc.right;
    m_draw_y = updateBar(SB_VERT, rc.bottom, m_height);
    m_draw_x = updateBar(SB_HORZ, rc.right, m_width);    
}

int SelectImage::calculateBar(int current, int max_value, DWORD pos)
{
    int thumbpos = HIWORD(pos);
    int action = LOWORD(pos);
    switch (action) {
    case SB_LINEUP:
        current -= 1;
        break;
    case SB_LINEDOWN:
        current += 1;
        break;
    case SB_PAGEUP:
        current -= m_size;
        break;
    case SB_PAGEDOWN:
        current += m_size;
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        current = thumbpos - 1;
        break;
    }
    if (current < 0) current = 0;
    else if (current > max_value) current = max_value;
    return current;
}

void SelectImage::setHScrollbar(DWORD position)
{
    RECT rc; GetClientRect(&rc);
    m_draw_x = calculateBar(m_draw_x, m_width - rc.right, position);
    SetScrollPos(SB_HORZ, m_draw_x);
    Invalidate(FALSE);
}

void SelectImage::setVScrollbar(DWORD position)
{
    RECT rc; GetClientRect(&rc);
    m_draw_y = calculateBar(m_draw_y, m_height-rc.bottom, position);
    SetScrollPos(SB_VERT, m_draw_y);
    Invalidate(FALSE);
}

void renderRect(HDC dc, const RECT& r)
{
    MoveToEx(dc, r.left, r.top, NULL);
    LineTo(dc, r.right, r.top);
    LineTo(dc, r.right, r.bottom);
    LineTo(dc, r.left, r.bottom);
    LineTo(dc, r.left, r.top);
}

void SelectImage::renderImage(HDC hdc, int width, int height)
{
    CDCHandle dc(hdc);
    dc.FillSolidRect(0, 0, width, height, GetSysColor(COLOR_WINDOWFRAME));
    if (!m_pimg) return;
    image_render_ex p; p.sx = m_draw_x; p.sy = m_draw_y;
    m_pimg->render(dc, 0, 0, &p);
    if (m_selected_x >= 0)
    {
        int size = (m_size == 0) ? m_width : m_size;
        int px = m_selected_x * size - m_draw_x;
        int py = m_selected_y * size - m_draw_y;
        int lx = px + size - 1;
        int ly = py + size - 1;
        CRect rc(px, py, lx, ly);
        HPEN old = dc.SelectPen(m_selected);
        renderRect(dc, rc);
        rc.DeflateRect(1,1);
        renderRect(dc, rc);
        dc.SelectPen(old);
    }
}

void SelectImage::updateSize()
{
    updateScrollsbars();
}

void SelectImage::mouseMove(const POINT& p)
{
    if (m_width == 0)
        return;
    if (m_size == 0)
        m_size = m_width;
    int image_x = p.x + m_draw_x;
    int image_y = p.y + m_draw_y;
    m_selected_x = image_x / m_size;
    m_selected_y = image_y / m_size;
    if (m_selected_x >= m_wcount || m_selected_y >= m_hcount)
    {
        m_selected_x = m_selected_y = -1;
    }
    Invalidate(FALSE);
}

void SelectImage::mouseLeave()
{
    m_selected_x = m_selected_y = -1;
    Invalidate(FALSE);
}

void SelectImage::mouseSelect()
{
    if (m_selected_x == -1) 
        return;
    if (::IsWindow(m_notify_wnd))
        SendMessage(m_notify_wnd, m_notify_msg, 0, 0); 
}

LRESULT SelectImageDlg::OnUser(UINT, WPARAM, LPARAM, BOOL&)
{
    tstring item;
    m_category.getSelectedItem(&item);
    for (int i = 0, e = m_images.getImagesCount(); i < e; ++i)
    {
        const ImageCollection::imdata &image = m_images.getImage(i);
        if (image.name == item)
        {
            m_atlas.setImage(image.image, image.image_size, image.file_path);
            SelectImageProps::ImageProps p;

            int len = 0;
            {
                tchar buffer[MAX_PATH+1];
                if (GetCurrentDirectory(MAX_PATH, buffer))
                    len = wcslen(buffer)+1;
            }            
            const tstring &fp = image.file_path;
            p.filename = (len < (int)fp.length()) ? fp.substr(len) : fp;

            int width = image.image->width();
            int height = image.image->height();
            tchar buffer[64];
            swprintf(buffer, L"%dx%d", width, height);
            p.image_size = buffer;
            int s = image.image_size;
            if (s > 0)
            {
                swprintf(buffer, L"%dx%d", s, s);
                p.icon_size = buffer;
                swprintf(buffer, L"%dx%d", width/s, height/s);
                p.icon_count = buffer;
            }
            else
            {
                int ms = min(width, height);
                swprintf(buffer, L"%dx%d", ms, ms);
                p.icon_size = buffer;
                p.icon_count = L"1";
            }
            m_props.setText(p);
            return 0;
        }
    }
    m_atlas.setImage(NULL, 0, L"");
    return 0;
}

LRESULT SelectImageDlg::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{   
    RECT rc;
    m_props.Create(m_hWnd, rcDefault);
    m_props.GetClientRect(&rc);
    m_props_size.cx = rc.right;
    m_props_size.cy = rc.bottom;
       
    GetClientRect(&rc);
    rc.bottom -= m_props_size.cy;

    m_vSplitter.Create(m_hWnd, rc);
    m_vSplitter.m_cxySplitBar = 3;
    m_vSplitter.SetSplitterRect();
    m_vSplitter.SetDefaultSplitterPos();

    RECT pane_left, pane_right;
    m_vSplitter.GetSplitterPaneRect(0, &pane_left);
    pane_left.right -= 3;
    m_vSplitter.GetSplitterPaneRect(1, &pane_right);
   
    m_category.Create(m_vSplitter, pane_left); // NULL, style, WS_EX_CLIENTEDGE);
    m_category.setNotyfy(m_hWnd, WM_USER);

    DWORD style = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
    m_atlas.Create(m_vSplitter, pane_right, NULL, style); // | WS_VSCROLL | WS_HSCROLL);
    m_atlas.setNotify(m_hWnd, WM_USER+1);
    m_vSplitter.SetSplitterPanes(m_category, m_atlas);

    m_images.scanImages();
    for (int i = 0, e = m_images.getImagesCount(); i < e; ++i)
    {
        const ImageCollection::imdata &image = m_images.getImage(i);
        m_category.addItem(image.name);
    }
    return 0;
}

LRESULT SelectImageDlg::OnSize(UINT, WPARAM, LPARAM, BOOL&)
{
    RECT rc;
    GetClientRect(&rc);
    LONG t = rc.bottom;
    rc.bottom -= m_props_size.cy;
    m_vSplitter.MoveWindow(&rc, FALSE);
    rc.top = rc.bottom; 
    rc.bottom = t;
    m_props.MoveWindow(&rc, FALSE);
    return 0;
}

LRESULT SelectImageDlg::OnSelectImage(UINT, WPARAM, LPARAM, BOOL&)
{
    if (::IsWindow(m_notify_wnd))
        ::PostMessage(m_notify_wnd, m_notify_msg, 0, 0);
    return 0;
}

ClickpadImage* SelectImageDlg::createImageSelected()
{
    return m_atlas.createImageFromSelected();
}
