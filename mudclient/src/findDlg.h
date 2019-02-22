#pragma once

class FindDlg : public CDialogImpl<FindDlg>
{
    CEdit m_text;
    CComboBox m_wnd;
    CButton m_find_end_begin;
    CButton m_find_begin_end;
public:
    enum { IDD = IDD_FIND };
    void setFocus() { PostMessage(WM_USER+1); }
    void setWindowName(int index, const tstring& name)
    {
        int count = m_wnd.GetCount();
        if (index >= 0 && index < count)
        {
            int selected = m_wnd.GetCurSel();
            m_wnd.InsertString(index, name.c_str());
            if (selected == index)
                m_wnd.SetCurSel(index);
            m_wnd.DeleteString(index+1);
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
            if (key >= '0' && key <= '0'+OUTPUT_WINDOWS && focused() && checkKeysState(false, true, false))
            {
                int index = key - '0';
                m_wnd.SetCurSel(index);
            }
            if (key == VK_TAB && focused())
            {
                HWND h = GetFocus();
                if (h == m_text)
                    ::SetFocus(m_wnd);
                if (h == m_wnd)
                    ::SetFocus(m_find_end_begin);
                if (h == m_find_end_begin || h == m_find_begin_end)
                    ::SetFocus(m_text);
                return TRUE;
            }
            if ((key == VK_UP || key == VK_DOWN) && focused())
            {
                 HWND h = GetFocus();
                 if (h == m_find_end_begin)
                     ::SetFocus(m_find_begin_end);
                 if (h == m_find_begin_end)
                     ::SetFocus(m_find_end_begin);
            }
        }
        return FALSE;
    }
    int getSelectedWindow() {
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
    int getDirection() {
        if (m_find_end_begin.GetCheck() == BST_CHECKED)
            return -1;
        return 1;
    }    
    bool focused()
    {
        HWND focus = GetFocus();
        return (focus == m_text || focus == m_wnd || focus == m_find_end_begin || focus == m_find_begin_end) ? true : false;    
    }

private:
    BEGIN_MSG_MAP(FindDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_USER+1, OnUser)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnSetFocus)
    END_MSG_MAP()
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
       m_text.Attach(GetDlgItem(IDC_EDIT_FINDTEXT));
       m_wnd.Attach(GetDlgItem(IDC_COMBO_FINDWINDOW));
       m_find_end_begin.Attach(GetDlgItem(IDC_RADIO_END_BEGIN));
       m_find_begin_end.Attach(GetDlgItem(IDC_RADIO_BEGIN_END));
       m_find_end_begin.SetCheck(BST_CHECKED);
       return 0;
    }
    LRESULT OnUser(UINT, WPARAM, LPARAM, BOOL&)
    {
       m_text.SetFocus();
       return 0;
    }
    LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_text.SetFocus();
        return 0;
    }
};

class FindView : public CWindowImpl<FindView>
{
    FindDlg m_dlg;
    SIZE m_size;
public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    void setFocus() { m_dlg.setFocus(); }
    bool focused() { return m_dlg.focused(); }
    void setWindowName(int index, const tstring& name) { m_dlg.setWindowName(index, name); }
    void selectWindow(int index) { m_dlg.selectWindow(index); }
    BOOL processMsg(MSG* pMsg) { return m_dlg.processMsg(pMsg); }
    int  getSelectedWindow() { return m_dlg.getSelectedWindow(); }
    void getTextToSearch(tstring* text) { m_dlg.getTextToSearch(text); }
    int  getDirection() {  return m_dlg.getDirection(); }
    SIZE getSize() { return m_size; }
private:
    BEGIN_MSG_MAP(PluginsView)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_dlg.Create(m_hWnd);
        RECT rc; m_dlg.GetClientRect(&rc);
        MoveWindow(&rc);
        m_dlg.ShowWindow(SW_SHOWNOACTIVATE);
        m_size.cx = rc.right;
        m_size.cy = rc.bottom;
        return 0;
    }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT rc; GetClientRect(&rc);
        m_dlg.MoveWindow(&rc);
        return 0;
    }
};
