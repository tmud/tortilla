#include "stdafx.h"
#include "common.h"

bool isVistaOrHigher()
{
    OSVERSIONINFOEX os;
    ZeroMemory(&os, sizeof(OSVERSIONINFOEX));
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&os);
    if ((os.wProductType != VER_NT_WORKSTATION) ||
        (os.dwMajorVersion < 6)) // if less Vista/7/8
        return false;
    return true;
}

void loadString(UINT id, tstring* string)
{
    WCHAR buffer[256];
    LoadString(GetModuleHandle(NULL), id, buffer, 256);
    string->assign(buffer);
}

int msgBox(HWND parent, const tstring& msg, UINT options)
{
    tstring title_text;
    loadString(IDR_MAINFRAME, &title_text);
    return MessageBox(parent, msg.c_str(), title_text.c_str(), options);
}

int msgBox(HWND parent, UINT msg, UINT options)
{
    tstring msg_text;
    loadString(msg, &msg_text);
    return msgBox(parent, msg_text, options);
}

int msgBox(HWND parent, const tstring& msg, const tstring&descr, UINT options)
{
    tstring msg_text(msg);
    if (!descr.empty()) {
        msg_text.append(L"\n\n");
        msg_text.append(descr);
    }
    return msgBox(parent, msg_text, options);
}

int msgBox(HWND parent, UINT msg, const tstring&descr, UINT options)
{
    tstring msg_text;
    loadString(msg, &msg_text);
    return msgBox(parent, msg_text, descr, options);
}

void getWindowText(HWND handle, tstring *string)
{
    int text_len = ::GetWindowTextLength(handle);
    MemoryBuffer tmp((text_len+2)*sizeof(WCHAR)); 
    WCHAR *buffer = (WCHAR*)tmp.getData();
    ::GetWindowText(handle, buffer, text_len+1);
    string->assign(buffer);
}

bool isOnlyDigits(const tstring& str)
{
   if (str.empty()) return false;
   return isOnlySymbols(str, L"0123456789");
}

bool isInt(const tstring& str)
{
   if (str.empty()) return false;
   const tchar* p = str.c_str();
   int len = str.length();
   if (*p == L'-') { p++; len--;}
   return (wcsspn(p, L"0123456789") != len) ? false : true;
}

bool isItNumber(const tstring& str)
{
     if (str.empty()) return false;
     const tchar* p = str.c_str();
     int len = str.length();
     if (*p == L'-') { p++; len--;}
     return (wcsspn(p, L"e0123456789.,") != len) ? false : true;
}

bool isOnlySpaces(const tstring& str)
{
   return isOnlySymbols(str, L" ");
}

bool isOnlySymbols(const tstring& str, const tstring& symbols)
{
    int pos = wcsspn(str.c_str(), symbols.c_str());
    return (pos != str.length()) ? false : true;
}

bool isOnlyFilnameSymbols(const tstring& str)
{
    int pos = wcscspn(str.c_str(), L"?*/\\|:\"<>");
    return (pos != str.length()) ? false : true;
}

bool w2int(const tstring& str, int *value)
{
    if (!isInt(str))
        return false;
    *value = _wtoi(str.c_str());
    return true;
}
void int2w(int value, tstring* str)
{
    wchar_t buffer[16];
    swprintf(buffer, L"%d", value);
    str->assign(buffer);
}

bool w2double(const tstring& str, double *value)
{
    if (!isItNumber(str))
        return false;
    *value = _wtof(str.c_str());
    return false;
}

void double2w(double value, int precision, tstring* str)
{
    wchar_t buffer1[16], buffer2[16];
    swprintf(buffer1, L"%%.%df", precision);
    swprintf(buffer2, buffer1, value);
    str->assign(buffer2);
}

double getMod(double value)
{
    double ord = 0;
    return modf(value, &ord);
}

bool isExistSymbols(const tstring& str, const tstring& symbols)
{
   int pos = wcscspn(str.c_str(), symbols.c_str());
   return (pos != str.length()) ? true : false;
}

void tstring_trimleft(tstring *str)
{
    int pos = wcsspn(str->c_str(), L" ");
    if (pos != 0)
        str->assign(str->substr(pos));
}

void tstring_trimright(tstring *str)
{
    if (str->empty())
        return;
    int last = str->size() - 1;
    int pos = last;
    while (str->at(pos) == L' ')
        pos--;
    if (pos != last)
        str->assign(str->substr(0, pos+1));
}

