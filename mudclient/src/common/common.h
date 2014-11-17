#pragma once

bool isVistaOrHigher();
void loadString(UINT id, tstring* string);
int msgBox(HWND parent, UINT msg, UINT options);
void getWindowText(HWND handle, tstring *string);
bool a2int(const std::string& str, int *value);
bool isExistSymbols(const tstring& str, const tstring& symbols);
bool isOnlyDigits(const tstring& str);
bool isOnlySpaces(const tstring& str);
COLORREF invertColor(COLORREF c);
bool sendToClipboard(HWND owner, const tstring& text);
bool getFromClipboard(HWND owner, tstring* text);

void tstring_trimleft(tstring *str);
void tstring_trimright(tstring *str);
void tstring_trim(tstring *str);
void tstring_trimsymbols(tstring *str, const tstring& symbols);
void tstring_toupper(tstring *str);
void tstring_tolower(tstring *str);
void tstring_replace(tstring *str, const tstring& what, const tstring& forr);
bool tstring_cmpl(const tstring& str, const WCHAR* lstr);

int  u8string_len(const u8string& str);
void u8string_substr(u8string *str, int from, int len);

class Separator
{
public:
    Separator(const tstring& str);
    int getSize() const;
    const tstring& operator[](int index);
private:
    std::vector<tstring> m_parts;
    tstring empty;
};

template<class T>
struct autodel
{   
   autodel(std::vector<T*>& v)
   {
       struct{ void operator() (T* cmd) { delete cmd; }} del;
       std::for_each(v.begin(), v.end(), del);
       v.clear();
   }
};

class MaskSymbolsBySlash
{
public:
    MaskSymbolsBySlash(const tstring& src, const tstring& symbols)
    {
        tchar x[3] = { L'\\', 0, 0 };
        const tchar *b = src.c_str();
        const tchar *e = b + src.length();
        const tchar* p = b + wcscspn(b, symbols.c_str());
        while (p != e)
        {
            result.append(tstring(b, p - b));
            x[1] = *p;
            result.append(x);
            b = p + 1;
            p = b + wcscspn(b, symbols.c_str());
        }
        result.append(b);
    }
    operator const tstring&() const
    {
        return result;
    }
private:
    tstring result;
};

class Ticker
{
    DWORD m_ticker;
public:
    Ticker() { sync(); }
    void sync() { m_ticker = GetTickCount(); }
    DWORD getDiff() const
    {
        DWORD diff = -1;
        DWORD tick = GetTickCount();
        if (tick >= m_ticker)
            diff = tick - m_ticker;
        else
        {   // overflow 49.7 days (MSDN GetTickCount)
            diff = diff - m_ticker;
            diff = diff + tick + 1;
        }
        return diff;
    }
};
