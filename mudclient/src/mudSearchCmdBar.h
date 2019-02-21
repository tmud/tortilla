#pragma once

class MudSearchCmdBar : public MudCommandBarModeHandler
{
    CStaticEx m_label;
    SIZE m_label_size;
    CStaticEx m_label2;
    SIZE m_label2_size;
    CEditEx m_edit;
    int m_target_window;

private:
    void setTargetWindow(int window)
    {
        m_target_window = window;
        tchar buffer[16];
        _itow(window, buffer, 10);
        tstring text(L"Окно: ");
        text.append(buffer);
        m_label2.SetWindowText(text.c_str());
    }

private:
    void calcSize(HWND wnd, SIZE *sz)
    {
        CWindow window(wnd);
        CDC dc(window.GetDC());
        int text_len = window.GetWindowTextLength();
        int buffer = (text_len + 1) * sizeof(tchar);
        MemoryBuffer mb(buffer);
        tchar *str = (tchar*)mb.getData();
        window.GetWindowText(str, text_len+1);
        GetTextExtentPoint32(dc, str, text_len, sz);
    }

    BOOL create(HWND parent)
    {
        RECT pos = { 0 };
        m_label.Create(parent, pos, NULL, WS_CHILD);
        m_label.SetWindowText(L"Поиск: ");
        m_label2.Create(parent, pos, NULL, WS_CHILD);        
        setTargetWindow(0);
        m_edit.Create(parent, pos, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL |ES_MULTILINE, WS_EX_CLIENTEDGE);        
        m_edit.setBackroundColor(RGB(255, 255, 0));
        return TRUE;
    }

    void resize(int width, int height)
    {
        RECT rc = { 0, 0, width, height };
        rc.left = 2;
        rc.right = rc.left + m_label_size.cx;
        int ty = (height - m_label_size.cy) / 2;
        rc.top = ty; rc.bottom = ty + m_label_size.cy;
        m_label.MoveWindow(&rc);

        int off = width - (rc.right + 4);
        off = off - (m_label2_size.cx + 32);
           
        RECT rc2 = { rc.right + 4, 0, 0, height };
        rc2.right = rc2.left + off;
        m_edit.MoveWindow(&rc2);

        rc.left = rc2.right + 4;
        rc.right = rc.left + m_label2_size.cx+16;
        m_label2.MoveWindow(&rc);
    }
    
    void setVisible(bool visible)
    {
        int cmdShow = visible ? SW_SHOW : SW_HIDE;
        m_label.ShowWindow(cmdShow);
        m_label2.ShowWindow(cmdShow);
        m_edit.ShowWindow(cmdShow);
    }

    BOOL translateMessage(MSG* pMsg, BOOL *enter)
    {
        if (pMsg->hwnd == m_edit)
        {
            if (pMsg->message == WM_CHAR)
            {
                DWORD key = pMsg->wParam;
                if (key == VK_ESCAPE)
                {
                    if (m_edit.GetWindowTextLength() != 0)
                        m_edit.clearText();
                    else
                        *enter = TRUE;
                }

           
                //BOOL result = processChar(pMsg->wParam);
                /*if (pMsg->wParam == VK_RETURN)
                {
                    *enter = TRUE;
                    if (propData->clear_bar)
                    else
                        clear();
                        selectText();
                }
                return result;*/
            }
            else if (pMsg->message == WM_KEYDOWN)
            {
                int key = pMsg->wParam;
                int target_window = key - '0';
                if (target_window >= 0 && target_window <= OUTPUT_WINDOWS && checkKeysState(false, true, false))
                {
                    setTargetWindow(target_window);
                    return TRUE;
                }
            }
        }
        

        return FALSE;
    }

    void setFont(HFONT font)
    {
        m_label.SetFont(font);
        calcSize(m_label, &m_label_size);
        m_label2.SetFont(font);
        calcSize(m_label2, &m_label2_size);
        m_edit.SetFont(font);
    }

    void setFocus()
    {
        m_edit.SetFocus();
    }

    void paste(const tstring& paste_text, BOOL *enter)
    {
    }

    void reset()
    {
    }

    void getCommands(MudCommandBarCommands* cmds)
    {
    }

    void historyCommands(const MudCommandBarCommands& cmds)
    {
    }
};
