#pragma once

#include "mudViewString.h"
#include "mudViewParser.h"
#include "propertiesPages/propertiesElements.h"

class MudView : public CWindowImpl<MudView>
{
    friend class MudViewHandler;
    PropertiesElements *propElements;
    int m_lines_count;
    int m_last_visible_line;
    bool m_last_updated;
    mudViewStrings m_strings;
    bool m_use_softscrolling;
    int  m_start_softscroll;

    POINT m_dragpt, m_dragpos;
    int  drag_begin, drag_end;
    int  drag_left, drag_right;
    std::vector<int> m_drag_beginline_len;
    std::vector<int> m_drag_endline_len;
    std::vector<int> m_drag_currentline_len;
    bool m_drag_boxmode;

    int m_find_string_index;
    int m_find_start_pos, m_find_end_pos;

    int m_id;

    std::vector<MudViewString*> m_dropped_strings;

    bool m_scrollLock;

public:
	DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND+1)
    MudView(PropertiesElements *elements, int id);
    ~MudView();
    void accLastString(parseData *parse_data);
    void addText(parseData* parse_data, parseData *copy_data, int *limited_strings = NULL);
    void pushText(parseData* parse_data, bool delete_last);
    void clearText();
    void truncateStrings(int maxcount);
    void setViewString(int index);
    int  getViewString() const;
    int  getLastString() const;
    bool lastStringDeleted() const;
    int  getStringsCount() const;
    int  getStringsOnDisplay() const;
    int  getSymbolsOnDisplay() const;
    MudViewString* getString(int idx) const;
    void updateProps();
    void updateSoftScrolling();
    void setSoftScrollingMode(bool mode);
    bool inSoftScrolling() const;
    bool isDragMode() const;
    int  findAndSelectText(int from, int direction, const tstring& text);
    int  getCurrentFindString();
    void clearFind();
    void clearDropped();
    void lockScroll(bool scrollmode);

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
        MESSAGE_HANDLER(WM_GETMINMAXINFO, OnMinMaxInfo)
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
        if (checkKeysState(true, false, false) || (GetKeyState(VK_RBUTTON) & 0x100)!=0 )
        {
            m_drag_boxmode = false;
            startDraging();
        }
        else if (checkKeysState(true, true, false))
        {
            m_drag_boxmode = true;
            startDraging();
        }
        return 0;
    }
    LRESULT OnLButtonUp(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        stopDraging();
        m_drag_boxmode = false;
        return 0;
    }
    LRESULT OnMouseMove(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        doDraging();
        return 0; 
    }
    LRESULT OnMinMaxInfo(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        LONG p = DEFAULT_SPLITTER_SIZE + MIN_DOCKPANE_SIZE;
        MINMAXINFO *minmax = (MINMAXINFO*) lparam;
        minmax->ptMinTrackSize.x = p;
        minmax->ptMinTrackSize.y = p;
        return 0;
    }
private:
    void deleteLastString();
    void removeDropped(parseData* parse_data);
    void calcStringsSizes(mudViewStrings& pds);
    void renderView();
    void renderString(CDC* dc, MudViewString *s, int left_x, int bottom_y, int index);
    void initRenderParams();
    void updateScrollbar(int new_visible_line);
    void setScrollbar(DWORD position);
    void mouseWheel(WORD position);
    int  checkLimit();
    void deleteBeginStrings(int count_from_begin);
    void startDraging();
    void stopDraging();
    void doDraging();
    bool checkDragging(int line, bool incborder);
    POINT getCursor() const;
    int   getCursorLine(int y) const;
    bool  isDragCursorLeft() const;
    enum dragline { BEGINLINE = 0, ENDLINE };
    int   calcDragSym(int x, dragline type) const;
    void  calcDragLine(int line, dragline type);
    void  calcDragArray(MudViewString* s, std::vector<int> &ld);   
    void renderDragSym(CDC *dc, const tstring& str, RECT& pos, COLORREF text, COLORREF bkg);
    void stopSoftScroll();
};

class MudViewHandler
{
    MudView* m_pView;
    MudView* m_pMirror;
    int m_orig_size;
    mudViewStrings *m_view_tmp;
    mudViewStrings *m_mirror_tmp;
public:
    MudViewHandler(MudView *view, MudView* mirror = NULL) : m_pView(view), m_pMirror(mirror), m_orig_size(0), 
        m_view_tmp(NULL), m_mirror_tmp(NULL)
    {
        assert(m_pView);
        m_view_tmp = new mudViewStrings;
        if (m_pMirror)
            m_mirror_tmp = new mudViewStrings;
    }
    ~MudViewHandler()
    {
        delete m_mirror_tmp;
        delete m_view_tmp;
    }

    SIZE getSizeInSymbols()
    {
        SIZE sz; 
        sz.cx = m_pView->getSymbolsOnDisplay();
        sz.cy =  m_pView->getStringsOnDisplay();
        return sz;
    }

    mudViewStrings& get() {
        m_orig_size = m_pView->m_strings.size();
        m_view_tmp->swap(m_pView->m_strings);
        m_pView->m_last_visible_line = -1;
        if (m_pMirror)
        {
            m_mirror_tmp->swap(m_pMirror->m_strings);
            m_pMirror->m_last_visible_line = -1;
        }
        return *m_view_tmp;
    }
    void update()
    {
        ColorsCollector pc;
        pc.process(m_view_tmp);

        mudViewStrings &vs = *m_view_tmp;
        m_pView->calcStringsSizes(vs);

        if (m_pMirror)
        {
            mudViewStrings &ms = *m_mirror_tmp;
            int size = vs.size();
            if (m_orig_size > size)
            {
                int todel = m_orig_size - size;
                int i = ms.size() - todel; 
                for (int j=i, e=ms.size(); j<e; ++j)
                    delete ms[j];
                ms.erase(ms.begin()+i, ms.end());
            }
            if (m_orig_size < size)
            {
                int count = size - m_orig_size;
                mudViewStrings newstr(count);
                for (int i = 0; count > 0; --count, ++i)
                    newstr[i] = new MudViewString;
                ms.insert(ms.end(), newstr.begin(), newstr.end());
            }
            // копируем строки
            int delta = ms.size() - size;
            for (int i = 0; i < size; ++i)
            {
                MudViewString *src = vs[i];
                MudViewString *dst = ms[i + delta];
                dst->copy(src);
            }
            // добавляем новые строки, если появились, при обработке массива строк
            mudViewStrings &new_ms = m_pMirror->m_strings;
            if (!new_ms.empty()) {
                ms.insert(ms.end(), new_ms.begin(), new_ms.end());
                new_ms.clear();
            }
            new_ms.swap(ms);
        }
        // добавляем новые строки
        mudViewStrings &new_vs = m_pView->m_strings;
        if (!new_vs.empty()) {
            vs.insert(vs.end(), new_vs.begin(), new_vs.end());
            new_vs.clear();
        }
        new_vs.swap(vs);
        if (m_pMirror)
        {
            int last = m_pMirror->getStringsCount();
            m_pMirror->setViewString(last);
        }
        int last = m_pView->getStringsCount();
        m_pView->setViewString(last);
    }
};
