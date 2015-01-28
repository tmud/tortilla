#include "stdafx.h"
#include "trigger.h"

int _findstrn(const utf8* str, const utf8* basestr, int basestr_len)
{
    if (!str || !basestr || basestr_len <= 0)
        return -1;

    int str_len = strlen(str);
    if (!str_len)
        return -1;

    do
    {
        const utf8* p = basestr;
        int len = basestr_len;

        // find first symbol
        while (len)
        {
            if (str[0] == *p)
                break;
            p++; len--;
        }
        if (len == 0)
            return -1;

        // check next symbols
        if (str_len > len)
            len = datalen;

        bool compared = true;
        int i = 1;
        for (; i < len; ++i)
        {
            if (keydata.at(i) != data[i])
            {
                compared = false;
                break;
            }
        }
        if (compared)
        {
            find_pos = data - data0;
            find_len = len;
            return true;
        }
        data += i;
        datalen -= i;
    } while (datalen);
}

TriggerKeyElement::TriggerKeyElement(): find_pos(-1), find_len(0)
{
}

bool TriggerKeyElement::init(const utf8* macro)
{
    reset();
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
    find_pos = -1;
    find_len = 0;
    keydata.clear();
}

bool TriggerKeyElement::initfail()
{
    assert(false);
    reset();
    return false;
}

bool TriggerKeyElement::findData(const utf8* data, int datalen)
{
    if (keydata.empty())
        return false;

    const utf8 *data0 = data;
    utf8 s = keydata.at(0);
    do
    {
        // find first symbol
        while (datalen)
        {
            if (s == *data)
                break;
            data++;
            datalen--;
        }
        if (datalen == 0)
            return false;

        // check next symbols
        int len = keydata.size();
        if (len > datalen)
            len = datalen;

        bool compared = true;
        int i = 1;
        for (; i < len; ++i)
        {
            if (keydata.at(i) != data[i])
            {
                compared = false; 
                break;
            }
        }
        if (compared)
        {
            find_pos = data - data0;
            find_len = len;
            return true;
        }
        data += i;
        datalen -= i;
    } while (datalen);
    return false;
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
    if (!data)
        return -1;
    int data_len = strlen(data);
    if (!data_len)
        return -1;

    m_data.write(data, data_len);
    int len = m_data.getSize();
    const utf8* p = (const utf8*)m_data.getData();

    // 1. find 'begin' key
    if (!m_key_begin.isFullComparsion())
    {
        if (!m_key_begin.findData(p, len))
        {
            m_data.clear();
            return -1;
        }
        if (!m_key_begin.isFullComparsion())
        {
            m_data.truncate(m_key_begin.getBegin());
            m_key_begin.truncate();
            return -1;
        }

        // full key found -> move pointer to search next data
        m_data.truncate(m_key_begin.getEnd());
        p = (const utf8*)m_data.getData();
        len = m_data.getSize();
    }

    // 2. find 'end' key
    if (!m_key_end.findData(p, len) || !m_key_end.isFullComparsion())
    {
        checkBufferLimit();
        return -1;
    }
    int len = m_key_end.getBegin();
    m_data.truncate(len);
    return len;
}

int Trigger::find(const utf8* data)
{
    if (!data || !m_key_end.isFullComparsion())
        return -1;

    int data_len = strlen(data);    
    int len = m_data.getSize();
    if (!data_len || len < data_len)
        return -1;

    strstr

    const utf8* p = (const utf8*)m_data.getData();            
    int pos = strncmp(data, p, data_len);


    return NULL; // m_find_buffer.c_str();
}

void Trigger::checkBufferLimit()
{
    if (m_data.getSize() > m_data_maxlen)
    {
        m_data.clear();
        m_key_begin.reset();
        m_key_end.reset();
    }
}
