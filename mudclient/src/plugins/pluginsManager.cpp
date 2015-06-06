#include "stdafx.h"
#include "pluginsManager.h"
#include "pluginsDlg.h"
#include "pluginsApi.h"
#include "api/api.h"
#include "pluginsParseData.h"
#include "inputProcessor.h"
extern luaT_State L;
extern Plugin* _cp;

PluginsManager::PluginsManager(PropertiesData *props) : m_propData(props), m_plugins_loaded(false)
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
    if (!profile_changed && m_plugins_loaded)
        return;
    m_profile = tmp;
    initPlugins();
    m_msdp_network.loadPlugins();
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
            else {
               delete plugin;
               if (plugin_index != -1)
                  m_plugins.erase(m_plugins.begin() + plugin_index);
            }
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
    m_plugins_loaded = true;
}

void PluginsManager::unloadPlugins()
{
    m_msdp_network.unloadPlugins();
    for(int i=0,e=m_plugins.size(); i<e; ++i)
        m_plugins[i]->unloadPlugin();
    m_plugins_loaded = false;
}

bool PluginsManager::pluginsPropsDlg()
{
    initPlugins();
    PluginsDlg dlg(&m_plugins);
    if (dlg.DoModal() != IDOK)
        return false;

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
    {
        Plugin *p = turn_off[i];
        m_msdp_network.unloadPlugin(p);
        p->unloadPlugin();
    }

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
       else
       {
           m_msdp_network.loadPlugin(p);
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
    return true;
}

Plugin* PluginsManager::findPlugin(HWND view)
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        std::vector<PluginsView*> &v = m_plugins[i]->dockpanes;
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
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        if (m_plugins[i]->state())
            m_plugins[i]->updateProps();
    }
}

void PluginsManager::processStreamData(MemoryBuffer *data)
{
    tstring stream((tchar*)data->getData());
    if (doPluginsStringMethod("streamdata", &stream))
    {
        data->alloc((stream.length()+1) * sizeof(tchar));
        tchar *out = (tchar*)data->getData();
        wcscpy(out, stream.c_str());
    }
}

void PluginsManager::processGameCmd(InputCommand* cmd)
{
    bool syscmd = (!cmd->empty && cmd->command.at(0) == m_propData->cmd_prefix);
    std::vector<tstring> p;
    p.push_back((syscmd) ? cmd->command.substr(1) : cmd->command);
    p.insert(p.end(), cmd->parameters_list.begin(), cmd->parameters_list.end());
    if (syscmd)
    {
        if (doPluginsTableMethod("syscmd", &p))
            concatCommand(p, true, cmd);
        return;
    }
    if (doPluginsTableMethod("gamecmd", &p))
        concatCommand(p, false, cmd);
}

void PluginsManager::processViewData(const char* method, int view, parseData* data)
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
            // restart plugins
            turnoffPlugin(NULL, i);
            i = 0;
        }
        lua_settop(L, 0);
    }
}

void PluginsManager::processBarCmd(tstring *cmd)
{
    if (cmd->empty())
        return;
    tchar separator[2] = { m_propData->cmd_separator, 0 };
    Tokenizer t(cmd->c_str(), separator);
    std::vector<tstring> cmds;
    t.moveto(&cmds);
    if (doPluginsTableMethod("barcmd", &cmds))
    {
        cmd->clear();
        for (int i = 0, e = cmds.size(); i < e; ++i)
        {
            if (i != 0) cmd->append(separator);
            cmd->append(cmds[i]);
        }
    }
}

void PluginsManager::processHistoryCmd(tstring *cmd)
{
    if (cmd->empty())
        return;
    const char* method = "historycmd";
    WideToUtf8 w2u(cmd->c_str());
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state()) continue;
        lua_pushstring(L, w2u);
        if (!p->runMethod(method, 1, 1))
        {
            // restart plugins
            turnoffPlugin(NULL, i);
            lua_settop(L, 0);
            i = 0;
        }
        else
        {
            if (lua_isboolean(L, -1))
            {
                int r = lua_toboolean(L, -1);
                if (!r) 
                {
                    cmd->clear();
                    lua_pop(L, 1);
                    break; 
                }
            }
            lua_pop(L, 1);
        }
    }
}

void PluginsManager::processConnectEvent()
{
    doPluginsMethod("connect", 0);
}

void PluginsManager::processDisconnectEvent()
{
    doPluginsMethod("disconnect", 0);
}

void PluginsManager::processTick()
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (p->state() && p->isErrorState())
            turnoffPlugin(NULL, i);
    }
}

void PluginsManager::processPluginsMethod(const char* method, int args)
{
     doPluginsMethod(method, args);
}

void PluginsManager::processPluginMethod(Plugin *p, char* method, int args)
{
    if (!p->state())
        return;
    if (!p->runMethod(method, args, 0))
    {
        int index = -1;
        for (int i = 0, e = m_plugins.size(); i < e; ++i)
        {
            if (m_plugins[i] == p) { index = i; break;}
        }
        if (index != -1)
            turnoffPlugin(NULL, index);
        lua_settop(L, 0);
    }
}

