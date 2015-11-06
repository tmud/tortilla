#include "stdafx.h"
#include "paramsHelper.h"

ParamsHelper::ParamsHelper(const tstring& param, bool include_anyid) : m_maxid(-1)
{
    pcre.setRegExp(!include_anyid ?  L"(%[0-9]){1}" : L"(%[0-9%]){1}");
    pcre.findAllMatches(param);
    for (int i=1,e=pcre.getSize(); i<e; ++i)
    {
        int pos = pcre.getFirst(i) + 1;
        tchar symbol = param.at(pos);
        if (symbol == L'%')
            { m_ids.push_back(-1); }
        else
        {
            int id = symbol - L'0';
            m_ids.push_back(id);
            if (id > m_maxid)
                m_maxid = id;
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

bool ParamsHelper::checkDoubles() const
{
    if (m_maxid == -1)
        return false;

    std::vector<int> indexes;
    indexes.resize(m_maxid+1, 0);
    for (int i=0,e=m_ids.size(); i<e; ++i)
    {
        int index = m_ids[i];
        if (index == -1) continue;
        if (indexes[index] != 0)
            return true;        
        indexes[index]++;
    }
    return false;
}
