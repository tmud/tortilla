#pragma once

/*
class WidndowStaticPosition
{
public:
    enum ALIGN { LEFT, RIGHT, CENTER };
    void attach(HWND wnd, ALIGN align)
    {
        w.Attach(wnd); 
        a = align;


        CWindow parent(w.GetParent());
        
        RECT rc;
        parent.GetWindowRect()

        w.GetClientRect(&rc);
        
        
    }
private:
    CWindow w;
    ALIGN a;
};
*/

class SettingsDlg : public CDialogImpl<SettingsDlg>
{
public:
    enum { IDD = IDD_SETTINGS };
    //CEdit m_edit_columns;
    //CUpDownCtrl m_spin_columns;
    //CEdit m_edit_rows;
    //CUpDownCtrl m_spin_rows;
    //CEdit m_edit_bsize;
    
private:
    BEGIN_MSG_MAP(SettingsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDlg)
        //MESSAGE_HANDLER(WM_SIZE, OnSize)
        //COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        //COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        //m_edit_columns.Attach(GetDlgItem(IDC_EDIT_COLUMNS));
//        m_spin_columns.Attach(GetDlgItem(IDC_SPIN_COLUMNS));
        //m_edit_rows.Attach(GetDlgItem(IDC_EDIT_ROWS));
//        m_spin_rows.Attach(GetDlgItem(IDC_SPIN_ROWS));
        //m_edit_bsize.Attach(GetDlgItem(IDC_EDIT_BSIZE));

//        m_spin_columns.SetBuddy(m_edit_columns);
  //      m_spin_rows.SetBuddy(m_edit_rows);

        

        return 0;
    }

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT rc;
        GetClientRect(&rc);

        
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        EndDialog(wID);
        return 0;
    }   
};