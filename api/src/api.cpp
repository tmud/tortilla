#include "stdafx.h"
#include "memoryBuffer.h"
#include "wideToAnsi.h"
#include "wideToUtf8.h"

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
        xmlInit();
    else if (reason_for_call == DLL_PROCESS_DETACH)
        xmlDestroy();
    return TRUE;
}

class Tokenizer : public std::vector<u8string>
{
public:
    Tokenizer(xstring string)
    {
        if (!string) return;
        const char *p = string;
        const char *e = p + strlen(string);
        while (p < e)
        {
            size_t len = strcspn(p, "/");
            push_back(u8string(p, len));
            p = p + len + 1;
        }
    }    
};

class PullOfBuffers {
public:
    PullOfBuffers() : next(0)
    {
        pull.resize(16);
        for (int i = 0, e = pull.size(); i < e; ++i)
            pull[i] = new MemoryBuffer();
    }
    ~PullOfBuffers() 
    {
        for (int i = 0, e = pull.size(); i < e; ++i)
            delete pull[i];
    }
    MemoryBuffer *get() 
    {
        int size = pull.size();
        MemoryBuffer *buffer = pull[next++];
        if (next == size) next = 0;
        return buffer;
    }
private:
  std::vector<MemoryBuffer*> pull;
  int next;
} _pull;

strbuf convert_utf8_to_wide(const utf8* string)
{
    MemoryBuffer *buffer = new MemoryBuffer;
    Utf8ToWideConverter u2w;
    u2w.convert(buffer, string, -1);
    return buffer;
}

strbuf convert_wide_to_utf8(const wchar_t* string)
{
    MemoryBuffer *buffer = new MemoryBuffer;
    WideToUtf8Converter w2u;    
    w2u.convert(buffer, string, -1);
    return buffer;
}

strbuf convert_ansi_to_wide(const char* string)
{
    MemoryBuffer *buffer = new MemoryBuffer;
    AnsiToWideConverter a2w;
    a2w.convert(buffer, string, -1);
    return buffer;
}

strbuf convert_wide_to_ansi(const wchar_t* string)
{
    MemoryBuffer *buffer = new MemoryBuffer;
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
    delete buffer;
}

xnode xml_load(const utf8* filename)
{
    Utf8ToWide u2w(filename);
    WideToAnsi w2a(u2w);
    return xmlLoad(w2a);
}

int xml_save(xnode node, const utf8* filename)
{
    Utf8ToWide u2w(filename);
    WideToAnsi w2a(u2w);
    return xmlSave(node, w2a);
}

xnode xml_open(const utf8* name)
{
    return xmlCreateRootNode(name);
}

void xml_delete(xnode node)
{
    xmlDeleteNode(node);
}
//------------------------------------------------------------------------------------
class EscSymbolsCoder
{
public:
    EscSymbolsCoder(const utf8* string) : m_string(string)
    {
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
    operator const utf8*() { return m_escaped.empty() ? m_string : m_escaped.c_str(); }

private:
    const utf8* m_string;
    u8string m_escaped;
};

class EscSymbolsDecoder
{
public:
    EscSymbolsDecoder(xstring string) : m_string(string) 
    {
        if (!string) return;
        const utf8 *b = strstr(string, "&#");
        if (!b) return;
        while (true)
        {
            m_decoded.append(string, b - string);
            const utf8* e = strchr(b + 2, ';');
            if (!e) { m_decoded.append(b, 2); b = b + 2; }
            else
            {
                u8string symbol(b + 2, e - b - 2);
                int size = symbol.size();
                if (size == 0 || size > 3 || strspn(symbol.c_str(), "0123456789") != size)
                { // incorrect bytecode
                    m_decoded.append(b, 2); b = b + 2;
                }
                else
                {
                    utf8 tmp[2] = { atoi(symbol.c_str()), 0 };
                    m_decoded.append(tmp); b = e + 1;                    
                }
            }
            string = b;
            b = strstr(string, "&#");
            if (!b) { m_decoded.append(string); break; }
        }
    }
    operator const xstring() 
    {
        if (m_decoded.empty())
            return m_string;
        MemoryBuffer* buffer = _pull.get();
        buffer->alloc( m_decoded.length() + 1 );
        char* output = (char*)buffer->getData();
        strcpy(output, m_decoded.c_str());
        return output;
    }

private:
    xstring m_string;
    u8string m_decoded;
};
//------------------------------------------------------------------------------------
void xml_set(xnode node, const utf8* name, const utf8* value)
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
            node = xmlCreateNode(node, tk[i].c_str());
    }
    int last = tk.size() - 1;
    EscSymbolsCoder esc(value);
    xmlSetAttribute(node, tk[last].c_str(), esc);
}

xstring xml_get_name(xnode node)
{
    xstring result = xmlGetName(node);
    return result ? result : "";
}

xstring xml_get_attr(xnode node, const utf8* name)
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
    EscSymbolsDecoder esc( xmlGetAttribute(node, tk[last].c_str()) );
    return esc;
}

void xml_set_text(xnode node, const utf8* text)
{
    EscSymbolsCoder esc(text);
    xmlSetText(node, esc);
}

xstring xml_get_text(xnode node)
{
    xstring result = xmlGetText(node);
    if (!result)
        return "";
    EscSymbolsDecoder esc(result);
    return esc;
}

xnode xml_create_child(xnode node, const utf8* childname)
{    
    return xmlCreateNode(node, childname);
}

xlist xml_request(xnode node, const utf8* request)
{
    return xmlRequest(node, request);
}

int xml_get_attr_count(xnode node)
{
    return xmlGetAttributesCount(node);
}

xstring xml_get_attr_name(xnode node, int index)
{
    return xmlGetAttributeName(node, index);
}

xstring xml_get_attr_value(xnode node, int index)
{
    return xmlGetAttributeValue(node, index);
}

xnode xml_move(xnode node, const utf8* path, int create)
{
    if (!node || !path)
        return NULL;
    u8string p(path);
    if (p.empty() || p.at(0) == ' ')
        return NULL;
    if (p.at(0) == '/')
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
           node = xmlCreateNode(node, tk[i].c_str());
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
        int sym_len = utf8_symlen(&str[p]);
        if (sym_len == 0 || sym_len > str_len)
            return -1;      // ошибка в строке - выходим
        len++;
        str_len -= sym_len;
        p += sym_len;
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
    MemoryBuffer *buffer = new MemoryBuffer(len);
    memcpy(buffer->getData(), str.c_str(), len);
    return buffer;
}