void tstring_trim(tstring *str)
{
    tstring_trimleft(str);
    tstring_trimright(str);
}

void tstring_trimsymbols(tstring *str, const tstring& symbols)
{
    if (str->empty() || symbols.empty()) return;

    tstring newstr;
    const tchar *b = str->c_str();
    const tchar *e = b + str->length();
    const tchar *p  = b + wcscspn(b, symbols.c_str());
    while (p != e)
    {
        newstr.append(b, p-b);
        b = p + 1;
        p = b + wcscspn(b, symbols.c_str());
    }
    newstr.append(b);
    str->swap(newstr);
}

void tstring_toupper(tstring *str)
{
    std::locale loc("");
    const std::ctype<wchar_t>& ct = std::use_facet<std::ctype<wchar_t> >(loc);
    std::transform(str->begin(), str->end(), str->begin(), std::bind1st(std::mem_fun(&std::ctype<wchar_t>::toupper), &ct));
}

void tstring_tolower(tstring *str)
{
    std::locale loc("");
    const std::ctype<wchar_t>& ct = std::use_facet<std::ctype<wchar_t> >(loc);
    std::transform(str->begin(), str->end(), str->begin(), std::bind1st(std::mem_fun(&std::ctype<wchar_t>::tolower), &ct));
}

void tstring_replace(tstring *str, const tstring& what, const tstring& forr)
{
    size_t pos = 0;
    while((pos = str->find(what, pos)) != tstring::npos)
    {
        str->replace(pos, what.length(), forr);
        pos += forr.length();
    }
}

bool tstring_cmpl(const tstring& str, const WCHAR* lstr)
{
    return (wcsncmp(str.c_str(), lstr, wcslen(lstr)) == 0) ? true : false;
}

int utf8_getbinlen(const utf8* str, int symbol)
{
    int p = 0;
    while (str && *str && symbol > 0)
    {
        const unsigned char &c = str[p];
        if (c < 0x80) { symbol--; p++; }
        else if (c < 0xc0 || c > 0xf7) break;  // error in bytes
        else
        {
            int sym_len = 2;
            if ((c & 0xf0) == 0xe0) sym_len = 3;
            else if ((c & 0xf8) == 0xf0) sym_len = 4;            
            p += sym_len;
            symbol--;
        }
    }
    return (symbol > 0) ? -1 : p;
}

int utf8_strnlen(const utf8* str, int str_len)
{
    assert(str);
    int len = 0;
    int p = 0;
    while (str_len > 0)
    {
        const unsigned char &c = str[p];
        if (c < 0x80) { len++; str_len--; p++; }
        else if (c < 0xc0 || c > 0xf7) break;  // error in bytes
        else
        {
            int sym_len = 2;
            if ((c & 0xf0) == 0xe0) sym_len = 3;
            else if ((c & 0xf8) == 0xf0) sym_len = 4;

            if (sym_len > str_len) break;      // error
            len++;
            str_len -= sym_len;
            p += sym_len;
        }
    }
    return len;
}

int u8string_len(const u8string& str)
{
    return utf8_strnlen(str.c_str(), str.length());
}

void u8string_substr(u8string *str, int from, int len)
{
    if (from < 0 || len <= 0) {
        str->clear(); return;
    }
    int begin = utf8_getbinlen(str->c_str(), from);    
    int afterlen = utf8_getbinlen(str->c_str(), from + len);
    if (begin == -1 || afterlen == -1) {
        str->clear();  return; 
    }
    u8string res(str->substr(begin, afterlen-begin));
    str->swap(res);
}

bool checkKeysState(bool shift, bool ctrl, bool alt)
{
    if ((GetKeyState(VK_SHIFT) < 0) != shift) return false;
    if ((GetKeyState(VK_CONTROL) < 0) != ctrl) return false;
    if ((GetKeyState(VK_MENU) < 0) != alt) return false;
    return true;
}

void MD5::update(const tstring& str)
{
    TW2U s(str.c_str());
    crc.update(s);
}

tstring MD5::getCRC()
{
    std::string crc(crc.digest().hex_str_value());
    TU2W c(crc.c_str());
    return tstring(c);
}

