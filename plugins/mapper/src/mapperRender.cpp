#include "stdafx.h"
#include "mapperRender.h"
#include "helpers.h"
#include "mapper.h"

#define ROOM_SIZE 32
#define MAP_EDGE 16

#define MENU_SETCOLOR 100
#define MENU_RESETCOLOR 101
#define MENU_NEWZONE_NORTH 102
#define MENU_NEWZONE_SOUTH 103
#define MENU_NEWZONE_WEST 104
#define MENU_NEWZONE_EAST 105
#define MENU_NEWZONE_UP   106
#define MENU_NEWZONE_DOWN 107

#define MENU_SETICON_FIRST 200  // max 100 icons
#define MENU_SETICON_LAST  299

extern Mapper* m_mapper_window;

MapperRender::MapperRender() : rr(ROOM_SIZE, 5)
{
    m_hscroll_pos = -1;
    m_hscroll_size = 0;
    m_vscroll_pos = -1;
    m_vscroll_size = 0;
    m_hscroll_flag = false;
    m_vscroll_flag = false;
    m_left = m_right = m_top = m_bottom = 0;
    m_block_center = true;
    m_track_mouse = false;
    m_menu_tracked_room = NULL;
}

void MapperRender::onCreate()
{
    createMenu();
    m_background.CreateSolidBrush(RGB(0,90,0));
    updateScrollbars(false);
}

void MapperRender::roomChanged(const ViewMapPosition& pos)
{
    viewpos = pos;
    updateScrollbars(false);
    Invalidate();

#ifdef _DEBUG
    char buffer[64];
    sprintf(buffer, "border left=%d, right=%d, top=%d, bottom=%d\r\n", m_left, m_right, m_top, m_bottom);
    OutputDebugStringA(buffer);
#endif
}

void MapperRender::onPaint()
{
    RECT pos; GetClientRect(&pos);
    CPaintDC dc(m_hWnd);
    CMemoryDC mdc(dc, pos);
    mdc.FillRect(&pos, m_background);

    RoomsLevel* level = viewpos.level;
    if (!level) return;

    int x = getRenderX();
    int y = getRenderY();
     
    rr.setDC(mdc);
    rr.setIcons(&m_icons);
    renderMap(level, x, y);

    if (viewpos.room)
    {
        room_pos p = findRoomPos(viewpos.room);
        if (p.valid())
        {
            int dx = x + p.x * ROOM_SIZE;
            int dy = y + p.y * ROOM_SIZE;
            rr.renderCursor(dx, dy, viewpos.cursor);
        }
    }
}

void MapperRender::renderMap(RoomsLevel* rlevel, int x, int y)
{
    Zone *zone = rlevel->getZone();
    int level = rlevel->getLevel();
    /*RoomsLevel *under2 = zone->getLevel(level-2, false);
    if (under2)
        renderLevel(under2, x+12, y+12, 1);*/
    RoomsLevel *under = zone->getLevel(level-1, false);
    if (under)
        renderLevel(under, x+6, y+6, 1);
    renderLevel(rlevel, x, y, 0);
    RoomsLevel *over = zone->getLevel(level+1, false);
    if (over)
        renderLevel(over, x-6, y-6, 2);
}

void MapperRender::renderLevel(RoomsLevel* level, int dx, int dy, int type)
{
    RECT rc;
    GetClientRect(&rc);

    int width = level->width();
    int height = level->height();

    for (int x=0; x<width; ++x)
    {
        for (int y=0; y<height; ++y)
        {
            Room *r = level->getRoom(x,y);
            if (!r) 
                continue;
            int px = ROOM_SIZE * x + dx;
            int py = ROOM_SIZE * y + dy;
            rr.render(px, py, r, type);            
        }
    }
}

MapperRender::room_pos MapperRender::findRoomPos(Room* room)
{    
    room_pos p;
    RoomsLevel* level = room->level;
    int width = level->width();
    int height = level->height();
    for (int x=0; x<width; ++x) {
    for (int y=0; y<height; ++y) {
      if (level->getRoom(x,y) == room)
        {  p.x = x; p.y = y;  return p;  }
    }}
    return p;
}

