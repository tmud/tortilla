#pragma once

#include "propertiesPages/propertiesData.h"
#include "logicHelper.h"

class InputCommand
{
public:
    InputCommand(const tstring& cmd);
    void replace_command(const tstring& cmd);
    tstring full_command;                   // full command (with parameters)
    tstring command;                        // only command
    tstring parameters;                     // only parameters (without command) as single line (without trimming)
    std::vector<tstring> parameters_list;   // list of parameters separately
    bool empty;
};

class InputCommandsList : private std::vector<InputCommand*>
{
    typedef std::vector<InputCommand*> base;
public:
    void parse(const tstring& cmds);

    InputCommandsList() {}
    ~InputCommandsList() { clear(); }    
    int size() const { return base::size(); }
    InputCommand* operator[] (int index) const { 
        return base::operator[](index);
    }
    void erase(int index) {
        InputCommand *cmd = base::operator[](index);
        delete cmd;
        base::erase(begin()+index);
    }
    void insert(int pos, InputCommandsList& cmds) {
        base::insert(begin() + pos, cmds.begin(), cmds.end());
        cmds.clear();
    }
    void clear() {
        struct{ void operator() (InputCommand* cmd) { delete cmd; }} del;
        std::for_each(begin(), end(), del);
    }
    void push_back(InputCommand *cmd) {
        base::push_back(cmd);
    }
};

class InputCommandTemplate
{
public:
    InputCommandTemplate(const tstring& cmd, tchar separator, tchar syscmd, tchar marker);
    void translate(InputCommandsList *cmd);
private:
    bool isbracket(const tchar *p);
    typedef std::pair<tstring,int> subcmd;
    std::vector<subcmd> m_subcmds;
    //tchar m_marker;
};

class InputProcessor
{   
public:
    InputProcessor(tchar separator, tchar prefix);
    ~InputProcessor();
    void process(const tstring& cmd, LogicHelper* helper, std::vector<tstring>* loop_cmds);
    InputCommandsList commands;

private:   
    void processSeparators(const tstring& sep_cmd, InputCommandsList* result);
    void processParameters(const tstring& cmd, InputCommand* params, tstring* result);
    tchar m_separator, m_prefix;
};


#ifdef _DEBUG
class InputCommandTemplateUnitTest
{
    static bool test(const tstring& str, const tstring& res);
public:
    static void run();
};
#define RUN_INPUTPROCESSOR_TESTS InputCommandTemplateUnitTest::run();
#else
#define RUN_INPUTPROCESSOR_TESTS
#endif
