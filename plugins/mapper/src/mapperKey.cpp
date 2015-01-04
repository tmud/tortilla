#include "stdafx.h"
#include "mapperKey.h"

MapperKey::MapperKey() : m_find_end_mode(false)
{
}

void MapperKey::updateProps(PropertiesMapper *props)
{
    bk.init(props->begin_key);
    ek.init(props->end_key);
}

void MapperKey::processNetworkData(MapperNetworkData &ndata)
{
    return;

    m_network_buffer.write(ndata.getData(), ndata.getDataLen());
    int datalen = m_network_buffer.getDataLen();
    if (!datalen) return;
    const WCHAR* data = m_network_buffer.getData();

    if (m_find_end_mode)
    {
        if (ek.findData(data, datalen) && ek.isKeyFull())
        {
        }
        
        if (!ek.findData(ndata.getData(), ndata.getDataLen()))
        {
            ndata.trimLeft(0);
            return;
        }
        int pos = ek.getAfterKey();
        int len = ndata.getDataLen() - pos;
        const WCHAR* data = ndata.getData() + len;

        m_find_end_mode = false;
        return;
    }

    if (bk.findData(ndata.getData(), ndata.getDataLen()) && bk.isKeyFull())
    {
        int pos = bk.getKey();
        ndata.trimLeft(pos);

        m_find_end_mode = true;
    }
}
