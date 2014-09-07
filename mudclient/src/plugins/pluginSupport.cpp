#include "stdafx.h"
#include "pluginSupport.h"

PluginsIdTableControl::PluginsIdTableControl(int firstid, int lastid) : first_id(firstid), last_id(lastid)
{
}

UINT PluginsIdTableControl::registerPlugin(Plugin*p, int code, bool button)
{
    int id = -1;
    for (int i = 0, e = plugins_id_table.size(); i < e; ++i){
        if (!plugins_id_table[i].plugin) { id = i; break; }
    }    
    if (id == -1) { plugins_id_table.push_back(idplugin());  id = plugins_id_table.size() - 1; }    
    idplugin &t = plugins_id_table[id];
    t.plugin = p; t.code = code; t.button = button;
    id = id + first_id;
    return (UINT)id;
}

UINT PluginsIdTableControl::unregisterByCode(Plugin*p, int code, bool button)
{
    int id = getIndex(p, code, button);
    if (id != -1) 
    {
        idplugin &t = plugins_id_table[id];
        t.plugin = NULL;
        id = id + first_id;
    }   
    return (UINT)id;
}

void PluginsIdTableControl::unregisterById(Plugin*p, UINT id)
{
    int _id = (int)id - first_id;
    int size = plugins_id_table.size();
    if (_id >= 0 && _id < size)
    {
        idplugin &t = plugins_id_table[_id];
        if (t.plugin && t.plugin == p)
        {
            t.plugin = NULL;
        }
    }
}

UINT PluginsIdTableControl::findId(Plugin*p, int code, bool button)
{
    int id = getIndex(p, code, button);
    if (id != -1)
        id = id + first_id;
    return (UINT)id;
}

void PluginsIdTableControl::runPluginCmd(UINT id)
{
    int _id = (int)id - first_id;
    int size = plugins_id_table.size();
    if (_id >= 0 && _id < size)
    {
        idplugin &t = plugins_id_table[_id];
        if (t.plugin)
            t.plugin->menuCmd(t.code);
    }
}

int PluginsIdTableControl::getIndex(Plugin*p, int code, bool button)
{
    for (int i = 0, e = plugins_id_table.size(); i < e; ++i)
    {
        idplugin &t = plugins_id_table[i];
        if (t.plugin == p && t.code == code && t.button == button)
        {
            return i;
        }
    }
    return -1;
}
