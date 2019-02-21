﻿#include "stdafx.h"
#include "accessors.h"
#include "pluginsManager.h"
#include "pluginsDlg.h"
#include "pluginsApi.h"
#include "api/api.h"
#include "pluginsParseData.h"
#include "inputProcessor.h"
extern luaT_State L;
extern Plugin* _cp;
extern wchar_t* plugin_buffer();

PluginsManager::PluginsManager() : m_plugins_loaded(false)
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
                    if (Plugin::isPluginEnabled(fd.cFileName))
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

    PluginsDataValues* modules = tortilla::pluginsData();
    PluginsDataValues new_modules;
    for (int i = 0, e = modules->size(); i < e; ++i)
    {
        const PluginData& v = modules->at(i);
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
    modules->swap(new_modules);

    // sort and set initial state
    PluginsList new_plugins;
    for (int i = 0, e = modules->size(); i < e; ++i)
    {
        const PluginData& v = modules->at(i);
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
        unloadPlugin(turn_off[i]);

    // turn on new plugins
    for (int i=0,e=turn_on.size(); i<e; ++i)
        loadPlugin(turn_on[i]);

    PluginsDataValues* modules = tortilla::pluginsData();
    PluginsDataValues new_modules;
    for (int i=0,e=m_plugins.size(); i<e; ++i)
    {
        Plugin *p = m_plugins[i];
        tstring name = p->get(Plugin::FILE);
        for (int j = 0, je = modules->size(); j < je; ++j)
        {
            if (modules->at(j).name == name)
            {
                PluginData& v = modules->at(j);
                v.state = p->state() ? 1 : 0;
                new_modules.push_back(v);
                break;
            }
        }
    }
    modules->swap(new_modules);
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

Plugin* PluginsManager::findPlugin(const tstring& name)
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        if (!name.compare(m_plugins[i]->get(Plugin::FILENAME)))
           return m_plugins[i];
    }
    return NULL;
}

bool PluginsManager::setPluginState(const tstring& name, const tstring& state)
{
    Plugin *p = findPlugin(name);
    if (!p)
    {
        initPlugins();
        p = findPlugin(name);
        if (!p)
        {
            tstring error(L"Ошибка при загрузке плагина '");
            error.append(name);
            error.append(L"'.");
            tmcLog(error.c_str());
            return true;
        }
    }
    if (state == L"on" || state == L"1" || state == L"load")
    {
        if (!p->state())
        {
            loadPlugin(p);
            setPluginState(p, true);
        }
        return true;
    }
    if (state == L"off" || state == L"0" || state == L"unload")
    {
        if (p->state())
        {
            unloadPlugin(p);
            setPluginState(p, false);
        }
        return true;
    }
    if (state == L"reload" || state == L"up")
    {
        if (p->state())
            unloadPlugin(p);
        loadPlugin(p);
        setPluginState(p, true);
        return true;
    }
    return false;
}

void PluginsManager::updateProps()
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (p->state())
            p->updateProps();
    }
}

bool PluginsManager::processMouseWheel(const POINT& pt, DWORD param)
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (p->state() && p->processMouseWheel(pt, param))
            return true;
    }
    return false;
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

void PluginsManager::processGameCmd(InputCommand cmd)
{
    std::vector<tstring> p;
    p.push_back(cmd->command);
    p.insert(p.end(), cmd->parameters_list.begin(), cmd->parameters_list.end());
    tstring error_msg;
    PluginsManager::TableMethodResult result = (cmd->system) ? doPluginsTableMethod("syscmd", &p, &error_msg) : doPluginsTableMethod("gamecmd", &p, &error_msg);
    if (result == TM_CHANGED)
        concatCommand(p, cmd->system, cmd);
    if (result == TM_DROPPED)
    {
        cmd->dropped = true;
        if (!error_msg.empty())
        {
            tstring src(cmd->srccmd);
            src.append(cmd->parameters);
            swprintf(plugin_buffer(), L"%s (%s)", error_msg.c_str(), src.c_str());
            pluginOut(plugin_buffer());
        }
    }
    if (result == TM_PROCESSED)
    {
        cmd->changed = true;
        cmd->command.clear();
    }
}

void PluginsManager::processViewData(const char* method, int view, parseData* data)
{
    PluginsParseData pdata(data, NULL);
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state() || p->isErrorState()) continue;
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

