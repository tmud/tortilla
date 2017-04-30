#include "stdafx.h"
#include "highlightHelper.h"

HighlightHelperImpl HighlightHelper::m_impl;
bool HighlightHelper::translateColor(tstring* param)
{
    return m_impl.checkText(param);
}

Pcre16 SubHighlightHelper::m_regexp;
SubHighlightHelper::SubHighlightHelper(const tstring& s)
{
    if (!m_regexp.valid())
        m_regexp.setRegExp(L"(?:^| )['{\"][^{}'\"]*?['}\"] ", true);
    m_regexp.findAllMatches(s);
    int count = m_regexp.getSize();
    if (count == 0) {
        parts.push_back(s);
        return;
    }
    int begin = 0;
    for (int i=1; i<count; ++i)
    {
        int from = m_regexp.getFirst(i);
        if (begin != from)
            parts.push_back( s.substr(begin, from-begin) );
        tstring p;
        m_regexp.getString(i, &p);
        parts.push_back( p );
        begin = m_regexp.getLast(i);
    }
    int last = m_regexp.getLast(count - 1);
    parts.push_back( s.substr(last) );
}

int SubHighlightHelper::size() const
{
    return parts.size();
}

const tstring& SubHighlightHelper::get(int index)
{
    return parts[index];
}

bool SubHighlightHelper::trimColor(int index, tstring* trimmed)
{
    const tstring& p = parts[index];
    if (p.length() < 4)
        return false;
    if (p.at(0) != ' ')
        trimmed->assign ( p.substr(1, p.length()-3) );
    else
        trimmed->assign ( p.substr(2, p.length()-4) );
    return true;
}
