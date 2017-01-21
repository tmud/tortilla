#include "stdafx.h"
#include "inputProcessor.h"

#ifdef _DEBUG

const tchar* param0 = L"P'AR;AM0";
const tchar* param1 = L"PAR\"AM}1";
const tchar* param2 = L"PA'R{AM2";
const tchar* param3 = L"PAR{A}M3";
const tchar* param4 = L"PARAM+";

class InputTestsParameters : public InputParameters
{
    int m_params_count;
public:
    InputTestsParameters(int count) : m_params_count(count) {}
    void getParameters(std::vector<tstring>* params) const
    {
        assert(m_params_count < 5);

        for (int i=0; i<=m_params_count; ++i) {
        switch(i) {
        case 0:
            params->push_back(param0);
        break;
        case 1:
            params->push_back(param1);
        break;
        case 2:
            params->push_back(param2);
        break;
        case 3:
            params->push_back(param3);
        break;
        default:
            params->push_back(param4);
        }}
    }
    void doNoValues(tstring* cmd) const
    {
    }
};

bool InputCommandTemplateUnitTests::test1(const tstring& str, int n, ...)
{
    InputPlainCommands cmds(str);
    InputTemplateParameters p; 
    p.separator = L';';
    p.prefix = L'#';

    InputTemplateCommands t;
    t.init(cmds, p);
    InputPlainCommands out;
    t.extract(&out);

    std::vector<tstring>* cs = out.ptr();

    if (cs->size() != n)
        return false;

    bool result = true;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; ++i)
    {
        const wchar_t* s = va_arg(args, wchar_t*);
        if (wcscmp(cs->at(i).c_str(), s))
            { result = false; break; }
    }
    va_end(args); 
    return result;
}

bool InputCommandTemplateUnitTests::test2(const tstring& str, int params, InputCommands *ref)
{
    InputPlainCommands cmds(str);
    InputTemplateParameters p;
    p.separator = L';';
    p.prefix = L'#';

    InputTemplateCommands t;
    t.init(cmds, p);
    t.makeTemplates();

    InputTestsParameters tp(params);
    InputCommands tcmds;
    t.makeCommands(&tcmds, &tp);

    if (tcmds.size() != ref->size())
        return false;

    for (int i=0, e=tcmds.size(); i<e; ++i)
    {
        InputCommand c1 = tcmds[i];
        InputCommand c2 = ref->operator[](i);
        if (c1->srccmd != c2->srccmd)
            return false;
        if (c1->srcparameters != c2->srcparameters)
            return false;
        if (c1->command != c2->command)
            return false;
        if (c1->system != c2->system)
            return false;
    }
    return true;
}

InputCommand InputCommandTemplateUnitTests::makecmd(bool system, const tstring& srccmd, const tstring& srcparams,
    const tstring& cmd, int n, ...)
{
    InputCommand c =  std::make_shared<InputCommandData>();
    c->srccmd = srccmd;
    c->srcparameters = srcparams;
    c->command = cmd;
    c->system = system;

    va_list args;
    va_start(args, n);
    for (int i=0; i<n; ++i)
    {
         const wchar_t* s = va_arg(args, wchar_t*);
         c->parameters_list.push_back(s);
    }
    va_end(args);
    return c;
}

void InputCommandTemplateUnitTests::run()
{
    assert( test1(L"", 1, L"") );
    assert( test1(L"aaaa;#bbbb dd ff gg;cccc", 3, L"aaaa", L"#bbbb dd ff gg", L"cccc") );
    assert( test1(L"  {aaaa \"fff;vvv\" {'':; }gg}", 1, L"  aaaa \"fff;vvv\" {'':; }gg" ) );
    assert( test1(L"#abc {asdasd};  def xxx;  #eee 'aaa' 'bbb' ", 3, L"#abc {asdasd}", L"  def xxx", L"#eee 'aaa' 'bbb' ") );
    assert( test1(L" #ddd {fff} 'f;f{f;f}f;f' {{fd;fd}f;f}", 1, L"#ddd {fff} 'f;f{f;f}f;f' {{fd;fd}f;f}") );
    assert( test1(L"#ddd {fff} ; {ffff 'f;f{f;f}f;f'}", 2, L"#ddd {fff} ", L" ffff 'f;f{f;f}f;f'") );
    assert( test1(L"#bbb '{};fff;\"aaa\"'", 1, L"#bbb '{};fff;\"aaa\"'") );

    InputCommands g1;
    g1.push_back(makecmd(false, L" ", L"", L" ", 0));
    assert(test2(L" ", 0, &g1));

    InputCommands g2;
    g2.push_back(makecmd(false, L"тест", L"  123 456  789", L"тест", 3, L"  123", L" 456", L"  789" ));
    assert(test2(L"тест  123 456  789", 3, &g2));

    InputCommands g3;
    g3.push_back(makecmd(false, L"тест2", L" aaa  bbb  ccc  ", L"тест2", 4, L" aaa", L"  bbb", L"  ccc", L"  "));
    assert(test2(L"тест2 aaa  bbb  ccc  ", 4, &g3));

    InputCommands t1;
    t1.push_back( makecmd(true, L"# wout", L" 1 test test2", L" wout", 3, L"1", L"test", L"test2") );
    assert( test2(L"# wout 1 test test2", 0, &t1) );

    InputCommands t2;
    t2.push_back(makecmd(true, L"#wout", L" 1 {red} blue", L"wout", 3, L"1", L"red", L"blue"));
    assert(test2(L"#wout 1 {red} blue", 0, &t2));

    InputCommands t3;
    t3.push_back(makecmd(true, L"#wout", L" 'a;b;c\"\"'", L"wout", 1, L"a;b;c\"\""));
    t3.push_back(makecmd(true, L"#ex", L" {v;d's}", L"ex", 1, L"v;d's"));
    assert(test2(L"#wout 'a;b;c\"\"';#ex {v;d's}", 0, &t3));

    InputCommands t4;
    t4.push_back(makecmd(true, L"#wout", L" %0", L"wout", 1, param0));
    assert(test2(L"#wout %0", 0, &t4));

    InputCommands t5;
    tstring p1p2(param1);
    p1p2.append(param2);
    t5.push_back(makecmd(true, L"#test", L" '%1%2' %3 %4 %0", L"test", 4, p1p2.c_str(), param3, param4, param0));
    assert(test2(L"#test '%1%2' %3 %4 %0", 4, &t5));

    InputCommands t6;
    t6.push_back(makecmd(true, L"#wait", L" 1.2 {#out {light red} {Время идти!};встать}", L"wait", 2, L"1.2", L"#out {light red} {Время идти!};встать"));
    assert(test2(L"#wait 1.2 {#out {light red} {Время идти!};встать}", 2, &t6));
}

#endif
