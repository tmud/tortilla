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
    m_track_mouse = false;
    m_drag_mode = DRAG_NONE;
    m_menu_handler = NULL;
    m_roomMoveTool = NULL;
    currentpos = std::make_shared<MapNullCursorImplementation>();
    viewpos = std::make_shared<MapNullCursorImplementation>();
}

void MapperRender::setMenuHandler(HWND handler_wnd)
{
    m_menu_handler = handler_wnd;
}

void MapperRender::setMoveToolHandler(MapperRenderRoomMoveTool *movetool)
{
    m_roomMoveTool = movetool;
}

MapCursor MapperRender::getViewPosition() const
{
    return viewpos;
}

MapCursor MapperRender::getCurrentPosition() const
{
    return currentpos;
}

void MapperRender::onCreate()
{
    createMenu();
    m_background.CreateSolidBrush(RGB(0,90,0));
    Invalidate();
}

void MapperRender::showCurrentOnScreen(bool centerScreen)
{
    if (!currentpos) {
        assert(false);
        return;
    }
    const Rooms3dCubePos &p = currentpos->pos();
    const Room* r = currentpos->room(p);
    if (!r) {
        assert(false);
        return;
    }
    const Rooms3dCubeSize &sz = currentpos->size();
    int px = (p.x - sz.left) * ROOM_SIZE + getRenderX();
    int py = (p.y - sz.top) * ROOM_SIZE + getRenderY();
    RECT rc;
    GetClientRect(&rc);

    // true if room is not fully visible
    bool not_visible_x_left = ((px + ROOM_SIZE) < ROOM_SIZE);
    bool not_visible_x_right = (px > (rc.right - ROOM_SIZE));
    bool not_visible_y_top = ((py + ROOM_SIZE) < ROOM_SIZE);
    bool not_visible_y_bottom =(py > (rc.bottom - ROOM_SIZE));

    if ((not_visible_x_left && not_visible_x_right) ||
        (not_visible_y_top && not_visible_y_bottom))
    {
        return; // cant render, screen to small
    }
    if (centerScreen)
    {
        // try place current pos on the window center
        int reqpx = (rc.right - ROOM_SIZE) / 2 - (p.x - sz.left) * ROOM_SIZE;
        int reqpy = (rc.bottom - ROOM_SIZE) / 2 - (p.y - sz.top) * ROOM_SIZE;
        scroll h = getHScroll();
        scroll v = getVScroll();
        h.pos = calcScrollPosByRender(reqpx, m_hscroll_flag);
        v.pos = calcScrollPosByRender(reqpy, m_vscroll_flag);
        setVScroll(v);
        setHScroll(h);
        Invalidate();
        return;
    }

    if (!m_hscroll_flag && !not_visible_x_left && !not_visible_x_right )
    {
        int reqpx = (rc.right - ROOM_SIZE) / 2 - (p.x - sz.left) * ROOM_SIZE;
        scroll h = getHScroll();
        h.pos = calcScrollPosByRender(reqpx, m_hscroll_flag);
        setHScroll(h);
    }

    if (!m_vscroll_flag && !not_visible_y_top && !not_visible_y_bottom)
    {
        int reqpy = (rc.bottom - ROOM_SIZE) / 2 - (p.y - sz.top) * ROOM_SIZE;
        scroll v = getVScroll();
        v.pos = calcScrollPosByRender(reqpy, m_vscroll_flag);
        setVScroll(v);
    }

    // move room into visible part
    if (not_visible_x_left)
    {
        int dx = (p.x - sz.left) * ROOM_SIZE;
        scroll h = getHScroll();
        int px = dx + calcRenderPosByScroll(h, m_hscroll_flag);
        while ((px + ROOM_SIZE) >= ROOM_SIZE);
        {
            h.pos -= ROOM_SIZE;
            px = dx + calcRenderPosByScroll(h, m_hscroll_flag);
        } 
        setHScroll(h);
    }
    else if (not_visible_x_right)
    {
        int dx = (p.x - sz.left) * ROOM_SIZE;
        scroll h = getHScroll();
        int px = dx + calcRenderPosByScroll(h, m_hscroll_flag);
        while (px <= (rc.right - ROOM_SIZE));
        {
            h.pos += ROOM_SIZE;
            px = dx + calcRenderPosByScroll(h, m_hscroll_flag);
        }
        setHScroll(h);
    }
    if (not_visible_y_top)
    {
        int dy = (p.y - sz.top) * ROOM_SIZE;
        scroll v = getVScroll();
        int py = dy + calcRenderPosByScroll(v, m_vscroll_flag);
        while ((py + ROOM_SIZE) >= ROOM_SIZE);
        {
            v.pos -= ROOM_SIZE;
            py = dy + calcRenderPosByScroll(v, m_vscroll_flag);
        }
        setVScroll(v);
    }
    else if (not_visible_y_bottom)
    {
        int dy = (p.y - sz.top) * ROOM_SIZE;
        scroll v = getVScroll();
        int py = dy + calcRenderPosByScroll(v, m_vscroll_flag);
        while (py <= (rc.bottom - ROOM_SIZE));
        {
            v.pos += ROOM_SIZE;
            py = dy + calcRenderPosByScroll(v, m_vscroll_flag);
        }
        setVScroll(v);
    }
    Invalidate();
}

