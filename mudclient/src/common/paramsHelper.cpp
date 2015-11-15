#include "stdafx.h"
#include "paramsHelper.h"

ParamsHelper::ParamsHelper(const tstring& param, unsigned int mode) : m_maxid(-1)
{
    pcre.setRegExp( (mode & DETECT_ANYID) ?  L"(%[0-9%]){1}" : L"(%[0-9]){1}");
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
