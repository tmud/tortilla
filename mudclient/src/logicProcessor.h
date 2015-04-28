#pragma once

#include "mudViewParser.h"
#include "logicHelper.h"
#include "inputProcessor.h"
#include "logsProcessor.h"
#include "network/network.h"

class LogicProcessorHost
{
public:
    virtual void connectToNetwork(const tstring& address, int port) = 0;
    virtual void disconnectFromNetwork() = 0;
    virtual void sendToNetwork(const tstring& data) = 0;    
    virtual MudViewString* getLastString(int view) = 0;
    virtual void accLastString(int view, parseData* parse_data) = 0;
    virtual void preprocessText(int view, parseData* parse_data) = 0;
    virtual void postprocessText(int view, parseData* parse_data) = 0;
    virtual void addText(int view, parseData* parse_data) = 0;
    virtual void clearText(int view) = 0;
    virtual void showWindow(int view, bool show) = 0;
    virtual void setWindowName(int view, const tstring& name) = 0;
    virtual void getMccpStatus(MccpStatus *status) = 0;
    virtual HWND getMainWindow() = 0;
    virtual void preprocessGameCmd(tstring* cmd) = 0;
    virtual void setOscColor(int index, COLORREF color) = 0;
    virtual void resetOscColors() = 0;
};

class LogicProcessorMethods
{
public:
    virtual void tmcLog(const tstring& msg) = 0;
    virtual void simpleLog(const tstring& msg) = 0;
    virtual void pluginLog(const tstring& msg) = 0;
    virtual void updateLog(const tstring& msg) = 0;
    virtual void updateActiveObjects(int type) = 0;
    virtual bool checkActiveObjectsLog(int type) = 0;
    virtual bool addSystemCommand(const tstring& cmd) = 0;
    virtual bool deleteSystemCommand(const tstring& cmd) = 0;
    virtual void doGameCommand(const tstring& cmd) = 0;
    virtual bool getConnectionState() = 0;
    virtual bool canSetVar(const tstring& var) = 0;
    virtual bool getVar(const tstring& var, tstring* value) = 0;
};

class parser;
#define DEF(fn) void impl_##fn(parser*);
typedef void(*syscmd_fun)(parser*);

class LogicProcessor : public LogicProcessorMethods
{
    PropertiesData *propData;
    LogicProcessorHost *m_pHost;
    MudViewParser m_parser;
    InputProcessor m_input;
    LogicHelper m_helper; 
    bool m_connecting;
    bool m_connected;
    tstring m_updatelog;
    LogsProcessor m_logs;
    int m_wlogs[OUTPUT_WINDOWS+1];
    std::map<tstring, syscmd_fun> m_syscmds;
    std::vector<tstring> m_plugins_cmds;
    Pcre16 m_prompt_pcre;
    MudViewParser m_parser2;
    struct stack_el {
        tstring text;
        int flags;
    };
    std::vector<stack_el> m_incoming_stack;
    enum PromptMode { OFF = 0, USER, UNIVERSAL };
    PromptMode m_prompt_mode;
    int  m_prompt_counter;
    Pcre16 m_univ_prompt_pcre;

public:
    LogicProcessor(PropertiesData *data, LogicProcessorHost *host);
    ~LogicProcessor();
    bool init();
    void processNetworkData(const WCHAR* text, int text_len);
    void processNetworkConnect();
    void processNetworkDisconnect();
    void processNetworkConnectError();
    void processNetworkError();
    void processNetworkMccpError();
    bool processHotkey(const tstring& hotkey);
    void processCommand(const tstring& cmd);
    void processTick();
    void processStackTick();
    void updateProps();
    void tmcLog(const tstring& cmd);
    void simpleLog(const tstring& cmd);
    void pluginLog(const tstring& cmd);
    void updateActiveObjects(int type);
    bool checkActiveObjectsLog(int type);
    bool addSystemCommand(const tstring& cmd);
    bool deleteSystemCommand(const tstring& cmd);
    void doGameCommand(const tstring& cmd);
    bool getConnectionState() { return m_connected; }
    bool canSetVar(const tstring& var)  { return m_helper.canSetVar(var); }    
    bool getVar(const tstring& var, tstring* value) { return m_helper.getVar(var, value); }

private:
    void syscmdLog(const tstring& cmd);
    void processSystemCommand(InputCommand* cmd);
    void processGameCommand(InputCommand* cmd);
    enum { SKIP_ACTIONS = 1, SKIP_SUBS = 2, SKIP_HIGHLIGHTS = 4, SKIP_PLUGINS = 8, GAME_LOG = 16, GAME_CMD = 32, 
           FROM_STACK = 64, FROM_TIMER = 128 };
    void updateLog(const tstring& msg);
    void updateProps(int update, int options);
    void regCommand(const char* name, syscmd_fun f);
    bool sendToNetwork(const tstring& cmd);
    void processNetworkError(const tstring& error);

    // Incoming data methods
    void processIncoming(const WCHAR* text, int text_len, int flags = 0, int window = 0);
    void printIncoming(parseData& parse_data, int flags, int window);
    void printParseData(parseData& parse_data, int flags, int window);
    void printStack(int flags = 0);
    bool processStack(parseData& parse_data, int flags);

public: // system commands
    DEF(drop);
    DEF(action);
    DEF(unaction);
    DEF(alias);
    DEF(unalias);
    DEF(clear);
    DEF(connect);
    DEF(cr);
    DEF(disconnect);
    DEF(sub);
    DEF(unsub);
    DEF(hotkey);
    DEF(unhotkey);
    DEF(help);
    DEF(password);
    DEF(hide);
    DEF(highlight);
    DEF(math);
    DEF(ifop);
    DEF(unhighlight);
    DEF(gag);
    DEF(ungag);
    DEF(antisub);
    DEF(unantisub);
    DEF(group);
    DEF(mccp);
    DEF(wshow);
    DEF(whide);
    DEF(wpos);
    void printex(int view, const std::vector<tstring>& params);
    DEF(wprint);
    DEF(print);
    DEF(message);
    DEF(tab);
    DEF(untab);
    DEF(timer);
    DEF(hidewindow);
    DEF(showwindow);
    void wlogf_main(int log, const tstring& file, bool newlog);
    void logf(parser *p, bool newlog);
    DEF(logs);
    DEF(logn);
    void wlogf(parser *p, bool newlog);
    void invalidwindow(parser *p, int view0, int view);
    DEF(wlog);
    DEF(wlogn);
    DEF(wname);
    DEF(var);
    DEF(unvar);
};
