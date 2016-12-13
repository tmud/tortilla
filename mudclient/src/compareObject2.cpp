#include "stdafx.h"
#include "compareObject.h"
#include "accessors.h"

CompareObjectVarsHelper::CompareObjectVarsHelper(const tstring& name, bool multivar) 
: pred(name), multiflag(multivar), multiflag_used(false), last(-1)
{
}

bool CompareObjectVarsHelper::next(tstring *value)
{
    if (!multiflag) {
        if (multiflag_used) return false;
        multiflag_used =  true;
        VarProcessor *vp = tortilla::getVars();
        return vp->getVar(pred, value);
    }
    VarProcessor *vp = tortilla::getVars();
    return vp->iterateVar(pred, last, value);
}
