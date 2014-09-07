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
    void processGameCmd(tstring& cmd);
    void processGameStrings(const char* method, int view, parseData* data);

private:
    void initPlugins();
    bool doAllPluginsMethod(const char* method, const wchar_t *text);
};
