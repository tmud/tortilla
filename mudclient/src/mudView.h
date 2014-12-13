#pragma once

#include "mudViewString.h"
#include "mudViewParser.h"
#include "propertiesPages/propertiesElements.h"

class MudView : public CWindowImpl<MudView>
{
    PropertiesElements *propElements;
    int m_lines_count;
    int m_last_visible_line;
    std::vector<MudViewString*> m_strings;
    bool m_last_string_updated;

    POINT m_dragpt;
    int  drag_begin, drag_end;
    int  drag_left, drag_right;
    std::vector<int> m_drag_line_len;

public:
	DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND+1)
    MudView(PropertiesElements *elements);
    ~MudView();
    void accLastString(parseData *parse_data);
    void addText(parseData* parse_data, MudView* mirror = NULL);
    void clearText();
    void truncateStrings(int maxcount);
    void setViewString(int index);
    int  getViewString() const;
    int  getLastString() const;
    bool isLastString() const;
    int  getStringsCount() const;
    int  getStringsOnDisplay() const;
    MudViewString* getString(int idx) const;
    void updateProps();

private:
	BEGIN_MSG_MAP(MudView)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { m_hWnd = NULL; return 0; }
	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&) { renderView(); return 0; }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ initRenderParams(); updateScrollbar(m_last_visible_line); return 0; }
    LRESULT OnScroll(UINT, WPARAM wparam, LPARAM, BOOL&){ setScrollbar(wparam); return 0; }
    LRESULT OnMouseWheel(UINT, WPARAM wparam, LPARAM, BOOL&) { mouseWheel(HIWORD(wparam)); return 0; }
    LRESULT OnLButtonDown(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        bool shift = (GetKeyState(VK_SHIFT) < 0) ? true : false;
        if (shift)
            startDraging();
        return 0;
    }
    LRESULT OnLButtonUp(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        stopDraging();
        return 0;
    }
    LRESULT OnMouseMove(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        doDraging();
        return 0; 
    }
private:
    void removeDropped(parseData* parse_data);
    void calcStringSizes(MudViewString *string);
    void renderView();
    void renderString(CDC* dc, MudViewString *s, int left_x, int bottom_y, int index);
    void initRenderParams();
    void updateScrollbar(int new_visible_line);
    void setScrollbar(DWORD position);
    void mouseWheel(WORD position);
    void checkLimit();
    void deleteStrings(int count_from_begin);

    void startDraging();
    void stopDraging();
    void doDraging();
    bool checkDragging(int line, bool accept_emptyline);
    bool checkDraggingSym(int line);
    POINT getCursor() const;
    int   getCursorLine(int y) const;
    int   getCursorSym(int x) const;
    void  calcDragLine(int line);
    void  renderDragSym(CDC *dc, const tstring& str, RECT& pos, COLORREF text, COLORREF bkg);
};
