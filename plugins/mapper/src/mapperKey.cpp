#include "stdafx.h"
#include "mapperKey.h"
/*
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
    return; //todo
    if (!m_find_end_mode)       // режим поиска начала ключа
    {
        if (!bk.isKeyUsable())  // начало ключа еще не найдено
        {
            if (!bk.findData(ndata.getData(), ndata.getDataLen()))
                return;
            bool t = bk.isKeyFull();
            if (t || bk.getAfterKey() == ndata.getDataLen())
            {
                if (!t)
                {
                    //todo
                    int x = 1;
                }

                // нашли полный ключ или частичный, но в конце блока данных
                int pos = bk.getKey();
                const WCHAR* key_begin = ndata.getData() + pos;
                int key_len = ndata.getDataLen() - pos;
                m_buffer.write(key_begin, key_len);
                ndata.trimLeft(pos);    //обрезаем данные
                
                if (!t) // в случае получения ключа в конце буфера
                    return;

                // продолжаем, но уже - поиск конца ключа
                bk.reset();
                m_find_end_mode = true;
            }
            else { return; }
        }
        else
        {
            // сюда попадаем в случае частичного ключа в конце блока данных в прошлый вызов
            // сначала допроверяем ключ
            m_buffer.write(ndata.getData(), ndata.getDataLen());
            ndata.trimLeft(0);

            int datalen = m_buffer.getDataLen();
            if (datalen < bk.getKeyTemplateLen())
                return;

            const WCHAR* data = m_buffer.getData();
            if (bk.findData(data, datalen) && bk.isKeyFull())
            {
                m_buffer.truncate(bk.getKeyLen());
                bk.reset();
                m_find_end_mode = true;
                // продолжаем уже с поиском конца ключа
            }
            else
            {
                // ключ не совпал - сбрасываем буфер на выход
                ndata.append(m_buffer.getData(), m_buffer.getDataLen());
                bk.reset();
                m_buffer.clear();                
                return;
            }
        }
    }
    else // поиск окончания ключа, но попадаем сюда только при новом вызове
    {
        m_buffer.write(ndata.getData(), ndata.getDataLen());
        ndata.trimLeft(0);
    }

    // в ndata могут быть данные, buffer - данные для поиска конца ключа    
    int datalen = m_buffer.getDataLen();
    const WCHAR* data = m_buffer.getData();

    if (ek.findData(data, datalen) && ek.isKeyFull())
    {
        // нашли окончание ключа
        int pos = ek.getAfterKey();        
        ndata.append(data + pos, datalen - pos);
        m_buffer.clear();
        m_find_end_mode = false;
        ek.reset();        
    }
}
*/