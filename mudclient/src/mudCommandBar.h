#pragma once

class MudCommandBar;
class MudCommandBarModeHandler
{
    MudCommandBar *m_bar;
public:
    MudCommandBarModeHandler(MudCommandBar* bar) : m_bar(bar) {}
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg) = 0;
    virtual void setCommandEventCallback(HWND hwnd, UINT msg) = 0;
};

class MudCommandBar :  public CWindowImpl<MudCommandBar, CStatusBarCtrl>
{
    enum BARMODE { DEFAULT = 0, SEARCH };
    class CEditEx : public CWindowImpl<CEditEx, CEdit>
    {
    public:
        DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName())
    private:
        BEGIN_MSG_MAP(CEditEx)
          MESSAGE_HANDLER(WM_PASTE, OnPaste)
        END_MSG_MAP()
        LRESULT OnPaste(UINT, WPARAM, LPARAM, BOOL& bHandled)
        {
            LRESULT result = ::SendMessage(GetParent(), WM_USER, 0, 0);
            if (!result)
                bHandled = FALSE;
            return 0;
        }
    };

    PropertiesData* propData;
    tstring m_cmdBarBuffer;
    MemoryBuffer m_getTextBuffer;
    CEditEx m_edit;

    std::map<BARMODE, MudCommandBarModeHandler*> m_modes;
    typedef std::map<BARMODE, MudCommandBarModeHandler*>::const_iterator mode_iterator;
    MudCommandBarModeHandler* m_current_mode;   
    MudCommandBarModeHandler* getHandler(BARMODE mode) const{
        mode_iterator it = m_modes.find(mode);
        return (it != m_modes.end()) ? it->second : NULL;
    }
public:    
    void setMode(BARMODE mode) { 
         MudCommandBarModeHandler *handler = getHandler(mode);
         if (handler) 
             m_current_mode = handler;
    }
    void setEventCallback(BARMODE mode, HWND hwnd, UINT msg)
    {
        MudCommandBarModeHandler *handler = getHandler(mode);
        if (handler)
            handler->setCommandEventCallback();
    }

public:
    BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (pMsg->hwnd == m_edit && m_current_mode)
            return m_current_mode->PreTranslateMessage(pMsg);
        return FALSE;
    }

    MudCommandBar(PropertiesData *data) : propData(data),
        m_current_mode(NULL)
    {
        m_getTextBuffer.alloc(256);
    }
    void setParams(int size, HFONT font)
    {
        SetMinHeight(size);
        m_edit.SetFont(font);
    }

    

    void setFocus()
    {
        m_edit.SetFocus();
    }

    void getCommand(tstring *cmd)
    {
        cmd->assign(m_cmdBarBuffer);
        m_cmdBarBuffer.clear();
        if (propData->clear_bar)
            clear();
        else
            selectText();
    }

    void addToHistory(const tstring& cmd)
    {
        if (cmd.empty())
            return;

        std::vector<tstring> &h = propData->cmd_history;
        if (m_history_index != -1)
        {
            if (h[m_history_index] == cmd)
                h.erase(h.begin() + m_history_index);
            clearHistory();
        }

        if (!h.empty())
        {
            int last = h.size() - 1;
            if (h[last] == cmd)
                return;
        }
        h.push_back(cmd);

        int size = h.size();
        if (size > propData->cmd_history_size)
        {
            size = size - propData->cmd_history_size;
            h.erase(h.begin(), h.begin() + size);
        }
    }

    void reset()
    {
        clearHistory();
        clear();
        m_undo.clear();
    }

private:
    BEGIN_MSG_MAP(MudCommandBar)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_USER, OnPaste)
    END_MSG_MAP()

    void setText(const tstring& text, int cursor = -1, bool add_undo = true)
    {
        tstring curtext;
        getText(&curtext);
        if (text != curtext)
        {
            if (add_undo)
                addUndo();
            m_edit.SetWindowText(text.c_str());
        }
        int cursorpos = (cursor == -1) ? text.length() : cursor;   // move cursor to end, as default
        m_edit.SetSel(cursorpos, cursorpos); 
    }

    void clear()
    {
        m_edit.SetWindowText(L"");
        clearTab();
    }

    
    void selectText()
    {
        int len = m_edit.GetWindowTextLength();
        if (len > 0)
            m_edit.SetSel(0, len);
        clearTab();
    }

    void getText(tstring *text)
    {
        int count = m_edit.GetWindowTextLength() + 1;
        int memlen = count*sizeof(WCHAR);
        m_getTextBuffer.alloc(memlen);
        WCHAR* buffer = (WCHAR*)m_getTextBuffer.getData();
        m_edit.GetWindowText(buffer, count);
        text->assign(buffer);
    }

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        m_edit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT rc; GetClientRect(&rc);
        rc.right -= 48;
        m_edit.MoveWindow(&rc);
        return 0;
    }

    LRESULT OnPaste(UINT, WPARAM, LPARAM, BOOL&)
    {
        // on paste from clipboard
        tstring text;
        if (getFromClipboard(m_hWnd, &text))
        {
            tstring_replace(&text, L"\t", L"    ");
            if (isExistSymbols(text, L"\r\n"))
            {
                // multiline paste
                addUndo();
                putTextToBuffer(text);
                m_cmdBarBuffer = text;
                SendMessage(m_callback_hwnd, m_callback_msg, 0, 0);
                return 1;
            }
            else
            {
                tstring bartext;
                getText(&bartext);
                int from = 0, to = 0;
                m_edit.GetSel(from, to);

                int curpos = to;
                if (from != to)
                    bartext.replace(from, to, L"");
                else
                    curpos = from + text.length();
                bartext.insert(from, text);
                setText(bartext);
                m_edit.SetSel(curpos,curpos);
                return 1;
            }
        }
        return 0;
    }

    void putTextToBuffer()
    {
        getText(&m_cmdBarBuffer);
    }
    void putTextToBuffer(const tstring& text)
    {
        m_cmdBarBuffer = text;
    }
};

class MudCommandBarDefault : public MudCommandBarModeHandler
{
    tstring m_history_const;
    int m_history_index;
    tstring m_tab_const;
    tstring m_tab;
    int m_lasttab;
    int m_lasthistorytab;
    int m_lastsystemtab;
    struct undo_data {
        tstring text;
        int cursor;
    };
    std::deque<undo_data> m_undo;
public:
    MudCommandBarDefault(MudCommandBar* bar) : MudCommandBarModeHandler(bar), m_history_index(-1), m_lasttab(0), m_lasthistorytab(0), m_lastsystemtab(0) {}
    BOOL PreTranslateMessage(MSG* pMsg);
    
private:
    void setEventCallback(HWND hwnd, UINT msg)
    {
        m_callback_hwnd = hwnd;
        m_callback_msg = msg;
    }
    HWND m_callback_hwnd;
    UINT m_callback_msg;

    BOOL processChar(UINT key);
    BOOL processKeyDown(UINT key);
    void addUndo();
    void initHistory();
    void onHistoryUp();
    void onHistoryDown();
    void onTab();
    void resetTabIndexes();
    void clearTab();
    void clearHistory();
};
