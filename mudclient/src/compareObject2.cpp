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


#ifdef _DEBUG
void CompareObjectUnitTests::run()
{
    std::vector<tstring> params;

    CompareObject co;
    co.init(L"abc %(xyz)1", false);
    assert( co.compare(L"abc xyz") );
    co.getParameters(&params);
    assert(params.size() == 2);
    assert(params[1].compare(L"xyz") == 0);
    assert( !co.compare(L"abc xyz1") );
    assert( !co.compare(L"abc xy") );
    assert( !co.compare(L"abc xyw") );
    assert( co.getKeyNoCuts() == L"abc %1" );

    VarProcessor *vp = tortilla::getVars();
    vp->setVar(L"var", L"סהו");
    vp->setVar(L"ttt", L"12");

    co.init(L"def %1 %($var)2 %($ttt;3)3", false);
    assert( co.getKeyNoCuts() == L"def %1 %2 %3" );
    assert( co.compare(L"def אבג סהו 123") );
    co.getParameters(&params);
    assert(params.size() == 4);
    assert(params[3].compare(L"123") == 0);
    assert( !co.compare(L"def אבג סהו 124") );

    vp->setVar(L"x", L"000");
    vp->setVar(L"y", L"111");

    co.init(L"%($ttt)3 klm %(a$x;b)2 %(qwe$y;)1", false);
    assert( co.getKeyNoCuts() == L"%3 klm %2 %1" );

    assert( co.compare(L"12 klm a000b qwe111") );
    co.getParameters(&params);
    assert(params.size() == 4);
    assert(params[1].compare(L"qwe111") == 0);
    assert(params[2].compare(L"a000b") == 0);

    assert( !co.compare(L"12 klm a000b wwe111") );
    assert( !co.compare(L"13 klm a000b qwe111") );
    assert( !co.compare(L"12 klm a000b qwe111x") );
    assert( !co.compare(L"12a klm a000b qwe111") );
    assert( !co.compare(L"12 klmn a000b qwe111") );
    assert( !co.compare(L"12 klm a001b qwe111") );
    assert( !co.compare(L"12 klm a000b qwe222") );

    // aaa not declared!
    co.init(L"%($aaa)2", false);
    assert( !co.compare(L"") );
    vp->setVar(L"aaa", L"");
    assert( co.compare(L"") );
    co.getParameters(&params);
    assert(params.size() == 3);
    assert(params[0].compare(L"") == 0);
    assert(params[1].compare(L"") == 0);
    assert(params[2].compare(L"") == 0);

    // multivars tests
    vp->setVar(L"x1", L"x1");
    vp->setVar(L"x2", L"x2");
    vp->setVar(L"x3", L"x3");
    vp->setVar(L"x4", L"x4");
    vp->setVar(L"x5", L"x5");

    co.init(L"abc %($x*)1", false);
    assert( co.compare(L"abc x4") );
    co.getParameters(&params);
    assert(params.size() == 2);
    assert(params[1].compare(L"x4") == 0);

    co.init(L"abc %($x*def)1", false);
    assert( co.compare(L"abc x5def") );

    co.init(L"abc %($y$x*;def)1", false);
    assert( co.compare(L"abc 111x5def") );

    co.init(L"abc %($y$x*def)1", false);
    assert( co.compare(L"abc 111000def") );
    assert( !co.compare(L"abc 110000def") );
    vp->setVar(L"y", L"110");
    assert( co.compare(L"abc 110000def") );

    vp->deleteAll();
}
#endif
