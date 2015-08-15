#pragma once

#include "propertiesPages/propertiesData.h"

class CEditEx : public CWindowImpl < CEditEx, CEdit >
{
    MemoryBuffer m_getTextBuffer;
    CBrush m_bgnd_brush;
public:
    DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName())
    void setText(const tstring& text, int cursor_position = -1)
    {
        SetWindowText(text.c_str());
        setCursor(cursor_position);
    }
    void getText(tstring* text)
    {
        int count = GetWindowTextLength() + 1;
        int memlen = count*sizeof(tchar);
        m_getTextBuffer.alloc(memlen);
        tchar* buffer = (tchar*)m_getTextBuffer.getData();
        GetWindowText(buffer, count);
        text->assign(buffer);
    }
    void selectText()
    {
        int len = GetWindowTextLength();
        if (len > 0)
            SetSel(0, len);
    }
    void clearText()
    {
        SetWindowText(L"");
    }
    void setCursor(int cursor_position = -1)
    {
        // move cursor to end, as default
        int pos = (cursor_position == -1) ? GetWindowTextLength() : cursor_position;
        SetSel(pos, pos);
    }
    void setBackroundColor(COLORREF color)
    {
        if (!m_bgnd_brush.IsNull())
            m_bgnd_brush.DeleteObject();
        m_bgnd_brush.CreateSolidBrush(color);
    }
private:
    BEGIN_MSG_MAP(CEditEx)
       MESSAGE_HANDLER(WM_CREATE, OnCreate)
       MESSAGE_HANDLER(WM_PASTE, OnPaste)
       MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        m_getTextBuffer.alloc(256);
        bHandled = FALSE;
        return 0;
    }
    LRESULT OnPaste(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        LRESULT result = ::SendMessage(GetParent(), WM_USER, 0, 0);
        if (!result)
            bHandled = FALSE;
        return 0;
    }
    LRESULT OnEraseBkgnd(UINT, WPARAM wparam, LPARAM lparam, BOOL&bHandled)
    {
        if (!m_bgnd_brush.IsNull())
        {
            RECT rc; GetClientRect(&rc);
            CDCHandle dc ( (HDC)wparam );
            dc.FillRect(&rc, m_bgnd_brush);
            bHandled = FALSE;
            return 1;
        }

        bHandled = FALSE;
        return 0;
    }
};

typedef std::vector<tstring> MudCommandBarCommands;

class MudCommandBarModeHandler
{
public:
    virtual ~MudCommandBarModeHandler() {}
    virtual BOOL create(HWND parent) = 0;
    virtual void resize(int width, int height) = 0;
    virtual BOOL translateMessage(MSG* pMsg, BOOL *enter) = 0;
    virtual void setFont(HFONT font) = 0;
    virtual void setFocus() = 0;
    virtual void paste(const tstring& paste_text, BOOL *enter) = 0;
    virtual void reset() = 0;
    virtual void getCommands(MudCommandBarCommands* cmds) = 0;
    virtual void historyCommands(const MudCommandBarCommands& cmds) = 0;
    virtual void setVisible(bool visible) = 0;
};

#include "mudGameCmdBar.h"
#include "mudSearchCmdBar.h"

class MudCommandBar : public CWindowImpl < MudCommandBar, CStatusBarCtrl >
{
public:
    enum BARMODE { DEFAULT = 0, SEARCH };
private:
    std::map<BARMODE, MudCommandBarModeHandler*> m_mode_handlers;
    typedef std::map<BARMODE, MudCommandBarModeHandler*>::iterator mode_handlers_iterator;
    BARMODE m_current_mode;
    PropertiesData* propData;
    HWND m_callback_hwnd;
    UINT m_callback_msg;

public:
    void setMode(BARMODE mode)
    {
        if (mode == m_current_mode)
            return;
        MudCommandBarModeHandler* h = getHandler();
        if (h)
            h->setVisible(false);
        m_current_mode = mode;
        h = getHandler();
        if (h)
        {
            h->setVisible(true);
            BOOL b = FALSE;
            OnSize(0, 0, 0, b);
            h->setFocus();
        }
    }

    BARMODE getMode()
    {
        return m_current_mode;
    }

    BOOL PreTranslateMessage(MSG* pMsg)
    {
       MudCommandBarModeHandler *h = getHandler();
       if (!h) return FALSE;
       BOOL enter = FALSE;
       BOOL result = h->translateMessage(pMsg, &enter);
       if (enter)
           OnEnter();
       return result;
    }

    MudCommandBar(PropertiesData *data) : m_current_mode(DEFAULT),
        propData(data), m_callback_hwnd(NULL), m_callback_msg(0)
    {
    }

    ~MudCommandBar()
    {
        mode_handlers_iterator it = m_mode_handlers.begin(), it_end = m_mode_handlers.end();
        for (; it != it_end; ++it)
            delete it->second;
    }
    
    void setParams(int size, HFONT font)
    {
        SetMinHeight(size);
        mode_handlers_iterator it = m_mode_handlers.begin(), it_end = m_mode_handlers.end();
        for (; it != it_end; ++it)
            it->second->setFont(font);
    }

    void setCommandEventCallback(HWND hwnd, UINT msg)
    {
        m_callback_hwnd = hwnd;
        m_callback_msg = msg;
    }

    void setFocus()
    {
        MudCommandBarModeHandler *h = getHandler();
        if (h) h->setFocus();
    }

    void getCommands(MudCommandBarCommands* cmds)
    {
        MudCommandBarModeHandler *h = getHandler();
        if (h)
            h->getCommands(cmds);
    }

    void addToHistory(const MudCommandBarCommands& cmds)
    {
        MudCommandBarModeHandler *h = getHandler();
        if (h)
            h->historyCommands(cmds);
    }

    void reset()
    {
        mode_handlers_iterator it = m_mode_handlers.begin(), it_end = m_mode_handlers.end();
        for (; it != it_end; ++it)
            it->second->reset();
        setMode(DEFAULT);
    }

private:
    MudCommandBarModeHandler* getHandler() {
        mode_handlers_iterator it = m_mode_handlers.find(m_current_mode);
        return (it != m_mode_handlers.end()) ? it->second : NULL;    
    } 

    BEGIN_MSG_MAP(MudCommandBar)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_USER, OnPaste)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        MudCommandBarModeHandler *default_ = new MudGameCmdBar(propData);
        if (default_->create(m_hWnd))
           m_mode_handlers[DEFAULT] = default_;
        else { delete default_; }
        MudCommandBarModeHandler *search = new MudSearchCmdBar();
        if (search->create(m_hWnd))
           m_mode_handlers[SEARCH] = search;
        else { delete search; delete default_; }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        MudCommandBarModeHandler *h = getHandler();
        if (h) {
            RECT rc; GetClientRect(&rc);
            h->resize(rc.right-48, rc.bottom);
        }
        return 0;
    }

    LRESULT OnPaste(UINT, WPARAM, LPARAM, BOOL&)
    {
        // on paste from clipboard
        tstring text;
        if (getFromClipboard(m_hWnd, &text))
        {           
            MudCommandBarModeHandler *h = getHandler();
            if (h) 
            {
                BOOL enter = FALSE;
                h->paste(text, &enter);
                if (enter)
                    OnEnter();
                return 1;
            }
        }
        return 0;
    }

    void OnEnter()
    {
        SendMessage(m_callback_hwnd, m_callback_msg, 0, 0);
    }
};
