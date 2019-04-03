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

struct PluginTriggerString
{
    std::vector<u8string> params;
};

class PluginsParseData
{
public:
    MemoryBuffer buffer;
    parseData *pdata;
    triggerParseData *tdata;
    std::vector<PluginViewString*> plugins_strings;
    int selected;
public:
    PluginsParseData(parseData *data, triggerParseData *trdata) : buffer(256), pdata(data), tdata(trdata), selected(-1) { convert(); }
    ~PluginsParseData() { convert_back(); std::for_each(plugins_strings.begin(), plugins_strings.end(), [](PluginViewString*s) { delete s; }); }
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

    bool getselected_len(int *len)
    {
        assert(len);
        PluginViewString *str = getselected_pvs();
        if (!str)
            return false;
        int s = 0;
        for (int i = 0, e = str->blocks.size(); i < e; ++i)
            s += u8string_len(str->blocks[i]);
        *len = s;
        return true;
    }

    bool getselected_sympos(int symbol, std::pair<int, int>* blockpos)
    {
        assert(blockpos);
        PluginViewString *str = getselected_pvs();
        int block = 0; int pos = 0;
        if (str && symbol > 0)
        {
            symbol -= 1;
            for (int i = 0, e = str->blocks.size(); i < e; ++i)
            {
                int size = u8string_len(str->blocks[i]);
                if (size > symbol)
                {
                    blockpos->first = i + 1;
                    blockpos->second = symbol + 1;
                    return true;
                }
                symbol -= size;
            }
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
            MudViewString *s = ss[selected];
            int last = ss.size() - 1;
            if (selected != last)
                ss[selected+1]->prev = s->prev;
            if (selected != 0)
                ss[selected-1]->next = s->next;
            delete s;
            ss.erase(ss.begin() + selected);
            if (tdata)
                tdata->markDeleted(selected);
            if (ss.empty() || selected == last)
                pdata->last_finished = true;
        }
        selected = -1;
    }

    void delete_strings(const std::vector<int>& strings)
    {
        for (int i=0,e=strings.size(); i<e; ++i)
        {
            selected = strings[i]-1;
            delete_selected();
        }
    }

    void deleteall()
    {
        std::for_each(plugins_strings.begin(), plugins_strings.end(), [](PluginViewString*s) {delete s;});
        plugins_strings.clear();
        pdata->clear();
        if (tdata)
            tdata->markDeletedAll();
        selected = -1;
    }

    void insert_new_string(bool gamecmd, bool system, int delta)
    {
        if (isselected())
        {
            plugins_strings.insert(plugins_strings.begin() + selected + delta, new PluginViewString);
            parseDataStrings &ss = pdata->strings;
            MudViewString *s = new MudViewString;
            s->gamecmd = gamecmd;
            s->system = system;
            ss.insert(ss.begin() + selected + delta, s);
            s->changed = true;
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
                dst->changed = true;
                return true;
            }
        }
        return false;
    }

    bool isselected() const
    {
        return (selected >= 0 && selected < size()) ? true : false;
    }

    int get_params()
    {
        if (tdata && isselected())
            return tdata->getParameters(selected);
        return 0;
    }

    bool get_key(tstring *key)
    {
        if (tdata && isselected())
            return tdata->getKey(selected, key);
        return false;
    }

    bool get_parameter(int index, tstring* param)
    {
        if (tdata && isselected())
            return tdata->getParameter(selected, index, param);
        return false;
    }

    bool translate(tstring *string)
    {
        if (tdata && isselected())
        {
           InputTranslateParameters tp;
           TriggerParseDataParameters params(tdata, selected);
           tp.doit(&params, string);
           return true;
        }
        return false;
    }
    
    bool replace(const tstring& string)
    {
        if (tdata && isselected())
        {
            CompareRange range;
            if (tdata->getCompareRange(selected, &range))
            {
                MudViewString *s = getselected();
                CompareData cdata(s);
                int pos = cdata.fold(range);
                if (pos == -1) return false;
                MudViewStringBlock &b = s->blocks[pos];
                if (string.empty())
                    s->blocks.erase(s->blocks.begin() + pos);
                else
                    b.string = string;
                PluginViewString *pvs = getselected_pvs();
                convert(s, pvs);
                s->changed = true;
                s->subs_processed = true;
                if (s->getTextLen() == 0) {
                    s->dropped = true;
                    s->show_dropped = false;
                }
                return true;
            }
        }
        return false;
    }

    bool color(const tstring& color)
    {
        if (tdata && isselected())
        {
           // return tdata->color(string);
        }
        return false;
    }

    enum StringChanged { ISC_UNKNOWN = 0, ISC_NOTCHANGED, ISC_CHANGED };
    StringChanged is_changed()
    {
        if (tdata && isselected())
        {
            MudViewString*s = getselected();
            tstring md5, crc;
            s->getMd5(&md5);
            tdata->getCRC(selected, &crc);
            return (md5 == crc) ? ISC_NOTCHANGED : ISC_CHANGED;
        }
        return ISC_UNKNOWN;
    }
    void synctexts() {
        convert_back();
    }
private:
    void convert()
    {
        parseDataStrings &strings = pdata->strings;
        for (int i = 0, e = strings.size(); i < e; ++i)
        {
            MudViewString* src = strings[i];
            PluginViewString* dst = new PluginViewString;
            convert(src, dst);
            plugins_strings.push_back(dst);
        }
        if (!strings.empty())
            selected = 0; // select first string
    }

    void convert(MudViewString* src, PluginViewString* dst)
    {
        WideToUtf8Converter w2u;
        dst->blocks.resize(src->blocks.size());
        for (int j = 0, je = src->blocks.size(); j < je; ++j)
        {
            const tstring &text = src->blocks[j].string;
            w2u.convert(&buffer, text.c_str(), text.length());
            dst->blocks[j].assign(buffer.getData());
        }
    }

    void convert_back()
    {
        Utf8ToWideConverter u2w;
        parseDataStrings &strings = pdata->strings;
        for (int i = 0, e = strings.size(); i < e; ++i)
        {
            MudViewString* dst = strings[i];
            PluginViewString* src = plugins_strings[i];
            int src_size = src->blocks.size();
            for (int j = 0, je = dst->blocks.size(); j < je; ++j)
            {
                if (j >= src_size)
                    break;
                const u8string &text = src->blocks[j];
                u2w.convert(&buffer, text.c_str(), text.length());
                dst->blocks[j].string.assign( (const wchar_t*)buffer.getData() );
            }
        }
    }
};
