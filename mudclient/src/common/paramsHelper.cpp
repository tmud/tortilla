#include "stdafx.h"
#include "paramsHelper.h"

Pcre16 ParamsHelper::cut;
bool ParamsHelper::m_cutinitialized = false;
ParamsHelper::ParamsHelper(const tstring& param, unsigned int mode) : m_maxid(-1)
{
    if (mode == EXTENDED)
        pcre.setRegExp(L"%(?:\\(.*\\))?[0-9]", true);
    else
      pcre.setRegExp( (mode & DETECT_ANYID) ?  L"(%[0-9%]){1}" : L"(%[0-9]){1}", true);
    pcre.findAllMatches(param);
    for (int i=1,e=pcre.getSize(); i<e; ++i)
    {
        tstring cut;
        int pos = pcre.getFirst(i) + 1;
        tchar symbol = param.at(pos);
        if (symbol == L'%')
            { m_ids.push_back(-1); continue; }
        else if (symbol == L'(')
        {
            int end = param.find(L')');
            cut.assign(param.substr(pos+1, end-pos-1));
            symbol = param.at(end+1);
        }
        int id = symbol - L'0';
        m_ids.push_back(id);
        if (id > m_maxid)
            m_maxid = id;
        if (!cut.empty())
            m_cuts[i-1] = cut;
    }
    if (m_maxid == -1)
        return;
    if (mode & BLOCK_DOUBLEID)
    {
        std::vector<int> indexes(m_maxid+1, 0);
        for (int i=m_ids.size()-1; i >= 0; --i)
        {
            int index = m_ids[i];
            if (index == -1) continue;
            if (indexes[index] != 0)
                m_ids[i] = -1;
            indexes[index]++;
        }
    }
}

int ParamsHelper::getSize() const 
{
    int size = pcre.getSize();
    return (size > 0) ? size-1 : 0;
}

int ParamsHelper::getFirst(int index) const
{
    return pcre.getFirst(index+1);
}

int ParamsHelper::getLast(int index) const 
{
    return pcre.getLast(index+1);
}

int ParamsHelper::getId(int index) const              // return index of parameter [0-9]
{
    return m_ids[index];
}

int ParamsHelper::getMaxId() const
{
    return m_maxid;
}

void ParamsHelper::cutParameter(int index, tstring* param)
{
    if (m_cuts.empty())
        return;
    std::map<int,tstring>::iterator it = m_cuts.find(index);
    if (it == m_cuts.end())
        return;
    if (!m_cutinitialized)
    {
        m_cutinitialized=true;
        cut.setRegExp(L"^(?:(-?[0-9]+),)?(-?[0-9]+)$", true);
    }
    cut.find(it->second);
    int sz = cut.getSize();
    if (sz != 3)
    {
        tstring skipped(L"%("), id;
        skipped.append(it->second);
        skipped.append(L")");
        int2w(getId(index), &id);
        skipped.append(id);
        param->assign(skipped);
        return;
    }
    tstring from, len;
    cut.getString(1, &from);
    cut.getString(2, &len);
    int f = 0; int l = 0;
    if (!from.empty())
    {
        w2int(from, &f);
        if (f <= 0) { f = 0; }
        else f = f - 1;
    }
    w2int(len, &l);
    if (l == 0)
        { param->clear(); return; }
    if (l < 0)
    {
        l = -l;
        int param_len = param->length();
        if (param_len <= l)
            { param->clear(); return; }
        l = param_len - l;
        tstring tmp(param->substr(0, l));
        int tmp_len = tmp.length();
        if (f < tmp_len)
            param->assign(tmp.substr(f));
        else
            param->clear();
        return;
    }
    int param_len = param->length();
    if (f < param_len)
    {
        tstring tmp(param->substr(f, l));
        param->swap(tmp);
    }
    else
    {
        param->clear();
    }
}
