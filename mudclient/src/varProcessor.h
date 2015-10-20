#pragma once

class VarProcessor
{
public:
    bool canSetVar(const tstring& var);
    bool processVars(tstring *p);
    bool processVarsStrong(tstring *p);
    bool getVar(const tstring& var, tstring *value);
    bool setVar(const tstring& var, const tstring& value);
    bool delVar(const tstring& var);
};
