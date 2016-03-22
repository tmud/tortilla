#include "stdafx.h"
#include "mapperPrompt.h"

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