Separator::Separator(const tstring& str)
{
    if (str.empty())
        return;

    const WCHAR *b = str.c_str();
    const WCHAR *e = b + str.length();
    
    bool word = (*b == L' ') ? false : true; 
    const WCHAR *p = b + 1;
    for (;p != e; ++p)
    {
        if (*p == L' ' && word)
        {
            tstring new_part(b, p-b);
            m_parts.push_back(new_part);
            word = false;
        }
        if (*p != L' ' && !word)
        {
            b = p;
            word = true;
        }
    }
    if (word)
    {
        tstring new_part(b, e-b);
        m_parts.push_back(new_part);
    }
}

int Separator::getSize() const
{
    return m_parts.size();
}
    
const tstring& Separator::operator[](int index)
{
    return (index >= 0 && index <getSize()) ? m_parts[index] : empty;
}

COLORREF invertColor(COLORREF c)
{
    return RGB((GetRValue(c) ^ 0xff),(GetGValue(c) ^ 0xff),(GetBValue(c) ^ 0xff));
}

bool sendToClipboard(HWND owner, const tstring& text)
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

bool getFromClipboard(HWND owner, tstring* text)
{
    if (!OpenClipboard(owner))
        return false;

    bool result = false;
    HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
    if (hClipboardData)
    {
        WCHAR *pchData = (WCHAR*)GlobalLock(hClipboardData);
        if (pchData)
        {
            text->assign(pchData);
            GlobalUnlock(hClipboardData);
            result = true;
        }
    }
    CloseClipboard();
    return result;
}

const int maxClassLen = 128;
tchar className[maxClassLen];
std::vector<HWND> clients;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM)
{
   if (GetClassName(hwnd,className,maxClassLen))
   {
       if (!wcscmp(className, MAINWND_CLASS_NAME))
           clients.push_back(hwnd);
   }
   return TRUE;
}

void sendCommandToWindow(HWND owner, const tstring& window, const tstring& cmd)
{
    EnumWindows(EnumWindowsProc, 0);
    int window_size = window.size();
    int cmd_size = cmd.size();

    int buffer_len = (window_size + cmd_size + 2)*sizeof(tchar) + 2*sizeof(int);
    unsigned char* buffer = new unsigned char[buffer_len];
    unsigned char* p = buffer;

    int tocopy = sizeof(int);
    memcpy(p, &window_size,tocopy); p += tocopy;
    tocopy = (window_size+1)*sizeof(tchar);
    memcpy(p, window.c_str(), tocopy); p += tocopy;
    tocopy = sizeof(int);
    memcpy(p, &cmd_size,tocopy); p += tocopy;
    tocopy = (cmd_size+1)*sizeof(tchar);
    memcpy(p, cmd.c_str(),tocopy); p += tocopy;

    COPYDATASTRUCT cd;
    cd.dwData = 0x55aa;
    cd.cbData = buffer_len;
    cd.lpData = buffer;

    for (int i=0,e=clients.size(); i<e;++i)
        SendMessage(clients[i], WM_COPYDATA, (WPARAM)owner, (LPARAM)&cd);
    delete []buffer;
    clients.clear();
}

bool readCommandToWindow(WPARAM wparam, LPARAM lparam, tstring* window, tstring* cmd)
{
    assert(window && cmd);
    COPYDATASTRUCT* cd = (COPYDATASTRUCT*)lparam;
    if (cd->dwData != 0x55aa)
        return false;
    if (cd->cbData == 0)
        return false;
    int len = cd->cbData;
    unsigned char *p = (unsigned char *)cd->lpData;
    int window_size = 0; int cmd_size = 0;

    int tocopy = sizeof(int);
    if (tocopy > len) return false;
    memcpy(&window_size, p, tocopy); p += tocopy; len -= tocopy;
    if (window_size < 0) return false;

    tocopy = (window_size+1)*sizeof(tchar);
    if (tocopy > len) return false;
    window->assign((const tchar*)p); p += tocopy; len -= tocopy;

    tocopy = sizeof(int);
    if (tocopy > len) return false;
    memcpy(&cmd_size, p, tocopy); p += tocopy; len -= tocopy;
    if (cmd_size < 0) return false;

    tocopy = (cmd_size+1)*sizeof(tchar);
    if (tocopy > len) return false;
    cmd->assign((const tchar*)p); p += tocopy; len -= tocopy;

    return (len == 0) ? true : false;
}


