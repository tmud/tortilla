#pragma once

class InputVarsAccessor
{
public:
    bool get(const tstring&name, tstring* value);
    void translateVars(tstring *cmd);
};

class InputParameters
{
public:
    virtual void getParameters(std::vector<tstring>* params) const = 0;
};

class InputTranslateParameters
{
public:
    void doit(const InputParameters *params, tstring *cmd);
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
                                            // full command as is = command + parameters
    tstring srccmd;                         // only command name with prefix as is
    tstring srcparameters;                  // original parameters as is without changes    
    tstring command;                        // command without spaces and prefix
    tstring parameters;                     // only parameters (without command) as single line (without trimming)
    std::vector<tstring> parameters_list;   // list of parameters separately
    tstring alias;                          // first alias (used in aliases)
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
    void erase(int pos) {
        InputCommand *cmd = base::operator[](pos);
        delete cmd;
        base::erase(begin()+pos);
    }
    void insert(int pos, InputCommands& cmds) {
        base::insert(begin() + pos, cmds.begin(), cmds.end());
        cmds.resize(0);
    }
    void clear() {
        std::for_each(begin(), end(), [](InputCommand *c){ delete c; });
    }
    void push_back(InputCommand *cmd) {
        base::push_back(cmd);
    }
};

class CompareObject;
struct InputSubcmd
{
    InputSubcmd(const tstring& cmd, bool syscmd) : srccmd(cmd), templ(cmd), system(syscmd) {}
    tstring srccmd;
    tstring templ;
    bool system;
};
class InputTemplateCommands : private std::vector<InputSubcmd>
{
    typedef std::vector<InputSubcmd> base;
public:
    void init(const InputPlainCommands& cmds, const InputTemplateParameters& params);
    void extract(InputPlainCommands* cmds);
    void makeTemplates();
    void makeCommands(InputCommands *cmds, const InputParameters* params);
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

#ifdef _DEBUG
class InputCommandTemplateUnitTest
{
    static bool test1(const tstring& str, int n, ...);
    static bool test2(const tstring& str, int params, InputCommands *ref);
    static InputCommand* makecmd(bool system, const tstring& srccmd, const tstring& srcparams, 
        const tstring& cmd, int n, ...);
public:
    static void run();
    
};
#define RUN_INPUTPROCESSOR_TESTS InputCommandTemplateUnitTest::run();
#else
#define RUN_INPUTPROCESSOR_TESTS
#endif