void MapperRender::showPosition(MapCursor pos, bool centerScreen, bool currentPosition)
{
    if (!pos) {
        assert(false);
        return;
    }

    auto saveScrolls = [this](int id) 
    {
        if (id < 0)
            return;
        scrolls s;
        s.h = getHScroll();
        s.h_flag = m_hscroll_flag;
        s.v = getVScroll();
        s.v_flag = m_vscroll_flag;
        m_scrolls[id] = s;
    };

    if (!viewpos)
    {
        viewpos = pos;
    }
    if (viewpos->valid() && !pos->valid())
    {
        int id = viewpos->zone()->id();
        saveScrolls(id);
    }
    if (!pos->valid())
    {
        Invalidate();
        return;
    }

    int id = (viewpos->valid()) ? viewpos->zone()->id() : -1;
    int newid = pos->zone()->id();
    viewpos = pos;
    if (newid != id) {
        saveScrolls(id);
    }
    if (currentPosition)
    {
        assert(pos);
        currentpos = pos;
    }
    if (currentPosition)
    {
        updateScrollbars();
        showCurrentOnScreen(centerScreen);
        return;
    }
    if (centerScreen)
    {
        centerScrollbars();
        saveScrolls(id);
        return;
    }
    siterator zt = m_scrolls.find(newid);
    if (zt != m_scrolls.end())
    {
        const scrolls &s = zt->second;
        setHScroll(s.h);
        m_hscroll_flag = s.h_flag;
        setVScroll(s.v);
        m_vscroll_flag = s.v_flag;
        Invalidate();
    }
    else
    {
        centerScrollbars();
        saveScrolls(newid);
    }
}

void MapperRender::onPaint()
{
    RECT pos; GetClientRect(&pos);
    CPaintDC dc(m_hWnd);
    CMemoryDC mdc(dc, pos);
    mdc.FillRect(&pos, m_background);
    rr.setDC(mdc);
    rr.setIcons(&m_icons);
    renderMap();
}

void MapperRender::renderMap()
{
    int render_x = getRenderX();
    int render_y = getRenderY();
    MapCursor pos = getViewPosition();
    if (!pos->valid()) return;

    const Rooms3dCubePos& p = pos->pos();
    const Rooms3dCubeSize& sz = pos->size();

    if (sz.minlevel <= (p.z - 1))
        renderLevel(p.z - 1, render_x + 6, render_y + 6, 1, pos);
    renderLevel(p.z, render_x, render_y, 0, pos);
    if (sz.maxlevel >= (p.z + 1))
        renderLevel(p.z + 1, render_x - 6, render_y - 6, 2, pos);

    if (pos->color() != RCC_NONE)
    {
        int cursor_x = (p.x - sz.left) * ROOM_SIZE + render_x;
        int cursor_y = (p.y - sz.top) * ROOM_SIZE + render_y;
        rr.renderCursor(cursor_x, cursor_y, (pos->color() == RCC_NORMAL) ? 0 : 1);
    }
}

void MapperRender::renderLevel(int z, int render_x, int render_y, int type, MapCursor pos)
{
    RECT rc;
    GetClientRect(&rc);
    const Rooms3dCubeSize& sz = pos->size();
    Rooms3dCubePos p; p.z = z;
    for (int x = 0; x < sz.width(); ++x)
    {
        p.x = x + sz.left;
        for (int y = 0; y < sz.height(); ++y)
        {
            p.y = y + sz.top;
            int px = ROOM_SIZE * x + render_x;
            int py = ROOM_SIZE * y + render_y;
            if ((px + ROOM_SIZE) < 0 || (py + ROOM_SIZE) < 0)
                continue;
            if ((px > rc.right) || (py > rc.bottom))
                continue;
            const Room *r = pos->room(p);
            if (!r)
            {
#ifdef _DEBUG
                //if (type == 0) rr.renderDummy(px, py);
#endif
                continue;
            }
            rr.render(px, py, r, type);
        }
    }
}

