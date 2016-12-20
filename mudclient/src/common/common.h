#pragma once 

bool isVistaOrHigher();
void loadString(UINT id, tstring* string);
int msgBox(HWND parent, const tstring& msg, UINT options);
int msgBox(HWND parent, UINT msg, UINT options);
void getWindowText(HWND handle, tstring *string);
bool w2int(const tstring& str, int *value);
void int2w(int value, tstring* str);
bool w2double(const tstring& str, double *value);
void double2w(double value, int precision, tstring* str);
double getMod(double value);
bool isExistSymbols(const tstring& str, const tstring& symbols);
bool isOnlyDigits(const tstring& str);
bool isInt(const tstring& str);
bool isItNumber(const tstring& str);
bool isOnlySpaces(const tstring& str);
bool isOnlySymbols(const tstring& str, const tstring& symbols);
bool isOnlyFilnameSymbols(const tstring& str);
COLORREF invertColor(COLORREF c);
bool sendToClipboard(HWND owner, const tstring& text);
bool getFromClipboard(HWND owner, tstring* text);
void sendCommandToWindow(HWND owner, const tstring& window, const tstring& cmd);
bool readCommandToWindow(WPARAM wparam, LPARAM lparam, tstring* window, tstring* cmd);

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

bool checkKeysState(bool shift, bool ctrl, bool alt);

void createWindowHook(HWND wnd, UINT test_msg);
void deleteWindowHook(HWND wnd);

#include "md5.h"
class MD5
{
public:
    void update(const tstring& str);
    tstring getCRC();
private:
    md5 crc;
};

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
       std::for_each(v.begin(), v.end(), [](T *obj){ delete obj; });
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

#ifdef _DEBUG
#define OUTPUT_BYTES(data, len, maxlen, label) OutputBytesBuffer(data, len, maxlen, label);
#define OUTPUT_OPTION(data, label) OutputTelnetOption(data, label);
void OutputBytesBuffer(const void *data, int len, int maxlen, const char* label);
void OutputTelnetOption(const void *data, const char* label);
#else
#define OUTPUT_BYTES(data, len, maxlen, label)
#define OUTPUT_OPTION(data, label)
#endif

class CReBarSettings
{
public:
    void Save(CReBarCtrl& ReBar, tstring *param);
    void Load(CReBarCtrl& ReBar, const tstring& param);
private:
    void Save(DWORD v, tstring* sv);
    bool Load(DWORD *v, tstring* sv);
};
