#include "stdafx.h"
#include "mapperPrompt.h"

/*MapperPrompt::MapperPrompt()
{    
}

void MapperPrompt::updateProps(PropertiesMapper *props)
{
    bp.init(props->begin_prompt);
    ep.init(props->end_prompt);
}

bool MapperPrompt::processNetworkData(MapperNetworkData &ndata)
{
    m_network_buffer.write(ndata.getData(), ndata.getDataLen());

    int datalen = m_network_buffer.getDataLen();
    if (!datalen)
        return false;
    const WCHAR* data = m_network_buffer.getData();
    const WCHAR* data_end = data + datalen;

    while (data != data_end)
    {
        const WCHAR *p = data;
        while (p != data_end && *p != 0xd && *p != 0xa)             // пытаемс€ выделить самосто€тельные строки
            p++;
        
        const WCHAR* msg = data;
        int dt = (p == data_end) ? 0 : 1;
        data = p + dt;
        if (msg == p)
            continue;

        int msg_len = p - msg;
        if (bp.findData(msg, msg_len) && ep.findData(msg, msg_len) && 
            bp.isKeyFull() && ep.isKeyFull())
        {
            const WCHAR* buffer = m_network_buffer.getData();
            int processed = p - buffer;
            m_network_buffer.truncate(processed);
            return true;
        }

        if (data == data_end)
        {
            const WCHAR* buffer = m_network_buffer.getData();
            int processed = msg - buffer;
            m_network_buffer.truncate(processed);
        }
    }
    return false;
}
*/
