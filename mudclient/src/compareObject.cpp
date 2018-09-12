#include "stdafx.h"
#include "compareObject.h"
#include "inputProcessor.h"

class CutRegexp {
public:
    static Pcre16& get() {
        static Pcre16 rxp;
        if (!rxp.valid()) {
            rxp.setRegExp(L"\\$[a-zA-Z_][0-9a-zA-Z_.]*\\*?;?", true);
        }
        return rxp;
    }
private:
    CutRegexp() {}
    ~CutRegexp() {}
    CutRegexp(CutRegexp const&);
    CutRegexp& operator= (CutRegexp const&);
};

CompareObject::CompareObject() : m_pkey_helper(NULL), m_fullstr_req(true), m_std_regexp(false), 
m_first_symbol(0), m_second_symbol(0){}
CompareObject::~CompareObject() { delete m_pkey_helper; }
CompareObject::CompareObject(const CompareObject& co) { cthis(co); }
CompareObject& CompareObject::operator=(const CompareObject& co) { cthis(co); return *this; }
void CompareObject::cthis(const CompareObject& co) {
    tstring regexp;
    co.m_pcre.getRegexp(&regexp);
    m_pcre.setRegExp(regexp, true);
    m_key = co.m_key;
    m_fullstr_req = co.m_fullstr_req;
    m_std_regexp = co.m_std_regexp;
    m_pkey_helper = NULL;
    m_first_symbol = co.m_first_symbol;
    m_second_symbol = co.m_second_symbol;
}
bool CompareObject::init(const tstring& key, bool endline_mode)
{
    reset();
    if (key.empty())
        return false;

    m_key = key;

    const ParamsHelper& keys = getKeyHelper();
    if (key.at(0) == L'$' && keys.getSize() == 0)      // regexp marker, нет %0, %1 и т.д.
    {
        tstring regexp(key.substr(1));
        checkVars(&regexp);
        if (m_pcre.setRegExp(regexp, true))
        {
            m_std_regexp = true;
            return true;
       }
    }

    tstring regexp;
    checkFirstSymbols(m_key_nocuts);
    createCheckPcre(m_key_nocuts, endline_mode, &regexp);
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
    if (!compareFirstSymbols(str)) // experemental
        return false;
    m_pcre.find(str);
    if (m_pcre.getSize() == 0)
        return false;
    if (!checkCuts())
        return false;
    m_str = str;
    return true;
}

bool CompareObject::checkCuts()
{
    const ParamsHelper& keys = getKeyHelper();
    for (int i=0,e=keys.getSize(); i<e; ++i) 
    {
        const tstring& cut = keys.getCutValue(i);
        if (cut.empty()) 
            continue;

        // get text on the cut position
        tstring param;
        if (!m_pcre.getString(i+1, &param))
        {
            assert(false);
            return false;
        }

        // translate cut
        Pcre16 &r = CutRegexp::get();
        r.findAllMatches(cut);
        int vars_count = r.getSize();

        // no vars in cut
        if (vars_count == 0) {
           if (cut != param)
               return false;
           continue;
        }

        // compare vars
        int from_param = 0;
        int from_cut = 0;
        for (int j=1; j<vars_count; ++j)
        {
            // compare part before var
            tstring prefix( cut.substr(from_cut, r.getFirst(j)-from_cut) );
            if (!prefix.empty() && param.compare(from_param, prefix.length(), prefix)) {            
               return false;
            }
            from_param += prefix.length();
            from_cut += prefix.length();

            tstring var;
            r.getString(j, &var);
            int varname_len = var.size();
            int last = varname_len - 1;
            if (var.at(last) == L';')
                last = last - 1;
            bool multivar = false;
            if (var.at(last) == L'*')
                { multivar = true; last = last - 1; }

             tstring var_name(var.substr(1, last));
             CompareObjectVarsHelper h(var_name, multivar);
             var.clear();

             bool compared = false;
             while (h.next(&var))
             {
                if (!param.compare(from_param, var.length(), var))
                {
                    from_param += var.length();
                    from_cut += varname_len;
                    compared = true;
                    break;
                }
             }
             if (!compared)
                 return false;
        }

        // check suffix after all vars
        int suffix_from = r.getLast(vars_count-1);
        tstring suffix( cut.substr(suffix_from) );
        if (!suffix.empty() ) {
            if (param.compare(from_param, suffix.length(), suffix)) {
                return false;
            }
            from_param += suffix.length();
        }

        int len = param.length();
        if  (from_param != len)
             return false;
    }
    return true;
}

void CompareObject::checkFirstSymbols(const tstring& key)
{
    if (key.size() < 2)
        return;
    tchar s = key.at(0);
    if (s == L'%' || s == L'$')
        return;
    if (s == L'^')
    {
        m_first_symbol = key.at(1);
        m_second_symbol = 0;
    }
    else
    {
        m_first_symbol = s;
        m_second_symbol = key.at(1);
    }
}

bool CompareObject::compareFirstSymbols(const tstring& str)
{
    if (!m_first_symbol || str.empty())
        return true;
    if (m_second_symbol == 0)
    {
        if (str.at(0) != m_first_symbol)
            return false;
        return true;
    }
    size_t pos = str.find(m_first_symbol);
    if (pos == tstring::npos)
        return false;
    pos++;
    if (str.length() <= pos)
        return false;
    if (str.at(pos) != m_second_symbol)
        return false;
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

    const ParamsHelper& keys = getKeyHelper();
    int maxid = keys.getMaxId()+1;
    if (maxid <= 0)
        maxid = 1;

    p.clear(); //if not empty
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

void CompareObject::getParametersRange(std::vector<CompareRange>* ranges) const
{
    assert(ranges);
    int count = m_pcre.getSize();
    if (count == 0) { ranges->clear(); return; }
    if (m_std_regexp)
    {
        ranges->resize(count);
        for (int i = 0; i < count; ++i) {
            CompareRange &r = ranges->at(i);
            r.begin = m_pcre.getFirst(i);
            r.end = m_pcre.getLast(i);
        }
        return;
    }
    const ParamsHelper& keys = getKeyHelper();
    int maxid = keys.getMaxId() + 1;
    if (maxid <= 0)
        maxid = 1;
    ranges->resize(maxid);
    CompareRange &c0 = ranges->at(0);
    c0.begin = m_pcre.getFirst(0);
    c0.end = m_pcre.getLast(0);
    int pi = 1;
    for (int i = 0, e = keys.getSize(); i < e; ++i)
    {
        int id = keys.getId(i);
        if (id == -1) continue;
        CompareRange &c = ranges->at(id);
        c.begin = m_pcre.getFirst(pi);
        c.end = m_pcre.getLast(pi);
        pi++;
    }
}

const ParamsHelper& CompareObject::getKeyHelper() const {
    if (!m_pkey_helper)
        m_pkey_helper = new ParamsHelper(m_key, true, &m_key_nocuts);
    return *m_pkey_helper;
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
    ParamsHelper ph(tmp, true);
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
    m_vars_pcre_parts.clear();

    //find vars like $var
    Pcre16 vars;
    vars.setRegExp(L"\\$[a-zA-Z_][0-9a-zA-Z_.]*");
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

const tstring& CompareObject::getKey() const
{
    return m_key;
}

const tstring& CompareObject::getKeyNoCuts() const
{
    return m_key_nocuts;
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
    delete m_pkey_helper;
    m_pkey_helper = NULL;
    m_first_symbol = 0;
    m_second_symbol = 0;
}
