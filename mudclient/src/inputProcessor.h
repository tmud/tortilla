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

struct InputTemplateParameters
{
    tchar separator;
    tchar prefix;
};

struct InputCommandData
{
    InputCommandData() : dropped(false), system(false), changed(false) {}
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
typedef std::shared_ptr<InputCommandData> InputCommand;

class InputCommands : private std::deque<InputCommand>
{
    typedef std::deque<InputCommand> base;
public:
    ~InputCommands() { clear(); }
    int size() const { return base::size(); }
    bool empty() const { return base::empty(); }
    InputCommand operator[] (int index) const { 
        return base::operator[](index);
    }
    void erase(int pos) {
        base::erase(begin()+pos);
    }
    void insert(int pos, InputCommands& cmds) {
        base::insert(begin() + pos, cmds.begin(), cmds.end());
        cmds.resize(0);
    }
    void push_back(InputCommands& cmds) {
        base::insert(end(), cmds.begin(), cmds.end());
        cmds.resize(0);
    }
    void clear() {
        base::clear();
    }
    void push_back(InputCommand cmd) {
        base::push_back(cmd);
    }
    void pop_back() {
        base::pop_back();
    }
    InputCommand pop_front()
    {
        InputCommand cmd = at(0);
        base::pop_front();
        return cmd;
    }
    void repeat(size_t count) 
    {
        if (count <= 1) return;
        size_t size = base::size();
        size_t newsize = count * size;
        base::resize(newsize);
        for (size_t k=1;k<count;++k)
        {
            for (size_t i=0;i<size;++i)
            {
                InputCommand c = at(i);
                size_t ci = k*size+i;
                at(ci) = std::make_shared<InputCommandData>(*c);
            }
        }
    }
    void append(InputCommands& cmds, int from)
    {
        base::insert(end(), cmds.begin()+from, cmds.end());
    }
    void swap(InputCommands& cmds)
    {
        base::swap(cmds);
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
    void fillsyscmd(InputCommand cmd);
    void fillgamecmd(InputCommand cmd);
    void parsecmd(const tstring& cmd);
    void markbrackets(tstring *cmd) const;
    void unmarkbrackets(tstring* parameters, std::vector<tstring>* parameters_list) const;
    bool isbracket(const tchar *p) const;
    bool isopenorspace(const tchar *p) const;
    bool iscloseorspace(const tchar *p) const;
    bool isbracketorspace(const tchar *p) const;
};

class InputCommandsVarsFilter
{
public:
    bool checkFilter(InputCommand cmd);
};

class InputCommandVarsProcessor
{
public:
    bool makeCommand(InputCommand cmd);
};

#ifdef _DEBUG
class InputCommandTemplateUnitTest
{
    static bool test1(const tstring& str, int n, ...);
    static bool test2(const tstring& str, int params, InputCommands *ref);
    static InputCommand makecmd(bool system, const tstring& srccmd, const tstring& srcparams, 
        const tstring& cmd, int n, ...);
public:
    static void run();
};
#define RUN_INPUTPROCESSOR_TESTS InputCommandTemplateUnitTest::run();
#else
#define RUN_INPUTPROCESSOR_TESTS
#endif
