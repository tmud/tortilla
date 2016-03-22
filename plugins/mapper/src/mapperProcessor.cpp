#include "stdafx.h"
#include "mapperProcessor.h"

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
    const WCHAR *p = macro.c_str();
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
        
        WCHAR s[2] = { *p, 0};
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
        }
        keydata.append(s);
        spec_sym = false;
    }
    return (keydata.empty()) ? false : true;
}

bool MapperKeyElement::findData(const WCHAR *data, int datalen)
{    
    if (keydata.empty())
        return false;
    
    const WCHAR *data0 = data;
    WCHAR s = keydata.at(0);
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
        for (; i<len; ++i)
        {
            if (keydata.at(i) != data[i])
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

MapperProcessor::MapperProcessor()
{
}

bool MapperProcessor::processNetworkData(const WCHAR* text, int textlen, RoomData* result)
{
    // collect network data for parsing
    m_network_buffer.write(text, textlen);
    int datalen = m_network_buffer.getDataLen();
    if (!datalen)
        return false;
    const WCHAR* data = m_network_buffer.getData();

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
    
    // 2. now find ee
    bool ee_result = ee.findData(data, datalen);
    if (!ee_result || !ee.isKeyFull())
        { checkBufferLimit(); return false; }

    // set data len to ee position
    datalen = ee.getKey();
    
    // 3. now check bn2 between bn and ee
    // if bn2 exist, find LAST bn2
    while (bn2.findData(data, datalen))
    {
        int newpos = bn2.getKey() + 1;
        if (bn2.isKeyFull())
           newpos = bn2.getAfterKey();
        data += newpos;
        datalen -= newpos;
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

bool MapperProcessor::searchData(const WCHAR* data, int datalen, RoomData* result)
{
    // now we searching all other tags (en,bd,ed,be)
    bool a = en.findData(data, datalen);
    bool b = bd.findData(data, datalen);
    bool c = ed.findData(data, datalen);
    bool d = be.findData(data, datalen);
    if (a && b && c && d)
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
    bd.init(props->begin_descr);
    ed.init(props->end_descr);
    be.init(props->begin_exits);
    ee.init(props->end_exits);
}
