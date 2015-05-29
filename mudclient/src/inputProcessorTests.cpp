#include "stdafx.h"
#include "inputProcessor.h"

#ifdef _DEBUG
bool InputCommandTemplateUnitTest::test(const tstring& str, int n, ...)
{
    InputCommandParameters p; 
    p.separator = L';';
    p.prefix = L'#';
    InputCommandTemplate t(str, p);

    if (t.size() != n)
        return false;

    va_list args;
    va_start(args, n);
    for (int i=0; i<(n*2); ++i)
    {
        int index = i/2;
        if (i % 2)
        {
            int system_flag = va_arg(args, int);
            if (t.getflag(index) != system_flag)
                return false;
        }
        else
        {
            const wchar_t* s = va_arg(args, wchar_t*);
            if (wcscmp(t.getcmd(index),s))
                return false;
        }
    }
    va_end(args);
    return true;
}

void InputCommandTemplateUnitTest::run()
{
    assert( test(L"", 1, L"", 0) );
    assert( test(L"aaaa;#bbbb dd ff gg;cccc", 3, L"aaaa", 0, L"bbbb dd ff gg", 1, L"cccc", 0) );
    assert( test(L"  {aaaa \"fff;vvv\" {'':; }gg}", 1, L"  aaaa \"fff;vvv\" {'':; }gg", 0 ) );
    assert( test(L"#abc {asdasd};  def xxx;  #eee 'aaa' 'bbb' ", 3, L"abc \t{asdasd\t}", 1, L"  def xxx", 0, L"eee \t'aaa\t' \t'bbb\t' ", 1) );
    assert( test(L" #ddd {fff} 'f;f{f;f}f;f' {{fd;fd}f;f}", 1, L"ddd \t{fff\t} \t'f;f{f;f}f;f\t' \t{{fd;fd}f;f\t}", 1) );
    assert( test(L"#ddd {fff} ; {ffff 'f;f{f;f}f;f'}", 2, L"ddd \t{fff\t} ", 1, L" ffff 'f;f{f;f}f;f'", 0) );
    assert( test(L"#bbb '{};fff;\"aaa\"'", 1, L"bbb \t'{};fff;\"aaa\"\t'", 1 ) );
}

#endif
