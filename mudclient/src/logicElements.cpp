#include "stdafx.h"
#include "logicElements.h"

extern PropertiesData* _pdata;
class CompareVarHelper : public CompareVar
{
    PropertiesData *m_pdata;
public:
    CompareVarHelper(PropertiesData *pdata) : m_pdata(pdata)  {}
    bool get(const tstring& var, tstring* value) const
    {
        int index = m_pdata->variables.find(var);
        if (index == -1) return false;
        const tstring& current_value = m_pdata->variables.get(index).value;
        value->assign(current_value);
        return true;
    }
};


CompareData::CompareData(MudViewString *s) : string(s), start(0)
{
    reinit();
}

void CompareData::reinit()
{
    fullstr.clear();
    std::vector<MudViewStringBlock> &vb = string->blocks;
    for (int i=start,e=vb.size(); i<e; ++i)          
       fullstr.append(vb[i].string);
}

void CompareData::del(CompareRange& range)
{
    if (!cut(range))
        return;
    std::vector<MudViewStringBlock> &vb = string->blocks;
    vb.erase(vb.begin()+range.begin, vb.begin()+range.end+1);
}

int CompareData::fold(CompareRange& range)
{
    if (!cut(range))
        return -1;
    if (range.begin != range.end)
    {
        std::vector<MudViewStringBlock> &vb = string->blocks;
        MudViewStringBlock &b = vb[range.begin];
        for (int i=range.begin+1; i<=range.end; ++i)    
            b.string.append(vb[i].string);
        vb.erase(vb.begin()+range.begin+1, vb.begin()+range.end+1);
    }
    return range.begin;
}

bool CompareData::cut(CompareRange& range)
{
    int size = fullstr.length();
    if (range.begin >=0 && range.begin < size &&
        range.end > 0 && range.end <= size)
    {
        range.begin = cutpos(range.begin, 0);    
        range.end = cutpos(range.end, 1);
        return true;
    }   
    return false;
}

bool CompareData::find(CompareRange& range)
{
    int size = fullstr.length();
    if (range.begin >=0 && range.begin < size &&
        range.end > 0 && range.end <= size)
    {
        range.begin = findpos(range.begin, 0);    
        range.end = findpos(range.end, 1);
        return true;
    }
    return false;
}

int CompareData::cutpos(int pos, int d)
{
    std::vector<MudViewStringBlock> &vb = string->blocks;
    int p = 0; int bi = start; pos -= d;
    for (int i=start,e=vb.size(); i<e; ++i)
    {
        MudViewStringBlock& b = vb[i];
        int len = b.string.length();
        int last = p + len - 1;
        if (pos >= p && pos <= last)
            { bi = i; p = pos - p; break; }
        p = p + len;
    }

    if (p == 0 && d == 0)       // begin dir first symbol
        return bi;
    MudViewStringBlock& b = vb[bi];
    int last = b.string.length()-1;
    if (p == last && d == 1)    // end dir last symbol
        return bi;

    MudViewStringBlock nb;
    nb.string = b.string.substr(p+d);
    b.string = b.string.substr(0, p+d);
    nb.params = b.params;
    nb.subs_protected = b.subs_protected;
    vb.insert(vb.begin()+(bi+1), nb);
    return bi+(1-d);
}

int CompareData::findpos(int pos, int d)
{
    std::vector<MudViewStringBlock> &vb = string->blocks;
    int p = 0; int bi = start; pos -= d;
    for (int i=start,e=vb.size(); i<e; ++i)
    {
        MudViewStringBlock& b = vb[i];
        int len = b.string.length();
        int last = p + len - 1;
        if (pos >= p && pos <= last)
            { bi = i; break; }
        p = p + len;
    }
    return bi;
}

Alias::Alias(const property_value& v) : m_key(v.key), m_cmd(v.value)
{
}

bool Alias::processing(const tstring& key, tstring *newcmd)
{
    if (key == m_key)
        { newcmd->assign(m_cmd); return true; }
    return false;
}

Hotkey::Hotkey(const property_value& v) : m_key(v.key), m_cmd(v.value)
{
}

bool Hotkey::processing(const tstring& key, tstring *newcmd)
{
    if (key == m_key)
        { newcmd->assign(m_cmd); return true; }
    return false;
}

Action::Action(const property_value& v) : m_value(v.value)
{
    m_compare.init(v.key);
}

