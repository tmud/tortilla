#pragma once

#include "resource.h"

void getWindowText(HWND handle, std::wstring *string)
{
    int text_len = ::GetWindowTextLength(handle);
    MemoryBuffer tmp((text_len + 2)*sizeof(WCHAR));
    WCHAR *buffer = (WCHAR*)tmp.getData();
    ::GetWindowText(handle, buffer, text_len + 1);
    string->assign(buffer);
}

bool sendToClipboard(HWND owner, const std::wstring& text)
{
    if (!OpenClipboard(owner))
        return false;

    if (!EmptyClipboard())
    {
        CloseClipboard();
        return false;
    }

    SIZE_T size = (text.length() + 1) * sizeof(WCHAR);
    HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, size);
    if (!hGlob)
    {
        CloseClipboard();
        return false;
    }

    WCHAR* buffer = (WCHAR*)GlobalLock(hGlob);
    wcscpy(buffer, text.c_str());
    GlobalUnlock(hGlob);
    bool result = (SetClipboardData(CF_UNICODETEXT, hGlob) == NULL) ? false : true;
    CloseClipboard();
    if (!result)
        GlobalFree(hGlob);
    return result;
}

LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
class CalcWindowDlg : public CDialogImpl<CalcWindowDlg>
{
    HHOOK m_hHook;
    Pcre  m_pcre;
    CEdit  m_regexp, m_text, m_result;

public:
    CalcWindowDlg() {}
    enum { IDD = IDD_CALCULATOR };
    LRESULT HookGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
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
    }
private:
    BEGIN_MSG_MAP(CalcWindowDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDlg)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroyDlg)
      COMMAND_ID_HANDLER(IDC_BUTTON_COPY, OnCopyToBuffer)
      COMMAND_HANDLER(IDC_EDIT_REGEXP, EN_CHANGE, OnEditChanged)
      COMMAND_HANDLER(IDC_EDIT_TESTSTRING, EN_CHANGE, OnEditChanged)
    END_MSG_MAP()

    LRESULT OnInitDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_hHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc,  NULL, GetCurrentThreadId() );
        m_regexp.Attach(GetDlgItem(IDC_EDIT_REGEXP));
        m_text.Attach(GetDlgItem(IDC_EDIT_TESTSTRING));
        m_result.Attach(GetDlgItem(IDC_EDIT_RESULT));
        update();
        return 0;
    }
    LRESULT OnDestroyDlg(UINT, WPARAM, LPARAM, BOOL&)
    {
        UnhookWindowsHookEx(m_hHook);
        return 0;
    }
    LRESULT OnCopyToBuffer(WORD, WORD, HWND, BOOL&)
    {
        std::wstring regexp;
        getWindowText(m_regexp, &regexp);
        regexp.insert(0, L"$");
        sendToClipboard(m_hWnd, regexp);
        return 0;
    }

    LRESULT OnEditChanged(WORD, WORD, HWND, BOOL&)
    {
        update();
        return 0;
    }

    void update()
    {
        std::wstring regexp, text;
        getWindowText(m_regexp, &regexp);
        getWindowText(m_text, &text);
        if (regexp.empty() || text.empty())
        {
            m_result.SetWindowText(L"");
            return;
        }
        if (!m_pcre.init(regexp.c_str()))
        {
            m_result.SetWindowText(L"Ошибка в регулярном выражении");
            return;
        }

        m_pcre.find(text.c_str());

        int n = m_pcre.size();
        if (n == 0)
        {
            m_result.SetWindowText(L"Нет совпадений");
        }
        else
        {
            std::wstring result;
            for (int i = 0; i < n; ++i)
            {
                std::wstring tmp;
                m_pcre.get(i, &tmp);

                WCHAR buffer[16];
                swprintf(buffer, L"%%%d=[", i);
                result.append(buffer);
                result.append(tmp);
                result.append(L"]\r\n");
            }
            m_result.SetWindowText(result.c_str());
        }
    }
};
