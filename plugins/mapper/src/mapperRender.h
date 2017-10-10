#pragma once

#include "roomObjects.h"
#include "mapCursor.h"
#include "mapperRoomRender.h"
#include "menuXP.h"

#define MENU_SETCOLOR       100
#define MENU_RESETCOLOR     101
#define MENU_NEWZONE_NORTH  102
#define MENU_NEWZONE_SOUTH  103
#define MENU_NEWZONE_WEST   104
#define MENU_NEWZONE_EAST   105
#define MENU_NEWZONE_UP     106
#define MENU_NEWZONE_DOWN   107
#define MENU_JOINZONE_NORTH 108
#define MENU_JOINZONE_SOUTH 109
#define MENU_JOINZONE_WEST  110
#define MENU_JOINZONE_EAST  111
#define MENU_JOINZONE_UP    112
#define MENU_JOINZONE_DOWN  113
#define MENU_MOVEROOM_NORTH 114
#define MENU_MOVEROOM_SOUTH 115
#define MENU_MOVEROOM_WEST  116
#define MENU_MOVEROOM_EAST  117
#define MENU_MOVEROOM_UP    118
#define MENU_MOVEROOM_DOWN  119

#define MENU_SETICON_FIRST  200  // max 100 icons
#define MENU_SETICON_LAST   299

class MapperRender : public CWindowImpl<MapperRender>
{
    CBrush m_background;
    MapCursor viewpos;
    MapCursor currentpos;
    MapperRoomRender rr;

    struct scroll {
        scroll() : pos(-1), maxpos(0) {}
        int pos, maxpos;
    };
    struct scrolls {
        scroll h,v;        
    };   
    std::map<int, scrolls> m_scrolls;
    typedef std::map<int, scrolls>::iterator siterator;

    bool m_hscroll_flag;
    bool m_vscroll_flag;
    bool m_block_center;

    bool m_track_mouse;
    bool m_drag_mode;
    POINT m_drag_point;

    CMenuXP m_menu;
    CImageList m_icons;
    const Room *m_menu_tracked_room;

    HWND m_menu_handler;
public:
    MapperRender();
    void setMenuHandler(HWND handler_wnd);
    void showPosition(MapCursor pos, bool resetScrolls);
    MapCursor getCurrentPosition();    
    const Room* menuTrackedRoom() const { return m_menu_tracked_room; }
    void clear() { m_scrolls.clear(); viewpos = nullptr; currentpos = nullptr; }

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
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseLButtonUp)
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
    LRESULT OnMouseMove(UINT, WPARAM, LPARAM lparam, BOOL&) { /*trackMouseLeave();*/  mouseMove(LOWORD(lparam), HIWORD(lparam)); return 0; }
    LRESULT OnMouseLeave(UINT, WPARAM, LPARAM lparam, BOOL&) { m_track_mouse = false;  mouseLeave(); return 0; }
    LRESULT OnMouseLButtonDown(UINT, WPARAM, LPARAM, BOOL&) { mouseLeftButtonDown(); return 0; }
    LRESULT OnMouseLButtonUp(UINT, WPARAM, LPARAM, BOOL&) { mouseLeftButtonUp(); return 0; }
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
    LRESULT OnMenuCommand(UINT, WPARAM wparam, LPARAM lparam, BOOL& bHandled)
    {
        if (!runMenuPoint(wparam, lparam))
            bHandled = FALSE;
        return 0;
    }
private:
    void renderMap(int render_x, int render_y);
    void renderLevel(int z, int render_x, int render_y, int type, MapCursor pos);
    MapCursor getCursor() const;
    const Room* findRoomOnScreen(int cursor_x, int cursor_y) const;
    void onCreate();
    void onPaint();
    void onSize();
    void onHScroll(DWORD position);
    void onVScroll(DWORD position);
    int  getRenderX() const;
    int  getRenderY() const;
    void updateScrollbars(bool center);
    void mouseLeftButtonDown();
    void mouseLeftButtonUp();
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
    bool runMenuPoint(DWORD wparam, LPARAM lparam);
    scroll getHScroll() const;
    scroll getVScroll() const;
    void setHScroll(const scroll& s);
    void setVScroll(const scroll& s);
};