const Room* MapperRender::findRoomOnScreen(int cursor_x, int cursor_y) const
{
    MapCursor pos = getViewPosition();
    if (!pos->valid()) return NULL;

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
    Invalidate();
}

void MapperRender::onVScroll(DWORD position)
{
    scroll s = getVScroll();
    if (s.pos < 0) return;
    int &vscroll_pos = s.pos;
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
    Invalidate();
}

void MapperRender::onSize()
{
    m_scrolls.clear();
    updateScrollbars();
}

int MapperRender::getRenderX() const
{
    return calcRenderPosByScroll(getHScroll(), m_hscroll_flag);
}

int MapperRender::getRenderY() const
{
    return calcRenderPosByScroll(getVScroll(), m_vscroll_flag);
}

int MapperRender::calcRenderPosByScroll(const scroll&s, bool flag) const
{
    int e2 = MAP_EDGE / 2;
    int delta = (flag) ? -s.pos : s.pos;
    return delta + e2;
}

int MapperRender::calcScrollPosByRender(int render, bool flag) const
{
    int e2 = MAP_EDGE / 2;
    int delta = render - e2;
    int pos = (flag) ? -delta : delta;
    return pos;
}

void MapperRender::centerScrollbars()
{
    updateScrollbars(true);
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

    MapCursor pos = getViewPosition();
    if (!pos->valid())
        return;

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
        m_hscroll_flag = false;
    }
    else
    {
        h.maxpos = width - window_width;
        m_hscroll_flag = true;
    }
    scroll v;
    if (height < window_height)
    {
        v.maxpos = window_height - height - 1;
        m_vscroll_flag = false;
    }
    else
    {
        v.maxpos = height - window_height;
        m_vscroll_flag = true;
    }
    if (center || !m_vscroll_flag)
        v.pos = v.maxpos / 2;
    else {
        v.pos = getVScroll().pos;
        if (v.pos > v.maxpos)
            v.pos = v.maxpos;
    }
    if (center || !m_hscroll_flag)
        h.pos = h.maxpos / 2;
    else
    {
        h.pos = getHScroll().pos;
        if (h.pos > h.maxpos)
            h.pos = h.maxpos;
    }

    if (!center)
    {
        int chpos = GetScrollPos(SB_HORZ);
        if (chpos >= 0 && chpos <= h.maxpos)
            h.pos = chpos;
        int cvpos = GetScrollPos(SB_VERT);
        if (cvpos >= 0 && cvpos <= v.maxpos)
            v.pos = cvpos;
    }
    setHScroll(h);
    setVScroll(v);
    Invalidate();
}

void MapperRender::mouseLeftButtonDown()
{
    if (m_drag_mode == DRAG_NONE) 
    {
        if (!m_roomMoveTool)
                return;
        POINT pt; GetCursorPos(&pt);
        int cursor_x = pt.x; 
        int cursor_y = pt.y;
        ScreenToClient(&pt);
        const Room *room = findRoomOnScreen(pt.x, pt.y);
        bool shift = (::GetKeyState(VK_SHIFT) < 0 );
        if(shift)
        {
            if (!room)
                return;
            if (!unselectRoom(room)) { selectRoom(room); }
            Invalidate();
            return;
        } 
        if (room && room->selected) {
             m_drag_mode = DRAG_ROOM;
             selectRoom(room); //move room to last position in vector
        } else {
             unselectAllRooms();
             Invalidate();
             m_drag_mode = DRAG_MAP;
        }
        GetCursorPos(&m_drag_point);
        SetCapture();
    }
}

void MapperRender::mouseLeftButtonUp()
{
    if (m_drag_mode == DRAG_NONE)
        return;
    ReleaseCapture();
    m_drag_mode = DRAG_NONE;
}

