#pragma once

#include "roomObjects.h"
#include "mapCursor.h"
#include "mapperRoomRender.h"
#include "menuXP.h"

class MapperRender : public CWindowImpl<MapperRender>
{
    CBrush m_background;
    MapCursor viewpos;
    MapCursor currentpos;
    MapperRoomRender rr;

    int m_hscroll_pos;
    int m_hscroll_size;
    int m_vscroll_pos;
    int m_vscroll_size;
    bool m_hscroll_flag;
    bool m_vscroll_flag;
    bool m_block_center;

    bool m_track_mouse;
    bool m_drag_mode;
    POINT m_drag_point;

    CMenuXP m_menu;
    CImageList m_icons;
    const Room *m_menu_tracked_room;

public:
    MapperRender();
    void showPosition(MapCursor pos);
    MapCursor getCurrentPosition();

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
    LRESULT OnMenuCommand(UINT, WPARAM wparam, LPARAM, BOOL& bHandled)
    {
        if (!runMenuPoint(LOWORD(wparam)))
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
    bool runMenuPoint(int id);
};
