#include "stdafx.h"
#include "varProcessor.h"

VarProcessor::VarProcessor()
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
    m_vars_regexp.setRegExp(L"\\$[0-9a-zA-Z_]+", true);
}

bool VarProcessor::processVars(tstring *p, const PropertiesValues &vars, bool strong_mode)
{
    m_vars_regexp.findAllMatches(*p);
    if (m_vars_regexp.getSize() == 0)
        return true;

    tstring newparam(p->substr(0, m_vars_regexp.getFirst(0)));
    for (int i = 0, e = m_vars_regexp.getSize() - 1; i <= e; ++i)
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

bool VarProcessor::setVar(PropertiesValues &vars, const tstring& var, const tstring& value)
{
    if (var.empty())
        return false;
    tchar b = var.at(0);
    if ((b >= L'a' && b <= L'z') || (b >= L'A' && b <= L'Z'))
    {
        int index = vars.find(var);
        vars.add(index, var, value, L"");
        return true;
    }
    return false;
}

bool VarProcessor::delVar(PropertiesValues &vars, const tstring& var)
{
    int index = vars.find(var);
    if (index == -1)
        return false;
    vars.del(index);
    return true;
}

bool VarProcessor::getVar(const PropertiesValues &vars, const tstring& var, tstring *value) const
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

void VarProcessor::getSpecVar(int id, tstring *value) const
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
