#include "stdafx.h"
#include "compareObject.h"

CompareObject::CompareObject() {}
CompareObject::~CompareObject() {}

bool CompareObject::init(const tstring& key)
{
    ParamsHelper ph(key);
    if (ph.checkDoubles())
        return false;
    m_key = key;

    tstring regexp;
    createCheckPcre(key, &regexp);
    checkVars(&regexp);
    bool result = m_pcre.setRegExp(regexp, true);
    assert(result);   
    return result;
}

bool CompareObject::checkToCompare(const tstring& str, const CompareVar* object)
{
    if (!m_vars_pcre_parts.empty())
    {
        // make regexp for comparing
        tstring regexp;
        for (int i = 0, e = m_vars_pcre_parts.size(); i < e; ++i)
        {
            const tstring& v = m_vars_pcre_parts[i];
            if (v.at(0) != '$')
                regexp.append(v);
            else
            {
                tstring value;
                if (!object->get(v.c_str() + 1, &value))
                    return false;
                regexp.append(value);
            }
        }
        bool result = m_pcre.setRegExp(regexp, true);
        assert(result);
        if (!result)
            return false;        
    }
    m_pcre.find(str);
    if (m_pcre.getSize() == 0)
        return false;
    m_str = str;
    return true;
}

void CompareObject::getParameters(std::vector<tstring>* params) const
{
    assert(params);    
    std::vector<tstring> &p = *params;
    int size = m_pcre.getSize();
    if (size == 0)  { p.clear(); return; }

    ParamsHelper keys(m_key);
    int maxid = keys.getMaxId()+1;
    if (maxid <= 0)
        maxid = 1;
    p.resize(maxid);
    
    int begin =  m_pcre.getFirst(0);
    int end =  m_pcre.getLast(0);
    p[0] = m_str.substr(begin, end-begin);  //default value of %0 parameter

    // pcre find values of %1
    for (int i=0,e=size-1; i<e; ++i)
    {
        int id = keys.getId(i);
        int begin = m_pcre.getFirst(i+1);
        int end = m_pcre.getLast(i+1);
        p[id] = m_str.substr(begin, end-begin);
    }
}

void CompareObject::createCheckPcre(const tstring& key, tstring *prce_template)
{
    //mask regexp special symbols
    tstring tmp;
    const WCHAR *symbols = L"*+/?|^$.[]()\\";
    const WCHAR *b = key.c_str();
    const WCHAR *e = b + key.length();

    // skip first ^ - it a part of regexp
    if (*b == '^')
    {
        tmp.append(L"^");
        b++;
    }

    const WCHAR* p = b + wcscspn(b, symbols);
    while (p != e)
    {
        tmp.append( tstring(b, p-b) );
        tmp.append(L"\\");
        WCHAR x[2] = { *p, 0 };
        tmp.append(x);
        b = p + 1;
        p = b + wcscspn(b, symbols);
    }
    tmp.append(b);

    // replace parameter %% to regexp .* (all combination of symbols)
    tstring_replace(&tmp, L"%%", L".*");

    // replace parameters, like %0 etc. to regexp
    int pos = 0;
    ParamsHelper ph(tmp);
    for (int i=0,e=ph.getSize(); i<e; ++i)
    {
        prce_template->append(tmp.substr(pos, ph.getFirst(i) - pos));
        pos = ph.getLast(i);
        tstring flag(tmp.substr(pos,1));
        if (flag == L"%") // %x% variant
        {
            int last = tmp.size() - 1;
            if (pos == last) // if %x% is last in string
                prce_template->append(L"(.*)");
            else
                prce_template->append(L"(.*?)"); 
            pos++;
        }
        else              // %x variant
          { prce_template->append(L"([^ ]+)"); }
    }
    prce_template->append(tmp.substr(pos));
}

void CompareObject::checkVars(tstring *pcre_template)
{
    //find vars like $var
    Pcre16 vars;
    vars.setRegExp(L"\\$[a-zA-Z0-9_]+");
    vars.findAllMatches(*pcre_template);
    if (vars.getSize() == 0)
        return;

    std::vector<tstring> vars_list;
    tstring tmp(*pcre_template);
    pcre_template->clear();
    int pos = 0;
    for (int i = 0, e = vars.getSize(); i < e; ++i)
    {
        pcre_template->append(tmp.substr(pos, vars.getFirst(i) - pos));
        int first = vars.getFirst(i);
        int last = vars.getLast(i);
        vars_list.push_back( tmp.substr(first, last - first) );
        if (last == tmp.size())
            pcre_template->append(L"(?:.*)");
        else
            pcre_template->append(L"(?:.*?)");
        pos = last;
    }
    pcre_template->append(tmp.substr(pos));

    // make new regexp template for vars processing
    tstring vars_template(*pcre_template);
    tstring_replace(&vars_template, L"(?:.*?)", L"(?var)");
    tstring_replace(&vars_template, L"(?:.*)", L"(?var)");

    // make array for future pcre constructor
    int index = 0;
    const tchar* b = vars_template.c_str();
    const tchar* e = b + vars_template.length();
    while (b < e)
    {
        const tchar* p = wcsstr(b, L"(?var)");
        if (!p)
        {
            if (b != e)
                m_vars_pcre_parts.push_back(b); 
            break; 
        }
        else
        {
            if (p != b)
                m_vars_pcre_parts.push_back( tstring(b, p-b) ); 
        }
        m_vars_pcre_parts.push_back(vars_list[index++]);
        b = p + 6; // len (?var)        
    }  
}

void CompareObject::getRange(CompareRange *range) const
{ 
    if (m_pcre.getSize() == 0)
        return;
    range->begin = m_pcre.getFirst(0);
    range->end = m_pcre.getLast(0); 
}
