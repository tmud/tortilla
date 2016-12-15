#pragma once

class VarProcessor
{
public:
    bool canSetVar(const tstring& var);
    bool processVars(tstring *p, bool vars_absent_result);
    bool processVarsStrong(tstring *p, bool vars_absent_result);
    bool getVar(const tstring& var, tstring *value);
    bool setVar(const tstring& var, const tstring& value);
    bool delVar(const tstring& var);
    bool iterateVar(const tstring& pred, int& last, tstring* value);
    void deleteAll();
};
