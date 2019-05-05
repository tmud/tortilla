#pragma once
#include "resource.h"

class AutoCloseHandle {
    HANDLE hfile;
public:
    AutoCloseHandle(HANDLE file) : hfile(file) {}
    ~AutoCloseHandle() { CloseHandle(hfile); }
};

class ParamsDialog : public CDialogImpl<ParamsDialog>
{
public:
    ParamsDialog() : rewrite_mode(false) {}
    enum { IDD = IDD_IMPORT_PARAMS };
    std::vector<std::wstring> strings;
    std::wstring cmdsymbol;
    std::wstring separator;
    bool rewrite_mode;

private:
    CEdit m_filepath, m_cmdsymbol, m_separator;
    CButton m_ok, m_browse;
    CStatic m_error_msg;
    CButton m_rewrite;
    CFont m_error_font;

    BEGIN_MSG_MAP(ParamsDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_BUTTON_BROWSE, OnBrowse)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_filepath.Attach(GetDlgItem(IDC_EDIT_FILEPATH));
        m_cmdsymbol.Attach(GetDlgItem(IDC_EDIT_CMDSYMBOL));
        m_separator.Attach(GetDlgItem(IDC_EDIT_CMDSEPARATOR));
        m_browse.Attach(GetDlgItem(IDC_BUTTON_BROWSE));
        m_ok.Attach(GetDlgItem(IDOK));
        m_error_msg.Attach(GetDlgItem(IDC_STATIC_ERROR));
        m_rewrite.Attach(GetDlgItem(IDC_CHECK_REWRITE));
        LOGFONT logfont;
        CFontHandle f(m_error_msg.GetFont());
        f.GetLogFont(&logfont);
        logfont.lfWeight = FW_BOLD;
        m_error_font.CreateFontIndirect(&logfont);
        m_error_msg.SetFont(m_error_font);
        m_filepath.SetReadOnly();
        m_cmdsymbol.SetLimitText(1);
        m_separator.SetLimitText(1);
        enableControls(FALSE);
        setDlgFocus(m_browse);
        CenterWindow(GetParent());
        return 0;
    }

    LRESULT OnBrowse(WORD, WORD, HWND, BOOL&)
    {
        SelectFileDlg dlg(m_hWnd, L"JMC3 config set(*.set)|*.set||");
        if (!dlg.DoModal())
            return 0;
        std::wstring file(dlg.GetFile());
        m_filepath.SetWindowText(file.c_str());
        std::wstring error;
        if (!loadFile(file.c_str(), &error))
        {
            enableControls(FALSE);
            m_error_msg.SetWindowText(error.c_str());
            return 0;
        }

        //get cmd symbol
        std::map<std::wstring, int> counter;
        typedef std::map<std::wstring, int>::iterator iterator;
        for (int i=0,e=strings.size(); i<e; ++i)
        {
            std::wstring wstr( strings[i].c_str() );
            std::wstring symbol( wstr.substr(0, 1) );
            int pos = wcsspn(symbol.c_str(), L"#$%&*!@~`:;'№^|\\/_=.,");
            if (pos != symbol.length()) 
                continue;
            iterator it = counter.find(symbol);
            if (it == counter.end())
                counter[symbol] = 1;
            else
                it->second++;
        }
        std::wstring maxsymbol; int maxsize = 0;
        iterator it = counter.begin(), it_end = counter.end();
        for (; it != it_end; ++it) {
            if (it->second > maxsize) { maxsize = it->second; maxsymbol = it->first; }
        }
        if (maxsymbol.empty())
        {
            enableControls(FALSE);
            m_error_msg.SetWindowText(L"Файл поврежден или не является файлом Jmc3!");
            return 0;
        }
        enableControls(TRUE);
        m_cmdsymbol.SetWindowText(maxsymbol.c_str());
        m_separator.SetWindowText(L";");
        setDlgFocus(m_ok);
        return 0;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        wchar_t buffer[4];
        m_cmdsymbol.GetWindowText(buffer, 2);
        cmdsymbol.assign(buffer);
        m_separator.GetWindowText(buffer, 2);
        separator.assign(buffer);
        rewrite_mode = (m_rewrite.GetCheck() == BST_CHECKED) ? true : false;
        EndDialog(IDOK);
        return 0;
    }

    LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
    {
        strings.clear();
        EndDialog(IDCANCEL);
        return 0;
    }

    void enableControls(BOOL enable)
    {
        if (!enable)
        {
            m_cmdsymbol.SetWindowText(L"");
            m_separator.SetWindowText(L"");
        }
        else
        {
            m_error_msg.SetWindowText(L"");
        }
        m_cmdsymbol.EnableWindow(enable);
        m_separator.EnableWindow(enable);
        m_ok.EnableWindow(enable);
    }

    void setDlgFocus(HWND control)
    {
        // in dialogs standard SetFocus does't working 
        SendMessage(WM_NEXTDLGCTL, (WPARAM)control, TRUE);
    }

private:
    bool loadFile(const wchar_t* file, std::wstring *error)
    {
        strings.clear();
        HANDLE hfile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
        {
            error->assign(L"Невозможно прочитать данный файл!");
            return false;
        }

        AutoCloseHandle _ach(hfile);

        DWORD high = 0;
        DWORD size = GetFileSize(hfile, &high);
        if (high != 0 || size > (512 * 1024))
        {
            error->assign(L"Файл слишком большого размера!");
            return false; 
        }
        if (size == 0)
        {
            error->assign(L"Файл пустой!");
            return false;
        }
        std::vector <std::string> config;
        DataQueue dq;
        const int buffer_size = 1024;
        MemoryBuffer buffer(buffer_size);
        while (size > 0)
        {
            DWORD toread = buffer_size;
            if (toread > size) toread = size;
            DWORD readed = 0;
            if (!ReadFile(hfile, buffer.getData(), toread, &readed, NULL) || readed != toread)
            {
                error->assign(L"Ошибка чтения файла!");
                return false;
            }
            dq.write(buffer.getData(), toread);
            parseQueue(dq, config);
            size -= toread;
        }
        char x = 0xa; dq.write(&x, 1);
        parseQueue(dq, config);
        for (int i = 0, e = config.size(); i < e; ++i)
        {
            std::string &s = config[i];
            int pos = strspn(s.c_str(), " "); //trim left
            if (pos != 0)
                s.assign(s.substr(pos));
            if (s.empty())
                continue;
            TA2W wide(s.c_str());
            std::wstring w(wide);
            strings.push_back(w);
        }
        return true;
    }

    void parseQueue(DataQueue &dq, std::vector<std::string>& out)
    {
        const char* b = (const char*)dq.getData();
        const char* e = b + dq.getSize();
        const char* p = b;
        while (p != e)
        {
            for (; p != e; ++p)
            {
                if (*p == 0xd || *p == 0xa)
                    break;
            }

            if (p != e)
            {
                std::string label(b, p - b);
                if (!label.empty())
                    out.push_back(label);
                p++;
                b = p;
            }
        }
        const char* b0 = (const char*)dq.getData();
        dq.truncate(b - b0);
     }
};
