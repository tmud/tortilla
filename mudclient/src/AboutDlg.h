#pragma once

class Link : public CWindowImpl < Link, CStatic >
{
public:
    void Attach(HWND static_control)
    {
        CWindow s(static_control);
        RECT pos;
        s.GetWindowRect(&pos);
        CWindow p(s.GetParent());
        p.ScreenToClient(&pos);
        std::wstring tmp;
        getWindowText(static_control, &tmp);
        HFONT f = s.GetFont();
        s.ShowWindow(SW_HIDE);
        Create(p, pos, tmp.c_str(), WS_CHILD | WS_VISIBLE | SS_NOTIFY);
        SetFont(f);
    }

private:
    BEGIN_MSG_MAP(Link)
        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
        MESSAGE_HANDLER(OCM_COMMAND, OnCommand)
        MESSAGE_HANDLER(OCM_CTLCOLORSTATIC, OnCtlColor)
    END_MSG_MAP()

    LRESULT OnSetCursor(UINT, WPARAM, LPARAM, BOOL&)
    {
        static HCURSOR hHand = NULL;
        if (hHand == NULL)
            hHand = LoadCursor(0, MAKEINTRESOURCE(IDC_HAND));
        if (hHand)
            ::SetCursor(hHand);
        return 1;
    };

    LRESULT OnCommand(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        int code = HIWORD(wParam);
        if (code == STN_CLICKED)
        {
            std::wstring tmp;
            getWindowText(m_hWnd, &tmp);
            std::wstring url(L"url.dll,FileProtocolHandler ");
            url.append(tmp);
            ShellExecute(NULL, L"open", L"rundll32.exe", url.c_str(), 0, SW_SHOWNORMAL);
        }
        return 0;
    }

    LRESULT OnCtlColor(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(0, 0, 255));
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
};

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
    Link m_link;
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_link.Attach(GetDlgItem(IDC_APP_SITE));
		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}
};
