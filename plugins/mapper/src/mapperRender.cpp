#include "stdafx.h"
#include "mapperRender.h"
#include "mapper.h"

#define ROOM_SIZE 32
#define MAP_EDGE 16
extern Mapper* m_mapper_window;

MapperRender::MapperRender() : rr(ROOM_SIZE, 5)
{
    m_hscroll_flag = false;
    m_vscroll_flag = false;
    m_block_center = true;
    m_track_mouse = false;
    m_drag_mode = false;
    m_menu_tracked_room = NULL;
    m_menu_handler = NULL;
}

void MapperRender::setMenuHandler(HWND handler_wnd)
{
    m_menu_handler = handler_wnd;
}

MapCursor MapperRender::getCursor() const
{
    MapCursor cursor = viewpos; // ? viewpos : currentpos;
    return (cursor && cursor->valid()) ? cursor : MapCursor();
}

void MapperRender::onCreate()
{
    createMenu();
    m_background.CreateSolidBrush(RGB(0,90,0));
    updateScrollbars(false);
}

void MapperRender::showPosition(MapCursor pos, bool resetScrolls)
{
    if (pos->valid())
        currentpos = pos;

    if (pos->valid() && viewpos && viewpos->valid() && !resetScrolls)
    {
        int id = viewpos->zone()->id();
        int newid = pos->zone()->id();
        viewpos = pos;
        scrolls s;
        s.h = getHScroll();
        s.v = getVScroll();
        m_scrolls[id] = s;
        if (id == newid) {
            Invalidate();
            return;
        }
        siterator zt = m_scrolls.find(newid);
        if (zt != m_scrolls.end()) {
            const scrolls &s = zt->second;
            setHScroll(s.h);
            setVScroll(s.v);
            return;
        }
    }
    viewpos = pos;
    updateScrollbars(true);
    if (resetScrolls) {
        int id = viewpos->zone()->id();
        viewpos = pos;
        scrolls s;
        s.h = getHScroll();
        s.v = getVScroll();
        m_scrolls[id] = s;   
    }
}

MapCursor MapperRender::getCurrentPosition()
{
    if (!currentpos)
        return std::make_shared<MapNullCursorImplementation>();
    std::shared_ptr<MapCursorInterface> cursor( currentpos->dublicate() );
    return cursor;    
}

void MapperRender::onPaint()
{
    RECT pos; GetClientRect(&pos);
    CPaintDC dc(m_hWnd);
    CMemoryDC mdc(dc, pos);
    mdc.FillRect(&pos, m_background);
    rr.setDC(mdc);
    rr.setIcons(&m_icons);
    renderMap(getRenderX(), getRenderY());
}

void MapperRender::renderMap(int render_x, int render_y)
{
    MapCursor pos = getCursor();
    if (!pos) return;

    const Rooms3dCubePos& p = pos->pos();
    const Rooms3dCubeSize& sz = pos->size();

    if (sz.minlevel <= (p.z-1))
        renderLevel(p.z-1, render_x+6, render_y+6, 1, pos);
    renderLevel(p.z, render_x, render_y, 0, pos);
    if (sz.maxlevel >= (p.z+1))
        renderLevel(p.z+1, render_x-6, render_y-6, 2, pos);

    if (pos->color() != RCC_NONE)
    {
        int cursor_x = (p.x - sz.left) * ROOM_SIZE + render_x;
        int cursor_y = (p.y - sz.top) * ROOM_SIZE + render_y;
        rr.renderCursor(cursor_x, cursor_y, (pos->color() == RCC_NORMAL) ? 0 : 1 );
    }
}

void MapperRender::renderLevel(int z, int render_x, int render_y, int type, MapCursor pos)
{
    RECT rc;
    GetClientRect(&rc);
    const Rooms3dCubeSize& sz = pos->size();
    Rooms3dCubePos p; p.z = z;
    for (int x=0; x<sz.width(); ++x)
    {
        p.x = x + sz.left;
        for (int y=0; y<sz.height(); ++y)
        {
            p.y = y + sz.top;
            const Room *r = pos->room(p);
            if (!r)
                continue;
            int px = ROOM_SIZE * x + render_x;
            int py = ROOM_SIZE * y + render_y;
            rr.render(px, py, r, type);
        }
    }
}

