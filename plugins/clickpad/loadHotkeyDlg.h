#pragma once

class LoadHotkeyDlg : public CDialogImpl<LoadHotkeyDlg>
{
    HWND m_center;
public:
     enum { IDD = IDD_LOADHOTKEY };
     LoadHotkeyDlg(HWND centerWindow) : m_center(centerWindow) {}
private:
    BEGIN_MSG_MAP(LoadHotkeyDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDlg)
        //MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
        //MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        CenterWindow(m_center);
        return 0;
    }

    LRESULT OnKillFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
        if (wID == IDOK)
        {
        
        }
		EndDialog(wID);
		return 0;
	}

};