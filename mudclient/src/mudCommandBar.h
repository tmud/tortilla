#pragma once

class MudCommandBar :  public CWindowImpl<MudCommandBar, CStatusBarCtrl>
{
    PropertiesData* propData;
    MemoryBuffer m_cmdBar;
    CEdit m_edit;
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
    std::vector<undo_data> m_undo;
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
        initCmdBar(1024);
        addUndo(L"", 0);
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
        WCHAR* buffer = (WCHAR*)m_cmdBar.getData();
        cmd->assign(buffer);
        buffer[0] = 0;
        if (propData->clear_bar)
            clear();
        else
            selectText();
    }

    void getSelected(tstring *cmd)
    {
        int from = 0, to = 0;
        m_edit.GetSel(from, to);
        if (from != to)
        {
            tstring text;
            getText(&text);
            cmd->assign(text.substr(from, to-from));
        }
    }

    void deleteSelected()
    {
        int from = 0, to = 0;
        m_edit.GetSel(from, to);
        if (from != to)
        {
            tstring text;
            getText(&text);
            addUndo(text, to);
            tstring new_cmd(text.substr(0, from));
            new_cmd.append(text.substr(to));
            setText(new_cmd, from);
        }
    }

    void insert(const tstring& cmd)
    {
        tstring str(cmd);
        tstring_trimsymbols(&str, L"\r\n");

        int from = 0, to = 0;
        m_edit.GetSel(from, to);
        tstring text;
        getText(&text);
        addUndo(text, to);
        if (from != to)
        {
            tstring new_cmd(text.substr(0, from));
            new_cmd.append(text.substr(to));
            text.assign(new_cmd);
        }
        text.insert(from, str);
        int cursor_pos = from + str.length();
        setText(text, cursor_pos);
    }

    void undo()
    {
        if (m_undo.empty()) 
            return;
        int last = m_undo.size() - 1;
        undo_data u = m_undo[last];
        m_undo.pop_back();
        setText(u.text, u.cursor);
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
    END_MSG_MAP()

    void setText(const tstring& text, int cursor = -1)
    {
        m_edit.SetWindowText(text.c_str());
        int cursorpos = (cursor == -1) ? text.length() : cursor;   // move cursor to end, as default
        m_edit.SetSel(cursorpos, cursorpos); 
    }

    void clear()
    {
        m_edit.SetWindowText(L"");
        clearTab();
    }

    void addUndo(const tstring& cmd, int cursor)
    {
        undo_data u; u.text = cmd; u.cursor = cursor;
        m_undo.push_back(u);
        if (m_undo.size() == 30)
            m_undo.erase(m_undo.begin());
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
        int len = (m_edit.GetWindowTextLength() + 1) * sizeof(WCHAR);
        MemoryBuffer tmp(len); WCHAR* buffer = (WCHAR*)tmp.getData();
        m_edit.GetWindowText(buffer, len);
        text->assign(buffer);
    }

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        m_edit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSize(UINT, WPARAM wparam, LPARAM lparam, BOOL& bHandled)
    {
        RECT rc; GetClientRect(&rc);
        rc.right -= 48;
        m_edit.MoveWindow(&rc);
        return 0;
    }

    BOOL processChar(UINT key)
    {
        if (key == VK_RETURN)
        {
            putTextToBuffer();
            SendMessage(m_callback_hwnd, m_callback_msg, 0, 0);
            return TRUE;
        }
        if (key != VK_TAB && key != VK_ESCAPE)
        {
            putTextToBuffer();
            WCHAR* buffer = (WCHAR*)m_cmdBar.getData();
            int from = 0, to = 0;
            m_edit.GetSel(from, to);
            addUndo(buffer, to);
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
            onHistoryUp();
        else if (key == VK_ESCAPE)
            { clear(); clearHistory(); }
        else if (key == VK_TAB)
            onTab();
        else if (key == 'V' && ::GetKeyState(VK_CONTROL) < 0 )
        {
            // on paste from clipboard
            tstring text;
            if (getFromClipboard(m_hWnd, &text) && isExistSymbols(text, L"\r\n"))
            {
                // multiline paste
                putTextToBuffer(text);
                SendMessage(m_callback_hwnd, m_callback_msg, 0, 0);
                return TRUE;
            }
            return FALSE;
        }
        else { return FALSE; }
        return TRUE;
    }

    void putTextToBuffer()
    {
        int len = m_edit.GetWindowTextLength();
        initCmdBar(len);
        m_edit.GetWindowText((WCHAR*)m_cmdBar.getData(), m_cmdBar.getSize());
    }

    void putTextToBuffer(const tstring& text)
    {
        int len = text.length();
        initCmdBar(len);
        wcscpy((WCHAR*)m_cmdBar.getData(), text.c_str());
    }

    void initCmdBar(int size)
    {
        m_cmdBar.alloc((size+1) * sizeof(WCHAR));
    }

    void initHistory()
    {
        if (m_history_const.empty() && m_history_index == -1)
        {
            getText(&m_history_const);
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