const Room* MapperRender::findRoomOnScreen(int cursor_x, int cursor_y) const
{
    MapCursor pos = getCursor();
    if (!pos) return NULL;

    const Rooms3dCubeSize& sz = pos->size();
    int sx = sz.width() * ROOM_SIZE;
    int sy = sz.height() * ROOM_SIZE;
    int left = getRenderX();
    int top = getRenderY();
    int right = left + sx - 1;
    int bottom = top + sy - 1;
    if (cursor_x >= left && cursor_x <= right && cursor_y >= top && cursor_y <= bottom)
    {
        Rooms3dCubePos p; 
        p.x = (cursor_x - left) / ROOM_SIZE + sz.left;
        p.y = (cursor_y - top) / ROOM_SIZE + sz.top;
        p.z = pos->pos().z;
        return pos->room(p);
    }
    return NULL;
}

void MapperRender::onHScroll(DWORD position)
{
    scroll s = getHScroll();
    if (s.pos < 0) return;
    int &hscroll_pos = s.pos;
    m_block_center = true;
    int thumbpos = HIWORD(position);
    int action = LOWORD(position);
    switch (action) {
    case SB_LINEUP:
        hscroll_pos = hscroll_pos - ROOM_SIZE / 4;
        break;
    case SB_LINEDOWN:
        hscroll_pos = hscroll_pos + ROOM_SIZE / 4;
        break;
    case SB_PAGEUP:
        hscroll_pos = hscroll_pos - ROOM_SIZE;
        break;
    case SB_PAGEDOWN:
        hscroll_pos = hscroll_pos + ROOM_SIZE;
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        hscroll_pos = thumbpos;
        break;
    }
    setHScroll(s);
}

void MapperRender::onVScroll(DWORD position)
{
    scroll s = getVScroll();
    if (s.pos < 0) return;
    int &vscroll_pos = s.pos;
    m_block_center = true;
    int thumbpos = HIWORD(position);
    int action = LOWORD(position);
    switch (action) {
    case SB_LINEUP:
        vscroll_pos = vscroll_pos - ROOM_SIZE / 4;
        break;
    case SB_LINEDOWN:
        vscroll_pos = vscroll_pos + ROOM_SIZE / 4;
        break;
    case SB_PAGEUP:
        vscroll_pos = vscroll_pos - ROOM_SIZE;
        break;
    case SB_PAGEDOWN:
        vscroll_pos = vscroll_pos + ROOM_SIZE;
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        vscroll_pos = thumbpos;
        break;
    }
    setVScroll(s);
}

void MapperRender::onSize()
{
    m_scrolls.clear();
    updateScrollbars(true);
}

int MapperRender::getRenderX() const
{
    scroll s = getHScroll();
    int x = (m_hscroll_flag) ? -s.pos : s.maxpos - s.pos;
    x = x + MAP_EDGE / 2;
    return x;
}

int MapperRender::getRenderY() const
{
    scroll s = getVScroll();
    int y = (m_vscroll_flag) ? -s.pos : s.maxpos - s.pos;
    y = y + MAP_EDGE / 2;
    return y;
}

void MapperRender::updateScrollbars(bool center)
{
/*#ifdef _DEBUG
    char buffer[64];
    int vmin = 0; int vmax = 0;
    GetScrollRange(SB_VERT, &vmin, &vmax);
    int hmin = 0; int hmax = 0;
    GetScrollRange(SB_HORZ, &hmin, &hmax);
    sprintf(buffer, "vpos[%d-%d] = %d(%d), hpos[%d-%d] = %d(%d)\r\n", vmin, vmax, GetScrollPos(SB_VERT), m_vscroll_pos,
        hmin, hmax, GetScrollPos(SB_HORZ), m_hscroll_pos);
    OutputDebugStringA(buffer);
#endif*/

    MapCursor pos = getCursor();
    if (!pos) {
        Invalidate();
        return;
    }

    const Rooms3dCubeSize& sz = pos->size();

    int width = sz.width()*ROOM_SIZE + MAP_EDGE;
    int height = sz.height()*ROOM_SIZE + MAP_EDGE;

    RECT rc; GetClientRect(&rc);
    int window_width = rc.right;
    int window_height = rc.bottom;

    scroll h;
    if (width < window_width)
    {
        h.maxpos = window_width - width - 1;
        if (h.pos == -1)
            h.pos = h.maxpos / 2;
        else if (center && !m_block_center)
            h.pos = h.maxpos / 2;
        m_hscroll_flag = false;
    }
    else
    {
        h.maxpos = width - window_width;
        m_hscroll_flag = true;
        m_block_center = false;
    }
    scroll v;
    if (height < window_height)
    {
        v.maxpos = window_height - height - 1;
        if (v.pos == -1)
            v.pos = v.maxpos / 2;
        else if (center && !m_block_center)
            v.pos = v.maxpos / 2;
        m_vscroll_flag = false;
    }
    else
    {
        v.maxpos = height - window_height;
        m_vscroll_flag = true;
        m_block_center = false;
    }
    setHScroll(h);
    setVScroll(v);
}

