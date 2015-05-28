#include "stdafx.h"
#include "inputProcessor.h"

#ifdef _DEBUG
bool InputCommandTemplateUnitTest::test(const tstring& str, const tstring& res)
{
    tchar s = L';'; tchar p = L'#'; tchar m = '\t';
    InputCommandTemplate t(str, s, p, m);
    // todo
    return true;
}

void InputCommandTemplateUnitTest::run()
{
    assert( test(L"aaaa;bbbb;cccc", L"") );

}

#endif
