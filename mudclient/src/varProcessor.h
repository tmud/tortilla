#pragma once
#include "propertiesPages/propertiesData.h"

class VarProcessor
{
    std::map<tstring, int> m_specvars;
    typedef std::map<tstring, int>::const_iterator citerator;
    enum { DATE = 0, TIME, DAY, MONTH, YEAR, HOUR, MINUTE, SECOND, MILLISECOND, TIMESTAMP };
    Pcre16 m_vars_regexp;
public:
    VarProcessor();
    bool canset(const tstring& var) const
    {
        citerator it = m_specvars.find(var);
        return (it == m_specvars.end()) ? true : false;
    }
    bool processVars(tstring *p, const PropertiesValues &vars, bool strong_mode);
    bool getVar(const PropertiesValues &vars, const tstring& var, tstring *value) const;
    void setVar(PropertiesValues &vars, const tstring& var, const tstring& value);
private:
    void getSpecVar(int id, tstring *value) const;
};
