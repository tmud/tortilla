#pragma once

class MudCommandBar :  public CWindowImpl<MudCommandBar, CEdit>
{       
    PropertiesData* propData;
    MemoryBuffer m_cmdBar;
    int m_history_index;    
    tstring m_tab;
    int m_lasttab;
    tstring m_historytab;
    int m_lasthistorytab;
    tstring m_undo;
    int m_undo_cursor;

public:
    MudCommandBar(PropertiesData *data) : propData(data), m_history_index(-1), m_lasttab(0), m_lasthistorytab(0), m_undo_cursor(-1)
    {
        initCmdBar(255);
    }

    void getCommand(tstring *cmd)
    {
        WCHAR* buffer = (WCHAR*)m_cmdBar.getData();
        cmd->assign(buffer);
        buffer[0] = 0;
        if (propData->clear_bar)
            clear();        
        else
            selecttext();
    }

    void getSelected(tstring *cmd)
    {
        int from = 0, to = 0;
        GetSel(from, to);
        if (from != to)
        {
            tstring text;
            gettext(&text);
            cmd->assign(text.substr(from, to-from));
        }
    }

    void deleteSelected()
    {
        int from = 0, to = 0;
        GetSel(from, to);
        if (from != to)
        {
            tstring text;
            gettext(&text);
            m_undo = text;
            m_undo_cursor = to;
            tstring new_cmd(text.substr(0, from));
            new_cmd.append(text.substr(to));
            SetWindowText(new_cmd.c_str());
            SetSel(from, from);
        }
    }

    void insert(const tstring& cmd)
    {
        int from = 0, to = 0;
        GetSel(from, to);
        tstring text;
        gettext(&text);
        m_undo = text;
        m_undo_cursor = to;
        if (from != to)
        {
            tstring new_cmd(text.substr(0, from));
            new_cmd.append(text.substr(to));
            text.assign(new_cmd);
        }
        text.insert(from, cmd);
        SetWindowText(text.c_str());
        int cursor_pos = from + cmd.length();
        SetSel(cursor_pos, cursor_pos);
    }

    void undo()
    {
        if (m_undo.empty()) 
            return;
        SetWindowText(m_undo.c_str());        
        SetSel(m_undo_cursor, m_undo_cursor);
        m_undo.clear();
    }

private:
    BEGIN_MSG_MAP(MudCommandBar)
        MESSAGE_HANDLER(WM_CHAR, OnChar)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    END_MSG_MAP()

    void setText(const tstring& text)
    {
        SetWindowText(text.c_str());
        int lastsym = text.length();
        SetSel(lastsym, lastsym);   // move cursor to end
    }

    void clear()
    {
        SetWindowText(L"");
        clearTab();
    }

    void selecttext()
    {
        int len = GetWindowTextLength();
        if (len > 0)
            SetSel(0, len);
        clearTab();
    }

    void gettext(tstring *text)
    {
        int len = (GetWindowTextLength() + 1) * sizeof(WCHAR);
        MemoryBuffer tmp(len); WCHAR* buffer = (WCHAR*)tmp.getData();
        GetWindowText(buffer, len);
        text->assign(buffer);
    }

    LRESULT OnChar(UINT, WPARAM wparam, LPARAM lparam, BOOL& bHandled)
    {
        if (wparam == VK_RETURN)
        {
            int len = GetWindowTextLength();
            int buffer_len = m_cmdBar.getSize()-1;
            if (buffer_len < len)
                initCmdBar(len);
            GetWindowText((LPTSTR)m_cmdBar.getData(), m_cmdBar.getSize());
            onChangeHistory();
            SendMessage(GetParent(), WM_USER, 0, 0);
            return 0;
        }
        if (wparam != VK_TAB && wparam != VK_ESCAPE)
        {
            clearTab();
            bHandled = FALSE;
            return 0;
        }
        return 0;   // disable system sound VK_TAB + VK_ESCAPE
    }

    LRESULT OnKeyDown(UINT, WPARAM wparam, LPARAM, BOOL& bHandled)
    {
        if (wparam == VK_DOWN)
            onHistoryDown();
        else if (wparam == VK_UP)        
            onHistoryUp();
        else if (wparam == VK_ESCAPE)
            { clear(); }
        else if (wparam == VK_TAB)
            onTab();
        else
            bHandled = FALSE;
        return 0;
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
        if (m_history_index == -1)
            return;

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

    void onChangeHistory()
    {
        WCHAR* buffer = (WCHAR*)m_cmdBar.getData();
        tstring cmd(buffer);
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
            h.erase(h.begin(), h.begin()+size);
        }
    }

    void onTab()
    {
        WCHAR prefix = propData->cmd_prefix;
        WCHAR cmd_prefix[2] = { prefix, 0 };
        int len = GetWindowTextLength();
        if (len == 0) {
            setText(cmd_prefix);
            return;
        }

        tstring text;
        getWindowText(m_hWnd, &text);
        int lastidx = text.length()-1;
        WCHAR last = text.at(lastidx);
        if (last == prefix)
            return;
        if (last == propData->cmd_separator) {
            text.append(cmd_prefix);
            setText(text);
            return;
        }

        bool syscmd = false;
        int i=lastidx;
        for (; i>=0; --i)
        {
            if (text.at(i) == propData->cmd_separator) break;
            if (text.at(i) == prefix) { syscmd = true; break; }
        }
        i = i + 1;  // i - first letter for tabbing cmd

        if (!m_historytab.empty())
            i = (text.at(0) == prefix) ? 1 : 0;

        tstring cmd(text.substr(0, i));
        text = text.substr(i);
        if (m_tab.empty())
            m_tab = text;
        
        bool canTabHistory = (cmd.empty() || cmd == cmd_prefix) ? true : false;
        if (canTabHistory && !m_historytab.empty())
        {
            int min_len = m_historytab.length();
            std::vector<tstring> &h = propData->cmd_history;
            for (int i=m_lasthistorytab,e=h.size(); i<e; ++i)
            {
                const tstring& tab = h[i];
                if (!tab.compare(0, min_len, m_historytab))
                {
                    if (!tab.compare(m_historytab))
                        continue;
                    m_lasthistorytab = i+1;
                    setText(tab);
                    return;
                }
            }
            m_historytab.clear();
        }      
        
        bool full_cmd = false;
        int min_len = m_tab.length();
        int index = -1;
        PropertiesList &list = (syscmd) ? propData->tabwords_commands : propData->tabwords;
        for (int i=m_lasttab,e=list.size(); i<e; ++i)
        {
            const tstring& tab = list.get(i);
            if (!tab.compare(0,min_len, m_tab))
            {
                if (!tab.compare(m_tab))
                    { full_cmd = true; continue; }
                index = i;
                break;
            }
        }
        
        if (index == -1)
        {
            cmd.append(m_tab);
            m_lasttab = 0;
        }
        else
        {
            const tstring& tab = list.get(index);                
            cmd.append(tab);
            m_lasttab = index+1;
            full_cmd = true;
        }

        if (propData->history_tab && full_cmd)
        {            
            int dp = (cmd.at(0) == prefix) ? 1 : 0;
            const WCHAR *p = cmd.c_str() + dp;
            if (wcschr(p, prefix))
                m_historytab.clear();
            else
                m_historytab.assign(cmd);
            m_lasthistorytab = 0;
        }

        setText(cmd);
    }

    void clearTab()
    {
        m_tab.clear();
        m_lasttab = 0;
        m_historytab.clear();
        m_lasthistorytab = 0;
    }
};
