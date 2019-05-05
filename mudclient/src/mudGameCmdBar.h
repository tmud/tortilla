#pragma once

#include "mudCommandBar.h"

class MudGameCmdBar : public MudCommandBarModeHandler
{
    PropertiesData* propData;
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
    MudCommandBarCommands m_cmds_buffer;

public:
    MudGameCmdBar(PropertiesData* pd) : propData(pd), m_history_index(-1), m_lasttab(0), m_lasthistorytab(0), m_lastsystemtab(0) {}

private:
    BOOL create(HWND parent)
    {
        RECT pos = { 0 };
        m_edit.Create(parent, pos, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL | ES_MULTILINE, WS_EX_CLIENTEDGE);
        return m_edit.IsWindow();
    }

    void resize(int width, int height)
    {
        RECT rc = { 0, 0, width, height };
        m_edit.MoveWindow(&rc);
    }

    void setVisible(bool visible)
    {
        m_edit.ShowWindow(visible ? SW_SHOW : SW_HIDE);
    }

    void setFont(HFONT font)
    {
        m_edit.SetFont(font);
    }

    void setFocus()
    {
        m_edit.SetFocus();
    }

    BOOL translateMessage(MSG* pMsg, BOOL *enter)
    {
        if (pMsg->hwnd == m_edit)
        {
            if (pMsg->message == WM_CHAR)
            {
                BOOL result = processChar(pMsg->wParam);
                if (pMsg->wParam == VK_RETURN)
                {
                    *enter = TRUE;
                    if (propData->clear_bar)
                        clear();
                    else
                        selectText();
                }
                return result;
            }
            else if (pMsg->message == WM_KEYDOWN)
                return processKeyDown(pMsg->wParam);
        }
        return FALSE;
    }

    void paste(const tstring& paste_text, BOOL *enter)
    {
        tstring text(paste_text);
        tstring_replace(&text, L"\t", L"    ");
        if (isExistSymbols(text, L"\r\n"))
        {
            // multiline paste
            addUndo();
            putTextToBuffer(text);
            if (!propData->clear_bar)
                selectText();

            *enter = TRUE;
        }
        else
        {
            tstring bartext;
            m_edit.getText(&bartext);
            int from = 0, to = 0;
            m_edit.GetSel(from, to);

            int curpos = to;
            if (from != to)
                bartext.replace(from, to, L"");
            else
                curpos = from + text.length();
            bartext.insert(from, text);
            m_edit.setText(bartext, curpos);
        }
    }

    void reset()
    {
        clearHistory();
        clear();
        m_undo.clear();
    }

    void getCommands(MudCommandBarCommands* cmds)
    {
        cmds->swap(m_cmds_buffer);
        m_cmds_buffer.clear();
    }

