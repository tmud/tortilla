#include "stdafx.h"
#include "compareObject.h"
#include "accessors.h"

CompareObjectVarsHelper::CompareObjectVarsHelper(const tstring& name, bool multivar) : pred(name), multflag(multivar)
{
}

bool CompareObjectVarsHelper::next(tstring *val)
{
    VarProcessor *vp = tortilla::getVars();
    return false;
}