typedef std::map<HWND, UINT> THWNDCollection;
HHOOK m_hHook = NULL;
THWNDCollection m_aWindows;

BOOL CALLBACK MyEnumProc(HWND hwnd, LPARAM lParam)
{
    TCHAR buf[16];
    GetClassName(hwnd, buf, sizeof(buf) / sizeof(TCHAR));
    if (_tcsncmp(buf, _T("#32768"), 6) == 0) { // special classname for menus
        *((HWND*)lParam) = hwnd;
        return FALSE;
    }
    return TRUE;
}

// Hook procedure for WH_GETMESSAGE hook type.
// This function is more or less a combination of MSDN KB articles
// Q187988 and Q216503. See MSDN for additional details
LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // If this is a keystrokes message, pass it to IsDialogMessage for tab
    // and accelerator processing
    LPMSG lpMsg = (LPMSG)lParam;

    // If this is a keystrokes message, pass it to IsDialogMessage for tab
    // and accelerator processing
    if ((nCode >= 0) && PM_REMOVE == wParam &&
        (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST))
    {
         // check if there is a menu active
        HWND hMenuWnd = NULL;
        EnumWindows(MyEnumProc, (LPARAM)&hMenuWnd);
        if (hMenuWnd == NULL) {
        HWND hWnd = NULL; HWND hActiveWindow = GetActiveWindow();
        THWNDCollection::iterator it = m_aWindows.begin();
        // check each window we manage to see if the message is meant for them
        while (it != m_aWindows.end())
        {
            hWnd = it->first;
            if (::IsWindow(hWnd) && ::IsDialogMessage(hWnd, lpMsg))
            {
                if (it->second)
                {
                    LRESULT result = SendMessage(hWnd, it->second, 0, lParam);
                    if (result) {  break; //processed
                    }
                }

                // The value returned from this hookproc is ignored, and it cannot
                // be used to tell Windows the message has been handled. To avoid
                // further processing, convert the message to WM_NULL before
                // returning.
                lpMsg->hwnd = NULL;
                lpMsg->message = WM_NULL;
                lpMsg->lParam = 0L;
                lpMsg->wParam = 0;
                break;
            }
            it++;
        }}
    }

    // Passes the hook information to the next hook procedure in
    // the current hook chain.
    return ::CallNextHookEx(m_hHook, nCode, wParam, lParam);
}

void createWindowHook(HWND hWnd, UINT test_msg)
{
    // make sure the hook is installed
    if (m_hHook == NULL)
    {
        m_hHook = ::SetWindowsHookEx(WH_GETMESSAGE, GetMessageProc, NULL, GetCurrentThreadId());
        // is the hook set?
        if (m_hHook == NULL)
            return;
    }
    // add the window to our list of managed windows
    m_aWindows[hWnd] = test_msg;
}

void deleteWindowHook(HWND hWnd)
{
    m_aWindows.erase(hWnd);
    if (m_aWindows.empty() && m_hHook)
    {
        ::UnhookWindowsHookEx(m_hHook);
        m_hHook = NULL;
    }
}

#ifdef _DEBUG
void OutputBytesBuffer(const void *data, int len, int maxlen, const char* label)
{
    if (maxlen > len) maxlen = len;
    std::string l("["); l.append(label);
    char tmp[32]; sprintf(tmp, " len=%d,show=%d]:\r\n", len, maxlen); l.append(tmp);
    OutputDebugStringA(l.c_str());
    const unsigned char *bytes = (const unsigned char *)data;
    len = maxlen;
    const int show_len = 32;
    unsigned char *buffer = new unsigned char[show_len];
    std::string hex;
    hex.reserve(160);
    while (len > 0)
    {
        int toshow = show_len;
        if (toshow > len) toshow = len;
        for (int i = 0; i < toshow; ++i)
        {
            sprintf(tmp, "%.2x ", bytes[i]);
            hex.append(tmp);
        }
        int empty = show_len - toshow;
        if (empty > 0)
        {
            std::string spaces(empty*3, L' ');
            hex.append(spaces);
        }

        memcpy(buffer, bytes, toshow);
        for (int i=0; i<toshow; ++i) {
            if (buffer[i] < 32) buffer[i] = '.';
        }
        hex.append((const char*)buffer, toshow);
        OutputDebugStringA(hex.c_str());
        OutputDebugStringA("\r\n");
        hex.clear();
        bytes += toshow;
        len -= toshow;
    }
    delete []buffer;
}

