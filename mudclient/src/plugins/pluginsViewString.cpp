#include "stdafx.h"
#include "pluginsViewString.h"

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
        const MudViewStringParams&p = b.params;
        if (p.use_ext_colors)
        {
            COLORREF t = p.text_color;
            COLORREF b = p.bkg_color;
            swprintf(buffer, L"%d,%d,%d;%d,%d,%d;", 
                GetRValue(t), GetGValue(t), GetBValue(t), 
                GetRValue(b), GetGValue(b), GetBValue(b) 
               );
        }
        else
        {
            swprintf(buffer, L"%d;%d;", p.text_color, p.bkg_color);
        }
        data->append(buffer);

        tstring a;
        if (p.italic_status) a.append(L"a");
        if (p.underline_status) a.append(L"u");
        if (p.intensive_status) a.append(L"^");
        if (p.blink_status) a.append(L"f");
        if (p.reverse_video) a.append(L"r");
        a.append(L";");
        data->append(a);
        data->append(b.string);
        data->append(L"\n");

        /*const tstring& s = b.string;
        if (s.find(L";;") != tstring::npos) {
            tstring new_eol(L";#");
            if (s.find(new_eol) != tstring::npos) {
                data->append(L"@");
                data->append(b.string);
                data->append(L";@");
            }
            else {
                data->append(L"#");
                data->append(b.string);
                data->append(L";#");
            }
        } else {
            data->append(L";");
            data->append(b.string);
            data->append(L";;");
        }*/
    }
}

void PluginsViewString::deserialize(const tstring& data)
{

}