bool Action::processing(CompareData& data, tstring* newcmd)
{    
    CompareVarHelper h(_pdata);
    if (!m_compare.checkToCompare(data.fullstr, &h))
        return false;
    
    std::vector<tstring> params;            // values of params in key
    m_compare.getParameters(&params);
    int params_count = params.size();

    // parse value and generate result
    int pos = 0;
    ParamsHelper values(m_value);

    newcmd->clear();
    for (int i=0,e=values.getSize(); i<e; ++i)
    {
        newcmd->append( m_value.substr(pos, values.getFirst(i)-pos) );
        int id = values.getId(i);
        if (id < params_count)
            newcmd->append(params[id]);            
        pos = values.getLast(i);        
    }
    newcmd->append(m_value.substr(pos));

    // drop mode -> change source MudViewString
    if (m_value.find(L"drop") != tstring::npos)
    {
        CompareRange range;
        m_compare.getRange(&range);
        data.del(range);
        if (data.string->blocks.empty())
            data.string->dropped = true;
    }
    return true;
}

Sub::Sub(const property_value& v) : m_value(v.value)
{
    m_compare.init(v.key);
}

bool Sub::processing(CompareData& data)
{
    CompareVarHelper h(_pdata);
    if (!m_compare.checkToCompare(data.fullstr, &h))
        return false;

    CompareRange range;
    m_compare.getRange(&range);

    CompareRange check(range);
    if (!data.find(check))
        return false;

    std::vector<MudViewStringBlock> &b = data.string->blocks;
    for (int i=check.begin; i<=check.end; ++i)
    {
        if (b[i].subs_protected)
            return false;
    }

    int pos = data.fold(range);
    if (pos == -1) return false;
    data.string->blocks[pos].string = m_value;
    data.start = pos+1;
    return true;
}

AntiSub::AntiSub(const property_value& v)
{
    m_compare.init(v.key);
}

bool AntiSub::processing(CompareData& data)
{
    CompareVarHelper h(_pdata);
    if (!m_compare.checkToCompare(data.fullstr, &h))
        return false;

    CompareRange range;
    m_compare.getRange(&range);
    if (!data.cut(range))
        return false;

    for (int i=range.begin; i<=range.end; ++i)
        data.string->blocks[i].subs_protected = 1;

    data.start = range.end+1;
    return true;
}

Gag::Gag(const property_value& v)
{
    m_compare.init(v.key);
}

bool Gag::processing(CompareData& data)
{
    CompareVarHelper h(_pdata);
    if (!m_compare.checkToCompare(data.fullstr, &h))
        return false;

    CompareRange range;
    m_compare.getRange(&range);

    CompareRange check(range);
    if (!data.find(check))
        return false;

    std::vector<MudViewStringBlock> &b = data.string->blocks;
    for (int i=check.begin; i<=check.end; ++i)
    {
        if (b[i].subs_protected)
            return false;
    }

    data.del(range);        
    data.start = range.end+1;
    return true;
}

Highlight::Highlight(const property_value& v)
{
    m_compare.init(v.key);
    m_hl.convertFromString(v.value);
}

bool Highlight::processing(CompareData& data)
{
    CompareVarHelper h(_pdata);
    if (!m_compare.checkToCompare(data.fullstr, &h))
        return false;
  
    CompareRange range;
    m_compare.getRange(&range);
    int pos = data.fold(range);
    if (pos == -1) return false;
    MudViewStringParams &p = data.string->blocks[pos].params;
    p.use_ext_colors = 1;
    p.ext_text_color = m_hl.textcolor;
    p.ext_bkg_color = m_hl.bkgcolor;
    p.underline_status = m_hl.underlined;
    p.blink_status = m_hl.border;
    p.italic_status = m_hl.italic;
    data.start = pos + 1;
    return true;
}

Timer::Timer() : timer(0), period(0)
{
}

void Timer::init(const property_value& v)
{
    id.assign(v.key);
    PropertiesTimer pt;
    pt.convertFromString(v.value);
    cmd.assign(pt.cmd);

    int t = _wtoi(pt.timer.c_str());
    if (t < 0)
        t = 0;

    timer = 0;
    period = t * 1000;
}

bool Timer::tick(int dt)
{
    if (period == 0)
        return false;

    timer += dt;
    if (timer < period)
        return false;

    timer -= period;    
    return true;
}

void Timer::reset()
{
    timer = 0;
}
