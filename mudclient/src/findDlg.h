#pragma once

LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
class FindDlg : public CDialogImpl<FindDlg>
{
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
            if (key >= '0' && key <= '0'+OUTPUT_WINDOWS && focused() && checkKeysState(false, true, false))
            {
                int index = key - '0';
                m_wnd.SetCurSel(index);
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
    bool focused()
    {
        HWND focus = GetFocus();
        return (focus == m_text || focus == m_wnd) ? true : false;    
    }

private:
    BEGIN_MSG_MAP(FindDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)     
    END_MSG_MAP()
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
       createWindowHook(m_hWnd);
       m_text.Attach(GetDlgItem(IDC_EDIT_FINDTEXT));
       m_wnd.Attach(GetDlgItem(IDC_COMBO_FINDWINDOW));
       m_find_end_begin.Attach(GetDlgItem(IDC_RADIO_END_BEGIN));
       m_find_begin_end.Attach(GetDlgItem(IDC_RADIO_BEGIN_END));
       return 0;
    }
    LRESULT OnDestroyDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
       deleteWindowHook(m_hWnd);
       return 0;
    }
};
