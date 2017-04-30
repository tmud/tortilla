#include "stdafx.h"
#include "paramsHelper.h"

Pcre16 ParamsHelper::pcre;
Pcre16 ParamsHelper::cut;
bool ParamsHelper::m_static_init = false;
ParamsHelper::ParamsHelper(const tstring& param, bool block_doubles)
{
    init(param, block_doubles, NULL);
}

ParamsHelper::ParamsHelper(const tstring& param, bool block_doubles, tstring *param_without_cuts)
{
    init(param, block_doubles, param_without_cuts);
}

void ParamsHelper::init(const tstring& param, bool block_doubles, tstring *nocuts)
{
    m_maxid = -1;
    if (!m_static_init) {
        m_static_init = true;
        pcre.setRegExp(L"%(?:\\([^%]+\\)[0-9%]|[0-9%])", true);
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
            int end = pv.last-1;
            pv.cut.assign(param.substr(pos+1, end-pos-2));
            symbol = param.at(end);
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
    if (nocuts) {
        if (m_ids.empty())
            nocuts->assign(param);
        else {
          nocuts->assign(param.substr(0, m_ids[0].first+1));
          int last = m_ids.size() - 1;
          for (int i=0;i<last;++i) {
              int from = m_ids[i].last-1;
              int to =  m_ids[i+1].first+1;
              nocuts->append(param.substr(from, to-from));
          }
          nocuts->append(param.substr(m_ids[last].last-1));
        }
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

const tstring& ParamsHelper::getCutValue(int index) const
{
    return m_ids[index].cut;
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

#ifdef _DEBUG
bool ParamsHelperUnitTests::testCutValue(ParamsHelper& ph, int index, const tchar* value) {
    const tstring& v = ph.getCutValue(index);
    return (v.compare(value) == 0);
}

bool ParamsHelperUnitTests::testCutParameter(ParamsHelper& ph, int index, const tchar* srcparam, const tchar* testparam){
    tstring p(srcparam);
    ph.cutParameter(index, &p);
    return (p.compare(testparam) == 0);
}

void ParamsHelperUnitTests::run()  {
    tstring cuts;
    ParamsHelper h1(L"%1 %(dfgdf) %2 %(fff)3", true, &cuts);
    assert(h1.getSize() == 3);
    assert(testCutValue(h1, 0, L""));
    assert(testCutValue(h1, 1, L""));
    assert(testCutValue(h1, 2, L"fff"));
    assert(cuts == L"%1 %(dfgdf) %2 %3");
    assert(h1.getId(0) == 1);
    assert(h1.getId(1) == 2);
    assert(h1.getId(2) == 3);

    cuts.clear();
    ParamsHelper h2(L"%(dfgdfg)1 %2 fff %1", true, &cuts);
    assert(h2.getSize() == 3);
    assert(testCutValue(h2, 0, L"dfgdfg"));
    assert(testCutValue(h2, 1, L""));
    assert(testCutValue(h2, 2, L""));
    assert(cuts == L"%1 %2 fff %1");
    assert(h2.getId(0) == -1);
    assert(h2.getId(1) == 2);
    assert(h2.getId(2) == 1);

    cuts.clear();
    ParamsHelper h3(L"%(dfgdfg)1 %2 fff %1", false, &cuts);
    assert(cuts == L"%1 %2 fff %1");
    assert(h3.getId(0) == 1);
    assert(h3.getId(1) == 2);
    assert(h3.getId(2) == 1);

    cuts.clear();
    ParamsHelper h4(L"%(%fgfg)2 %(doit checkit)% %($var)4 ggg", true, &cuts);
    assert(h4.getSize() == 2);
    assert(h4.getMaxId() == 4);
    assert(testCutValue(h4, 0, L"doit checkit"));
    assert(testCutValue(h4, 1, L"$var"));
    assert(h4.getId(0) == -1);
    assert(h4.getId(1) == 4);

    ParamsHelper h5(L"%(-1)1 abc", true);
    assert(testCutParameter(h5, 0, L"тест", L"тес"));

    ParamsHelper h6(L"%(5)1 abc", true);
    assert(testCutParameter(h6, 0, L"тест", L"тест"));

    ParamsHelper h7(L"%(2)1 abc", true);
    assert(testCutParameter(h7, 0, L"тест", L"те"));

    ParamsHelper h8(L"%(2,2)1 abc", true);
    assert(testCutParameter(h8, 0, L"тecт", L"ec"));

    ParamsHelper h9(L"%(3,-2)1 abc", true);
    assert(testCutParameter(h9, 0, L"тест", L""));

    ParamsHelper h10(L"%(2,-2)1 abc", true);
    assert(testCutParameter(h10, 0, L"тест", L"е"));

    ParamsHelper h11(L"abc", true);
    assert(h11.getSize() == 0);

    ParamsHelper h12(L"abc %%", true);
    assert(h12.getSize() == 1);
}
#endif