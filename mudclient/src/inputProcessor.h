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
    virtual void doNoValues(tstring* cmd) const = 0;
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
    void move(InputPlainCommands& cmds){
        for (int i=0,e=cmds.size();i<e;++i)
            base::push_back(cmds[i]);
    }
    std::vector<tstring>* ptr() {
        return this;
    }
};

class InputRepeatCommands
{
public:
    void process(InputPlainCommands* cmds, tchar prefix)
    {
        std::vector<tstring> *pc = cmds->ptr();
        for (int i=0,e=pc->size();i<e;++i)
        {
           tstring result;
           int count = check(pc->at(i), prefix, &result);
           if (count > 0 && count <= 100)
           {
               int rec = check(result, prefix, NULL);
               if (rec >= 0)
                   break;
               std::vector<tstring> tmp(count, result);
               pc->erase(pc->begin()+i);
               pc->insert(pc->begin()+i, tmp.begin(), tmp.end());
               e = pc->size();
               i = i + count;
           }
        }
    }
private:
    int check(const tstring& cmd, tchar prefix, tstring* result)
    {
        if (cmd.empty() || cmd[0] != prefix)
            return -1;
        size_t pos = cmd.find(L' ');
        if (pos == -1) return -1;
        tstring tmp(cmd.substr(1, pos - 1));
        int count = 0;
        if (w2int(tmp, &count))
        {
            tmp.assign(cmd.substr(pos));
            tstring_trimleft(&tmp);
            if (result)
                result->assign(tmp);
            if (tmp.empty())
                return (count == 0) ? 0 : -1;
            return count;
        }
        return -1;
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
    bool empty() const { return base::empty(); }
    InputCommand* operator[] (int index) const { 
        return base::operator[](index);
    }
    void remove(int pos) {
        base::erase(begin()+pos);
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
        base::clear();
    }
    void push_back(InputCommand *cmd) {
        base::push_back(cmd);
    }
    void pop_back() {
        base::pop_back();
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
    int  size() const;
private:
    const tchar MARKER = L'\t';
    InputTemplateParameters _params;
    void fillsyscmd(InputCommand *cmd);
    void fillgamecmd(InputCommand *cmd);
    void parsecmd(const tstring& cmd);
    void markbrackets(tstring *cmd) const;
    void unmarkbrackets(tstring* parameters, std::vector<tstring>* parameters_list) const;
    bool isbracket(const tchar *p) const;
    bool isopenorspace(const tchar *p) const;
    bool iscloseorspace(const tchar *p) const;
};

class InputCommandVarsProcessor
{
public:
    bool makeCommand(InputCommand *cmd);
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
