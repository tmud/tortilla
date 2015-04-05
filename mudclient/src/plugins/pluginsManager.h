#pragma once

#include "plugin.h"

class PluginsManager
{
   PropertiesData *m_propData;
   PluginsList m_plugins;
   tstring m_profile;

public:
    PluginsManager(PropertiesData *props);
    ~PluginsManager();
    void loadPlugins(const tstring& group, const tstring& profile);
    void unloadPlugins();
    bool pluginsPropsDlg();
    Plugin* findPlugin(HWND view);
    void updateProps();
    void processStreamData(MemoryBuffer *data);
    void processGameCmd(tstring* cmd);
    void processViewData(const char* method, int view, parseData* data);
    void processBarCmd(tstring *cmd);
    void processHistoryCmd(tstring *cmd);
    void processConnectEvent();
    void processDisconnectEvent();
    void processTick();
    void processPluginsMethod(const char* method, int args);
    void processPluginMethod(Plugin *p, char* method, int args);

private:
    void initPlugins();
    bool doPluginsStringMethod(const char* method, tstring *str);
    bool doPluginsTableMethod(const char* method, std::vector<tstring>* table);
    void doPluginsMethod(const char* method, int args);
    void turnoffPlugin(const char* error, int plugin_index);
    void terminatePlugin(Plugin* p);
};
