#pragma once

//#include "propertiesPages/propertiesData.h"
//#include "logicHelper.h"
#include "compareObject.h"

class InputPlainCommands : private std::vector<tstring> 
{
    typedef std::vector<tstring> base;
public:
    InputPlainCommands();
    InputPlainCommands(const tstring& cmd);
    bool empty() const { return base::empty(); }
    int size() const { return base::size(); }
    const tstring& operator[] (int index) const { 
        return base::operator[](index);
    }
    void erase(int index) {
        base::erase(begin()+index);
    }
    void push_back(const tstring& cmd){
        base::push_back(cmd);
    }
    std::vector<tstring>* ptr() {
        return this;
    }
};

struct InputTemplateParameters
{
    tchar separator;
    tchar prefix;
};

typedef std::pair<tstring,int> InputSubcmd;
class InputTemplateCommands : private std::vector<InputSubcmd>
{
public:
    void init(const InputPlainCommands& cmds, const InputTemplateParameters& params);

private:
    const tchar MARKER = L'\t';
    void parsecmd(const tstring& cmd, const InputTemplateParameters& params);
    void markbrackets(tstring *cmd) const;
    bool isbracket(const tchar *p) const;
};



class InputCommand
{
public:
    //InputCommand(const tstring& cmd);
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


/*class InputCommandTemplate
{
public:
    InputCommandTemplate();
    bool init(const tstring& key, const tstring& value, const InputCommandParameters& params);
    bool compare(const tstring& str);
    void translate(InputCommandsList *cmd) const;
private:
    void markbrackets(tstring *cmd);
    bool isbracket(const tchar *p);
private:
    CompareObject m_key;
    typedef std::pair<tstring,int> subcmd;
    std::vector<subcmd> m_subcmds;
#ifdef _DEBUG
public:
    const tchar* getcmd(int index) const { return m_subcmds[index].first.c_str(); }
    int getflag(int index) const { return m_subcmds[index].second; }
    int size() const { return m_subcmds.size(); }
#endif
};*/

/*
class InputProcessor
{
public:
    InputProcessor(tchar separator, tchar prefix);
    ~InputProcessor();
    void process(const tstring& cmd, LogicHelper* helper, std::vector<tstring>* loop_cmds);
    InputCommandsList commands;

private:
    //void processSeparators(const tstring& sep_cmd, InputCommandsList* result);
    void processParameters(const tstring& cmd, InputCommand* params, tstring* result);
    tchar m_separator, m_prefix;
};
*/

#ifdef _DEBUG
class InputCommandTemplateUnitTest
{
    static bool test(const tstring& str, int n, ...);
public:
    static void run();
};
#define RUN_INPUTPROCESSOR_TESTS InputCommandTemplateUnitTest::run();
#else
#define RUN_INPUTPROCESSOR_TESTS
#endif