Room* MapperRender::findRoomOnScreen(int cursor_x, int cursor_y) const
{
    RoomsLevel *level = viewpos.level;
    if (!level) return NULL;

    int sx = level->width() * ROOM_SIZE;
    int sy = level->height() * ROOM_SIZE;
    int left = getRenderX();
    int top = getRenderY();
    int right = left + sx - 1;
    int bottom = top + sy - 1;

    if (cursor_x >= left && cursor_x <= right && cursor_y >= top && cursor_y <= bottom)
    {
        int x = (cursor_x - left) / ROOM_SIZE;
        int y = (cursor_y - top) / ROOM_SIZE;
        return level->getRoom(x, y);
    }
    return NULL;
}

void MapperRender::onHScroll(DWORD position)
{
    if (m_hscroll_pos < 0) return;
    m_block_center = true;
    int thumbpos = HIWORD(position);
    int action = LOWORD(position);
    switch (action) {
    case SB_LINEUP:
        m_hscroll_pos = m_hscroll_pos - ROOM_SIZE / 4;
        break;
    case SB_LINEDOWN:
        m_hscroll_pos = m_hscroll_pos + ROOM_SIZE / 4;
        break;
    case SB_PAGEUP:
        m_hscroll_pos = m_hscroll_pos - ROOM_SIZE;
        break;
    case SB_PAGEDOWN:
        m_hscroll_pos = m_hscroll_pos + ROOM_SIZE;
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        m_hscroll_pos = thumbpos;
        break;
    }
    if (m_hscroll_pos < 0) m_hscroll_pos = 0;
    else if (m_hscroll_pos > m_hscroll_size) m_hscroll_pos = m_hscroll_size;    
    SetScrollPos(SB_HORZ, m_hscroll_pos);
    Invalidate();
}

void MapperRender::onVScroll(DWORD position)
{
    if (m_vscroll_pos < 0) return;
    m_block_center = true;
    int thumbpos = HIWORD(position);
    int action = LOWORD(position);
    switch (action) {
    case SB_LINEUP:
        m_vscroll_pos = m_vscroll_pos - ROOM_SIZE / 4;        
        break;
    case SB_LINEDOWN:
        m_vscroll_pos = m_vscroll_pos + ROOM_SIZE / 4;
        break;
    case SB_PAGEUP:
        m_vscroll_pos = m_vscroll_pos - ROOM_SIZE;        
        break;
    case SB_PAGEDOWN:
        m_vscroll_pos = m_vscroll_pos + ROOM_SIZE;        
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        m_vscroll_pos = thumbpos;
        break;
    }
    if (m_vscroll_pos < 0) m_vscroll_pos = 0;
    else if (m_vscroll_pos > m_vscroll_size) m_vscroll_pos = m_vscroll_size;
    SetScrollPos(SB_VERT, m_vscroll_pos);
    Invalidate();
}

void MapperRender::onSize()
{
    updateScrollbars(true);
    Invalidate();
}

int MapperRender::getRenderX() const
{
    int x = (m_hscroll_flag) ? -m_hscroll_pos : m_hscroll_size - m_hscroll_pos;
    x = x + MAP_EDGE / 2 - m_left;
    return x;
}

int MapperRender::getRenderY() const
{
    int y = (m_vscroll_flag) ? -m_vscroll_pos : m_vscroll_size - m_vscroll_pos;    
    y = y + MAP_EDGE / 2 - m_top;
    return y;
}

