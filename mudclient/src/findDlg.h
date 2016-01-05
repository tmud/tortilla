#pragma once

class FindDlg : public CDialogImpl<FindDlg>
{
    CEdit m_text;
    CComboBox m_wnd;
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
private:
    BEGIN_MSG_MAP(FindDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    END_MSG_MAP()
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
       m_text.Attach(GetDlgItem(IDC_EDIT_FINDTEXT));
       m_wnd.Attach(GetDlgItem(IDC_COMBO_FINDWINDOW));
       return 0;
    }
};

class FindView : public CWindowImpl<FindView>
{
    FindDlg m_dlg;
public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    void setFocus() { PostMessage(WM_USER); }
    void setWindowName(int index, const tstring& name) { m_dlg.setWindowName(index, name); }
    void selectWindow(int index) { m_dlg.selectWindow(index); }
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
