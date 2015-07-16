#include "stdafx.h"
#include "mudCommandBar.h"

BOOL MudCommandBarDefault::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_CHAR)
        return processChar(pMsg->wParam);
    else if (pMsg->message == WM_KEYDOWN)
        return processKeyDown(pMsg->wParam);
}

BOOL MudCommandBarDefault::processChar(UINT key)
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

BOOL MudCommandBarDefault::processKeyDown(UINT key)
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
        clear(); clearHistory();
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
                setText(u.text, u.cursor, false);
            }
        }
        else { return FALSE; }
    }
    else  { return FALSE; }
    return TRUE;
}

void MudCommandBarDefault::addUndo()
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

void MudCommandBarDefault::initHistory()
{
    if (m_history_const.empty() && m_history_index == -1)
    {
        getText(&m_history_const);
    }
}

void MudCommandBarDefault::onHistoryUp()
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
        setText(h[m_history_index]);
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
                setText(h[f]);
                clearTab();
                return;
            }
        }
    }
    clearTab();
}

void MudCommandBarDefault::onHistoryDown()
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
        setText(h[m_history_index]);
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
                setText(h[f]);
                clearTab();
                return;
            }
        }
        setText(hc);
        clearHistory();
    }
    clearTab();
}

 void MudCommandBarDefault::onTab()
 {
     tchar prefix = propData->cmd_prefix;
     tchar separator = propData->cmd_separator;
     tchar cmd_prefix[2] = { prefix, 0 };

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
                 setText(tab);
                 return;
             }
         }
     }

     if (m_tab.empty())
     {
         int last = m_tab_const.size() - 1;
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
             setText(cmd);
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

 void MudCommandBarDefault::resetTabIndexes()
 {
     m_lasttab = 0;
     m_lasthistorytab = 0;
     m_lastsystemtab = 0;
 }

 void MudCommandBarDefault::clearTab()
 {
     resetTabIndexes();
     m_tab_const.clear();
     m_tab.clear();
 }

 void MudCommandBarDefault::clearHistory()
 {
     m_history_index = -1;
     m_history_const.clear();
 }
