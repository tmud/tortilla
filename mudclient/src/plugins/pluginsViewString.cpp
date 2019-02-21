#include "stdafx.h"
#include "pluginsViewString.h"
#include "pluginSupport.h"

extern wchar_t* plugin_buffer();
void PluginsViewString::serialize(tstring *data)
{
    data->clear();
    data->reserve(128);
    tchar *buffer = plugin_buffer();
    int count = blocks.size();
    for (int i=0;i<count;++i)
    {
        const MudViewStringBlock &b = blocks[i];
        tstring colors;
        PluginColorSerialize pcs;
        pcs.serialize(b, &colors);
        data->append(colors);

        int count = 1;
        tstring delimeter;
        for (;count<4;++count)
        {
            delimeter.assign(count, L';');
            if (b.string.find(delimeter) == tstring::npos)
                break;
        }
        if (count == 4)
        {
            if (b.string.find(L"]];") != tstring::npos)
            {
                data->append(int_to_wstring(count+1));
                data->append(b.string);
                data->append(L"]=];");
            }
            else
            {
                data->append(int_to_wstring(count));
                data->append(b.string);
                data->append(L"]];");
            }
        }
        else
        {
            data->append(int_to_wstring(count));
            data->append(b.string);
            data->append(delimeter);
        }
    }
}

void PluginsViewString::deserialize(const tstring& data)
{
    blocks.clear();
    MudViewStringBlock block;
    const tchar *b = data.c_str();
    const tchar *e = b + data.length();
    while(b != e)
    {
        bool result = false;
        block.params.reset();
        do 
        {
            PluginColorSerialize pcs;
            int processed = pcs.deserialize(b, &block);
            if (processed == -1) break;
            b = b + processed;

            // text
            if (b == e) break;
            if (*b < L'1' || *b > L'5') break;
            if (*b == L'4') {
               const tchar* p = wcsstr(b, L"]];");
               if (!p) break;
               block.string.assign(b+1, p-b-1);
               b = p + 3;
            } else if (*b == L'5') {
               const tchar* p = wcsstr(b, L"]=];");
               if (!p) break;
               block.string.assign(b+1, p-b-1);
               b = p + 4;
            } else
            {
                int count = *b - L'0';
                tstring delimeter(count, L';');
                const tchar* p = wcsstr(b, delimeter.c_str());
                if (!p) break;
                block.string.assign(b+1, p-b-1);
                b = p + count;
            }

            result = true;
        } while(0);
        if (!result)
        {
            block.params.reset();
            block.string = L"<error>";
            break;
        }
        blocks.push_back(block);
    }
}
