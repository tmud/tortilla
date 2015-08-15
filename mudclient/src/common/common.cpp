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

int msgBox(HWND parent, UINT msg, UINT options)
{
    tstring msg_text, title_text;
    loadString(msg, &msg_text);
    loadString(IDR_MAINFRAME, &title_text);
    return MessageBox(parent, msg_text.c_str(), title_text.c_str(), options);
}

void getWindowText(HWND handle, tstring *string)
{
    int text_len = ::GetWindowTextLength(handle);
    MemoryBuffer tmp((text_len+2)*sizeof(WCHAR)); 
    WCHAR *buffer = (WCHAR*)tmp.getData();
    ::GetWindowText(handle, buffer, text_len+1);
    string->assign(buffer);
}

bool isOnlyDigitsA(const std::string& str)
{
    if (str.empty()) return false;
    const char* p = str.c_str();
    int len = str.length();
    if (*p == '-') { p++; len--; }
    return (strspn(p, "0123456789") != len) ? false : true;
}

bool isOnlyDigits(const tstring& str)
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

bool a2int(const std::string& str, int *value)
{
    if (!isOnlyDigitsA(str))
        return false;
    *value = atoi(str.c_str());
    return true;
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
    std::transform(str->begin(), str->end(), str->begin(), ::toupper);
}

void tstring_tolower(tstring *str)
{
    std::transform(str->begin(), str->end(), str->begin(), ::tolower);
}

void tstring_replace(tstring *str, const tstring& what, const tstring& forr)
{
    size_t pos = 0;
    while((pos = str->find(what, pos)) != std::string::npos)
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
    while (str && symbol > 0)
    {
        const unsigned char &c = str[p];
        if (c < 0x80) { symbol--; p++; }
        else if (c < 0xc0 || c > 0xf7) break;  // error in bytes
        else
        {
            int sym_len = 2;
            if ((c & 0xf0) == 0xe0) sym_len = 3;
            else if ((c & 0xf8) == 0xf0) sym_len = 4;
            else if (c >= 0xf8) break;         // error
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
    from = utf8_getbinlen(str->c_str(), from);
    len = utf8_getbinlen(str->c_str(), from + len);
    u8string res(str->substr(from, len));
    str->swap(res);
}

bool checkKeysState(bool shift, bool ctrl, bool alt)
{
    if ((GetKeyState(VK_SHIFT) < 0) != shift) return false;
    if ((GetKeyState(VK_CONTROL) < 0) != ctrl) return false;
    if ((GetKeyState(VK_MENU) < 0) != alt) return false;
    return true;
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