void PluginsManager::terminatePlugin(Plugin* p)
{
    if (!p) return;
    int index = -1;
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        if (p == m_plugins[i])  { index = i; break; }
    }
    p->setOn(false);
    if (index != -1)
    {
        PluginsDataValues &modules = m_propData->plugins;
        modules[index].state = 0;
    }
}

void PluginsManager::doPluginsMethod(const char* method, int args)
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state()) continue;
        if (!p->runMethod(method, args, 0))
        {
            // restart plugins
            turnoffPlugin(NULL, i);
            lua_settop(L, 0);
            i = 0;
        }
    }
}

bool PluginsManager::doPluginsStringMethod(const char* method, tstring *str)
{
    bool processed = false;
    WideToUtf8 w2u(str->c_str());
    lua_pushstring(L, w2u);
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state()) continue;
        bool not_supported = false;
        if (!p->runMethod(method, 1, 1, &not_supported) || (!lua_isstring(L, -1) && !lua_isnil(L, -1)))
        {
            // restart plugins
            turnoffPlugin("Неверный тип полученного значения. Требуется string|nil", i);
            lua_settop(L, 0);
            lua_pushstring(L, w2u);
            i = 0;
            processed = false;
        }
        if (!not_supported)
            processed = true;
        if (lua_isnil(L, -1))
            break;
    }
    if (lua_isstring(L, -1))
    {
        if (processed)
        {
            Utf8ToWide u2w(lua_tostring(L, -1));
            str->assign(u2w);
        }
        lua_pop(L, 1);
        return processed;
    }
    lua_pop(L, 1);
    str->clear();
    return true;
}

bool PluginsManager::doPluginsTableMethod(const char* method, std::vector<tstring>* table)
{
    WideToUtf8 w2u;
    lua_newtable(L);
    for (int j = 0, je = table->size(); j < je; ++j)
    {
        const tstring& s = table->at(j);
        w2u.convert(s.c_str(), s.length());
        lua_pushinteger(L, j + 1);
        lua_pushstring(L, w2u);
        lua_settable(L, -3);
    }
    bool processed = false;
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state()) continue;
        bool not_supported = false;
        if (!p->runMethod(method, 1, 1, &not_supported) || (!lua_istable(L, -1) && !lua_isnil(L, -1)) )
        {
            // restart plugins
            turnoffPlugin("Неверный тип полученного значения. Требуется table|nil", i);
            lua_settop(L, 0);
            lua_newtable(L);
            for (int j = 0, je = table->size(); j < je; ++j)
            {
                const tstring& s = table->at(j);
                w2u.convert(s.c_str(), s.length());
                lua_pushinteger(L, j + 1);
                lua_pushstring(L, w2u);
                lua_settable(L, -3);
            }
            i = 0;
            processed = false;
        }
        if (!not_supported)
            processed = true;
        if (lua_isnil(L, -1))
            break;
    }
    if (lua_istable(L, -1))
    {
        if (processed)
        {
            lua_len(L, -1);
            int len = lua_tointeger(L, -1);
            lua_pop(L, 1);
            Utf8ToWide u2w;
            table->clear();
            for (int i = 0; i < len; ++i)
            {
                lua_pushinteger(L, i + 1);
                lua_gettable(L, -2);
                u2w.convert(lua_tostring(L, -1));
                lua_pop(L, 1);
                table->push_back(tstring(u2w));
            }
        }
        lua_pop(L, 1);
        return processed;
    }
    lua_pop(L, 1);
    table->clear();
    return false;
}

void PluginsManager::turnoffPlugin(const char* error, int plugin_index)
{
    // error in plugin - turn it off
    Plugin *p = m_plugins[plugin_index];
    Plugin *old = p;
    _cp = p;
    if (error)
        pluginError(error);
    pluginError("Плагин отключен!");
    p->setOn(false);
    _cp = old;
    PluginsDataValues &modules = m_propData->plugins;
    modules[plugin_index].state = 0;
}

void PluginsManager::processToSend(Network* network)
{
    m_msdp_network.sendExist(network);
}

void PluginsManager::concatCommand(std::vector<tstring>& parts, bool system, InputCommand* cmd)
{
    if (parts.empty())
        return;

    tstring newcmd;
    if (system)
    {
        tchar prefix[2] = { m_propData->cmd_prefix, 0 };
        newcmd.append(prefix);
    }
    newcmd.append(parts[0]);
    cmd->command.assign(newcmd);
    cmd->parameters_list.clear();

    tstring symbols(L"{}\"' ");
    tstring params;
    for (int i=1,e=parts.size();i<e;++i)
    {
        const tstring& s = parts[i];
        cmd->parameters_list.push_back(s);
        if (i != 1)
            params.append(L" ");
        if (!system)
            params.append(s);
        else
        {
            if (!isExistSymbols(s, symbols))
                params.append(s);
            else
            {
                params.append(L"{");
                params.append(s);
                params.append(L"}");
            }
        }
    }
    cmd->parameters.assign(params);
    if (!params.empty())
    {
        newcmd.append(L" ");
        newcmd.append(params);
    }
    cmd->full_command.assign(newcmd);
}
