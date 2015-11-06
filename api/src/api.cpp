#include "stdafx.h"
#include "memoryBuffer.h"
#include "wideToAnsi.h"
#include "wideToUtf8.h"
#include "fimage.h"

#ifdef _DEBUG
#pragma comment(lib, "libxml2d.lib")
#else
#pragma comment(lib, "libxml2.lib")
#endif

#include "xml.h"
#include "../api.h"
#include <vector>

BOOL APIENTRY DllMain(HMODULE, DWORD reason_for_call, LPVOID)
{
    if (reason_for_call == DLL_PROCESS_ATTACH)
    {
        xmlInit();
        fimage_init();
    }
    else if (reason_for_call == DLL_PROCESS_DETACH)
    {
        fimage_release();
        xmlDestroy();
    }
    return TRUE;
}

class Tokenizer : public std::vector<std::wstring>
{
public:
    Tokenizer(const wchar_t* string)
    {
        if (!string) return;
        const wchar_t *p = string;
        const wchar_t *e = p + wcslen(string);
        while (p < e)
        {
            size_t len = wcscspn(p, L"/");
            std::wstring t(p, len);
            push_back(t);
            p = p + len + 1;
        }
    }
};

class PullOfBuffers {
public:
    PullOfBuffers()
    {
    }
    ~PullOfBuffers() 
    {
        for (int i = 0, e = m_free.size(); i < e; ++i) delete m_free[i];
    }
    MemoryBuffer* get() 
    {
        if (!m_free.empty())
        {
            int last = m_free.size()-1;
            MemoryBuffer *b = m_free[last];
            m_free.pop_back();
            return b;
        }
        return new MemoryBuffer(64);
    }
    MemoryBuffer* get(int bytes)
    {
        MemoryBuffer *b = get();
        b->alloc(bytes);
        return b;
    }
    void free(MemoryBuffer* b)
    {
        m_free.push_back(b);
    }
private:
   std::vector<MemoryBuffer*> m_free;  
} _pull;

strbuf convert_utf8_to_wide(const utf8* string)
{
    MemoryBuffer *buffer = _pull.get();
    Utf8ToWideConverter u2w;
    u2w.convert(buffer, string, -1);
    return buffer;
}

strbuf convert_wide_to_utf8(const wchar_t* string)
{
    MemoryBuffer *buffer = _pull.get();
    WideToUtf8Converter w2u;    
    w2u.convert(buffer, string, -1);
    return buffer;
}

strbuf convert_ansi_to_wide(const char* string)
{
    MemoryBuffer *buffer = _pull.get();
    AnsiToWideConverter a2w;
    a2w.convert(buffer, string, -1);
    return buffer;
}

strbuf convert_wide_to_ansi(const wchar_t* string)
{
    MemoryBuffer *buffer = _pull.get();
    WideToAnsiConverter w2a;
    w2a.convert(buffer, string, -1);
    return buffer;
}

void* strbuf_ptr(strbuf b)
{
    if (!b) return NULL;
    MemoryBuffer *buffer = (MemoryBuffer*)b;
    return buffer->getData();
}

void strbuf_destroy(strbuf b)
{
    if (!b) return;
    MemoryBuffer *buffer = (MemoryBuffer*)b;
    _pull.free(buffer);
}

strbuf strbuf_new(int bytes)
{
    if (bytes < 0) return NULL;
    MemoryBuffer *buffer = _pull.get(bytes);
    memset(buffer->getData(), 0, bytes);
    return buffer;
}

xnode xml_load(const wchar_t* filename)
{
    WideToAnsi w2a(filename);
    return xmlLoad(w2a);
}

int xml_save(xnode node, const wchar_t* filename)
{
    WideToAnsi w2a(filename);
    return xmlSave(node, w2a);
}

xnode xml_open(const wchar_t* name)
{
    return xmlCreateRootNode(TW2U(name));
}

