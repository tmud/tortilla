#pragma once

class InputVarsAccessor
{
public:
    bool get(const tstring&name, tstring* value);
    void tanslateVars(tstring *cmd);
};

class InputPlainCommands : private std::vector<tstring> 
{
    typedef std::vector<tstring> base;
public:
    InputPlainCommands();
    InputPlainCommands(const tstring& cmd);
    bool empty() const { return base::empty(); }
    int size() const { return base::size(); }
    void clear() { base::clear(); }
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

struct InputCommand
{
    InputCommand() : dropped(false), system(false), changed(false) {}

    //InputCommand(const tstring& cmd);
    //void replace_command(const tstring& cmd);    

                                            // full command as is = command + parameters
    tstring srccmd;                         // only command name (may be left spaces)
    tstring parameters;                     // only parameters (without command) as single line (without trimming)
    tstring command;                        // command without spaces
    std::vector<tstring> parameters_list;   // list of parameters separately
    bool dropped;
    bool system;
    bool changed;
};

class InputCommands : private std::vector<InputCommand*>
{
    typedef std::vector<InputCommand*> base;
public:
    ~InputCommands() { clear(); }
    int size() const { return base::size(); }
    InputCommand* operator[] (int index) const { 
        return base::operator[](index);
    }
    void erase(int index) {
        InputCommand *cmd = base::operator[](index);
        delete cmd;
        base::erase(begin()+index);
    }
    void insert(int pos, InputCommands& cmds) {
        base::insert(begin() + pos, cmds.begin(), cmds.end());
        cmds.clear();
    }
    void clear() {
        std::for_each(begin(), end(), [](InputCommand *c){ delete c; });
    }
    void push_back(InputCommand *cmd) {
        base::push_back(cmd);
    }
};

typedef std::pair<tstring,int> InputSubcmd;
class InputTemplateCommands : private std::vector<InputSubcmd>
{
public:
    void init(const InputPlainCommands& cmds, const InputTemplateParameters& params);
    void extract(InputPlainCommands* cmds);
    void makeTemplates();
    void tranlateVars();
    void makeCommands(InputCommands *cmds);
private:
    const tchar MARKER = L'\t';
    InputTemplateParameters _params;
    void fillsyscmd(InputCommand *cmd);
    void fillgamecmd(InputCommand *cmd);
    void parsecmd(const tstring& cmd);
    void markbrackets(tstring *cmd) const;
    void unmarkbrackets(tstring* parameters, std::vector<tstring>* parameters_list) const;
    bool isbracket(const tchar *p) const;
};

/*class InputCommandTemplate
{
public:
    InputCommandTemplate();
    bool init(const tstring& key, const tstring& value, const InputCommandParameters& params);
    bool compare(const tstring& str);
    void translate(InputCommands *cmd) const;
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
    InputCommands commands;

private:
    //void processSeparators(const tstring& sep_cmd, InputCommands* result);
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
