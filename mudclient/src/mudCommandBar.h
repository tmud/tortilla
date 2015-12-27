#pragma once

class MudCommandBar :  public CWindowImpl<MudCommandBar, CStatusBarCtrl>
{
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
    HWND m_callback_hwnd;
    UINT m_callback_msg;

public:
    BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (pMsg->hwnd == m_edit)
        {
            if (pMsg->message == WM_CHAR)
                return processChar(pMsg->wParam);
            else if (pMsg->message == WM_KEYDOWN)
                return processKeyDown(pMsg->wParam);
        }
        return FALSE;
    }

    MudCommandBar(PropertiesData *data) : propData(data), m_history_index(-1), m_lasttab(0), m_lasthistorytab(0), m_lastsystemtab(0),
        m_callback_hwnd(NULL), m_callback_msg(0)
    {
        m_getTextBuffer.alloc(256);
    }

    void setParams(int size, HFONT font)
    {
        SetMinHeight(size);
        m_edit.SetFont(font);
    }

    void setCommandEventCallback(HWND hwnd, UINT msg)
    {
        m_callback_hwnd = hwnd;
        m_callback_msg = msg;
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

    void setCommand(const tstring& cmd)
    {
        setText(cmd, -1, false);
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

    void addUndo()
    {
        undo_data u;
        getText(&u.text);
        if (!m_undo.empty())
        {
            int last = m_undo.size() - 1;
            if (m_undo[last].text == u.text)
                return;
        }
        int from = 0, to = 0;
        m_edit.GetSel(from, to);
        u.cursor = to;
        m_undo.push_back(u);
        if (m_undo.size() == 30)
            m_undo.pop_front();
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

    BOOL processChar(UINT key)
    {
        if (key == VK_RETURN)
        {
            addUndo();
            putTextToBuffer();
            SendMessage(m_callback_hwnd, m_callback_msg, 0, 0);
            return TRUE;
        }
        if (key != VK_TAB && key != VK_ESCAPE)
        {
            addUndo();
            putTextToBuffer();
            clearTab();
            clearHistory();
            return FALSE;
        }
        return TRUE;   // disable system sound VK_TAB + VK_ESCAPE
    }

    BOOL processKeyDown(UINT key)
    {
        if (key == VK_DOWN)
        {
            if (m_history_index == -1)
                clear();
            else
                onHistoryDown();
        }
        else if (key == VK_UP)
        {
            int from = 0, to = 0;
            m_edit.GetSel(from, to);
            if (from != to)
            {
                int len = m_edit.GetWindowTextLength();
                m_edit.SetSel(len, len);
                return TRUE;
            }
            onHistoryUp();
        }
        else if (key == VK_ESCAPE)
            { clear(); clearHistory(); }
        else if (key == VK_TAB)
            onTab();
        else if (key == 'Z')
        {
            if (checkKeysState(false, true, false))
            {
                if (!m_undo.empty()) {
                int last = m_undo.size() - 1;
                undo_data u = m_undo[last];
                m_undo.pop_back();
                setText(u.text, u.cursor, false);
                }
            }
            else { return FALSE; }
        }
        else  { return FALSE; }
        return TRUE;
    }

    void putTextToBuffer()
    {
        getText(&m_cmdBarBuffer);
    }

    void putTextToBuffer(const tstring& text)
    {
        m_cmdBarBuffer = text;
    }

    void initHistory()
    {
        if (m_history_const.empty() && m_history_index == -1)
        {
            int start = -1; int end = -1;
            m_edit.GetSel(start, end);
            if (start == end)
                getText(&m_history_const);
            else
            {
                std::vector<tstring> &h = propData->cmd_history;
                if (!h.empty())
                {
                     tstring t;
                     getText(&t);
                     int last = h.size() - 1;
                     if (h[last] == t)
                         m_history_index = last;
                }
            }
        }
    }

    void onHistoryUp()
    {
        std::vector<tstring> &h = propData->cmd_history;
        if (h.empty())
            return;

        initHistory();
        const tstring& hc = m_history_const;
        if (hc.empty())
        {
            if (m_history_index == -1)
                m_history_index = h.size()-1;
            else if (m_history_index != 0)
                m_history_index -= 1;
            setText(h[m_history_index]);
        }
        else
        {
            int last = h.size() - 1;
            int f = m_history_index;
            if (f == -1) f = last;
            else f = f - 1;
            for (;f>=0; f--) {
            if (!h[f].compare(0, hc.length(), hc))
            {
                m_history_index = f;
                setText(h[f]);
                clearTab();
                return;
            }}
        }
        clearTab();
    }

    void onHistoryDown()
    {
        assert (m_history_index != -1);        
        std::vector<tstring> &h = propData->cmd_history;
        initHistory();
        const tstring& hc = m_history_const;
        if (hc.empty())
        {
            int last_history = h.size()-1;
            if (m_history_index != last_history)
                m_history_index += 1;
            else
            {
                clear();
                clearHistory();
                return;
            }
            setText(h[m_history_index]);
        }
        else
        {
            int f = m_history_index;
            if (f == -1) f = 0;
            else f = f + 1;
            for (int last=h.size()-1; f<=last; f++) {
            if (!h[f].compare(0, hc.length(), hc))
            {
                m_history_index = f;
                setText(h[f]);
                clearTab();
                return;
            }}
            setText(hc);
            clearHistory();
        }
        clearTab();
    }

    void onTab()
    {
        WCHAR prefix = propData->cmd_prefix;
        WCHAR separator =  propData->cmd_separator;
        WCHAR cmd_prefix[2] = { prefix, 0 };

        tstring text;
        getText(&text);
        if (text.empty()) {
            setText(cmd_prefix);
            return;
        }

        bool can_use_history = false;
        bool syscmd = false;

        bool it_first_tab = m_tab_const.empty() && m_tab.empty();
        if (it_first_tab)
        {
            int lastidx = text.size()-1;
            int i = lastidx;
            for (; i>=0; --i)
                {  if (text.at(i) == separator) break; }
            if (i != lastidx && text[i+1] == prefix)
                syscmd = true;
            if (i >= 0)
            {
                if (syscmd) i++;
                m_tab_const.assign( text.substr(0, i+1) );
                m_tab.assign( text.substr(i+1) );
            }
            else
            {
                m_tab_const.assign(syscmd ? cmd_prefix : L"");
                m_tab.assign(syscmd ? text.substr(1) : text);
                can_use_history = true; // История не работает в составных командах(;), только как полной строкой
            }
        }
        else
        {
            if (m_tab_const.empty())
                can_use_history = true;
            else if (m_tab_const == cmd_prefix) {
                can_use_history = true;
                syscmd = true;
            }
            else
            {
                 int last = m_tab_const.size()-1;
                 if (m_tab_const[last] == prefix)
                     syscmd = true;
            }
        }

        if (propData->history_tab && can_use_history)
        {
            tstring htab(m_tab_const);
            htab.append(m_tab);
            int min_len = htab.length();
            const std::vector<tstring> &h = propData->cmd_history;
            for (int i = m_lasthistorytab, e=h.size()-1; i <= e; ++i)
            {
                int back_index = e-i;  // reverse tabbing in commands history
                const tstring& tab = h[back_index];
                if (!tab.compare(0, min_len, htab))
                {
                    if (!tab.compare(htab))
                        continue;
                    m_lasthistorytab = i + 1;
                    setText(tab);
                    return;
                }
            }
        }

        if (m_tab.empty())
        {
           int last = m_tab_const.size()-1;
           if (m_tab_const[last] != prefix)
               m_tab_const.append(cmd_prefix);
            setText(m_tab_const);
            resetTabIndexes();
            return;
        }

        tstring ctab(syscmd ? cmd_prefix : L"");
        ctab.append(m_tab);
        int min_len = ctab.length();

        PropertiesList &list = propData->tabwords;
        for (int i=m_lasttab,e=list.size(); i<e; ++i)
        {
            const tstring& tab = list.get(i);
            if (!tab.compare(0, min_len, ctab))
            {
                if (!tab.compare(ctab))
                    continue;
                m_lasttab = i+1;
                tstring cmd(m_tab_const);
                if (syscmd)
                    cmd.append(tab.substr(1));
                else
                    cmd.append(tab);
                setText(cmd);
                return;
            }
        }

        if (syscmd)
        {
            const tstring &ctab = m_tab;
            int min_len = ctab.length();
            PropertiesList &list = propData->tabwords_commands;
            for (int i=m_lastsystemtab,e=list.size(); i<e; ++i)
            {
                const tstring& tab = list.get(i);
                if (!tab.compare(0, min_len, ctab))
                {
                    if (!tab.compare(ctab))
                        continue;
                    m_lastsystemtab = i+1;
                    tstring cmd(m_tab_const);
                    cmd.append(tab);
                    setText(cmd);
                    return;
                }
            }
        }

        resetTabIndexes();
        tstring cmd(m_tab_const);
        cmd.append(m_tab);
        setText(cmd);
    }

    void resetTabIndexes()
    {
        m_lasttab = 0;
        m_lasthistorytab = 0;
        m_lastsystemtab = 0;
    }

    void clearTab()
    {
        resetTabIndexes();
        m_tab_const.clear();
        m_tab.clear();
    }

    void clearHistory()
    {
        m_history_index = -1;
        m_history_const.clear();
    }
};
