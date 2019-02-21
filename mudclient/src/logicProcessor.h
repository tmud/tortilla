﻿#pragma once

#include "mudViewParser.h"
#include "logicHelper.h"
#include "logicPipeline.h"
#include "logsProcessor.h"
#include "network/network.h"
#include "waitCmds.h"
#include "plugins/pluginsViewString.h"

class LogicProcessorHost
{
public:
    virtual void connectToNetwork(const tstring& address, int port) = 0;
    virtual void disconnectFromNetwork() = 0;
    virtual void sendToNetwork(const tstring& data) = 0;
    virtual MudViewString* getLastString(int view) = 0;
    virtual bool saveViewData(int view, tstring& filename) = 0;
    virtual void accLastString(int view, parseData* parse_data) = 0;
    virtual void preprocessText(int view, parseData* parse_data) = 0;
    virtual void postprocessText(int view, parseData* parse_data) = 0;
    virtual void addText(int view, parseData* parse_data) = 0;
    virtual void clearText(int view) = 0;
    virtual void showWindow(int view, bool show) = 0;
    virtual void setWindowName(int view, const tstring& name) = 0;
    virtual void getMccpStatus(MccpStatus *status) = 0;
    virtual HWND getMainWindow() = 0;
    virtual void preprocessCommand(InputCommand cmd) = 0;
    virtual void setOscColor(int index, COLORREF color) = 0;
    virtual void resetOscColors() = 0;
    virtual PluginsTriggersHandler* getPluginsTriggers() = 0;
    virtual void clearDropped(int view) = 0;
    virtual void loadProfile(const tstring& profile, const tstring& group, tstring* error) = 0;
    virtual void lockWindow(int view, bool lockmode) = 0;
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
    virtual void processPluginCommand(const tstring& cmd) = 0;
    virtual bool getConnectionState() = 0;
    virtual void windowOutput(int window, const std::vector<tstring>& msgs) = 0;
    virtual void pluginsOutput(int window, const MudViewStringBlocks& v) = 0;
    virtual void windowClear(int window) = 0;
    virtual bool setComponent(const tstring& name, bool mode) = 0;
};

class parser;
#define DEF(fn) void impl_##fn(parser*);
typedef void(*syscmd_fun)(parser*);

class LogicProcessor : public LogicProcessorMethods
{
    LogicProcessorHost *m_pHost;
    MudViewParser m_parser;
    LogicHelper m_helper; 
    bool m_connecting;
    bool m_connected;
    tstring m_updatelog;
    LogsProcessor m_logs;
    int m_wlogs[OUTPUT_WINDOWS+1];
    int m_clog;
    std::map<tstring, syscmd_fun> m_syscmds;
    std::vector<tstring> m_plugins_cmds;
    Pcre16 m_prompt_pcre;
    MudViewParser m_parser2;
    struct stack_el {
        tstring text;
        int flags;
    };
    std::vector<stack_el> m_incoming_stack;
    enum PromptMode { OFF = 0, USER, UNIVERSAL, IACGA };
    PromptMode m_prompt_mode;
    int  m_prompt_counter;
    Pcre16 m_univ_prompt_pcre;
    std::vector<tstring> m_plugins_log_cache;
    std::vector<tstring> m_plugins_log_toblocked;
    bool m_plugins_log_tocache;
    bool m_plugins_log_blocked;
    WaitCommands m_waitcmds;
    LogicPipeline m_pipeline;
    InputCommands m_commands_queue;
    LogicTriggered m_triggered_debug;
public:
    LogicProcessor(LogicProcessorHost *host);
    ~LogicProcessor();
    bool init();
    void processNetworkData(const WCHAR* text, int text_len);
    void processNetworkConnect();
    void processNetworkDisconnect();
    void processNetworkConnectError();
    void processNetworkError();
    void processNetworkMccpError();
    void processUserCommand(const InputPlainCommands& cmds);
    void processPluginCommand(const tstring& cmd);
    bool processHotkey(const tstring& hotkey);
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
    bool getConnectionState() { return m_connected; }
    void windowClear(int window);
    void windowOutput(int window, const std::vector<tstring>& msgs);
    void pluginsOutput(int window, const MudViewStringBlocks& v);
private:
    void processCommand(const tstring& cmd);
    void processCommands(const InputPlainCommands& cmds);
    void processQueueCommand();
    void makeCommands(const InputPlainCommands& cmds, InputCommands* rcmds);
    void runCommands(InputCommands& cmds);
    void runCommand(InputCommand cmd, InputCommands& inserts);
    bool processAliases(InputCommands& cmds);
    void syscmdLog(const tstring& cmd);
    void recognizeSystemCommand(tstring* cmd, tstring* error);
    void processSystemCommand(InputCommand cmd);
    void processGameCommand(InputCommand cmd);
    bool setComponent(const tstring& name, bool mode);
    enum { SKIP_NONE = 0, SKIP_ACTIONS = 0x1, SKIP_SUBS = 0x2, SKIP_HIGHLIGHTS = 0x4,
           SKIP_PLUGINS_BEFORE = 0x8, SKIP_PLUGINS_AFTER = 0x10, SKIP_PLUGINS = 0x18,
           SKIP_COMPONENT_GAGS = 0x20, SKIP_COMPONENT_SUBS = 0x40,
           SKIP_COMPONENT_ANTISUBS = 0x80, SKIP_COMPONENT_PLUGINS = 0x100,
           WORK_OFFLINE = 0x200, GAME_LOG = 0x400, GAME_CMD = 0x800, FROM_STACK = 0x1000,
           FROM_TIMER = 0x2000, NEW_LINE = 0x4000 };
    void updateLog(const tstring& msg);
    void updateProps(int update, int options);
    void regCommand(const char* name, syscmd_fun f, bool skip_autoset = false);
    bool sendToNetwork(const tstring& cmd);
    void processNetworkError(const tstring& error);

    // Incoming data methods
    void processIncoming(const WCHAR* text, int text_len, int flags, int window);
    void printIncoming(parseData& parse_data, int flags, int window);
    void pipelineParseData(parseData& parse_data, int flags, int window);
    void printParseData(parseData& parse_data, int flags, int window, LogicPipelineElement *pe);
    void processLuaTriggers(parseData& parse_data, int flags, LogicPipelineElement *pe);
    void processActionsTriggers(parseData& parse_data, int flags, LogicPipelineElement *pe, LogicTriggered* triggered);
    void printStack(int flags = 0);
    bool processStack(parseData& parse_data, int flags);
    LogicTriggered* triggered(int mode);
    void printTriggered(parseData& parse_data, int mode, const tstring& prefix, bool process_highlights);

public: // system commands
    DEF(drop);
    DEF(stop);
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
    void printex(int view, const parser* p, int from, bool enable_actions_subs);
    void printex(int view, const std::vector<tstring>& params, bool enable_actions_subs);
    DEF(wprint);
    DEF(print);
    DEF(message);
    DEF(component);
    DEF(tab);
    DEF(untab);
    DEF(timer);
    DEF(untimer);
    DEF(uptimer);
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
    void clogf(parser *p, bool newlog);
    void clogf_main(const tstring& file, bool newlog);
    DEF(clog);
    DEF(clogn);
    DEF(wname);
    DEF(var);
    DEF(unvar);
    DEF(wait);
    DEF(plugin);
    DEF(load);
    DEF(savelog);
    DEF(none);
    DEF(wlock);
    DEF(wunlock);
    DEF(debug_tr);
    DEF(strop);
};
