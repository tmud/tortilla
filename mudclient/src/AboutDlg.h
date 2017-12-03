#pragma once

class RichEditDll {
public:
    RichEditDll() : hInstRich(NULL) {}
    ~RichEditDll() { if (hInstRich) FreeLibrary(hInstRich); }
    bool load()
    {
        if (!hInstRich)
            hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
        return (hInstRich) ? true : false;
    }
private:
    HINSTANCE hInstRich;
};
extern RichEditDll _rich_edit;

class RichEdit : public CWindowImpl<RichEdit, CRichEditCtrl>
{
    typedef CWindowImpl<RichEdit, CRichEditCtrl> ParentClass;
    struct RtfStream
    {
        LPCSTR pstr;
        DWORD pos;
    };

public:
    RichEdit() {}
    ~RichEdit() {}
    HWND Create(HWND parent, const RECT& pos, DWORD dwStyle, DWORD dwExStyle)
    {
        if (!_rich_edit.load())
            return NULL;
        RECT tmp = pos;
        return ParentClass::Create(parent, tmp, NULL, dwStyle, dwExStyle);
    }

    bool LoadRTF(void *data, int len)
    {
        // Stream RTF into control
        RtfStream st = { (LPCSTR)data, 0 };
        EDITSTREAM es = { 0 };
        es.dwCookie = (DWORD)&st;
        es.dwError = 0;
        es.pfnCallback = _StreamReadCallback;
        StreamIn(SF_RTF, es);
        return true;
    }

private:
    static DWORD CALLBACK _StreamReadCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb)
    {
        RtfStream *pS = reinterpret_cast<RtfStream *>(dwCookie);
        ATLASSERT(pS);
        LPCSTR pstr = pS->pstr + pS->pos;
        ATLASSERT(!::IsBadStringPtrA(pstr, -1));
        LONG len = ::lstrlenA(pstr);
        if (cb > len) cb = len;
        ::CopyMemory(pbBuff, pstr, cb);
        pS->pos += cb;
        *pcb = cb;
        return 0;
    }

    LRESULT OnCreate(UINT umsg, WPARAM wparam, LPARAM lparam, BOOL&)
    {
        LRESULT lRet = DefWindowProc(umsg, wparam, lparam);
        ShowScrollBar(SB_VERT);
        SetEventMask(GetEventMask() | ENM_LINK);
        SetAutoURLDetect(TRUE);
        return lRet;
    }

    LRESULT OnLink(int, LPNMHDR pnmh, BOOL&)
    {
        ENLINK *link = (ENLINK *)pnmh;
        if (link->msg == WM_LBUTTONDOWN)
        {
            int from = link->chrg.cpMin;
            int to = link->chrg.cpMax;
            MemoryBuffer textbuffer((to - from + 1) * sizeof(WCHAR));
            WCHAR *text = (WCHAR*)textbuffer.getData();
            GetTextRange(from, to, text);
            std::wstring url(L"url.dll,FileProtocolHandler ");
            url.append(text);
            ShellExecute(NULL, L"open", L"rundll32.exe", url.c_str(), 0, SW_SHOWNORMAL);
        }
        return 0;
    }

    BEGIN_MSG_MAP(RichEdit)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      REFLECTED_NOTIFY_CODE_HANDLER(EN_LINK, OnLink)
    END_MSG_MAP()
};

/*class CWelcomeDlg : public CDialogImpl<CWelcomeDlg>
{
    RichEdit m_rich;
public:
    enum { IDD = IDD_WELCOME };
private:
    BEGIN_MSG_MAP(CWelcomeDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_USER, OnFocus)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        CenterWindow(GetParent());
        RECT pos;
        ::GetWindowRect(GetDlgItem(IDC_STATIC_RICHBOXPLACE), &pos);
        ScreenToClient(&pos);             
        m_rich.Create(m_hWnd, pos, WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, WS_EX_CLIENTEDGE);

        HRSRC res = FindResource(NULL, MAKEINTRESOURCE(IDR_BINARY_WELCOME), L"BINARY");
        int size = SizeofResource(NULL, res);
        HGLOBAL resourceData = LoadResource(NULL, res);
        void* binaryData = LockResource(resourceData);
        MemoryBuffer rtfdata(size);
        memcpy(rtfdata.getData(), binaryData, size);
        UnlockResource(resourceData);
        m_rich.LoadRTF(rtfdata.getData(), rtfdata.getSize());
        PostMessage(WM_USER, 0, 0);
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        EndDialog(wID);
        return 0;
    }

    LRESULT OnFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_rich.SetFocus();
        return 0;
    }
};
*/