void MapperRender::mouseLeftButtonDown()
{
    if (!m_drag_mode) {
        m_drag_mode = true;
        GetCursorPos(&m_drag_point);
        SetCapture();
    }
}

void MapperRender::mouseLeftButtonUp()
{
    ReleaseCapture();
    m_drag_mode = false;
}

void MapperRender::mouseMove(int x, int y)
{
    if (!m_drag_mode) return;
    POINT pos;
    GetCursorPos(&pos);

    int dx = pos.x - m_drag_point.x;
    int dy = pos.y - m_drag_point.y;
    m_drag_point = pos;

    scroll h = getHScroll();
    scroll v = getVScroll();

    h.pos = h.pos - dx;
    v.pos = v.pos - dy;

    setHScroll(h);
    setVScroll(v);    
}

void MapperRender::mouseLeave()
{
}

void MapperRender::mouseRightButtonDown()
{
    POINT pt; GetCursorPos(&pt);
    int cursor_x = pt.x; 
    int cursor_y = pt.y;
    ScreenToClient(&pt);

    const Room *room = findRoomOnScreen(pt.x, pt.y);
    if (!room)
        return;
    m_menu_tracked_room = room;
    RoomHelper c(room);
    m_menu.SetItemState(MENU_NEWZONE_NORTH, c.isExplored(RD_NORTH));
    m_menu.SetItemState(MENU_NEWZONE_SOUTH, c.isExplored(RD_SOUTH));
    m_menu.SetItemState(MENU_NEWZONE_WEST, c.isExplored(RD_WEST));
    m_menu.SetItemState(MENU_NEWZONE_EAST, c.isExplored(RD_EAST));
    m_menu.SetItemState(MENU_NEWZONE_UP, c.isExplored(RD_UP));
    m_menu.SetItemState(MENU_NEWZONE_DOWN, c.isExplored(RD_DOWN));

    m_menu.SetItemState(MENU_JOINZONE_NORTH, c.isZoneExit(RD_NORTH));
    m_menu.SetItemState(MENU_JOINZONE_SOUTH, c.isZoneExit(RD_SOUTH));
    m_menu.SetItemState(MENU_JOINZONE_WEST, c.isZoneExit(RD_WEST));
    m_menu.SetItemState(MENU_JOINZONE_EAST, c.isZoneExit(RD_EAST));
    m_menu.SetItemState(MENU_JOINZONE_UP, c.isZoneExit(RD_UP));
    m_menu.SetItemState(MENU_JOINZONE_DOWN, c.isZoneExit(RD_DOWN));

    m_menu.SetItemState(MENU_MOVEROOM_NORTH, c.isZoneExit(RD_NORTH));
    m_menu.SetItemState(MENU_MOVEROOM_SOUTH, c.isZoneExit(RD_SOUTH));
    m_menu.SetItemState(MENU_MOVEROOM_WEST, c.isZoneExit(RD_WEST));
    m_menu.SetItemState(MENU_MOVEROOM_EAST, c.isZoneExit(RD_EAST));
    m_menu.SetItemState(MENU_MOVEROOM_UP, c.isZoneExit(RD_UP));
    m_menu.SetItemState(MENU_MOVEROOM_DOWN, c.isZoneExit(RD_DOWN));

    m_menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NOANIMATION, cursor_x - 2, cursor_y - 2, m_hWnd, NULL);
}

