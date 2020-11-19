#pragma once

const int kAnsiCodePage = 1251;

class WideToAnsiConverter
{
public:
    int convert(MemoryBuffer *output, const wchar_t *wide, int wide_len)
    {
        int symbols_count = WideCharToMultiByte(kAnsiCodePage, 0, wide, wide_len, NULL, 0, "-", NULL);
        output->alloc(symbols_count + 1);
        char* buffer = (char*)output->getData();
        WideCharToMultiByte(kAnsiCodePage, 0, wide, wide_len, buffer, symbols_count, "-", NULL);
        buffer[symbols_count] = 0;
        return symbols_count;
    }
};

class WideToAnsi
{
public:
    WideToAnsi() {}
    WideToAnsi(const wchar_t *wide, int len = -1)
    {
        int wide_len = (len == -1) ? wcslen(wide) : len;
        convert(wide, wide_len);
    }

    int convert(const wchar_t *wide, int wide_len)
    {
        WideToAnsiConverter con;
        return con.convert(&m_convertBuffer, wide, wide_len);    
    }

    operator const char* () 
    {
        return m_convertBuffer.getData();
    }

private:
    MemoryBuffer m_convertBuffer;
};

class AnsiToWideConverter
{
public:
    int convert(MemoryBuffer *output, const char *ansi, int ansi_len = -1)
    {
        int symbols_count = MultiByteToWideChar(kAnsiCodePage, 0, ansi, ansi_len, NULL, 0);
        int buffer_required = (symbols_count + 1) * sizeof(wchar_t);
        output->alloc(buffer_required);
        wchar_t* buffer = (wchar_t*)output->getData();
        MultiByteToWideChar(kAnsiCodePage, 0, ansi, ansi_len, buffer, buffer_required);
        buffer[symbols_count] = 0;
        return symbols_count;
    }
};

class AnsiToWide
{
public:
    AnsiToWide() {}
    AnsiToWide(const char *ansi, int len = -1)
    {
        int ansi_len = (len == -1) ? strlen(ansi) : len;
        convert(ansi, ansi_len);       
    }

    int convert(const char *ansi, int ansi_len)
    {
        AnsiToWideConverter con;
        return con.convert(&m_convertBuffer, ansi, ansi_len);        
    }

    operator const wchar_t* () 
    {
        return (wchar_t*)m_convertBuffer.getData();
    }

private:
    MemoryBuffer m_convertBuffer;
};
