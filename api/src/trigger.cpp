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
    while (true)
    {
        // find first symbol
        while (len)
        {
            if (str[0] == *p) break;
            p++; len--;
        }
        if (len == 0)
            return not_found;

        // check next symbols
        bool compared = true;
        int next_len = min(len, str_len);
        for (int i = 1; i < next_len; ++i) {
            if (str[i] != p[i]) { compared = false; break; }
        }
        if (compared)
            break;
        p++; len--;
    }
    return findpos(p - where_str, min(len, str_len));
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
    pos = not_found;
    keydata.clear();
    if (!macro)
        return false;
    keydata.assign(macro);
    return (keydata.empty()) ? false : true;
}

void TriggerKeyElement::reset()
{
    pos = not_found;
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
    {
        m_data.truncate(m_key_end.getEnd() + m_key_begin.getLen());
        reset();
    }

    if (!data)
        return 0;
    m_data.write(data, strlen(data));

    int len = m_data.getSize();
    if (!len)
        return 0;
    const utf8*p = (const utf8*)m_data.getData();

    // 1. find 'begin' key
    if (!m_key_begin.isFullComparsion())
    {
        if (!m_key_begin.findData(p, len))
        {
            m_data.clear();
            return 0;
        }
        m_data.truncate(m_key_begin.getBegin());
        m_key_begin.truncate();
        if (!m_key_begin.isFullComparsion())
            return 0;
        p = (const utf8*)m_data.getData();
        len = m_data.getSize();
    }

    // 2. full begin key found -> move pointer to search next data
    int key_len = m_key_begin.getLen();
    p = p + key_len;
    len = len - key_len;

    // 3. find 'end' key
    if (!m_key_end.findData(p, len) || !m_key_end.isFullComparsion())
    {
        if (m_data.getSize() > m_data_maxlen)
            reset();
        return 0;
    }

    // 4. found full end key - return len of block between keys
    return m_key_end.getBegin() - m_key_begin.getLen();
}

int Trigger::find(int from, const utf8* data)
{
    if (!data || !m_key_end.isFullComparsion() || from < 0)
        return -1;
    block_data b = getdata();
    if (from >= b.second)
        return -1;
    return from + _findstrn(data, b.first+from, b.second-from);
}

const utf8* Trigger::get(int from, int len)
{
    if (!m_key_end.isFullComparsion())
        m_find_buffer.clear();
    else
    {
        block_data b = getdata();
        int bd_len = b.second;
        if (from < 0 || from >= bd_len || len <= 0 || (from + len) > bd_len)
            m_find_buffer.clear();
        else
            m_find_buffer.assign(b.first + from, len);
    }
    return m_find_buffer.c_str();
}

int Trigger::datalen()
{
    if (!m_key_end.isFullComparsion())
        return 0;
    block_data b = getdata();
    return b.second;
}

Trigger::block_data Trigger::getdata()
{
    int len = m_key_end.getBegin(); // end расчитывался уже за begin, это длина блока между begin/end
    const utf8* p = (const utf8*)m_data.getData();
    p = p + m_key_begin.getLen();
    return block_data(p, len);
}

void Trigger::reset()
{
    m_key_begin.reset();
    m_key_end.reset();
}
