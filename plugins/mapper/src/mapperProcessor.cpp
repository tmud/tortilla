#include "stdafx.h"
#include "mapperProcessor.h"

#define MASK_NUMBER 1
#define MASK_NUMBER_LETTER 2

MapperKeyElement::MapperKeyElement()
{
    reset();
}

void MapperKeyElement::reset()
{
    key = -1;
    keylen = 0;
}

bool MapperKeyElement::init(const tstring& macro)
{
    reset();
    keydata.clear();
    const tchar *p = macro.c_str();
    bool spec_sym = false;
    for (;*p; ++p)
    {
        if (*p == L'\\') 
        {
            if (!spec_sym)
                { spec_sym = true; continue; }
            spec_sym = false;
        }
        if (!spec_sym)
           { keydata.append(p, 1); continue; }        
        
        tchar s[2] = { *p, 0};
        switch (*p) {
        case L'$':
            s[0] = 0x1b;
        break;
        case L'n':
            s[0] = 0xa;
        break;
        case L'r':
            s[0] = 0xd;
        break;
        case L's':
            s[0] = 0x20;
        break;
        case L'\\':
            s[0] = L'\\';
        break;
        case L'd':
            s[0] = MASK_NUMBER;
        break;
        case L'w':
            s[0] = MASK_NUMBER_LETTER;
        break;
        }
        keydata.append(s);
        spec_sym = false;
    }
    return (keydata.empty()) ? false : true;
}

bool MapperKeyElement::findData(const tchar *data, int datalen)
{    
    if (keydata.empty())
        return false;
    
    const tchar *data0 = data;
    tchar s = keydata.at(0);
    do
    {
        // find first symbol
        while (datalen)
        {
            if (compare(s, *data))
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
        for (; i<len; ++i)
        {
            if (!compare(keydata.at(i), data[i]))
                { compared = false; break; }
        }
        if (compared)
        {
            key = data - data0;
            keylen = len;
            return true;
        }
        data += i;
        datalen -= i;
    } while (datalen);

    return false;
}

bool MapperKeyElement::compare(tchar keydata, tchar symbol) const
{
    if (keydata == MASK_NUMBER)
        return (symbol >= L'0' && symbol <= L'9');
    if (keydata == MASK_NUMBER_LETTER)
    {
        if ((symbol >= L'0' && symbol <= L'9') || (symbol >= L'a' && symbol <= L'z') || (symbol >= L'A' && symbol <= L'Z'))
            return true;
        if ((symbol >= L'à' && symbol <= L'ÿ') || (symbol >= 'À' && symbol <= L'ß'))
            return true;
        return false;        
    }
    return keydata == symbol;
}

MapperProcessor::MapperProcessor()
{
}

bool MapperProcessor::processNetworkData(const tchar* text, int textlen, RoomData* result)
{
    // collect network data for parsing
    m_network_buffer.write(text, textlen);
    int datalen = m_network_buffer.getDataLen();
    if (!datalen)
        return false;
    const tchar* data = m_network_buffer.getData();

    // 1. find key data of begin name
    if (!bn.isKeyFull())
    {
        if (!bn.findData(data, datalen))
        {
            m_network_buffer.clear();
            return false;
        }
        if (!bn.isKeyFull())
        {
            m_network_buffer.truncate(bn.getKey());
            bn.truncate();
            return false;
        }

        // full key found -> move pointer to search next data
        m_network_buffer.truncate(bn.getAfterKey());
        data = m_network_buffer.getData();
        datalen = m_network_buffer.getDataLen();
    }
    
    //todo! remove
    tstring tmp_data(data, datalen);

    // 2. now find ee
    bool ee_result = ee.findData(data, datalen);
    if (!ee_result || !ee.isKeyFull())
        { checkBufferLimit(); return false; }

    // set data len to ee position
    datalen = ee.getKey();
    
    // 3. now check bn2 between bn and ee
    // if bn2 and en exist, find LAST bn2
    const tchar* data2 = data;
    int datalen2 = datalen;
    while (bn2.findData(data2, datalen2))
    {
        int newpos = bn2.getKey() + 1;
        if (bn2.isKeyFull())
           newpos = bn2.getAfterKey();
        data2 += newpos;
        datalen2 -= newpos;
        if (en.findData(data2, datalen2))
        {
            data = data2; datalen = datalen2;
            break;
        }
    }

    bool r = searchData(data, datalen, result);
    m_network_buffer.truncate(ee.getAfterKey());
    bn.reset();
    if (r) // additional check of room data
    {
        tstring &n = result->name;
        int size = n.size();
        for (int i=0; i<size; ++i)
        { if (n.at(i) < 32)
                return false;
        }
    }
    return r;
}

bool MapperProcessor::searchData(const tchar* data, int datalen, RoomData* result)
{
    // now we searching all other tags (bv,ev,en,bd,ed,be)
    bool a = en.findData(data, datalen);
    if (a) a = bd.findData(data, datalen);
    if (a) a = ed.findData(data, datalen);
    if (a) a = be.findData(data, datalen);
    if (a) a = bv.findData(data, datalen);
    if (a) a = ev.findData(data, datalen);
    if (a)
    {
        int nl = (en.getKey() + 0);
        result->name.assign(data, nl);
        
        int d = bd.getAfterKey();
        int dl = (ed.getKey() - d);
        if (dl > 0)
            result->descr.assign(&data[d], dl);
        
        int e = be.getAfterKey();
        int el = datalen - e;
        result->exits.assign(&data[e], el);

        int v = bv.getAfterKey();
        int vl = (ev.getKey() - v);
        if (vl > 0)
            result->vnum.assign(&data[v], vl);

        return true;
    }
    return false;
}

void MapperProcessor::checkBufferLimit()
{
    if (m_network_buffer.getDataLen() > 2048)  // 2kb limit of symbols (4 kb of bytes).
    {
       m_network_buffer.clear();
       bn.reset();
    }
}

void MapperProcessor::updateProps(PropertiesMapper *props)
{
    bn.init(props->begin_name);
    bn2.init(props->begin_name);
    en.init(props->end_name);
    bv.init(props->begin_vnum);
    ev.init(props->end_vnum);
    bd.init(props->begin_descr);
    ed.init(props->end_descr);
    be.init(props->begin_exits);
    ee.init(props->end_exits);
}

MapperPrompt::MapperPrompt()
{
}

void MapperPrompt::updateProps(PropertiesMapper *props)
{
    bp.init(props->begin_prompt);
    ep.init(props->end_prompt);
    m_network_buffer.clear();
}

bool MapperPrompt::processNetworkData(const WCHAR* text, int textlen)
{
    m_network_buffer.write(text, textlen);
    int datalen = m_network_buffer.getDataLen();
    if (!datalen)
        return false;
    const WCHAR* data = m_network_buffer.getData();
    const WCHAR* data_end = data + datalen;

    while (data != data_end)
    {
        const WCHAR *p = data;
        while (p != data_end && *p != 0xd && *p != 0xa)
            p++;

        const WCHAR* msg = data;
        int dt = (p == data_end) ? 0 : 1;
        data = p + dt;
        if (msg == p)
            continue;

        int msg_len = p - msg;
        //OutputDebugStringW(x.c_str());

        if (bp.findData(msg, msg_len) && ep.findData(msg, msg_len))
        {
            const WCHAR* buffer = m_network_buffer.getData();
            int processed = p - buffer;
            m_network_buffer.truncate(processed);
            return true;
        }
    }

    m_network_buffer.clear();
    return false;
}
