#pragma once
#include "splitterEx.h"
#include "resource.h"
#include "imageCollection.h"

class SelectImageCategory : public CDialogImpl<SelectImageCategory>
{
    CListBox m_list;
    int m_selected;
    HWND m_notify_wnd;
    UINT m_notify_msg;

public:
    enum { IDD = IDD_SELECTFILE };
    SelectImageCategory() : m_selected(-1), m_notify_wnd(NULL), m_notify_msg(0) {}
    void addItem(const tstring& text) {
        m_list.AddString(text.c_str());
    }
    void getSelectedItem(tstring *text) {
        int sel = m_list.GetCurSel();
        if (sel != -1)
        {
            int len = m_list.GetTextLen(sel);
            tchar *buffer = new tchar[len+1];
            m_list.GetText(sel, buffer);
            text->assign(buffer);
            delete []buffer;
        }
    }
    void setNotyfy(HWND wnd, UINT message) {
        m_notify_wnd = wnd; 
        m_notify_msg = message;
    }
private:
    BEGIN_MSG_MAP(SelectImageCategory)
        MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        COMMAND_HANDLER(IDC_LIST_IMAGES, LBN_SELCHANGE, OnListChanged)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
        m_list.Attach(GetDlgItem(IDC_LIST_IMAGES));
        return 0;
    }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&) {
        CRect rc; GetClientRect(&rc);
        rc.DeflateRect(2, 2);
        m_list.MoveWindow(&rc);
        return 0;
    }
    LRESULT OnListChanged(WORD, WORD, HWND, BOOL&)
    {
        int sel = m_list.GetCurSel();
        if (sel != m_selected) {
            m_selected = sel;
            if (m_notify_wnd && ::IsWindow(m_notify_wnd))
                ::SendMessage(m_notify_wnd, m_notify_msg, 0, 0);
        }
        return 0;
    }
};

class SelectImage : public CWindowImpl<SelectImage>
{
    BigImageData m_img;
    int m_wcount, m_hcount;
    int m_draw_x, m_draw_y;
    int m_selected_x, m_selected_y;
    bool m_mouseleave;
    CPen m_selected;
    HWND m_notify_wnd;
    UINT m_notify_msg;
public:
    SelectImage() : m_wcount(0),m_hcount(0), m_draw_x(0), m_draw_y(0),
        m_selected_x(-1), m_selected_y(-1), m_mouseleave(false), m_notify_wnd(0), m_notify_msg(0) {}
    void setImage(const BigImageData& image);
    void clearImage();
    void setNotify(HWND wnd, UINT msg) { m_notify_wnd = wnd; m_notify_msg = msg; } 
    ClickpadImage* createImageFromSelected();
private:
    int  updateBar(int bar, int window_size, int image_size);
    void updateScrollsbars();
    int  calculateBar(int current, int max_value, DWORD pos);
    void setVScrollbar(DWORD pos);
    void setHScrollbar(DWORD pos);
    void renderImage(HDC hdc, int width, int height);
    void updateSize();
    void mouseMove(const POINT& p);
    void mouseLeave();
    void mouseSelect();
    BEGIN_MSG_MAP(SelectImage)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_PAINT, OnPaint)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
      MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
      MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
      MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
      MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnSelectImage)
      MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnSelectImage)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
        m_selected.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNTEXT));
        return 0;
    }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&) {
        updateSize();
        return 0;
    }
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
        RECT rc; GetClientRect(&rc);
        CPaintDC dc(m_hWnd);
        CMemoryDC mdc(dc, rc);
        renderImage(mdc, rc.right, rc.bottom);
        return 0;
    }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&) {
        return 1;
    }
    LRESULT OnVScroll(UINT, WPARAM wparam, LPARAM, BOOL&) {
        setVScrollbar(wparam);
        return 0;
    }
    LRESULT OnHScroll(UINT, WPARAM wparam, LPARAM, BOOL&) {
        setHScrollbar(wparam);
        return 0;
    }
    LRESULT OnMouseMove(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        if (!m_mouseleave)
        {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = m_hWnd;
            TrackMouseEvent(&tme);
            m_mouseleave = true;
        }
        POINT p = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        mouseMove(p);
        return 0;
    }
    LRESULT OnMouseLeave(UINT, WPARAM, LPARAM, BOOL&) {
        m_mouseleave = false;
        mouseLeave();
        return 0;
    }
    LRESULT OnSelectImage(UINT, WPARAM, LPARAM, BOOL&) {
        mouseSelect();
        return 0;
    }
};

class SelectImageProps : public CDialogImpl<SelectImageProps>
{
    CStatic m_filename, m_image_size, m_icon_size, m_icon_count;
public:
     enum { IDD = IDD_PROPS };
     struct ImageProps{
         tstring filename;
         tstring image_size;
         tstring icon_size;
         tstring icon_count;     
     };
     void setText(const ImageProps& p) {
         m_filename.SetWindowText(p.filename.c_str());
         m_image_size.SetWindowText(p.image_size.c_str());
         m_icon_size.SetWindowText(p.icon_size.c_str());
         m_icon_count.SetWindowText(p.icon_count.c_str());
     }
private:
     BEGIN_MSG_MAP(SelectImageProps)
         MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
     END_MSG_MAP()
     LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
         m_filename.Attach(GetDlgItem(IDC_STATIC_FILENAME));
         m_image_size.Attach(GetDlgItem(IDC_STATIC_IMAGESIZE));
         m_icon_size.Attach(GetDlgItem(IDC_STATIC_ICONSIZE));
         m_icon_count.Attach(GetDlgItem(IDC_STATIC_ICONCOUNT));
         return 0;
     }
};

class SelectImageDlg : public CWindowImpl<SelectImageDlg>
{
    CSplitterWindowExT<true, 1, 4> m_vSplitter;
    SelectImageCategory m_category;
    SelectImage m_atlas;
    SelectImageProps m_props;
    SIZE m_props_size;
    HWND m_notify_wnd;
    UINT m_notify_msg;
public:
    SelectImageDlg() : m_notify_wnd(NULL), m_notify_msg(0) { m_props_size.cx=m_props_size.cy=0; }
    ~SelectImageDlg() { if (IsWindow()) DestroyWindow(); m_hWnd = NULL; }
    void setNotify(HWND wnd, UINT msg) { m_notify_wnd = wnd; m_notify_msg = msg; }
    ClickpadImage* createImageSelected();
private:
    BEGIN_MSG_MAP(SelectImageDlg)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_USER, OnSelectCategory)
      MESSAGE_HANDLER(WM_USER+1, OnSelectImage)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSelectCategory(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSelectImage(UINT, WPARAM, LPARAM, BOOL&);
};
