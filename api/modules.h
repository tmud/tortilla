#pragma once
// Набор вспомогательных классов, не связанных с api (для независимых от клиента модулей)

class S2W
{
public:
    S2W(const char* string) : buffer(NULL)
    {
        int symbols_count = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
        int buffer_required = symbols_count+1;
        buffer = new wchar_t[buffer_required];
        MultiByteToWideChar(CP_UTF8, 0, string, -1, buffer, buffer_required);
        buffer[symbols_count] = 0;
    }
    ~S2W()  { delete []buffer; }
    operator const wchar_t*() const { return (buffer) ? buffer : L""; }
private:
    wchar_t* buffer;
};

class W2S
{
public:
    W2S(const wchar_t* wide_string) : buffer(NULL)
    {
        int buffer_required = WideCharToMultiByte(CP_UTF8, 0, wide_string, -1, NULL, 0, NULL, NULL);
        char* buffer = new char [buffer_required+1];
        WideCharToMultiByte(CP_UTF8, 0, wide_string, -1, buffer, buffer_required, NULL, NULL);
        buffer[buffer_required] = 0;        
    }
     ~W2S()  { delete []buffer; }
    operator const char*() const { return (buffer) ? buffer : ""; }
private:
    char* buffer;
};