void MapperRender::updateScrollbars(bool center)
{
    /*char buffer[64];
    int vmin = 0; int vmax = 0;
    GetScrollRange(SB_VERT, &vmin, &vmax);
    int hmin = 0; int hmax = 0;
    GetScrollRange(SB_HORZ, &hmin, &hmax);    
    sprintf(buffer, "vpos[%d-%d] = %d(%d), hpos[%d-%d] = %d(%d)\r\n", vmin, vmax, GetScrollPos(SB_VERT), m_vscroll_pos,
        hmin, hmax, GetScrollPos(SB_HORZ), m_hscroll_pos);
    OutputDebugStringA(buffer);*/
    
    RoomsLevel *level = viewpos.level;
    if (!level) return;

    int width = level->width();
    int height = level->height();
        
    const RoomsLevelBox &b = level->box();
    int old_dx = m_right;
    int old_dy = m_bottom;
    m_left = b.left * ROOM_SIZE;
    m_right = (width-b.right-1) * ROOM_SIZE;
    m_top = b.top * ROOM_SIZE;
    m_bottom = (height-b.bottom-1) * ROOM_SIZE;

    /*sprintf(buffer, "border left=%d, right=%d, top=%d, bottom=%d\r\n", m_left, m_right, m_top, m_bottom);
    OutputDebugStringA(buffer);*/

    width = width*ROOM_SIZE + MAP_EDGE;
    height = height*ROOM_SIZE + MAP_EDGE;

    RECT rc; GetClientRect(&rc);
    int window_width = rc.right;
    int window_height = rc.bottom;
    
    if (width < window_width)
    {
        m_hscroll_size = window_width - width - 1;        
        if (m_hscroll_pos == -1)
            m_hscroll_pos = m_hscroll_size / 2;
        else if (center && !m_block_center)
            m_hscroll_pos = m_hscroll_size / 2;
        m_hscroll_flag = false;
    }
    else
    {
        m_hscroll_size = width - window_width;      
        m_hscroll_flag = true;
        m_block_center = false;
    }
    if (m_hscroll_size == 0) { m_hscroll_size = 1; m_hscroll_pos = 0; }
    else
    {
        m_hscroll_size = m_hscroll_size + m_left + m_right;
        m_hscroll_pos += (m_right - old_dx);
    }
    SetScrollRange(SB_HORZ, 0, m_hscroll_size);
    if (m_hscroll_pos < 0) m_hscroll_pos = 0;
    else if (m_hscroll_pos > m_hscroll_size) m_hscroll_pos = m_hscroll_size;
    SetScrollPos(SB_HORZ, m_hscroll_pos);
       
    if (height < window_height)
    {
        m_vscroll_size = window_height - height - 1;
        if (m_vscroll_pos == -1)
            m_vscroll_pos = m_vscroll_size / 2;
        else if (center && !m_block_center)
            m_vscroll_pos = m_vscroll_size / 2;
        m_vscroll_flag = false;
    }
    else
    {
        m_vscroll_size = height - window_height;        
        m_vscroll_flag = true;
        m_block_center = false;
    }
    if (m_vscroll_size == 0) { m_vscroll_size = 1; m_vscroll_pos = 0; }
    else
    {
        m_vscroll_size = m_vscroll_size + m_top + m_bottom;
        m_vscroll_pos += (m_bottom - old_dy);
    }
    SetScrollRange(SB_VERT, 0, m_vscroll_size);
    if (m_vscroll_pos < 0) m_vscroll_pos = 0;
    else if (m_vscroll_pos > m_vscroll_size) m_vscroll_pos = m_vscroll_size;
    SetScrollPos(SB_VERT, m_vscroll_pos + m_top);
}

void MapperRender::mouseMove(int x, int y)
{
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

    Room *room = findRoomOnScreen(pt.x, pt.y);
    if (!room)
        return;
    m_menu_tracked_room = room;
    RoomHelper rh(room);
    m_menu.SetItemState(MENU_NEWZONE_NORTH, rh.isExplored(RD_NORTH));
    m_menu.SetItemState(MENU_NEWZONE_SOUTH, rh.isExplored(RD_SOUTH));
    m_menu.SetItemState(MENU_NEWZONE_WEST, rh.isExplored(RD_WEST));
    m_menu.SetItemState(MENU_NEWZONE_EAST, rh.isExplored(RD_EAST));
    m_menu.SetItemState(MENU_NEWZONE_UP, rh.isExplored(RD_UP));
    m_menu.SetItemState(MENU_NEWZONE_DOWN, rh.isExplored(RD_DOWN));

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
        m_menu.AppendODPopup(pictures, new CMenuXPText(0, L"Значок"));
        m_menu.AppendSeparator();
    }            
    m_menu.AppendODMenu(new CMenuXPText(MENU_SETCOLOR, L"Цвет..."));
    m_menu.AppendODMenu(new CMenuXPText(MENU_RESETCOLOR, L"Сбросить цвет"));
    m_menu.AppendSeparator();

    CMenuXP *newzone = new CMenuXP();
    newzone->CreatePopupMenu();
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_NORTH, L"на север"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_SOUTH, L"на юг"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_WEST, L"на запад"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_EAST, L"на восток"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_UP, L"вверх"));
    newzone->AppendODMenu(new CMenuXPText(MENU_NEWZONE_DOWN, L"вниз"));
    m_menu.AppendODPopup(newzone, new CMenuXPText(0, L"Начать новую зону"));
}

bool MapperRender::runMenuPoint(int id)
{    
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
    
    if (id >= MENU_NEWZONE_NORTH && id <= MENU_NEWZONE_DOWN)
    {
        RoomDir dir = (RoomDir)(id - MENU_NEWZONE_NORTH);
        m_mapper_window->newZone(m_menu_tracked_room, dir);
        return true;
    }
    
    return false;
}
