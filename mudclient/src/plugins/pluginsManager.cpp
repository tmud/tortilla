#include "stdafx.h"
#include "pluginsManager.h"
#include "pluginsDlg.h"
#include "pluginsApi.h"
#include "api/api.h"
#include "pluginsParseData.h"
extern luaT_State L;

void collectGarbage()
{
    if (L)
      lua_gc(L, LUA_GCSTEP, 1);
}

PluginsManager::PluginsManager(PropertiesData *props) : m_propData(props)
{ 
}

PluginsManager::~PluginsManager() 
{
     autodel<Plugin> _z(m_plugins);
}

void PluginsManager::loadPlugins(const tstring& group, const tstring& profile)
{
    tstring tmp(group);
    tmp.append(profile);
    bool profile_changed = (tmp == m_profile) ? false : true;
    if (!profile_changed)
        return;
    m_profile = tmp;
    initPlugins();
}

void PluginsManager::initPlugins()
{
    std::vector<tstring> files;
    {
        ChangeDir cd;
        if (!cd.changeDir(L"plugins"))
            return;

        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(L"*.*", &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    if (Plugin::isPlugin(fd.cFileName))
                        files.push_back(fd.cFileName);
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
    }

    for (int j = 0, je = files.size(); j < je; ++j)
    {
        int plugin_index = -1;
        for (int i = 0, e = m_plugins.size(); i < e; ++i)
            if (files[j] == m_plugins[i]->get(Plugin::FILE)) { plugin_index = i; break; }
        Plugin *plugin = (plugin_index != -1) ? m_plugins[plugin_index] : new Plugin;
        if (!plugin->isloaded())
        {
            if (plugin->loadPlugin(files[j].c_str()))
            {
                if (plugin_index == -1)
                    m_plugins.push_back(plugin);
            }
            else { delete plugin; }
        }
    }
    files.clear();

    PluginsDataValues &modules = m_propData->plugins;
    PluginsDataValues new_modules;
    for (int i = 0, e = modules.size(); i < e; ++i)
    {
        const PluginData& v = modules[i];
        bool exist = false;
        for (int j = 0, je = m_plugins.size(); j < je; ++j)
            if (v.name == m_plugins[j]->get(Plugin::FILE)) { exist = true; break; }
        if (exist) new_modules.push_back(v);
    }
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        const tstring& name = m_plugins[i]->get(Plugin::FILE);
        bool exist = false;
        for (int j = 0, je = new_modules.size(); j < je; ++j)
            if (new_modules[j].name == name) { exist = true; break; }
        if (!exist) { PluginData pd; pd.name = name; pd.state = 0; new_modules.push_back(pd); }
    }
    modules.swap(new_modules);      

    // sort and set initial state
    PluginsList new_plugins;
    for (int i = 0, e = modules.size(); i < e; ++i)
    {
        const PluginData& v = modules[i];
        for (int j = 0, je = m_plugins.size(); j < je; ++j)
        if (v.name == m_plugins[j]->get(Plugin::FILE))
        {
            new_plugins.push_back(m_plugins[j]); break;
        }       
        bool state = (v.state == 1) ? true : false;
        new_plugins[i]->setOn(state);
    }
    m_plugins.swap(new_plugins);
}

void PluginsManager::unloadPlugins()
{
    for(int i=0,e=m_plugins.size(); i<e; ++i)
        m_plugins[i]->unloadPlugin();
}

