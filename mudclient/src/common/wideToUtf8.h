#pragma once

class WideToUtf8Converter
{
public:
    int convert(MemoryBuffer *output, const wchar_t *wide, int wide_len)
    {
        int buffer_required = WideCharToMultiByte(CP_UTF8, 0, wide, wide_len, NULL, 0, NULL, NULL);
        output->alloc(buffer_required + 1);
        char* buffer = (char*)output->getData();
        WideCharToMultiByte(CP_UTF8, 0, wide, wide_len, buffer, buffer_required, NULL, NULL);
        buffer[buffer_required] = 0;
        return buffer_required;
    }
};

class WideToUtf8
{
public:
    WideToUtf8() {}
    WideToUtf8(const wchar_t *wide, int len = -1)
    {
        int wide_len = (len == -1) ? wcslen(wide) : len;
        convert(wide, wide_len);
    }
    int convert(const wchar_t *wide, int wide_len)
    {
        WideToUtf8Converter con;
        return con.convert(&m_convertBuffer, wide, wide_len);
    }
    operator const char* () 
    {
        return m_convertBuffer.getData();
    }
    int len() const
    {
        return m_convertBuffer.getSize();
    }

private:
    MemoryBuffer m_convertBuffer;
};

class Utf8ToWideConverter
{
public:
    int convert(MemoryBuffer *output, const char *utf8, int utf8_len)
    {        
        if (utf8_len > 0)
        {
            int len = u8string_testbin(utf8, utf8_len);
            if (len != utf8_len)
            {
                int x = 1; //todo
            }
        }

        int symbols_count = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_len, NULL, 0);
        int buffer_required = (symbols_count + 1) * sizeof(wchar_t);
        output->alloc(buffer_required);
        wchar_t* buffer = (wchar_t*)output->getData();
        MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_len, buffer, buffer_required);
        buffer[symbols_count] = 0;
        return symbols_count;
    }
private:
    typedef char utf8;
    int u8string_testbin(const utf8* str, int len)
    {
        int strlen = 0;
        const utf8* p = str;
        const utf8* e = p + len;
        while (p != e)
        {
            int sym_len = 0;
            const unsigned char c = *p;
            if (c < 0x80) sym_len = 1;
            else if (c < 0xc0 || c > 0xf7) {} // incorrect bytes
            else
            {
                sym_len = 2;
                if ((c & 0xf0) == 0xe0) sym_len = 3;
                else if ((c & 0xf8) == 0xf0) sym_len = 4;
            }
            if (!sym_len)
                return -1;
            if (p+sym_len > e)
            {
                return strlen;
            }
            strlen = strlen + sym_len;
            p = p + sym_len;
        }
        return strlen;
    }
};

class Utf8ToWide
{
public:
    Utf8ToWide() {}
    Utf8ToWide(const char *utf8, int len = -1)
    {
        convert(utf8, -1);
    }
    int convert(const char *utf8, int utf8_len = -1)
    {
        Utf8ToWideConverter con;
        return con.convert(&m_convertBuffer, utf8, utf8_len);
    }
    operator const wchar_t* () const
    {
        return (wchar_t*)m_convertBuffer.getData();
    }
    int len() const
    {
        return m_convertBuffer.getSize();
    }

private:
    MemoryBuffer m_convertBuffer;
};
