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

private:
    MemoryBuffer m_convertBuffer;
};

class Utf8ToWideConverter
{
public:
    int convert(MemoryBuffer *output, const char *utf8, int utf8_len)
    {
        int symbols_count = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_len, NULL, 0);
        int buffer_required = (symbols_count + 1) * sizeof(wchar_t);
        output->alloc(buffer_required);
        wchar_t* buffer = (wchar_t*)output->getData();
        MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_len, buffer, buffer_required);
        buffer[symbols_count] = 0;
        return symbols_count;
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

private:
    MemoryBuffer m_convertBuffer;
};

class U2W
{
public:
    U2W(const char* param)
    {
        Utf8ToWide u2w(param);
        m_param.assign(u2w);
    }
    U2W(const std::string& param)
    {
        Utf8ToWide u2w(param.c_str());
        m_param.assign(u2w);        
    }
    operator const tstring&()
    {
        return m_param; 
    }
    
private:
    tstring m_param;
};

class W2U
{
public:
    W2U(const tstring& param)
    {
        WideToUtf8 w2u(param.c_str());
        m_param.assign(w2u);
    }
    operator const char*() 
    {
        return m_param.c_str(); 
    }
    operator std::string&()
    {
        return m_param;
    }
private:
    std::string m_param;
};