void PluginsManager::pluginsPropsDlg()
{
    initPlugins();
    PluginsDlg dlg(&m_plugins);
    if (dlg.DoModal() != IDOK)
        return;

    // grouping plugins to turning off and turning on
    PluginsList turn_on, turn_off;
    for (int i=0,e=m_plugins.size(); i<e; ++i)
    {
        Plugin *p = m_plugins[i];
        bool new_state = dlg.getNewState(i);
        if (p->state() != new_state)
        {
            if (new_state)
                turn_on.push_back(p);
            else
                turn_off.push_back(p);            
        }
    }

    // turn off plugins first
    for (int i = 0, e = turn_off.size(); i < e; ++i)
        turn_off[i]->unloadPlugin();
 
    // turn on new plugins
    for (int i=0,e=turn_on.size(); i<e; ++i)
    {
        Plugin *p = turn_on[i];
        if (!p->reloadPlugin())
        {
            tstring error(L"Ошибка при загрузке плагина '");
            error.append(p->get(Plugin::FILE));
            error.append(L"'. Плагин работать не будет.");
            tmcLog(error.c_str());
       }
    }
        
    PluginsDataValues &modules = m_propData->plugins;
    PluginsDataValues new_modules;
    for (int i=0,e=m_plugins.size(); i<e; ++i)
    {
        Plugin *p = m_plugins[i];
        tstring name = p->get(Plugin::FILE);
        for (int j = 0, je = modules.size(); j < je; ++j)
        {
            if (modules[j].name == name)
            {
                PluginData& v = modules[j];
                v.state = p->state() ? 1 : 0;
                new_modules.push_back(v);
                break;
            }
        }
    }
    modules.swap(new_modules);
}

Plugin* PluginsManager::findPlugin(HWND view)
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        std::vector<PluginsView*> &v = m_plugins[i]->views;
        for (int j = 0, je = v.size(); j < je; ++j)
        {
            if (*v[j] == view)
                { return m_plugins[i]; }
        }
    }
    return NULL;
}

void PluginsManager::updateProps()
{
}

void PluginsManager::processStreamData(MemoryBuffer *data)
{
    if (doAllPluginsMethod("streamdata", (wchar_t*)data->getData()))
    {
        const char* newdata = lua_tostring(L, -1);
        int newdata_len = strlen(newdata);
        Utf8ToWideConverter u2w;
        u2w.convert(data, newdata, newdata_len);
        lua_pop(L, 1);
    }
}

void PluginsManager::processGameCmd(tstring& cmd)
{
    if (!cmd.empty() && cmd.at(0) == m_propData->cmd_prefix)
    {
        if (doAllPluginsMethod("syscmd", cmd.c_str() + 1))
        {
            Utf8ToWide u2w(lua_tostring(L, -1));
            lua_pop(L, 1);
            tchar prefix[2] = { m_propData->cmd_prefix, 0 };
            cmd.assign(prefix);
            cmd.append(u2w);
        }
        return;
    }

    if (doAllPluginsMethod("gamecmd", cmd.c_str()))
    {
        Utf8ToWide u2w(lua_tostring(L, -1));
        lua_pop(L, 1);
        cmd.assign(u2w);
    }
}

void PluginsManager::processGameStrings(const char* method, int view, parseData* data)
{
    PluginsParseData pdata(data);
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state()) continue;       
        lua_pushinteger(L, view);
        luaT_pushobject(L, &pdata, LUAT_VIEWDATA);
        if (!p->runMethod(method, 2, 0))
        {
            pluginError(L, method, "Ошибка в методе. Плагин отключен!");
            p->setOn(false);
            PluginsDataValues &modules = m_propData->plugins;
            modules[i].state = 0;
            i = 0; // restart plugins
        }
        lua_settop(L, 0);       
    }
}

bool PluginsManager::doAllPluginsMethod(const char* method, const wchar_t *text)
{
    WideToUtf8 w2u(text);
    lua_pushstring(L, w2u);
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state()) continue;

        if (!p->runMethod(method, 1, 1) || !lua_isstring(L, -1))
        {
            // error in plugin - turn it off
            pluginError(L, method, "Ошибка в методе. Плагин отключен!");
            p->setOn(false);
            PluginsDataValues &modules = m_propData->plugins;
            modules[i].state = 0;
            
            // restart plugins
            lua_settop(L, 0);            
            lua_pushstring(L, w2u);
            i = 0;
        }        
    }    
    if (lua_isstring(L, -1))
        return true;
    lua_settop(L, 0);
    return false;
}
