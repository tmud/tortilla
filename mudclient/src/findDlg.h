#pragma once

//LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
class FindDlg : public CDialogImpl<FindDlg>
{
    HHOOK m_hHook;
    CEdit m_text;
    CComboBox m_wnd;
    CButton m_find_end_begin;
    CButton m_find_begin_end;
public:
    enum { IDD = IDD_FIND };
    void setFocus() { m_text.SetFocus(); }
    void setWindowName(int index, const tstring& name)
    {
        int count = m_wnd.GetCount();
        if (index >= 0 && index < count)
        {
            m_wnd.SetDlgItemText(index, name.c_str());
            return;
        }
        count = count - index;
        for(; count > 0; --count)
            m_wnd.AddString(L"");
        m_wnd.AddString(name.c_str());
    }
    void selectWindow(int index) {
        m_wnd.SetCurSel(index);
    }
    BOOL processMsg(MSG* pMsg)
    {
        if (pMsg->message == WM_KEYDOWN)
        {
            WPARAM key = pMsg->wParam;
            if (key == VK_ESCAPE && focused())
            {
                /*if (m_text.GetWindowTextLength() != 0)
                {
                    m_text.SetWindowText(L"");
                    return FALSE;
                }*/
                return TRUE;
            }
            if (key == VK_RETURN && focused())
                return TRUE;
            /*if (key == VK_TAB)
            {
                HWND focus = GetFocus();
                if (focus == m_text)
                    m_wnd.SetFocus();
                if (focus == m_wnd)
                    m_text.SetFocus();
                return FALSE;
            }*/
            if (key >= '0' && key <= '0'+OUTPUT_WINDOWS && focused() && checkKeysState(false, true, false))
            {
                int index = key - '0';
                m_wnd.SetCurSel(index);
            }
        }
        return FALSE;
    }
    int  getSelectedWindow() { 
        int index = m_wnd.GetCurSel();
        return index;
    }
    void getTextToSearch(tstring* text) {
        int len = m_text.GetWindowTextLength();
        wchar_t *buffer = new wchar_t[len+1];
        m_text.GetWindowText(buffer, len+1);
        text->assign(buffer);
        delete []buffer;
    }
    bool focused()
    {
        HWND focus = GetFocus();
        return (focus == m_text || focus == m_wnd) ? true : false;    
    }

    /*LRESULT HookGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        LPMSG lpMsg = (LPMSG)lParam;
        if (nCode >= 0 && PM_REMOVE == wParam)
        {
            // Don't translate non-input events.
            if ((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST))
            {
                if (IsDialogMessage(lpMsg))
                {
                    // The value returned from this hookproc is ignored, 
                    // and it cannot be used to tell Windows the message has been handled.
                    // To avoid further processing, convert the message to WM_NULL 
                    // before returning.
                    lpMsg->message = WM_NULL;
                    lpMsg->lParam = 0;
                    lpMsg->wParam = 0;
                }
            }
        }
        return CallNextHookEx(m_hHook, nCode, wParam, lParam);
    }*/

private:
    BEGIN_MSG_MAP(FindDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)
    END_MSG_MAP()
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
       //m_hHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc,  NULL, GetCurrentThreadId() );
       m_text.Attach(GetDlgItem(IDC_EDIT_FINDTEXT));
       m_wnd.Attach(GetDlgItem(IDC_COMBO_FINDWINDOW));
       m_find_end_begin.Attach(GetDlgItem(IDC_RADIO_END_BEGIN));
       m_find_begin_end.Attach(GetDlgItem(IDC_RADIO_BEGIN_END));
       return 0;
    }
    LRESULT OnDestroyDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        //UnhookWindowsHookEx(m_hHook);
        return 0;
    }
};

/*class FindView : public CWindowImpl<FindView>
{
    FindDlg m_dlg;
public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    void setFocus() { PostMessage(WM_USER); }
    bool isFocused() { return m_dlg.focused(); }
    void setWindowName(int index, const tstring& name) { m_dlg.setWindowName(index, name); }
    void selectWindow(int index) { m_dlg.selectWindow(index); }
    BOOL processMsg(MSG* pMsg) { return m_dlg.processMsg(pMsg); }
    int  getSelectedWindow() { return m_dlg.getSelectedWindow(); }
    void getTextToSearch(tstring* text) { m_dlg.getTextToSearch(text); }
private:
    BEGIN_MSG_MAP(PluginsView)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_SETFOCUS, OnUser)
        MESSAGE_HANDLER(WM_USER, OnUser)
    END_MSG_MAP()   
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_dlg.Create(m_hWnd);
        RECT rc; m_dlg.GetClientRect(&rc);
        rc.right +=  (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXBORDER)) * 2;
        rc.bottom += (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYBORDER)) * 2 + GetSystemMetrics(SM_CYSMCAPTION);
        MoveWindow(&rc);
        m_dlg.ShowWindow(SW_SHOWNOACTIVATE);
        return 0;
    }    
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT rc; GetClientRect(&rc);
        m_dlg.MoveWindow(&rc);
        return 0;
    }
    LRESULT OnUser(UINT, WPARAM wparam, LPARAM lparam, BOOL&)
    {
        m_dlg.setFocus();
        return 0;
    }
};
*/