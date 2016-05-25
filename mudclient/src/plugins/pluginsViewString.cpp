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
    const tchar *b = data.c_str();
    const tchar *e = b + data.length();
    while(b != e)
    {
        bool result = false;
        block.params.reset();
        do 
        {
            const tchar *p = wcschr(b, L';');   // text color
            if (!p) break;
            tstring text_color(b, p-b);
            b = p + 1;
            p = wcschr(b, L';');   // background color
            if (!p)  break;
            tstring background_color(b, p-b);
            b = p + 1;
            p = wcschr(b, L';');   // params
            if (!p) break;
            tstring params(b, p-b);
            b = p + 1;

            {  // text color
                Tokenizer tc(text_color.c_str(), L",");
                tc.trimempty();
                if (tc.size()!=1 && tc.size()!=3) break;
                if (tc.size()==1)
                {
                    if (!getcolor(tc[0], &block.params.text_color))
                        break;
                }
                else
                {
                    tbyte r,g,b;
                    if (!(getcolor(tc[0],&r) && getcolor(tc[1],&g) && getcolor(tc[2],&b)))
                        break;
                    block.params.ext_text_color = RGB(r,g,b);
                    block.params.use_ext_colors = 1;
                }
            }

            {   // background color
                Tokenizer tc(background_color.c_str(), L",");
                tc.trimempty();
                if (tc.size()!=1 && tc.size()!=3) break;
                if (tc.size()==1)
                {
                    if (!getcolor(tc[0], &block.params.bkg_color))
                        break;
                }
                else
                {
                    tbyte r,g,b;
                    if (!(getcolor(tc[0],&r) && getcolor(tc[1],&g) && getcolor(tc[2],&b)))
                        break;
                    block.params.ext_bkg_color = RGB(r,g,b);
                    block.params.use_ext_colors = 1;
                }
            }

            // params
            if (!isOnlySymbols(params, L"^iufr")) break;
            for (int i=0,s=params.size();i<s;++i)
            {
                switch(params[i]) {
                case L'^': block.params.intensive_status = 1; break;
                case L'i': block.params.italic_status = 1; break;
                case L'u': block.params.underline_status = 1; break;
                case L'f': block.params.blink_status = 1; break; 
                case L'r': block.params.reverse_video = 1; break;
                }
            }

            // text
            if (b == e) break;
            if (*b < L'1' || *b > L'5') break;
            if (*b == L'4') {
               p = wcsstr(b, L"]];");
               if (!p) break;
               block.string.assign(b+1, p-b-1);
               b = p + 3;
            } else if (*b == L'5') {
               p = wcsstr(b, L"]=];");
               if (!p) break;
               block.string.assign(b+1, p-b-1);
               b = p + 4;
            } else
            {
                int count = *b - L'0';
                tstring delimeter(count, L';');
                p = wcsstr(b, delimeter.c_str());
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
