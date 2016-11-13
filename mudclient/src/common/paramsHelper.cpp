#include "stdafx.h"
#include "paramsHelper.h"

Pcre16 ParamsHelper::pcre;
Pcre16 ParamsHelper::cut;
bool ParamsHelper::m_static_init = false;
ParamsHelper::ParamsHelper(const tstring& param, bool block_doubles) : m_maxid(-1)
{
    if (!m_static_init) {
        m_static_init = true;
        pcre.setRegExp(L"%(?:\\(.*\\))?[0-9%]", true);
        cut.setRegExp(L"^(?:(-?[0-9]+),)?(-?[0-9]+)$", true);
    }
    pcre.findAllMatches(param);
    for (int i=1,e=pcre.getSize(); i<e; ++i)
    {
        param_values pv;
        pv.first = pcre.getFirst(i);
        pv.last = pcre.getLast(i);
        int pos = pv.first + 1;
        tchar symbol = param.at(pos);
        if (symbol == L'(')
        {
            int end = param.find(L')');
            pv.cut.assign(param.substr(pos+1, end-pos-1));
            symbol = param.at(end+1);
        }
        if (symbol == L'%')
            pv.id = -1;
        else
        {
            pv.id = symbol - L'0';
            if (pv.id > m_maxid)
                m_maxid = pv.id;
        }
        m_ids.push_back(pv);
    }
    if (m_maxid == -1)
        return;
    if (block_doubles)
    {
        std::vector<int> indexes(m_maxid+1, 0);
        for (int i=m_ids.size()-1; i >= 0; --i)
        {
            int index = m_ids[i].id;
            if (index == -1) continue;
            if (indexes[index] != 0)
                m_ids[i].id = -1;
            indexes[index]++;
        }
    }
}

int ParamsHelper::getSize() const 
{
    return m_ids.size();
}

int ParamsHelper::getFirst(int index) const
{
    return m_ids[index].first;
}

int ParamsHelper::getLast(int index) const 
{
    return m_ids[index].last;
}

int ParamsHelper::getId(int index) const              // return index of parameter [0-9]
{
    return m_ids[index].id;
}

void ParamsHelper::getCutValue(int index, tstring* cutvalue)
{
    cutvalue->assign(m_ids[index].cut);
}

int ParamsHelper::getMaxId() const
{
    return m_maxid;
}

void ParamsHelper::cutParameter(int index, tstring* param)
{
    const tstring& cutvalue = m_ids[index].cut;
    if (cutvalue.empty())
        return;
    cut.find(cutvalue);
    int sz = cut.getSize();
    if (sz != 3)
    {
        tstring skipped(L"%("), id;
        skipped.append(cutvalue);
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
