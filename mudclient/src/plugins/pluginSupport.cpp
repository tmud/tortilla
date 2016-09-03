#include "stdafx.h"
#include "pluginSupport.h"

PluginsIdTableControl::PluginsIdTableControl(int firstid, int lastid) : first_id(firstid), last_id(lastid)
{
}

UINT PluginsIdTableControl::registerPlugin(Plugin*p, int code, bool button)
{
    int id = -1;
    for (int i = 0, e = plugins_id_table.size(); i < e; ++i){
        if (!plugins_id_table[i].plugin) { id = i; break; }
    }    
    if (id == -1) { plugins_id_table.push_back(idplugin());  id = plugins_id_table.size() - 1; }    
    idplugin &t = plugins_id_table[id];
    t.plugin = p; t.code = code; t.button = button;
    id = id + first_id;
    return (UINT)id;
}

UINT PluginsIdTableControl::unregisterByCode(Plugin*p, int code, bool button)
{
    int id = getIndex(p, code, button);
    if (id != -1) 
    {
        idplugin &t = plugins_id_table[id];
        t.plugin = NULL;
        id = id + first_id;
    }   
    return (UINT)id;
}

void PluginsIdTableControl::unregisterById(Plugin*p, UINT id)
{
    int _id = (int)id - first_id;
    int size = plugins_id_table.size();
    if (_id >= 0 && _id < size)
    {
        idplugin &t = plugins_id_table[_id];
        if (t.plugin && t.plugin == p)
        {
            t.plugin = NULL;
        }
    }
}

UINT PluginsIdTableControl::findId(Plugin*p, int code, bool button)
{
    int id = getIndex(p, code, button);
    if (id != -1)
        id = id + first_id;
    return (UINT)id;
}

void PluginsIdTableControl::runPluginCmd(UINT id)
{
    int _id = (int)id - first_id;
    int size = plugins_id_table.size();
    if (_id >= 0 && _id < size)
    {
        idplugin &t = plugins_id_table[_id];
        if (t.plugin)
            t.plugin->menuCmd(t.code);
    }
}

int PluginsIdTableControl::getIndex(Plugin*p, int code, bool button)
{
    for (int i = 0, e = plugins_id_table.size(); i < e; ++i)
    {
        idplugin &t = plugins_id_table[i];
        if (t.plugin == p && t.code == code && t.button == button)
        {
            return i;
        }
    }
    return -1;
}

void PluginColorSerialize::serialize(const MudViewStringBlock &b, tstring* color)
{
    tchar buffer[32];
    const MudViewStringParams&p = b.params;
    if (p.use_ext_colors)
    {
        COLORREF t = p.ext_text_color;
        COLORREF b = p.ext_bkg_color;
        swprintf(buffer, L"%d,%d,%d;%d,%d,%d;", 
           GetRValue(t), GetGValue(t), GetBValue(t), 
           GetRValue(b), GetGValue(b), GetBValue(b) 
          );
    }
    else
    {
         swprintf(buffer, L"%d;%d;", p.text_color, p.bkg_color);
    }
    color->append(buffer);
    tstring a;
    if (p.italic_status) a.append(L"i");
    if (p.underline_status) a.append(L"u");
    if (p.intensive_status) a.append(L"^");
    if (p.blink_status) a.append(L"f");
    if (p.reverse_video) a.append(L"r");
    a.append(L";");
    color->append(a);
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

int PluginColorSerialize::deserialize(const tchar* color, MudViewStringBlock *out)
{
    int processed = -1;
    do {
    const tchar *b = color;
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
    tstring pars(b, p-b);
    b = p + 1;

    MudViewStringParams params;
    {  // text color
       Tokenizer tc(text_color.c_str(), L",");
       tc.trimempty();
       if (tc.size()!=1 && tc.size()!=3) break;
       if (tc.size()==1)
       {
           if (!getcolor(tc[0], &params.text_color))
                break;
       }
      else
       {
           tbyte r,g,b;
           if (!(getcolor(tc[0],&r) && getcolor(tc[1],&g) && getcolor(tc[2],&b)))
               break;
           params.ext_text_color = RGB(r,g,b);
           params.use_ext_colors = 1;
       }
   }

   {   // background color
       Tokenizer tc(background_color.c_str(), L",");
       tc.trimempty();
       if (tc.size()!=1 && tc.size()!=3) break;
       if (tc.size()==1)
       {
           if (!getcolor(tc[0], &params.bkg_color))
               break;
       }
       else
       {
            tbyte r,g,b;
            if (!(getcolor(tc[0],&r) && getcolor(tc[1],&g) && getcolor(tc[2],&b)))
                break;
            params.ext_bkg_color = RGB(r,g,b);
            params.use_ext_colors = 1;
       }
   }

   // params
   if (!isOnlySymbols(pars, L"^iufr")) break;
   for (int i=0,s=pars.size();i<s;++i)
   {
       switch(pars[i]) {
       case L'^': params.intensive_status = 1; break;
       case L'i': params.italic_status = 1; break;
       case L'u': params.underline_status = 1; break;
       case L'f': params.blink_status = 1; break; 
       case L'r': params.reverse_video = 1; break;
        }
   }
   processed = b - color;
   out->params = params;

   } while(0);

   return processed;
}
