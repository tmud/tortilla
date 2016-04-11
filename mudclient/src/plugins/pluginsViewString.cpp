#include "stdafx.h"
#include "pluginsViewString.h"

extern wchar_t* plugin_buffer();
void PluginsViewString::serialize(tstring *data)
{
    data->reserve(128);   
    int count = blocks.size();
    serint(data, count);
    for (int i=0;i<count;++i)
    {
        const MudViewStringBlock &b = blocks[i];
        const MudViewStringParams& p = b.params;
        serint(data, p.italic_status);

    
    }

}

void PluginsViewString::delimeter(tstring*data)
{
    if (!data->empty())
        data->append(L",");
}
void PluginsViewString::deserialize(const tstring& data)
{

}

void PluginsViewString::serint(tstring*data, int value)
{
    delimeter(data);
    tchar *b = plugin_buffer();
    _itow(value, b, 10);    
    data->append(b);
}