void OutputTelnetOption(const void *data, const char* label)
{
    OutputDebugStringA(label);
    char tmp[32];
    unsigned char byte = *(const char*)data;
    sprintf(tmp, ": %d (%.2x)\r\n", byte, byte);
    OutputDebugStringA(tmp);
}
#endif

void CReBarSettings::Save(CReBarCtrl& ReBar, tstring *param)
{
    ATLASSERT(ReBar.IsWindow());    
    DWORD cbBandCount = ReBar.GetBandCount();
    Save(cbBandCount, param);
    for (UINT i = 0; i < cbBandCount; i++)
    {
        REBARBANDINFO rbi;
        memset(&rbi, 0, sizeof(rbi));
        rbi.cbSize = sizeof(rbi);
        rbi.fMask = RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE;
        ReBar.GetBandInfo(i, &rbi);
        Save(rbi.wID, param);
        Save(rbi.cx, param);
        DWORD break_line = (rbi.fStyle & RBBS_BREAK) ? 1 : 0;
        Save(break_line, param);
        DWORD visible = (rbi.fStyle & RBBS_HIDDEN) ? 0 : 1;
        Save(visible, param);
    }
}

bool CReBarSettings::IsVisible(CReBarCtrl& ReBar, DWORD wID)
{
     DWORD cbBandCount = ReBar.GetBandCount();
     for (UINT i = 0; i < cbBandCount; i++)
     {
        REBARBANDINFO rbi;
        memset(&rbi, 0, sizeof(rbi));
        rbi.cbSize = sizeof(rbi);
        rbi.fMask = RBBIM_ID | RBBIM_STYLE;
        ReBar.GetBandInfo(i, &rbi);
        if (wID == rbi.wID) {
            return (rbi.fStyle & RBBS_HIDDEN) ? false : true;
        }
     }
     return false;
}


void CReBarSettings::Load(CReBarCtrl& ReBar, const tstring& param)
{
    ATLASSERT(ReBar.IsWindow());
    tstring s(param);
    DWORD cbBandCount = 0;
    if (!Load(&cbBandCount, &s))
        return;
    struct BandInfo
    {
        DWORD ID;
        DWORD cx;
        bool BreakLine;
        bool Visible;
    };
    BandInfo* bands = new BandInfo[cbBandCount];
    for (DWORD i = 0; i < cbBandCount; i++)
    {
        BandInfo &bi = bands[i];
        DWORD brk = 0; DWORD visible = 0;
        if (!Load(&bi.ID, &s) || !Load(&bi.cx, &s) || !Load(&brk, &s) || !Load(&visible, &s))
        {
            delete []bands;
            return;
        }
        bi.BreakLine = (brk == 0) ? false : true;
        bi.Visible = (visible == 0) ? false : true;
    }

    for (DWORD i = 0; i < cbBandCount; i++)
    {
        ReBar.MoveBand(ReBar.IdToIndex(bands[i].ID), i);
        REBARBANDINFO rbi;
        rbi.cbSize = sizeof(rbi);
        rbi.fMask = RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE;
        ReBar.GetBandInfo(i, &rbi);
        rbi.cx = bands[i].cx;
        if (bands[i].BreakLine)
            rbi.fStyle |= RBBS_BREAK;
        else
            rbi.fStyle &= (~RBBS_BREAK);
        if (bands[i].Visible)
            rbi.fStyle &= (~RBBS_HIDDEN);
        else
            rbi.fStyle |= RBBS_HIDDEN;
        ReBar.SetBandInfo(i, &rbi);
    }
    delete []bands;
}

void CReBarSettings::Save(DWORD v, tstring* sv)
{
    wchar_t buffer[16];
    swprintf(buffer, L"%u;", v);
    sv->append(buffer);
}

bool CReBarSettings::Load(DWORD *v, tstring* sv)
{
    size_t pos = sv->find(L';');
    if (pos == tstring::npos)
        return false;
    tstring value(sv->substr(0, pos));
    if (!isOnlyDigits(value))
        return false;
    int n = 0;
    if (!w2int(value, &n))
        return false;
    *v = static_cast<DWORD>(n);
    tstring tmp(sv->substr(pos+1));
    sv->swap(tmp);
    return true;
}
