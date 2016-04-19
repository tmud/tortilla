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
        if (p.italic_status) a.append(L"i");
        if (p.underline_status) a.append(L"u");
        if (p.intensive_status) a.append(L"^");
        if (p.blink_status) a.append(L"f");
        if (p.reverse_video) a.append(L"r");
        a.append(L";");
        data->append(a);
        data->append(b.string);
        data->append(L"\t");
    }
}

bool getcolor(const tchar* c, tbyte* color)
{
   bool result = false;
   int clr = wstring_to_int(c, &result);
   if (result && clr >= 0 && clr <= 255)
   {
       *color = static_cast<tbyte>(clr);
       return true;
   }
   return false;
}

void PluginsViewString::deserialize(const tstring& data)
{
    blocks.clear();
    MudViewStringBlock block;
    Tokenizer t(data.c_str(), L"\t");
    for (int i=0,e=t.size();i<e;++i)
    {
        bool result = true;
        block.params.reset();
        do {
            const tchar *b = t[i];
            const tchar *p = wcschr(b, L';');   // text color
            if (!p) { result = false; break; }
            tstring text_color(b, p-b);
            b = p + 1;
            p = wcschr(b, L';');   // background color
            if (!p) { result = false; break; }
            tstring background_color(b, p-b);
            b = p + 1;
            p = wcschr(b, L';');   // params
            if (!p) { result = false; break; }
            tstring params(b, p-b);
            b = p + 1;
            block.string.assign(b);

            {  // text color
                Tokenizer tc(text_color.c_str(), L",");
                tc.trimempty();
                if (tc.size()!=1 && tc.size()!=3) { result = false; break; }
                if (tc.size()==1)
                {
                    result = getcolor(tc[0], &block.params.text_color);
                    if (!result) break;
                }
                else
                {
                    tbyte r,g,b;
                    result = (getcolor(tc[0],&r) && getcolor(tc[1],&g) && getcolor(tc[2],&b));
                    if (!result) break;
                    block.params.ext_text_color = RGB(r,g,b);
                    block.params.use_ext_colors = 1;
                }
            }

            {   // background color
                Tokenizer tc(background_color.c_str(), L",");
                tc.trimempty();
                if (tc.size()!=1 && tc.size()!=3) { result = false; break; }
                if (tc.size()==1)
                {
                    result = getcolor(tc[0], &block.params.bkg_color);
                    if (!result) break;
                }
                else
                {
                    tbyte r,g,b;
                    result = (getcolor(tc[0],&r) && getcolor(tc[1],&g) && getcolor(tc[2],&b));
                    if (!result) break;
                    block.params.ext_bkg_color = RGB(r,g,b);
                    block.params.use_ext_colors = 1;
                }
            }

            // params
            if (!isOnlySymbols(params, L"iu^fr")) { result = false; break; }
            for (int i=0,e=params.size();i<e;++i)
            {
                switch(params[i]) {
                case L'^': block.params.intensive_status = 1; break;
                case L'i': block.params.italic_status = 1; break;
                case L'u': block.params.underline_status = 1; break;
                case L'f': block.params.blink_status = 1; break; 
                case L'r': block.params.reverse_video = 1; break;
                }
            }
        } while(0);
        if (!result)
        {
            block.params.reset();
            block.string = L" ??? ";
        }
        blocks.push_back(block);
    }
}
