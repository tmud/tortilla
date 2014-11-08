#pragma once

#include "MudViewString.h"

struct PluginViewString
{
    std::vector<u8string> blocks;
    void getText(u8string *text) const
    {
        for (int i = 0, e = blocks.size(); i < e; ++i)
            text->append(blocks[i]);
    }
    int getTextLen() const
    {
        int len = 0;
        for (int i = 0, e = blocks.size(); i < e; ++i)
            len += u8string_len(blocks[i]);
        return len;
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
    int getindex() const { return selected+1; }
    bool select(int index)
    {
        if (index >= 1 && index <= size()) { selected = index-1; return true; }
        return false;
    }
    
    PluginViewString* getselected_pvs()
    {
        return (isselected()) ? plugins_strings[selected] : NULL;
    }

    MudViewString* getselected()
    {
        return (isselected()) ? pdata->strings[selected] : NULL;
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
        if (block >= 1 && block <= size)
        {
            str->assign(vs->blocks[block-1]);
            return true;
        }
        return false;
    }

    void delete_selected()
    {
        if (isselected())
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

    void insert_new_string()
    {
        if (isselected())
        {
            plugins_strings.insert(plugins_strings.begin() + selected + 1, new PluginViewString);
            parseDataStrings &ss = pdata->strings;
            ss.insert(ss.begin() + selected + 1, new MudViewString);
        }
    }

    bool copy_block(int block, int dst_string, int dst_block)
    {        
        PluginViewString *src_pvs = getselected_pvs();
        MudViewString *src = getselected();
        if (src_pvs && dst_block >= 1)
        {
            int blocks = src_pvs->blocks.size();
            int strings = plugins_strings.size();
            if (block >= 1 && block <= blocks && dst_string >= 1 && dst_string <= strings)
            {
                PluginViewString *dst_pvs = plugins_strings[dst_string-1];
                MudViewString *dst = pdata->strings[dst_string-1];
                int blocks2 = dst_pvs->blocks.size();
                if (dst_block > blocks2)
                {
                    int count = dst_block - blocks2;
                    for (; count > 0; --count)
                    {
                        dst_pvs->blocks.push_back(u8string());
                        dst->blocks.push_back(MudViewStringBlock());
                    }
                }
                dst_pvs->blocks[dst_block - 1] = src_pvs->blocks[block - 1];
                dst->blocks[dst_block - 1] = src->blocks[block - 1];
                return true;
            }
        }
        return false;
    }

    bool isselected() const
    {
        return (selected >= 0 && selected < size()) ? true : false;
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
