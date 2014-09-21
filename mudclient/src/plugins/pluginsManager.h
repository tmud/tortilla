#pragma once

#include "plugin.h"
void collectGarbage();

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
    void pluginsPropsDlg();
    Plugin* findPlugin(HWND view);
    void updateProps();
    void processStreamData(MemoryBuffer *data);
    void processGameCmd(tstring* cmd);
    void processViewData(const char* method, int view, parseData* data);
    void processBarCmd(tstring *cmd);
    void processHistoryCmd(tstring *cmd);

private:
    void initPlugins();
    bool doPluginsStringMethod(const char* method, tstring *str);
    bool doPluginsTableMethod(const char* method, std::vector<tstring>* cmds);
    void turnoffPlugin(const char* method, int plugin_index);
};
