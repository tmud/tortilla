#include "stdafx.h"
#include "accessors.h"
#include "varProcessor.h"
#include "propertiesPages/propertiesData.h"

class VarProcessorImpl
{
    std::map<tstring, int> m_specvars;
    typedef std::map<tstring, int>::const_iterator citerator;
    enum { DATE = 0, TIME, DAY, MONTH, YEAR, HOUR, MINUTE, SECOND, MILLISECOND, TIMESTAMP };
    Pcre16 m_vars_regexp;
    Pcre16 m_var_regexp;
public:
    VarProcessorImpl();
    bool canset(const tstring& var) const
    {
        citerator it = m_specvars.find(var);
        return (it == m_specvars.end()) ? true : false;
    }
    bool processVars(tstring *p, const PropertiesValues &vars, bool strong_mode, bool vars_absent_result);
    bool getVar(const PropertiesValues &vars, const tstring& var, tstring *value) const;
    bool setVar(PropertiesValues &vars, const tstring& var, const tstring& value);
    bool delVar(PropertiesValues &vars, const tstring& var);
private:
    void getSpecVar(int id, tstring *value) const;
} vars_processor;

PropertiesValues& getVars()
{
    return tortilla::getProperties()->variables;
}

bool VarProcessor::canSetVar(const tstring& var)
{
    return vars_processor.canset(var);
}

bool VarProcessor::processVars(tstring *p, bool vars_absent_result)
{
    return vars_processor.processVars(p, getVars(), false, vars_absent_result);
}

bool VarProcessor::processVarsStrong(tstring *p, bool vars_absent_result)
{
    return vars_processor.processVars(p, getVars(), true, vars_absent_result);
}

bool VarProcessor::getVar(const tstring& var, tstring *value)
{
    return vars_processor.getVar(getVars(), var, value);
}

bool VarProcessor::setVar(const tstring& var, const tstring& value)
{
    return vars_processor.setVar(getVars(), var, value);
}

bool VarProcessor::delVar(const tstring& var)
{
    return vars_processor.delVar(getVars(), var);
}

VarProcessorImpl::VarProcessorImpl()
{
    std::map<tstring, int> &v = m_specvars;
    v[L"DATE"] = DATE;
    v[L"TIME"] = TIME;
    v[L"DAY"] = DAY;
    v[L"MONTH"] = MONTH;
    v[L"YEAR"] = YEAR;
    v[L"HOUR"] = HOUR;
    v[L"MINUTE"] = MINUTE;
    v[L"SECOND"] = SECOND;
    v[L"MILLISECOND"] = MILLISECOND;
    v[L"TIMESTAMP"] = TIMESTAMP;
    m_vars_regexp.setRegExp(L"\\$[a-zA-Z_][0-9a-zA-Z_.]*", true);
    m_var_regexp.setRegExp(L"^[a-zA-Z_][0-9a-zA-Z_.]*$", true);
}

bool VarProcessorImpl::processVars(tstring *p, const PropertiesValues &vars, bool strong_mode, bool vars_absent_result)
{
    m_vars_regexp.findAllMatches(*p);
    if (m_vars_regexp.getSize() == 0)
        return vars_absent_result;

    tstring newparam(p->substr(0, m_vars_regexp.getFirst(1)));
    for (int i=1, e=m_vars_regexp.getSize()-1; i<=e; ++i)
    {
        tstring tmp, var;
        m_vars_regexp.getString(i, &tmp);

        int end = -1;
        tstring name(tmp.substr(1));
        if (getVar(vars, name, &var))
            newparam.append(var);
        else
        {
            // ищем переменную по укороченному имени без _
            const tchar *p = wcschr(name.c_str(), L'_');
            if (!p && strong_mode)  // в переменной нет _
                return false;
            if (p)
            {
                int len = p-name.c_str();
                tmp.assign(name.c_str(), len);
                if (getVar(vars, tmp, &var))
                {
                    newparam.append(var);
                    end = m_vars_regexp.getFirst(i) + len + 1;
                }
                else if (strong_mode)
                    return false;
            }
        }
 
        if (end == -1)
            end = m_vars_regexp.getLast(i);
        if (i < e)
        {
            int next = m_vars_regexp.getFirst(i + 1);
            newparam.append(p->substr(end, next - end));
        }
        else
        {
            newparam.append(p->substr(end));
        }
     }
     p->swap(newparam);
     return true;
}

bool VarProcessorImpl::setVar(PropertiesValues &vars, const tstring& var, const tstring& value)
{
    if (var.empty())
        return false;
    m_var_regexp.find(var);
    if (m_var_regexp.getSize() != 0)
    {
        int index = vars.find(var);
        vars.add(index, var, value, L"");
        return true;
    }
    return false;
}

bool VarProcessorImpl::delVar(PropertiesValues &vars, const tstring& var)
{
    int index = vars.find(var);
    if (index == -1)
        return false;
    vars.del(index);
    return true;
}

bool VarProcessorImpl::getVar(const PropertiesValues &vars, const tstring& var, tstring *value) const
{
     citerator it = m_specvars.find(var);
     if (it == m_specvars.end())
     {
        int index = vars.find(var);
        if (index == -1)
            return false;
        value->assign(vars.get(index).value);
        return true;
     }
     getSpecVar(it->second, value);
     return true;
}

void VarProcessorImpl::getSpecVar(int id, tstring *value) const
{
    wchar_t buffer[24];
    if (id == TIMESTAMP)
    {
        SYSTEMTIME st;
        FILETIME ft;
        ULARGE_INTEGER ularge;
        GetSystemTime(&st);
	    SystemTimeToFileTime(&st,&ft);

        ularge.LowPart = ft.dwLowDateTime;
        ularge.HighPart = ft.dwHighDateTime;

	    // convert filetime to unix timestamp
	    swprintf(buffer, L"%I64d", ularge.QuadPart / 10000000 - 11644473600); 
        value->assign(buffer);
        return;
    }

    SYSTEMTIME st;
	GetLocalTime(&st);
    switch (id) 
    {
        case DATE:
	        swprintf(buffer, L"%02d-%02d-%d", st.wDay, st.wMonth, st.wYear);
            break;
        case TIME:
	        swprintf(buffer, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
            break;
        case DAY:
	        swprintf(buffer, L"%02d", st.wDay);
            break;
        case MONTH:
	        swprintf(buffer, L"%02d", st.wMonth);
            break;
        case YEAR:
	        swprintf(buffer, L"%d", st.wYear);
            break;
        case HOUR:
	        swprintf(buffer, L"%02d", st.wHour);
            break;
        case MINUTE:
	        swprintf(buffer, L"%02d", st.wMinute);
            break;
        case SECOND:
            swprintf(buffer, L"%02d", st.wSecond);
            break;
        case MILLISECOND:
            swprintf(buffer, L"%03d", st.wMilliseconds);
            break;
     }
     value->assign(buffer);
}
