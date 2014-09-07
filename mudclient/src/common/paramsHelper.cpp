#include "stdafx.h"
#include "paramsHelper.h"

ParamsHelper::ParamsHelper(const tstring& param) : m_maxid(-1)
{
    pcre.setRegExp(L"(%[0-9]){1}");
    pcre.findAllMatches(param);

    for (int i=0,e=pcre.getSize(); i<e; ++i)
    {
        int pos = pcre.getFirst(i) + 1;
        WCHAR symbol = param.at(pos);
        int id = symbol - L'0';
        m_ids.push_back(id);
        if (id > m_maxid)
            m_maxid = id;
    }
}

int ParamsHelper::getSize() const 
{
    return pcre.getSize();
}

int ParamsHelper::getFirst(int index) const
{
    return pcre.getFirst(index);
}

int ParamsHelper::getLast(int index) const 
{
    return pcre.getLast(index);
}

int ParamsHelper::getId(int index) const              // return index of parameter [0-9]
{
    return m_ids[index];
}

int ParamsHelper::getMaxId() const
{
    return m_maxid;
}

bool ParamsHelper::checkDoubles() const
{
    if (m_maxid == -1)
        return false;

    std::vector<int> indexes;
    indexes.resize(m_maxid+1, 0);
    for (int i=0,e=m_ids.size(); i<e; ++i)
    {
        int index = m_ids[i];
        if (indexes[index] != 0)
            return true;        
        indexes[index]++;
    }
    return false;
}
