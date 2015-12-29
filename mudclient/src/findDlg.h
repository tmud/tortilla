#pragma once

class FindDlg : public CDialogImpl<FindDlg>
{
public:
    enum { IDD = IDD_FIND };
private:
    BEGIN_MSG_MAP(FindDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    END_MSG_MAP()
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
       return 0;
    }
};

class FindView : public CWindowImpl<FindView>
{
    FindDlg m_dlg;
public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
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
        return 0;
    }    
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT rc; GetClientRect(&rc);
        m_dlg.MoveWindow(&rc);
        return 0;
    }
};