    void historyCommands(const MudCommandBarCommands& cmds)
    {
        if (cmds.size() != 1)
            return;

        const tstring& cmd = cmds[0];
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

private:
    BOOL processChar(UINT key)
    {
        if (key == VK_RETURN)
        {
            addUndo();
            putTextToBuffer();
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
            onHistoryUp();
        else if (key == VK_ESCAPE)
        {
            clear();
            clearHistory();
        }
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
                    m_edit.setText(u.text, u.cursor);
                }
            }
            else { return FALSE; }
        }
        else  { return FALSE; }
        return TRUE;
    }

    void putTextToBuffer()
    {
        tstring text;
        m_edit.getText(&text);
        putTextToBuffer(text);
    }

    void putTextToBuffer(const tstring& text)
    {
        m_cmds_buffer.clear();
        const tchar *separators = L"\r\n";
        if (!isExistSymbols(text, separators))
        {
            m_cmds_buffer.push_back(text);
        }
        else
        {
            const tchar *p = text.c_str();
            const tchar *e = p + text.length();
            while (p < e)
            {
                size_t len = wcscspn(p, separators);
                if (len > 0)
                    m_cmds_buffer.push_back(tstring(p, len));
                p = p + len + 1;
            }
            int last = text.size() - 1;
            if (last > 0)
            {
                tchar last_char = text.at(last);
                if (last_char != L'\r' && last_char != L'\n')
                {
                    int last_cmd = m_cmds_buffer.size() - 1;
                    m_edit.setText(m_cmds_buffer[last_cmd]);
                    m_cmds_buffer.pop_back();
                }
            }
        }
    }

    void initHistory()
    {
        if (m_history_const.empty() && m_history_index == -1)
        {
            m_edit.getText(&m_history_const);
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
                m_history_index = h.size() - 1;
            else if (m_history_index != 0)
                m_history_index -= 1;
            m_edit.setText(h[m_history_index]);
        }
        else
        {
            int last = h.size() - 1;
            int f = m_history_index;
            if (f == -1) f = last;
            else f = f - 1;
            for (; f >= 0; f--) {
                if (!h[f].compare(0, hc.length(), hc))
                {
                    m_history_index = f;
                    m_edit.setText(h[f]);
                    clearTab();
                    return;
                }
            }
        }
        clearTab();
    }

    void onHistoryDown()
    {
        assert(m_history_index != -1);
        std::vector<tstring> &h = propData->cmd_history;
        initHistory();
        const tstring& hc = m_history_const;
        if (hc.empty())
        {
            int last_history = h.size() - 1;
            if (m_history_index != last_history)
                m_history_index += 1;
            else
            {
                clear();
                clearHistory();
                return;
            }
            m_edit.setText(h[m_history_index]);
        }
        else
        {
            int f = m_history_index;
            if (f == -1) f = 0;
            else f = f + 1;
            for (int last = h.size() - 1; f <= last; f++) {
                if (!h[f].compare(0, hc.length(), hc))
                {
                    m_history_index = f;
                    m_edit.setText(h[f]);
                    clearTab();
                    return;
                }
            }
            m_edit.setText(hc);
            clearHistory();
        }
        clearTab();
    }

    void addUndo()
    {
        undo_data u;
        m_edit.getText(&u.text);
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

    void onTab()
    {
        tchar prefix = propData->cmd_prefix;
        tchar separator = propData->cmd_separator;
        tchar cmd_prefix[2] = { prefix, 0 };

        tstring text;
        m_edit.getText(&text);
        if (text.empty()) {
            m_edit.setText(cmd_prefix);
            return;
        }

        bool can_use_history = false;
        bool syscmd = false;

        bool it_first_tab = m_tab_const.empty() && m_tab.empty();
        if (it_first_tab)
        {
            int lastidx = text.size() - 1;
            int i = lastidx;
            for (; i >= 0; --i)
            {
                if (text.at(i) == separator) break;
            }
            if (i != lastidx && text[i + 1] == prefix)
                syscmd = true;
            if (i >= 0)
            {
                if (syscmd) i++;
                m_tab_const.assign(text.substr(0, i + 1));
                m_tab.assign(text.substr(i + 1));
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
                int last = m_tab_const.size() - 1;
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
            for (int i = m_lasthistorytab, e = h.size() - 1; i <= e; ++i)
            {
                int back_index = e - i;  // reverse tabbing in commands history
                const tstring& tab = h[back_index];
                if (!tab.compare(0, min_len, htab))
                {
                    if (!tab.compare(htab))
                        continue;
                    m_lasthistorytab = i + 1;
                    m_edit.setText(tab);
                    return;
                }
            }
        }

        if (m_tab.empty())
        {
            int last = m_tab_const.size() - 1;
            if (m_tab_const[last] != prefix)
                m_tab_const.append(cmd_prefix);
            m_edit.setText(m_tab_const);
            resetTabIndexes();
            return;
        }

        tstring ctab(syscmd ? cmd_prefix : L"");
        ctab.append(m_tab);
        int min_len = ctab.length();

        PropertiesList &list = propData->tabwords;
        for (int i = m_lasttab, e = list.size(); i < e; ++i)
        {
            const tstring& tab = list.get(i);
            if (!tab.compare(0, min_len, ctab))
            {
                if (!tab.compare(ctab))
                    continue;
                m_lasttab = i + 1;
                tstring cmd(m_tab_const);
                if (syscmd)
                    cmd.append(tab.substr(1));
                else
                    cmd.append(tab);
                m_edit.setText(cmd);
                return;
            }
        }

        if (syscmd)
        {
            const tstring &ctab = m_tab;
            int min_len = ctab.length();
            PropertiesList &list = propData->tabwords_commands;
            for (int i = m_lastsystemtab, e = list.size(); i < e; ++i)
            {
                const tstring& tab = list.get(i);
                if (!tab.compare(0, min_len, ctab))
                {
                    if (!tab.compare(ctab))
                        continue;
                    m_lastsystemtab = i + 1;
                    tstring cmd(m_tab_const);
                    cmd.append(tab);
                    m_edit.setText(cmd);
                    return;
                }
            }
        }

        resetTabIndexes();
        tstring cmd(m_tab_const);
        cmd.append(m_tab);
        m_edit.setText(cmd);
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

    void clear()
    {
        m_edit.clearText();
        clearTab();
    }

    void selectText()
    {
        m_edit.selectText();
        clearTab();
    }
};
