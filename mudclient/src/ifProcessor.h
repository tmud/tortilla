#pragma once

class IfProcessor
{
public:
    IfProcessor()
    {
        m_if_regexp.setRegExp(L"^('.*'|\".*\"|{.*}|[^ =~!<>]+) *(=|!=|<|>|<=|>=) *('.*'|\".*\"|{.*}|[^ =~!<>]+)$", true);
        m_vars_regexp.setRegExp(L"\\$[^ ]+", true);
    }    
    enum Result { IF_SUCCESS = 0, IF_FAIL, IF_ERROR };
    Result compare(const tstring& param, const PropertiesValues &vars)
    {
        m_if_regexp.find(param);
        if (m_if_regexp.getSize() != 4)
            return IF_ERROR;

        tstring p1, p2, cond;
        m_if_regexp.getString(1, &p1);  //1st parameter
        m_if_regexp.getString(3, &p2);  //2nd parameter
        m_if_regexp.getString(2, &cond);//condition
        if (processVars(&p1, vars) && processVars(&p2, vars))
        {
            if (isOnlyDigits(p1) && isOnlyDigits(p2))
            {
                int n1 = _wtoi(p1.c_str());
                int n2 = _wtoi(p2.c_str());
                if (n1 == n2 && (cond == L"=" || cond == L"<=" || cond == L">="))
                    return IF_SUCCESS;
                if (n1 < n2 && (cond == L"<" || cond == L"<=" || cond == L"!="))
                    return IF_SUCCESS;
                if (n1 > n2 && (cond == L">" || cond == L">=" || cond == L"!="))
                    return IF_SUCCESS;
            }
            else
            {
                int result = wcscmp(p1.c_str(), p2.c_str());
                if (result == 0 && (cond == L"=" || cond == L"<=" || cond == L">="))
                    return IF_SUCCESS;
                if (result < 0 && (cond == L"<" || cond == L"<=" || cond == L"!="))
                    return IF_SUCCESS;
                if (result > 0 && (cond == L">" || cond == L">=" || cond == L"!="))
                    return IF_SUCCESS;
            }
        }        
        return IF_FAIL;
    }

private:
    bool processVars(tstring *p, const PropertiesValues &vars)
    {
        m_vars_regexp.findAllMatches(*p);
        if (m_vars_regexp.getSize() == 0)
            return true;
                
        tstring newparam(p->substr(0, m_vars_regexp.getFirst(0)));
        for (int i = 0, e = m_vars_regexp.getSize() - 1; i <= e; ++i)
        {
            tstring tmp;
            m_vars_regexp.getString(i, &tmp);
            int index = vars.find(tmp.substr(1));
            if (index == -1)
                return false;
            
            const tstring &var = vars.get(index).value;
            newparam.append(var);

            if (i < e)
            {
                int end = m_vars_regexp.getLast(i);
                int next = m_vars_regexp.getFirst(i + 1);
                newparam.append(p->substr(end, next - end));
            }
            else
            {
                int end = m_vars_regexp.getLast(i);
                newparam.append(p->substr(end));
            }
        }
        p->assign(newparam);
        return true;
    }

private:
    Pcre16 m_if_regexp;
    Pcre16 m_vars_regexp;
};
