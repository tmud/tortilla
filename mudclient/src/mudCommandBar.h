#pragma once

class MudCommandBar :  public CWindowImpl<MudCommandBar, CStatusBarCtrl>
{
    PropertiesData* propData;
    MemoryBuffer m_cmdBar;
    CEdit m_edit;
    int m_history_index;
    tstring m_tab;
    int m_lasttab;
    int m_lasthistorytab;
    struct undata {
        tstring text;
        int cursor;
    };
    std::vector<undata> m_undo;
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

    MudCommandBar(PropertiesData *data) : propData(data), m_history_index(-1), m_lasttab(0), m_lasthistorytab(0),
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
        undata u = m_undo[last];
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
            m_history_index = -1;
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
        m_history_index = -1;
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
        undata u; u.text = cmd; u.cursor = cursor;
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
            { clear(); m_history_index = -1; }
        else if (key == VK_TAB)
            onTab();
        else
            return FALSE;
        return TRUE;
    }

    void putTextToBuffer()
    {
        int len = m_edit.GetWindowTextLength();
        int buffer_len = m_cmdBar.getSize() - 1;
        if (buffer_len < len)
            initCmdBar(len);
        m_edit.GetWindowText((LPTSTR)m_cmdBar.getData(), m_cmdBar.getSize());
    }

    void initCmdBar(int size)
    {
        m_cmdBar.alloc((size+1) * sizeof(WCHAR));
    }

    void onHistoryUp()
    {
        std::vector<tstring> &h = propData->cmd_history;
        if (h.empty())
            return;

        if (m_history_index == -1)
            m_history_index = h.size() - 1;
        else
        {
            if (m_history_index != 0)
                m_history_index -= 1;
        }
        const tstring& cmd = h[m_history_index];
        setText(cmd);
        clearTab();
    }

    void onHistoryDown()
    {
        assert (m_history_index != -1);
        std::vector<tstring> &h = propData->cmd_history;
        int last_history = h.size() -1;
        if (m_history_index != last_history)
            m_history_index += 1;
        else
        {
            clear();
            m_history_index = -1;
            return;
        }
        const tstring& cmd = h[m_history_index];
        setText(cmd);
        clearTab();
    }

    void onTab()
    {
        // tabbing simplest variants
        WCHAR prefix = propData->cmd_prefix;
        WCHAR cmd_prefix[2] = { prefix, 0 };
        int len = m_edit.GetWindowTextLength();
        if (len == 0) {
            setText(cmd_prefix);
            return;
        }

        tstring text;
        getText(&text);
        int lastidx = text.length()-1;
        WCHAR last = text.at(lastidx);
        if (last == prefix)
            return;
        if (last == propData->cmd_separator) {
            text.append(cmd_prefix);
            setText(text);
            return;
        }

        // check system/not is command
        bool syscmd = false;
        int i=lastidx;
        for (; i>=0; --i)
        {
            if (text.at(i) == propData->cmd_separator) break;
            if (text.at(i) == prefix) { syscmd = true; break; }
        }
        i = i + 1;                          // i - first letter of tab part

        tstring cmd(text.substr(0, i));     // const part
        if (m_tab.empty())
            m_tab.assign(text.substr(i));   // tabbing part
 
        if (propData->history_tab)
        {
            bool extra = false;
            bool canTabHistory = (cmd.empty() || cmd == cmd_prefix) ? true : false;
            if (!canTabHistory && m_lasthistorytab != 0)
            {
                const tstring &tab = propData->cmd_history[m_lasthistorytab - 1];
                if (tab == text) { canTabHistory = true; extra = true; }
            }
            if (canTabHistory)
            {
                tstring ctab(syscmd ? cmd_prefix : L"");
                ctab.append(m_tab);
                int min_len = ctab.length();
                const std::vector<tstring> &h = propData->cmd_history;
                for (int i = m_lasthistorytab, e = h.size(); i < e; ++i)
                {
                    int back_index = (e-1)-i;  // reverse tabbing back
                    const tstring& tab = h[back_index];
                    if (!tab.compare(0, min_len, ctab))
                    {
                        if (!tab.compare(ctab))
                            continue;
                        m_lasthistorytab = i + 1;
                        setText(tab);
                        return;
                    }
                }
                if (extra) { syscmd = (cmd.at(0) == prefix); cmd.assign(syscmd ? cmd_prefix : L""); }
            }
        }

        int min_len = m_tab.length();
        int index = -1;
        PropertiesList &list = (syscmd) ? propData->tabwords_commands : propData->tabwords;
        for (int i=m_lasttab,e=list.size(); i<e; ++i)
        {
            const tstring& tab = list.get(i);
            if (!tab.compare(0,min_len, m_tab))
            {
                if (!tab.compare(m_tab))
                    continue;
                index = i;
                break;
            }
        }

        if (index == -1)
        {
            cmd.append(m_tab);
            m_lasttab = 0;
            m_lasthistorytab = 0;
        }
        else
        {
            const tstring& tab = list.get(index);
            cmd.append(tab);
            m_lasttab = index+1;
        }
        setText(cmd);
    }

    void clearTab()
    {
        m_tab.clear();
        m_lasttab = 0;
        m_lasthistorytab = 0;
    }
};
