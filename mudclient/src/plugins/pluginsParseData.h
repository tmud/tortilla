#pragma once

#include "MudViewString.h"

struct PluginViewString
{
    std::vector<u8string> blocks;
    void gettext(u8string *text)
    {
        for (int i = 0, e = blocks.size(); i < e; ++i)
            text->append(blocks[i]);
    }
};

class PluginsParseData
{
public:
    parseData *pdata;
    std::vector<PluginViewString*> plugins_strings;
    int selected;

public:    
    PluginsParseData(parseData *data) : pdata(data), selected(-1) { convert(); }
    ~PluginsParseData() { convert_back(); autodel<PluginViewString> _z(plugins_strings); }
    int size() const { return plugins_strings.size(); }
    bool select(int index)
    {
        if (index >= 0 && index < size()) { selected = index; return true; }
        return false;
    }
    
    PluginViewString* getselected_pvs()
    {
        if (selected >= 0 && selected < size())
            return plugins_strings[selected];
        return NULL;
    }

    MudViewString* getselected()
    {
        if (selected >= 0 && selected < size())
            return pdata->strings[selected];
        return NULL;
    }

    bool getPrompt(u8string *str)
    {
        MudViewString*s = getselected();
        if (s) 
        {
            tstring text;
            s->getPrompt(&text);
            WideToUtf8 w2u;
            w2u.convert(text.c_str(), text.length());
            str->assign(w2u);
            return true;
        }
        return false;
    }

    bool getselected_block(int block, u8string* str)
    {
        PluginViewString* vs = getselected_pvs();
        if (!vs) return false;
        int size = vs->blocks.size();
        if (block >= 0 && block < size)
        {
            str->assign(vs->blocks[block]);
            return true;
        }
        return false;
    }

    void delete_selected()
    {
        if (selected >= 0 && selected < size())
        {
            delete plugins_strings[selected];
            plugins_strings.erase(plugins_strings.begin() + selected);
            parseDataStrings &ss = pdata->strings;
            delete ss[selected];
            ss.erase(ss.begin() + selected);
            if (selected == 0)
                pdata->update_prev_string = false;
        }
        selected = -1;
    }

private:
    void convert()
    {
        MemoryBuffer buffer(256);
        WideToUtf8Converter w2u;
        parseDataStrings &strings = pdata->strings;
        for (int i = 0, e = strings.size(); i < e; ++i)
        {
            MudViewString* src = strings[i];
            PluginViewString* dst = new PluginViewString;
            for (int j = 0, je = src->blocks.size(); j < je; ++j)
            {
                const tstring &text = src->blocks[j].string;
                w2u.convert(&buffer, text.c_str(), text.length());
                dst->blocks.push_back( (const char*)buffer.getData() );
            }
            plugins_strings.push_back(dst);
        }
    }

    void convert_back()
    {
        MemoryBuffer buffer(256);
        Utf8ToWideConverter u2w;
        parseDataStrings &strings = pdata->strings;
        for (int i = 0, e = strings.size(); i < e; ++i)
        {
            MudViewString* dst = strings[i];
            PluginViewString* src = plugins_strings[i];
            for (int j = 0, je = dst->blocks.size(); j < je; ++j)
            {
                const u8string &text = src->blocks[j];
                u2w.convert(&buffer, text.c_str(), text.length());
                dst->blocks[j].string.assign( (const wchar_t*)buffer.getData() );
            }
        }
    }
};