bool PluginsManager::processTriggers(parseData& parse_data, int string, std::vector<TriggerAction>& triggers)
{
    int i = string; int last = parse_data.strings.size() - 1;
    MudViewString *s = parse_data.strings[i];
    CompareData cd(s);
    bool incomplstr = (i == last && !parse_data.last_finished);

    bool processed = false;
    for (int j = 0, je = m_plugins.size(); j < je; ++j)
    {
        Plugin *p = m_plugins[j];
        if (!p->state() || p->isErrorState())
            continue;
        std::vector<PluginsTrigger*>& vt = p->triggers;
        if (vt.empty())
            continue;
        for (int k = 0, ke = vt.size(); k < ke; ++k)
        {
            PluginsTrigger *t = vt[k];
            if (!t->isEnabled())
                continue;
            TriggerAction action = t->compare(cd, incomplstr);
            if (action)
            {
                triggers.push_back(action);
                // проверка всех триггеров на эту строку
                processed = true;
            }
        }
    }
    return processed;
}

void PluginsManager::processBarCmds(InputPlainCommands* cmds)
{
    assert(cmds);
    if (cmds->empty())
        return;
    doPluginsTableMethod("barcmd", cmds->ptr(), NULL);
}

void PluginsManager::processHistoryCmds(const InputPlainCommands& cmds, InputPlainCommands* history)
{
    assert(history);
    if (cmds.empty())
        return;

    const char* method = "historycmd";
    WideToUtf8 w2u;
    for (int j = 0, je = cmds.size(); j<je; ++j) 
    {
        tstring cmd(cmds[j]);
        if (cmd.empty())
            continue;
        for (int i = 0, e = m_plugins.size(); i < e; ++i)
        {
            Plugin *p = m_plugins[i];
            if (!p->state()) continue;
            w2u.convert(cmd.c_str(), cmd.length());
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
                        cmd.clear();
                        lua_pop(L, 1);
                        break; 
                    }
                }
                lua_pop(L, 1);
            }
        }
        if (!cmd.empty())
            history->push_back(cmd);
    }
}

void PluginsManager::processConnectEvent()
{
    doPluginsMethod("connect", 0);
}

void PluginsManager::processDisconnectEvent()
{
    m_msdp_network.reset();
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

void PluginsManager::processSecondTick()
{
    doPluginsMethod("tick", 0);
}

void PluginsManager::processDebugTick()
{
#ifdef _DEBUG
    doPluginsMethod("debugtick", 0);
#endif
}

void PluginsManager::processPluginsMethod(const char* method, int args)
{
    doPluginsMethod(method, args);
}

void PluginsManager::processPluginsMethodCopyArgs(const char* method, int args)
{
    // save args
    std::deque<lua_ref> refs;
    for (int i = 0; i < args; ++i) {
        lua_ref ref;
        ref.createRef(L);
        refs.push_front(ref);
    }
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state() || p->isErrorState()) continue;
        // restore args
        for (lua_ref &r : refs) {
            r.pushValue(L);
        }
        bool not_supported = false;
        if (!p->runMethod(method, args, 0, &not_supported))
        {
            // restart plugins
            turnoffPlugin(NULL, i);
            lua_settop(L, 0);
            i = 0;
        }
        if (not_supported)
            lua_settop(L, 0);
    }
    // remove args
    for (lua_ref &r : refs){
        r.unref(L);
    }
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

void PluginsManager::setPluginState(Plugin* p, bool state)
{
    if (!p) return;
    int index = -1;
    for (int i = 0, e = m_plugins.size(); i < e; ++i) {
        if (p == m_plugins[i])  { index = i; break; }
    }
    if (index != -1)
    {
        PluginsDataValues* modules = tortilla::pluginsData();
        modules->at(index).state = (state) ? 1 : 0;
    }
}

void PluginsManager::terminatePlugin(Plugin* p)
{
    if (!p) return;
    p->setOn(false);
    setPluginState(p, false);
}

bool PluginsManager::loadPlugin(Plugin* p)
{
    if (!p->reloadPlugin())
    {
        tstring error(L"Плагин '");
        error.append(p->get(Plugin::FILE));
        error.append(L"'отключен.");
        pluginOut(error.c_str());
        return false;
    }
    m_msdp_network.loadPlugin(p);
    return true;
}

