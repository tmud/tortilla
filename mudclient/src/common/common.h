#pragma once

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
void tstring_toupper(tstring *str);
void tstring_tolower(tstring *str);
void tstring_replace(tstring *str, const tstring& what, const tstring& forr);
bool tstring_cmpl(const tstring& str, const WCHAR* lstr);

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
