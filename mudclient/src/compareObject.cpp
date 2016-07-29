#include "stdafx.h"
#include "compareObject.h"
#include "inputProcessor.h"

CompareObject::CompareObject() : m_fullstr_req(true), m_std_regexp(false) {}
CompareObject::~CompareObject() {}

bool CompareObject::init(const tstring& key, bool endline_mode)
{
    reset();
    if (key.empty())
        return false;

    m_key = key;
    if (key.at(0) == L'$')      // regexp marker
    {
       ParamsHelper ph(key, ParamsHelper::DETECT_ANYID);
       if (ph.getSize() == 0)
       {
           const tchar *k = key.c_str();
           //.*,[^, +,]+
           if (wcsstr(k, L".*") || wcsstr(k, L" +") || wcsstr(k, L"]+") || wcsstr(k, L"[^"))
           {
               if (m_pcre.setRegExp(key.substr(1), true))
               {
                   m_std_regexp = true;
                   return true;
               }
           }
       }
    }

    tstring regexp;
    createCheckPcre(key, endline_mode, &regexp);
    checkVars(&regexp);
    bool result = m_pcre.setRegExp(regexp, true);
    assert(result);
    return result;
}

bool CompareObject::initOnlyVars(const tstring& key)
{
    reset();
    if (key.empty())
       return false;
    tstring regexp(key);
    maskRegexpSpecialSymbols(&regexp, false);
    checkVars(&regexp);
    bool result = m_pcre.setRegExp(regexp, true);
    assert(result);
    return result;
}

bool CompareObject::compare(const tstring& str)
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
                InputVarsAccessor va;
                tstring value;
                tstring varname(v.c_str() + 1);
                if (!va.get(varname, &value))
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
    if (m_pcre.getSize() == 0)  { p.clear(); return; }

    if (m_std_regexp)
    {
        int count = m_pcre.getSize();
        p.resize(count);
        for (int i=0; i<count; ++i)
            m_pcre.getString(i, &p[i]);
        return;
    }

    ParamsHelper keys(m_key, ParamsHelper::BLOCK_DOUBLEID);
    int maxid = keys.getMaxId()+1;
    if (maxid <= 0)
        maxid = 1;
    p.resize(maxid);

    int begin =  m_pcre.getFirst(0);
    int end =  m_pcre.getLast(0);
    p[0] = m_str.substr(begin, end-begin);  //default value of %0 parameter

    // pcre find values of %1
    int pi = 1;
    for (int i=0,e=keys.getSize(); i<e; ++i)
    {
        int id = keys.getId(i);
        if (id == -1) continue;
        int begin = m_pcre.getFirst(pi);
        int end = m_pcre.getLast(pi);
        pi++;
        p[id] = m_str.substr(begin, end-begin);
    }
}

void CompareObject::createCheckPcre(const tstring& key, bool endline_mode, tstring *prce_template)
{
    tstring tmp(key);
    if (endline_mode)
    {
        tstring &k = tmp;
        int last = k.size() - 1;
        if (last != 0 && k.at(last) == L'$')
        {
            if (k.at(last-1) != L'$')
                m_fullstr_req = false;
            k = k.substr(0, last);
        }
    }

    //mask regexp special symbols
    maskRegexpSpecialSymbols(&tmp, true);

    // replace parameters, like %0 etc. to regexp
    int pos = 0; int len = tmp.length();
    ParamsHelper ph(tmp, ParamsHelper::DETECT_ANYID|ParamsHelper::BLOCK_DOUBLEID);
    for (int i=0,e=ph.getSize(); i<e; ++i)
    {
        prce_template->append(tmp.substr(pos, ph.getFirst(i) - pos));
        pos = ph.getLast(i);
        int id = ph.getId(i);
        if (pos == len)   // for last %x in string
            prce_template->append( (id == -1) ? L".*" : L"(.*)");
        else
            prce_template->append( (id == -1) ? L".*?" : L"(.*?)");

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
    for (int i=1, e=vars.getSize(); i<e; ++i)
    {
        int dp = vars.getFirst(i);
        if (dp > 1 && tmp.at(dp - 1) == L'\\' && tmp.at(dp - 2) != L'\\')
            continue;

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

bool CompareObject::isFullstrReq() const
{
    return m_fullstr_req;
}

void CompareObject::getKey(tstring* key) const
{
    key->assign(m_key);
}

void CompareObject::maskRegexpSpecialSymbols(tstring *pcre_template, bool use_first_arrow)
{
    //mask regexp special symbols
    tstring tmp;
    const tchar *symbols = L"*+/?|^$.[]()\\";
    const tchar *b = pcre_template->c_str();
    const tchar *e = b + pcre_template->length();

    // skip first ^ - it a part of regexp
    if (use_first_arrow && *b == '^')
    {
        tmp.append(L"^");
        b++;
    }

    const tchar* p = b + wcscspn(b, symbols);
    while (p != e)
    {
        bool skip_slash = false;
        tmp.append( tstring(b, p-b) );
        if (*p == L'$' && p+1!=e)
        {
            if (*(p + 1) == L'$') p++;
            else {  skip_slash = true; }
        }
        if (!skip_slash)
            tmp.append(L"\\");
        tchar x[2] = { *p, 0 };
        tmp.append(x);
        b = p + 1;
        p = b + wcscspn(b, symbols);
    }
    tmp.append(b);
    pcre_template->swap(tmp);
}

void CompareObject::reset()
{
    m_pcre.clear();
    m_key.clear();
    m_str.clear();
    m_vars_pcre_parts.clear();
    m_fullstr_req = true;
    m_std_regexp = false;
}