void PluginsManager::unloadPlugin(Plugin *p)
{
    m_msdp_network.unloadPlugin(p);
    p->unloadPlugin();
}

void PluginsManager::doPluginsMethod(const char* method, int args)
{
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state() || p->isErrorState()) continue;
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
        if (!p->state() || p->isErrorState()) continue;
        bool not_supported = false;
        if (!p->runMethod(method, 1, 1, &not_supported) || (!lua_isstring(L, -1) && !lua_isnil(L, -1)))
        {
            // restart plugins
			tstring msg(L"Неверный тип результата из метода '");
			msg.append(TA2W(method));
			msg.append(L"' Требуется string|nil");
            turnoffPlugin(msg.c_str(), i);
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

PluginsManager::TableMethodResult PluginsManager::doPluginsTableMethod(const char* method, std::vector<tstring>* table, tstring* error_msg)
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
    TableMethodResult result = TM_NOTPROCESSED;
    for (int i = 0, e = m_plugins.size(); i < e; ++i)
    {
        Plugin *p = m_plugins[i];
        if (!p->state() || p->isErrorState()) continue;
        bool not_supported = false;
        if (!p->runMethod(method, 1, 1, &not_supported) || (!lua_istable(L, -1) && !lua_isnil(L, -1) && !lua_isboolean(L, -1) && !lua_isstring(L, -1)) )
        {
            // restart plugins
            tstring msg(L"Неверный тип результата из метода '");
            msg.append(TA2W(method));
            msg.append(L"'. Требуется table|nil|boolean|string");
            turnoffPlugin(msg.c_str(), i);
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
            result = TM_NOTPROCESSED;
        }
        if (!not_supported)
            result = TM_CHANGED;
        if (lua_isnil(L, -1)) 
        {
            result = TM_PROCESSED;
            break;
        }
        if (lua_isboolean(L, -1))
        {
            int res = lua_toboolean(L, -1);
            result = (!res) ? TM_DROPPED : TM_PROCESSED;
            break;
        }
        if (lua_isstring(L, -1))
        {
            result = TM_DROPPED;
            if (error_msg)
            {
                tstring msg(TU2W (lua_tostring(L,-1)));
                swprintf(plugin_buffer(), L"'%s': %s", p->get(Plugin::FILE), msg.c_str() );
                error_msg->assign( plugin_buffer() );
            }
             break;
        }
    }
    if (lua_istable(L, -1) && result == TM_CHANGED)
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
    return result;
}

void PluginsManager::turnoffPlugin(const tchar* error, int plugin_index)
{
    // error in plugin - turn it off
    Plugin *p = m_plugins[plugin_index];
    Plugin *old = _cp;
    _cp = p;
    if (error)
        pluginLog(error);
    swprintf(plugin_buffer(), L"Плагин %s отключен!", p->get(Plugin::FILE));
    pluginOut(plugin_buffer());
    p->setOn(false);
    _cp = old;
    PluginsDataValues* modules = tortilla::pluginsData();
    modules->at(plugin_index).state = 0;
}

void PluginsManager::concatCommand(std::vector<tstring>& parts, bool system, InputCommand cmd)
{
    if (parts.empty())
    {
        cmd->command.clear();
        cmd->parameters.clear();
        cmd->parameters_list.clear();
        cmd->parameters_spacesbefore.clear();
        cmd->changed =  true;
        return;
    }

    tstring newcmd(parts[0]);
    if (newcmd != cmd->command)
    {
        cmd->changed = true;
        cmd->command.assign(newcmd);
    }
    if (cmd->parameters_list.size() != parts.size()-1)
        cmd->changed = true;
    else
    {
        for (int i=1,e=parts.size();i<e;++i)
            if (cmd->parameters_list[i-1] != parts[i]) { cmd->changed = true; break; }
    }
    if (!cmd->changed)
        return;

    cmd->parameters_list.clear();
    cmd->parameters_spacesbefore.clear();
    tstring symbols(L"{}\"' ");
    tstring params;
    for (int i=1,e=parts.size();i<e;++i)
    {
        const tstring& s = parts[i];
        cmd->parameters_list.push_back(s);
        if (i != 1 && system)
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
}
