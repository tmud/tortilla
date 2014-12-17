#pragma once

#include "mapperObjects.h"
#include "mapperRoomRender.h"
#include "menuXP.h"

class MapperRender : public CWindowImpl<MapperRender>
{
    enum CursorType { CT_NONE = 0, CT_CURRENT_POS, CT_POSSIBLE_POS };
    
    Room *m_room;
    RoomsLevel *m_level;

    int m_hscroll_pos;
    int m_hscroll_size;    
    int m_vscroll_pos;
    int m_vscroll_size;        
    bool m_hscroll_flag;
    bool m_vscroll_flag;
    int  m_left, m_right, m_top, m_bottom;
    bool m_block_center;

    bool m_track_mouse;

    CBrush m_background;
    CMenuXP m_menu;
    CImageList m_icons;
    MapperRoomRender rr;
    Room *m_menu_tracked_room;

public:
    MapperRender();
    void renderByRoom(Room *room);
    void renderByLevel(RoomsLevel *level);

private:
	BEGIN_MSG_MAP(MapperRender)
        MESSAGE_HANDLER(WM_COMMAND, OnMenuCommand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseRButtonDown)
        MESSAGE_HANDLER(WM_MEASUREITEM, OnMenuMeasureItem)  
        MESSAGE_HANDLER(WM_DRAWITEM, OnMenuDrawItem)        
	END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnPaint(UINT, WPARAM, LPARAM lparam, BOOL&) { onPaint(); return 0; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&) { onSize(); return 0; }
    LRESULT OnVScroll(UINT, WPARAM wparam, LPARAM, BOOL&) { onVScroll(wparam);  return 0; }
    LRESULT OnHScroll(UINT, WPARAM wparam, LPARAM, BOOL&) { onHScroll(wparam);  return 0; }
    LRESULT OnMouseMove(UINT, WPARAM, LPARAM lparam, BOOL&) { trackMouseLeave();  mouseMove(LOWORD(lparam), HIWORD(lparam)); return 0; }
    LRESULT OnMouseLeave(UINT, WPARAM, LPARAM lparam, BOOL&) { m_track_mouse = false;  mouseLeave(); return 0; }
    LRESULT OnMouseRButtonDown(UINT, WPARAM, LPARAM, BOOL&) { mouseRightButtonDown(); return 0; }
    LRESULT OnMenuMeasureItem(UINT, WPARAM, LPARAM lparam, BOOL&) 
    {
        m_menu.MeasureItem((LPMEASUREITEMSTRUCT)lparam);
        return 0; 
    }
    LRESULT OnMenuDrawItem(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        m_menu.DrawItem((LPDRAWITEMSTRUCT)lparam);
        return 0;
    }
    LRESULT OnMenuCommand(UINT, WPARAM wparam, LPARAM, BOOL& bHandled)
    {
        if (!runMenuPoint(LOWORD(wparam)))
            bHandled = FALSE;
        return 0;
    }

private:
    void renderMap(RoomsLevel* rlevel, int x, int y);
    void renderLevel(RoomsLevel* level, int dx, int dy, int type);

    struct room_pos {
        room_pos() : x(-1), y(-1) {}
        bool valid() const { return (x >= 0) ? true : false; }
        int x, y;
    };
    room_pos findRoomPos(Room* room);

    Room* findRoomOnScreen(int cursor_x, int cursor_y) const;
    void onCreate();
    void onPaint();
    void onSize();
    void onHScroll(DWORD position);
    void onVScroll(DWORD position);
    int  getRenderX() const;
    int  getRenderY() const;
    void updateScrollbars(bool center);
    void mouseMove(int x, int y);
    void mouseLeave();
    void mouseRightButtonDown();    
    void trackMouseLeave()
    {
        if (m_track_mouse) return;
        m_track_mouse = true;
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        tme.dwHoverTime = 0;
        TrackMouseEvent(&tme);
    }
    void createMenu();
    bool runMenuPoint(int id);
};