void MapperRender::mouseMove()
{
    if (m_drag_mode == DRAG_NONE) return;
    POINT pos;
    GetCursorPos(&pos);

    int dx = pos.x - m_drag_point.x;
    int dy = pos.y - m_drag_point.y;
    m_drag_point = pos;

    if (m_drag_mode == DRAG_ROOM)
    {
        ScreenToClient(&pos);
        int x = pos.x - getRenderX();
        if (x < 0)  x = x / ROOM_SIZE - 1;
        else x = x / ROOM_SIZE;
        int y = pos.y - getRenderY();
        if (y < 0) y = y / ROOM_SIZE - 1;
        else y = y / ROOM_SIZE;

        /*char b[64];
        sprintf(b, "x=%d,y=%d\r\n", x, y);
        OutputDebugStringA(b);*/

        assert (m_roomMoveTool);
        if (!m_selected_rooms.empty())
            m_roomMoveTool->roomMoveTool(m_selected_rooms, x, y);
        return;
    }

    scroll h = getHScroll();
    scroll v = getVScroll();

    h.pos = h.pos + (dx * (m_hscroll_flag ? -1 : 1));
    v.pos = v.pos + (dy * (m_vscroll_flag ? -1 : 1));

    setHScroll(h);
    setVScroll(v);
    Invalidate();
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

    bool shift = (::GetKeyState(VK_SHIFT) < 0 );
    if (!shift) {
        if (!room->selected)
            unselectAllRooms();
    }
    selectRoom(room);
    Invalidate();

    bool multiselection = (m_selected_rooms.size() == 1) ? false : true;
    if (multiselection) {
        CMenuXP &m = m_multiply_rooms_menu;
        m.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, cursor_x - 2, cursor_y - 2, m_hWnd, NULL);
        return;
    }

    RoomHelper c(room);
    CMenuXP &m = m_single_room_menu;
    m.SetItemState(MENU_NEWZONE_NORTH, c.isExplored(RD_NORTH));
    m.SetItemState(MENU_NEWZONE_SOUTH, c.isExplored(RD_SOUTH));
    m.SetItemState(MENU_NEWZONE_WEST, c.isExplored(RD_WEST));
    m.SetItemState(MENU_NEWZONE_EAST, c.isExplored(RD_EAST));
    m.SetItemState(MENU_NEWZONE_UP, c.isExplored(RD_UP));
    m.SetItemState(MENU_NEWZONE_DOWN, c.isExplored(RD_DOWN));

    m.SetItemState(MENU_JOINZONE_NORTH, c.isZoneExit(RD_NORTH));
    m.SetItemState(MENU_JOINZONE_SOUTH, c.isZoneExit(RD_SOUTH));
    m.SetItemState(MENU_JOINZONE_WEST, c.isZoneExit(RD_WEST));
    m.SetItemState(MENU_JOINZONE_EAST, c.isZoneExit(RD_EAST));
    m.SetItemState(MENU_JOINZONE_UP, c.isZoneExit(RD_UP));
    m.SetItemState(MENU_JOINZONE_DOWN, c.isZoneExit(RD_DOWN));

    m.SetItemState(MENU_MOVEROOM_NORTH, c.isZoneExit(RD_NORTH));
    m.SetItemState(MENU_MOVEROOM_SOUTH, c.isZoneExit(RD_SOUTH));
    m.SetItemState(MENU_MOVEROOM_WEST, c.isZoneExit(RD_WEST));
    m.SetItemState(MENU_MOVEROOM_EAST, c.isZoneExit(RD_EAST));
    m.SetItemState(MENU_MOVEROOM_UP, c.isZoneExit(RD_UP));
    m.SetItemState(MENU_MOVEROOM_DOWN, c.isZoneExit(RD_DOWN));

    m.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, cursor_x - 2, cursor_y - 2, m_hWnd, NULL);
}