void MapperRender::createMenu()
{
    m_icons.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 0);
    HANDLE hBmp = LoadImage(NULL, L"plugins\\mapper.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBmp)
        m_icons.Add((HBITMAP)hBmp, RGB(128, 0, 128));
    m_menu.CreatePopupMenu();
    if (m_icons.GetImageCount() > 0)
    {
        CMenuXP *pictures = new CMenuXP();
        pictures->CreatePopupMenu();
        for (int i = 0, e = m_icons.GetImageCount(); i <= e; i++)
        {
            if (i == 0)
              { pictures->AppendODMenu(new CMenuXPButton(MENU_SETICON_FIRST, NULL)); continue; }            
            if (i % 6 == 0) pictures->Break();
            pictures->AppendODMenu(new CMenuXPButton(i + MENU_SETICON_FIRST, m_icons.ExtractIcon(i - 1)));
        }
        m_menu.AppendODPopup(pictures, new CMenuXPText(0, L"������"));
        m_menu.AppendSeparator();
    }
    m_menu.AppendODMenu(new CMenuXPText(MENU_SETCOLOR, L"����..."));
    m_menu.AppendODMenu(new CMenuXPText(MENU_RESETCOLOR, L"�������� ����"));
    m_menu.AppendSeparator();

    CMenuXP *newzone = new CMenuXP();
    newzone->CreatePopupMenu();
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_NORTH, L"�� �����"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_SOUTH, L"�� ��"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_WEST, L"�� �����"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_EAST, L"�� ������"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_UP, L"�����"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_DOWN, L"����"));
    m_menu.AppendODPopup(newzone, new CMenuXPText(0, L"������ ����� ����"));

    CMenuXP *joinzone = new CMenuXP();
    joinzone->CreatePopupMenu();
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_NORTH, L"�� �����"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_SOUTH, L"�� ��"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_WEST, L"�� �����"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_EAST, L"�� ������"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_UP, L"�����"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_DOWN, L"����"));
    m_menu.AppendODPopup(joinzone, new CMenuXPText(0, L"������� ����"));

    CMenuXP *moveroom = new CMenuXP();
    moveroom->CreatePopupMenu();
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_NORTH, L"�� �����"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_SOUTH, L"�� ��"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_WEST, L"�� �����"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_EAST, L"�� ������"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_UP, L"�����"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_DOWN, L"����"));
    m_menu.AppendODPopup(moveroom, new CMenuXPText(0, L"��������� ������� � ����"));
}

bool MapperRender::runMenuPoint(DWORD wparam, LPARAM lparam)
{
    WORD id = LOWORD(wparam);
    if (id == MENU_SETCOLOR)
    {
        COLORREF color = m_menu_tracked_room->color;
        if (!m_menu_tracked_room->use_color)
            color = RGB(180, 180, 180);
        CColorDialog dlg(color, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            m_menu_tracked_room->color = dlg.GetColor();
            m_menu_tracked_room->use_color = 1;
            Invalidate();
        }
        return true;
    }

    if (id == MENU_RESETCOLOR)
    {
        m_menu_tracked_room->color = 0;
        m_menu_tracked_room->use_color = 0;
        Invalidate();
        return true;
    }

    if (id >= MENU_SETICON_FIRST && id <= MENU_SETICON_LAST)
    {
        int icon = id - MENU_SETICON_FIRST;
        m_menu_tracked_room->icon = icon;
        Invalidate();
        return true;
    }
    if (m_menu_handler)
    {
        ::PostMessage(m_menu_handler, WM_COMMAND, wparam, lparam);
        return true;
    }
    return false;
}

MapperRender::scroll MapperRender::getHScroll() const
{
    scroll s;
    s.pos = GetScrollPos(SB_HORZ);
    int min = 0;
    GetScrollRange(SB_HORZ, &min, &s.maxpos);
    return s;
}

MapperRender::scroll MapperRender::getVScroll() const
{
    scroll s;
    s.pos = GetScrollPos(SB_VERT);
    int min = 0;
    GetScrollRange(SB_VERT, &min, &s.maxpos);
    return s;
}

void MapperRender::setHScroll(const scroll& s)
{
    SetScrollRange(SB_HORZ, 0, s.maxpos);
    int pos = s.pos;
    if (pos < 0) pos = 0;
    else if (pos > s.maxpos) pos = s.maxpos;
    SetScrollPos(SB_HORZ, pos);
    Invalidate();
}

void MapperRender::setVScroll(const scroll& s)
{
    SetScrollRange(SB_VERT, 0, s.maxpos);
    int pos = s.pos;
    if (pos < 0) pos = 0;
    else if (pos > s.maxpos) pos = s.maxpos;
    SetScrollPos(SB_VERT, pos);
    Invalidate();
}
