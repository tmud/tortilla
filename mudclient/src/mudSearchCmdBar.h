#pragma once

class MudSearchCmdBar : public MudCommandBarModeHandler
{
    CStatic m_label;
    SIZE m_label_size;
    CStatic m_label2;
    SIZE m_label2_size;
    CEditEx m_edit;
    CEditEx m_edit2;
private:
    void calcSize(HWND wnd, SIZE *sz)
    {
        CWindow window(wnd);
        CDC dc(window.GetDC());
        int text_len = window.GetWindowTextLength();
        int buffer = (text_len + 1) * sizeof(tchar);
        MemoryBuffer mb(buffer);
        tchar *str = (tchar*)mb.getData();
        window.GetWindowText(str, text_len);
        GetTextExtentPoint32(dc, str, text_len, sz);
    }

    BOOL create(HWND parent)
    {
        RECT pos = { 0 };
        m_label.Create(parent, pos, NULL, WS_CHILD);
        m_label.SetWindowText(L"Поиск: ");
        m_label2.Create(parent, pos, NULL, WS_CHILD);
        m_label2.SetWindowText(L"Окно: ");
        COLORREF color = RGB(255, 255, 0);
        m_edit.Create(parent, pos, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL, WS_EX_STATICEDGE);        
        m_edit.setBackroundColor(color);
        m_edit2.Create(parent, pos, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL, WS_EX_STATICEDGE);
        m_edit2.setBackroundColor(color);
        m_edit2.SetWindowText(L"0");
        m_edit2.SetLimitText(1);
        return TRUE;
    }

    void resize(int width, int height)
    {
        RECT rc = { 0, 1, width, height-2 };
        rc.left = 2;
        rc.right = rc.left + m_label_size.cx;
        int ty = (height - m_label_size.cy) / 2;
        rc.top = ty; rc.bottom = ty + m_label_size.cy;
        m_label.MoveWindow(&rc);

        int off = width - (rc.right + 4);
        off = off - (m_label2_size.cx + 8);
        off = off - 64; // size of edit2
        
        RECT rc2 = { rc.right + 4, 1, 0, height-2 };
        rc2.right = rc2.left + off;
        m_edit.MoveWindow(&rc2);

        rc.left = rc2.right + 4;
        rc.right = rc.left + m_label2_size.cx;
        m_label2.MoveWindow(&rc);

        rc2.left = rc.right + 4;
        rc2.right = rc2.left + 64;
        m_edit2.MoveWindow(&rc2);
    }
    
    void setVisible(bool visible)
    {
        int cmdShow = visible ? SW_SHOW : SW_HIDE;
        m_label.ShowWindow(cmdShow);
        m_label2.ShowWindow(cmdShow);
        m_edit.ShowWindow(cmdShow);
        m_edit2.ShowWindow(cmdShow);
    }

    BOOL translateMessage(MSG* pMsg, BOOL *enter)
    {
        if (pMsg->hwnd == m_edit)
        {
            if (pMsg->message == WM_CHAR)
            {
                if (pMsg->wParam == VK_ESCAPE)
                {
                    if (m_edit.GetWindowTextLength() != 0)
                        m_edit.clearText();
                    else
                        *enter = TRUE;
                }

                if (pMsg->wParam == VK_TAB)
                {
                    m_edit2.SetFocus();
                }

                //BOOL result = processChar(pMsg->wParam);
                /*if (pMsg->wParam == VK_RETURN)
                {
                    *enter = TRUE;
                    if (propData->clear_bar)
                        clear();
                    else
                        selectText();
                }
                return result;*/
            }
            /*else if (pMsg->message == WM_KEYDOWN)
                return processKeyDown(pMsg->wParam);*/
        }
        if (pMsg->hwnd == m_edit2)
        {
            if (pMsg->message == WM_CHAR)
            {
                if (pMsg->wParam == VK_TAB)
                {
                    m_edit.SetFocus();
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
        m_edit2.SetFont(font);
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
