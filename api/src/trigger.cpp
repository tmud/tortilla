#include "stdafx.h"
#include "trigger.h"

const findpos not_found(-1, 0);
findpos _findstrn_min(const utf8* str, const utf8* where_str, int where_len)
{
    if (!str || !where_str || where_len <= 0)
        return not_found;
    int str_len = strlen(str);
    if (!str_len)
        return not_found;

    const utf8* p = where_str;
    int len = where_len;

    // find first symbol
    while (len)
    {
       if (str[0] == *p)
           break;
       p++; len--;
    }
    if (len == 0)
       return not_found;

    // check next symbols
    len = min(len, str_len);
    int i = 1;
    for (; i < len; ++i)
    {
        if (str[i] != p[i])
            break;
    }
    return findpos(p - where_str, i);
}

int _findstrn(const utf8* str, const utf8* where_str, int where_len)
{
    if (!str || !where_str || where_len <= 0)
        return -1;
    int str_len = strlen(str);
    if (!str_len)
        return -1;

    const utf8* p = where_str;
    int len = where_len;
    do 
    {
        // find first symbol
        while (len)
        {
            if (str[0] == *p)
                break;
            p++; len--;
        }
        if (len == 0 || str_len > len)
            return -1;

        // check next symbols
        bool compared = true;
        for (int i = 1; i < str_len; ++i)
        {
            if (str[i] != p[i]) { compared = false;  break; }
        }
        if (compared)
            return (p - where_str);
        p++; len--;
    } while (len);
    return -1;
}

TriggerKeyElement::TriggerKeyElement(): pos(not_found)
{
}

bool TriggerKeyElement::init(const utf8* macro)
{
    reset();
    keydata.clear();
    bool spec_sym = false;
    const utf8*p = macro;
    for (; *p; ++p)
    {
        if (*p == '\\')
        {
            if (spec_sym)
                keydata.append(p, 1);
            spec_sym = !spec_sym;
            continue;
        }
        if (spec_sym)
        {
            utf8 s[2] = { 0, 0 };
            switch (*p) {
            case '$':
                s[0] = 0x1b;
                break;
            case 'n':
                s[0] = 0xa;
                break;
            case 'r':
                s[0] = 0xd;
                break;
            case 's':
                s[0] = 0x20;
                break;
            default:
                spec_sym = false;
                break;
            }
            if (spec_sym)
                keydata.append(s);
            spec_sym = false;
            continue;
        }

        utf8 c = *p;
        if (c < 0x80) { keydata.append(p, 1); }
        else if (c < 0xc0 || c > 0xf7) 
            return initfail();
        else
        {
            int len = 1;
            if ((c & 0xf0) == 0xe0) len = 2;
            else if ((c & 0xf8) == 0xf0) len = 3;
            else return initfail();
            const utf8*p2 = p;
            for (; len > 0; len--)
            {
                p2++;
                if (!*p2 || ((*p2 & 0xc0) != 0x80))
                    return initfail();
            }
            keydata.append(p, p2 - p);
            p = p2;
        }
    }
    return (keydata.empty()) ? false : true;
}

void TriggerKeyElement::reset()
{
    pos = not_found;    
}

bool TriggerKeyElement::initfail()
{
    assert(false);
    reset();
    keydata.clear();
    return false;
}

bool TriggerKeyElement::findData(const utf8* data, int datalen)
{
    if (keydata.empty())
        return false;
    pos = _findstrn_min(keydata.c_str(), data, datalen);
    return (pos == not_found) ? false : true;
}

Trigger::Trigger() : m_data_maxlen(0)
{
}

bool Trigger::init(const utf8* begin, const utf8* end, int max_len)
{
    if (!begin || !end || max_len <= 0)
        return false;
    if (!m_key_begin.init(begin) || !m_key_end.init(end))
        return false;
    m_data_maxlen = max_len;
    return true;
}

int Trigger::add(const utf8* data)
{
    if (m_key_end.isFullComparsion())
        reset();

    int data_len = (data) ? strlen(data) : 0;
    if (!data_len)
        return 0;

    m_data.write(data, data_len);

    // 1. find 'begin' key
    if (!m_key_begin.isFullComparsion())
    {
        if (!m_key_begin.findData((const utf8*)m_data.getData(), m_data.getSize()))
        {
            m_data.clear();
            return 0;
        }
        m_data.truncate(m_key_begin.getBegin());
        m_key_begin.truncate();
        if (!m_key_begin.isFullComparsion())
            return 0;
    }

    // 2. full begin key found -> move pointer to search next data
    block_data b = getdata();

    // 3. find 'end' key
    if (!m_key_end.findData(b.first, b.second) || !m_key_end.isFullComparsion())
    {
        if (m_data.getSize() > m_data_maxlen)
            reset();
        return 0;
    }

    // 4. found full end key - return len of block between keys
    return m_key_end.getBegin() - m_key_begin.getLen();
}

int Trigger::find(const utf8* data)
{
    if (!data || !m_key_end.isFullComparsion())
        return -1;
    block_data b = getdata();
    return _findstrn(data, b.first, b.second);
}

const utf8* Trigger::get(int from, int len)
{
    if (!m_key_end.isFullComparsion())
        m_find_buffer.clear();
    else
    {
        block_data b = getdata();
        int bd_len = b.second;
        if (from < 0 || from >= bd_len || len <= 0 || (from + len) >= bd_len)
            m_find_buffer.clear();
        else
            m_find_buffer.assign(b.first + from, len);
    }
    return m_find_buffer.c_str();
}

Trigger::block_data Trigger::getdata()
{
    int len = m_data.getSize();
    const utf8* p = (const utf8*)m_data.getData();
    p = p + m_key_begin.getLen();
    len = len - m_key_begin.getLen();
    return block_data(p, len);
}

void Trigger::reset()
{
    m_data.clear();
    m_key_begin.reset();
    m_key_end.reset();
}
