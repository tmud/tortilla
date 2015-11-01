#include "stdafx.h"
#include "clickpad.h"
#include "selectImageDlg.h"

extern ImageCollection *m_image_collection;

bool SelectImage::setImage(const BigImageData& image)
{
    const Image *ci = image.image;
    if (!ci || ci->width() <= 0 || ci->height() <= 0 || image.image_size < 0) {
        clearImage();
        return false;
    }
    m_img = image;
    m_width = image.image->width();
    m_height = image.image->height();
    m_size = image.image_size;
    if (m_size > 0)
    {
        int border = m_img.image_border;
        m_wcount = (m_width + border) / (m_size + border);
        m_hcount = (m_height + border) / (m_size + border);
    }
    else
    {
        m_wcount = m_hcount = 1;
    }

    COLORREF color =  RGB(255, 255, 0);
    COLORREF c = 0;
    if (image.image->getcolor(0, 0, &c)) 
        color = RGB((GetRValue(c) ^ 0xff),(GetGValue(c) ^ 0xff),(GetBValue(c) ^ 0xff));
    if (!m_selected.IsNull())
        m_selected.DeleteObject();
    m_selected.CreatePen(PS_SOLID, 1, color);

    updateScrollsbars();
    Invalidate(FALSE);
    return true;
}

void SelectImage::clearImage()
{
    m_img.destroy();
    m_width = m_height = m_size = 0;
    m_wcount = m_hcount = 0;
    Invalidate(FALSE);
    if (!m_selected.IsNull())
        m_selected.DeleteObject();
}

void SelectImage::getParams(SelectImageParams *params)
{
    params->filepath = m_img.file_path;
    params->width = m_width;
    params->height = m_height;
    params->size = m_size;
    params->countw = m_wcount;
    params->counth = m_hcount;
}

ClickpadImage* SelectImage::createImageFromSelected()
{
    if (m_selected_x == -1)
        return NULL;
    int border = m_img.image_border;
    int x = m_selected_x * (m_size+border);
    int y = m_selected_y * (m_size+border);
    int w = (m_size == 0) ? m_width : m_size;
    int h = (m_size == 0) ? m_height : m_size;

    Image *cut = new Image();
    if (!cut->cut(*m_img.image, x, y, w, h))
        { delete cut; return NULL; }

    ClickpadImage *new_image = new ClickpadImage();
    new_image->create(cut, m_img.file_path, m_selected_x, m_selected_y);
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
    if (m_img.image) {
    m_draw_y = updateBar(SB_VERT, rc.bottom, m_img.image->height());
    m_draw_x = updateBar(SB_HORZ, rc.right, m_img.image->width());
    } else {
        m_draw_x = m_draw_y = 0;
    }
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
    Image* image = m_img.image;
    if (!image) return;
    image_render_ex p; p.sx = m_draw_x; p.sy = m_draw_y;

    image->render(dc, 0, 0, &p);
    if (m_selected_x >= 0)
    {
        int border = m_img.image_border;
        int size = (m_size == 0) ? m_width : m_size;
        int px = m_selected_x * (size+border) - m_draw_x;
        int py = m_selected_y * (size+border) - m_draw_y;
        int lx = px + size - 1;
        int ly = py + size - 1;
        CRect rc(px, py, lx, ly);
        if (!m_selected.IsNull()) {
        HPEN old = dc.SelectPen(m_selected);
        renderRect(dc, rc);
        rc.DeflateRect(1,1);
        renderRect(dc, rc);
        dc.SelectPen(old); }
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

    int border = m_img.image_border;
    m_selected_x = image_x / (m_size + border);
    m_selected_y = image_y / (m_size + border);
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

LRESULT SelectImageDlg::OnSelectCategory(UINT, WPARAM, LPARAM, BOOL&)
{
    if (!m_image_collection)
        return 0;

    tstring item;
    m_category.getSelectedItem(&item);
    for (int i = 0, e = m_image_collection->getImagesCount(); i < e; ++i)
    {
        const BigImageData &image = m_image_collection->getImage(i);
        if (image.name == item)
        {
            if (!m_atlas.setImage(image))
                break;
            SelectImageParams params;
            m_atlas.getParams(&params);

            SelectImageProps::ImageProps p;

            tstring image_dir;
            getImagesDir(&image_dir);
            {
                tchar buffer[MAX_PATH+1];
                if (GetCurrentDirectory(MAX_PATH, buffer))
                {
                    int len = wcslen(buffer)+1;
                    image_dir = image_dir.substr(len);
                }
                else
                    image_dir.clear();
            }
            image_dir.append(params.filepath);
            p.filename = image_dir;

            tchar buffer[64];
            swprintf(buffer, L"%dx%d", params.width, params.height);
            p.image_size = buffer;
            int s = params.size;
            if (s > 0)
            {
                swprintf(buffer, L"%dx%d", s, s);
                p.icon_size = buffer;
                swprintf(buffer, L"%dx%d", params.countw, params.counth);
                p.icon_count = buffer;
            }
            else
            {
                int ms = min(params.width, params.height);
                swprintf(buffer, L"%dx%d", ms, ms);
                p.icon_size = buffer;
                p.icon_count = L"1";
            }
            m_props.setText(p);
            return 0;
        }
    }
    m_atlas.clearImage();
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

    if (m_image_collection) {
    for (int i = 0, e = m_image_collection->getImagesCount(); i < e; ++i)
    {
        const BigImageData &image = m_image_collection->getImage(i);
        m_category.addItem(image.name);
    }}
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