void xml_delete(xnode node)
{
    xmlDeleteNode(node);
}
//------------------------------------------------------------------------------------
class EscSymbolsCoder
{
public:
    EscSymbolsCoder(const wchar_t* wide_string)
    {
        if (!wide_string) return;
        TW2U buffer(wide_string);
        const utf8* string = buffer;
        m_string.assign(string);
        utf8 supported_esc[2] = { 27, 0 };
        int pos = strcspn(string, supported_esc);
        if (pos != strlen(string))
        {
            utf8 buffer[8];
            const utf8 *b = string;
            while (true)
            {
                size_t pos = strcspn(b, supported_esc);
                m_escaped.append(b, pos);
                if (!b[pos]) break;
                sprintf(buffer, "&#%d;", b[pos]);
                m_escaped.append(buffer);
                b = b + pos + 1;
            }
        }
    }
    operator const utf8*() { return m_escaped.empty() ? m_string.c_str() : m_escaped.c_str(); }

private:
    u8string m_string;
    u8string m_escaped;
};

class EscSymbolsDecoder
{
public:
    EscSymbolsDecoder(xstring string)
    {
        assert(string);
        const utf8 *b = strstr(string, "&#");
        if (!b) { m_result.assign(string); return; }
        u8string decoded;
        while (true)
        {
            decoded.append(string, b - string);
            const utf8* e = strchr(b + 2, ';');
            if (!e) { decoded.append(b, 2); b = b + 2; }
            else
            {
                u8string symbol(b + 2, e - b - 2);
                int size = symbol.size();
                if (size == 0 || size > 3 || strspn(symbol.c_str(), "0123456789") != size)
                { // incorrect bytecode
                    decoded.append(b, 2); b = b + 2;
                }
                else
                {
                    utf8 tmp[2] = { atoi(symbol.c_str()), 0 };
                    decoded.append(tmp); b = e + 1;
                }
            }
            string = b;
            b = strstr(string, "&#");
            if (!b) { decoded.append(string); break; }
        }
        m_result.assign(decoded);
    }
    operator const utf8*() 
    {
        return m_result.c_str();
    }

private:
    u8string m_result;
};

//------------------------------------------------------------------------------------
void xml_set(xnode node, const wchar_t* name, const wchar_t* value)
{
    Tokenizer tk(name);
    for (int i = 0, e = tk.size() - 1; i < e; ++i)
    {
        xml::request r(node, tk[i].c_str());
        if (r.size() > 1)
            return;
        if (r.size() == 1)
            node = r[0];
        else
            node = xmlCreateNode(node, TW2U(tk[i].c_str()));
    }
    int last = tk.size() - 1;
    EscSymbolsCoder esc(value);
    xmlSetAttribute(node, TW2U(tk[last].c_str()), esc);
}

xstringw xml_get_name(xnode node)
{
    xstring result = xmlGetName(node);
    return convert_utf8_to_wide(result ? result : "");
}

xstringw xml_get_attr(xnode node, const wchar_t* name)
{
    Tokenizer tk(name);
    for (int i = 0, e = tk.size() - 1; i < e; ++i)
    {
        xml::request r(node, tk[i].c_str());
        if (r.size() != 1)
            return NULL;
        node = r[0];
    }
    int last = tk.size() - 1;
    xstring s = xmlGetAttribute(node, TW2U(tk[last].c_str()));
    return (s) ? convert_utf8_to_wide(EscSymbolsDecoder(s)) : NULL;
}

void xml_set_text(xnode node, const wchar_t* text)
{
    EscSymbolsCoder esc(text);
    xmlSetText(node, esc);
}

xstringw xml_get_text(xnode node)
{
    xstring result = xmlGetText(node);
    if (!result)
        return "";
    return convert_utf8_to_wide(EscSymbolsDecoder(result));
}

xnode xml_create_child(xnode node, const wchar_t* childname)
{    
    return xmlCreateNode(node, TW2U(childname));
}

