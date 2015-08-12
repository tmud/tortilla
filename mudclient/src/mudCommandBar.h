#pragma once

#include "propertiesPages/propertiesData.h"

class CEditEx : public CWindowImpl < CEditEx, CEdit >
{
    MemoryBuffer m_getTextBuffer;
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
private:
    BEGIN_MSG_MAP(CEditEx)
       MESSAGE_HANDLER(WM_CREATE, OnCreate)
       MESSAGE_HANDLER(WM_PASTE, OnPaste)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_getTextBuffer.alloc(256);
        return 0;
    }
    LRESULT OnPaste(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        LRESULT result = ::SendMessage(GetParent(), WM_USER, 0, 0);
        if (!result)
            bHandled = FALSE;
        return 0;
    }
};

typedef std::vector<tstring> MudCommandBarCommands;

class MudCommandBarModeHandler
{
public:
    virtual BOOL create(HWND parent) = 0;
    virtual void resize(int width, int height) = 0;
    virtual BOOL translateMessage(MSG* pMsg, BOOL *enter) = 0;
    virtual void setFont(HFONT font) = 0;
    virtual void setFocus() = 0;
    virtual void paste(const tstring& paste_text, BOOL *enter) = 0;
    virtual void reset() = 0;
    virtual void getCommands(MudCommandBarCommands* cmds) = 0;
    virtual void historyCommands(MudCommandBarCommands* cmds) = 0;
};

class MudCommandBarGameMode : public MudCommandBarModeHandler
{
    PropertiesData* propData;
    CEditEx m_edit;
    tstring m_cmdBarBuffer;
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
    MudCommandBarGameMode(PropertiesData* pd) : propData(pd), m_history_index(-1), m_lasttab(0), m_lasthistorytab(0), m_lastsystemtab(0) {}

private:
    BOOL create(HWND parent) 
    {
       RECT pos = {0};
       m_edit.Create(parent, pos, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);       
       return m_edit.IsWindow();
    }

    void resize(int width, int height)
    {
        RECT rc = { 0, 0, width, height };
        m_edit.MoveWindow(&rc);
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
               if (pMsg->wParam == VK_RETURN)
                   *enter = TRUE;
               return processChar(pMsg->wParam);
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
            m_edit.setText(bartext);
            m_edit.SetSel(curpos, curpos);
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
        const tchar *separators = L"\r\n";
        if (!isExistSymbols(m_cmdBarBuffer, separators))
        {
            cmds->push_back(m_cmdBarBuffer);
            m_cmdBarBuffer.clear();
            return;
        }

       const tchar *p = m_cmdBarBuffer.c_str();
       const tchar *e = p + m_cmdBarBuffer.length();
       while (p < e)
       {
          size_t len = wcscspn(p, separators);
          if (len > 0)
            cmds->push_back(tstring(p, len));
          p = p + len + 1;
       }
       m_cmdBarBuffer.clear();
    }

    void historyCommands(MudCommandBarCommands* cmds)
    {
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

                    //tstring curtext;
                    //m_edit.getText(&curtext);
                    //if (u.text != curtext)
                    {
                        //if (add_undo)
                        //    addUndo();
                        m_edit.setText(u.text);
                    }
                    
                    m_edit.setCursor(u.cursor);
                }
            }
            else { return FALSE; }
        }
        else  { return FALSE; }
        return TRUE;
    }

    void putTextToBuffer()
    {
        m_edit.getText(&m_cmdBarBuffer);
    }

    void putTextToBuffer(const tstring& text)
    {
        m_cmdBarBuffer.assign(text);
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

    /*void selectText()
    {
        m_edit.selectText();
        clearTab();
    }*/

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
};

/*class MudCommandBarSearchMode : public MudCommandBarModeHandler
{
private:
};*/

class MudCommandBar : public CWindowImpl < MudCommandBar, CStatusBarCtrl >
{    
    enum BARMODE { DEFAULT = 0, SEARCH };
    std::map<BARMODE, MudCommandBarModeHandler*> m_mode_handlers;
    typedef std::map<BARMODE, MudCommandBarModeHandler*>::iterator mode_handlers_iterator;
    BARMODE m_current_mode;

    PropertiesData* propData;

    HWND m_callback_hwnd;
    UINT m_callback_msg;

public:
    void setMode(BARMODE mode)
    {
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

    void getCommands(std::vector<tstring>& cmds)
    {
        MudCommandBarModeHandler *h = getHandler();
        if (!h) return;
            
    }

    /*void getCommand(tstring *cmd)
    {
        cmd->assign(m_cmdBarBuffer);
        m_cmdBarBuffer.clear();
        if (propData->clear_bar)
            clear();
        else
            selectText();
    }*/

   /* void addToHistory(const tstring& cmd)
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
    }*/

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
        MudCommandBarModeHandler *default_ = new MudCommandBarGameMode(propData);
        if (default_ && default_->create(m_hWnd))
           m_mode_handlers[DEFAULT] = default_;
        //m_mode_handlers[SEARCH] = new MudCommandBarSearchMode();
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