void MapperRender::createMenu()
{
    m_icons.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 0);
    CBitmap icons;
    icons.LoadBitmap(IDB_ICONS);
    m_icons.Add(icons, RGB(128, 0, 128));
    m_single_room_menu.CreatePopupMenu();
    CMenuXP &m = m_single_room_menu;
    if (m_icons.GetImageCount() > 0)
    {
        CMenuXP *pictures = new CMenuXP();
        pictures->CreatePopupMenu();
        for (int i = 0, e = m_icons.GetImageCount(); i < e; i++)
        {
            if (i != 0 && i % 6 == 0) pictures->Break();
            pictures->AppendODMenu(new CMenuXPButton(i + MENU_SETICON_FIRST, m_icons.ExtractIcon(i)));            
        }
        m.AppendODPopup(pictures, new CMenuXPText(0, L"Значок"));
        m.AppendODMenu(new CMenuXPText(MENU_RESETICON, L"Удалить значок"));
        m.AppendSeparator();
    }
    m.AppendODMenu(new CMenuXPText(MENU_SETCOLOR, L"Цвет..."));
    m.AppendODMenu(new CMenuXPText(MENU_RESETCOLOR, L"Сбросить цвет"));
    /*m.AppendSeparator();

    CMenuXP *newzone = new CMenuXP();
    newzone->CreatePopupMenu();
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_NORTH, L"на север"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_SOUTH, L"на юг"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_WEST, L"на запад"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_EAST, L"на восток"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_UP, L"вверх"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_DOWN, L"вниз"));
    m.AppendODPopup(newzone, new CMenuXPText(0, L"Начать новую зону"));

    CMenuXP *joinzone = new CMenuXP();
    joinzone->CreatePopupMenu();
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_NORTH, L"на север"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_SOUTH, L"на юг"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_WEST, L"на запад"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_EAST, L"на восток"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_UP, L"вверх"));
    joinzone->AppendODMenu(new CMenuXPText(MENU_JOINZONE_DOWN, L"вниз"));
    m.AppendODPopup(joinzone, new CMenuXPText(0, L"Склеить зону"));

    CMenuXP *moveroom = new CMenuXP();
    moveroom->CreatePopupMenu();
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_NORTH, L"на север"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_SOUTH, L"на юг"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_WEST, L"на запад"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_EAST, L"на восток"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_UP, L"вверх"));
    moveroom->AppendODMenu(new CMenuXPText(MENU_MOVEROOM_DOWN, L"вниз"));
    m.AppendODPopup(moveroom, new CMenuXPText(0, L"Перенести комнату в зону"));*/

    m_multiply_rooms_menu.CreatePopupMenu();
    CMenuXP &m2 = m_multiply_rooms_menu;
    if (m_icons.GetImageCount() > 0)
    {
        CMenuXP *pictures = new CMenuXP();
        pictures->CreatePopupMenu();
        for (int i = 0, e = m_icons.GetImageCount(); i < e; i++) {
            if (i != 0 && i % 6 == 0) pictures->Break();
            pictures->AppendODMenu(new CMenuXPButton(i + MENU_SETICON_FIRST, m_icons.ExtractIcon(i)));            
        }
        m2.AppendODPopup(pictures, new CMenuXPText(0, L"Значок"));
        m2.AppendODMenu(new CMenuXPText(MENU_RESETICON, L"Удалить значок"));
        m2.AppendSeparator();
    }
    m2.AppendODMenu(new CMenuXPText(MENU_SETCOLOR, L"Цвет..."));
    m2.AppendODMenu(new CMenuXPText(MENU_RESETCOLOR, L"Сбросить цвет"));
    /*m2.AppendSeparator();
    m2.AppendODMenu(new CMenuXPText(MENU_NEWZONE, L"Создать новую зону..."));*/
}

bool MapperRender::runMenuPoint(DWORD wparam, LPARAM lparam)
{
    WORD id = LOWORD(wparam);
    if (id == MENU_SETCOLOR)
    {
        COLORREF color = RGB(180, 180, 180);
        if (m_selected_rooms.size() == 1) {
            const Room *room = m_selected_rooms[0];
            if (room->use_color)
                color = room->color;
        }
        CColorDialog dlg(color, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            color = dlg.GetColor();
            for (const Room* r: m_selected_rooms) {
                r->color = color;
                r->use_color = 1;
            }
            unselectAllRooms();
            Invalidate();
        }
        return true;
    }

    if (id == MENU_RESETICON)
    {
        for (const Room* r: m_selected_rooms) {
             r->icon = 0;
        }
        unselectAllRooms();
        Invalidate();
        return true;
    }

    if (id == MENU_RESETCOLOR)
    {
        for (const Room* r: m_selected_rooms) {
            r->color = 0;
            r->use_color = 0;
        }
        unselectAllRooms();
        Invalidate();
        return true;
    }

    if (id >= MENU_SETICON_FIRST && id <= MENU_SETICON_LAST)
    {
        int icon = (id - MENU_SETICON_FIRST)+1;
        for (const Room* r: m_selected_rooms) {
             r->icon = icon;
        }
        unselectAllRooms();
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
    int pos = s.pos;
    if (pos < 0) pos = 0;
    else if (pos > s.maxpos) pos = s.maxpos;
    SetScrollRange(SB_HORZ, 0, s.maxpos);
    SetScrollPos(SB_HORZ, pos);
}

void MapperRender::setVScroll(const scroll& s)
{
    int pos = s.pos;
    if (pos < 0) pos = 0;
    else if (pos > s.maxpos) pos = s.maxpos;
    SetScrollRange(SB_VERT, 0, s.maxpos);
    SetScrollPos(SB_VERT, pos);
}