xlist xml_request(xnode node, const wchar_t* request)
{
    return xmlRequest(node, TW2U(request));
}

int xml_get_attr_count(xnode node)
{
    return xmlGetAttributesCount(node);
}

xstringw xml_get_attr_name(xnode node, int index)
{
    return convert_utf8_to_wide(xmlGetAttributeName(node, index));
}

xstringw xml_get_attr_value(xnode node, int index)
{
    return convert_utf8_to_wide(xmlGetAttributeValue(node, index));
}

xnode xml_move(xnode node, const wchar_t* path, int create)
{
    if (!node || !path)
        return NULL;
    std::wstring p(path);
    if (p.empty() || p.at(0) == L' ')
        return NULL;
    if (p.at(0) == L'/')
    {
        p = p.substr(1);
        while (true)
        {
            xml::node parent = xmlGetParent(node);
            if (!parent) break;
            node = parent;
        }
        if (p.empty())
            return node;
    }

    xml::request r(node, p.c_str());
    if (!create)
    {
        if (r.size() != 0)
            return r[0];
        return NULL;
    }

    Tokenizer tk(p.c_str());
    for (int i = 0, e = tk.size()-1; i <= e; ++i)
    {
        xml::request r(node, tk[i].c_str());
        if (r.size() != 0 && i != e)
           node = r[0];
        else
           node = xmlCreateNode(node, TW2U(tk[i].c_str()));
    }
    return node;
}

void xml_list_delete(xlist list)
{
    xmlFreeList(list);
}

int xml_list_size(xlist list)
{
    return xmlGetListSize(list);
}

xnode xml_list_getnode(xlist list, int index)
{
    return xmlGetListNode(list, index);
}

int utf8_symlen(const utf8* symbol)
{
    int len = 0;
    if (symbol && *symbol)
    {
        unsigned char c = *symbol;
        if (c < 0x80) len = 1;
        else if (c >= 0xc0)
        {
            if ((c & 0xe0) == 0xc0) len = 2;
            else if ((c & 0xf0) == 0xe0) len = 3;
            else if ((c & 0xf8) == 0xf0) len = 4;
        }
    }
    return len;
}

int utf8_strnlen(const utf8* str, int str_len)
{
    if (!str) return 0;
    int len = 0;
    int p = 0;
    while (str_len > 0)
    {
        const unsigned char &c = str[p];
        if (c < 0x80) { len++; str_len--; p++; }
        else if (c < 0xc0 || c > 0xf7) return -1;  // ошибка в строке - выходим
        else
        {
            int sym_len = 2;
            if ((c & 0xf0) == 0xe0) sym_len = 3;
            else if ((c & 0xf8) == 0xf0) sym_len = 4;
            if (sym_len > str_len) return -1;      // ошибка - выходим
            len++;
            str_len -= sym_len;
            p += sym_len;
        }
    }
    return len;
}

int utf8_strlen(const utf8* string)
{
    return utf8_strnlen(string, ::strlen(string));
}

int utf8_sympos(const utf8* string, int index)
{
    if (!string || !*string || index < 0)
        return -1;
    int pos = 0;
    while (index > 0)
    {
        int len = utf8_symlen(string);
        if (len == 0)
            return -1;
        string += len;
        pos += len;
        index--;
    }
    return pos;
}

strbuf utf8_trim(const utf8* string)
{
    u8string str(string);
    int pos = strspn(str.c_str(), " ");
    if (pos != 0)
        str.assign(str.substr(pos));
    if (!str.empty())
    {
        int last = str.size() - 1;
        pos = last;
        while (str.at(pos) == ' ')
            pos--;
        if (pos != last)
            str.assign(str.substr(0, pos + 1));
    }
    int len = str.length()+1;
    MemoryBuffer *buffer = _pull.get(len);
    memcpy(buffer->getData(), str.c_str(), len);
    return buffer;
}
