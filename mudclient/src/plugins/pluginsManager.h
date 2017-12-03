#pragma once

#include "plugin.h"
#include "msdp/msdpNetwork.h"
#include "pluginsTriggersHandler.h"

class PluginsManager : public PluginsTriggersHandler
{
   PluginsList m_plugins;
   tstring m_profile;
   MsdpNetwork m_msdp_network;
   bool m_plugins_loaded;

public:
    PluginsManager();
    ~PluginsManager();
    void loadPlugins(const tstring& group, const tstring& profile);
    void unloadPlugins();
    bool pluginsPropsDlg();
    Plugin* findPlugin(HWND view);
    Plugin* findPlugin(const tstring& name);
    bool setPluginState(const tstring& name, const tstring& state);
    void updateProps();
    void processStreamData(MemoryBuffer *data);
    void processGameCmd(InputCommand cmd);
    void processViewData(const char* method, int view, parseData* data);
    void processBarCmds(InputPlainCommands* cmds);
    void processHistoryCmds(const InputPlainCommands& cmds, InputPlainCommands* history);
    void processConnectEvent();
    void processDisconnectEvent();
    void processTick();
    void processSecondTick();
    void processDebugTick();
    void processPluginsMethod(const char* method, int args);
    void processPluginMethod(Plugin *p, char* method, int args);
    MsdpNetwork* getMsdp() { return &m_msdp_network; }
    bool processMouseWheel(const POINT& pt, DWORD param);

private:
    bool processTriggers(parseData& parse_data, int string, std::vector<TriggerAction>& triggers );
    void concatCommand(std::vector<tstring>& parts, bool system, InputCommand cmd);
    void initPlugins();
    bool doPluginsStringMethod(const char* method, tstring *str);
    enum TableMethodResult { TM_NOTPROCESSED = 0, TM_PROCESSED, TM_DROPPED, TM_CHANGED };
    TableMethodResult doPluginsTableMethod(const char* method, std::vector<tstring>* table, tstring* error_msg);
    void doPluginsMethod(const char* method, int args);
    void turnoffPlugin(const tchar* error, int plugin_index);
    void setPluginState(Plugin* p, bool state);
    void terminatePlugin(Plugin* p);
    bool loadPlugin(Plugin* p);
    void unloadPlugin(Plugin *p);
};
