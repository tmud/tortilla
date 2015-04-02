#include "stdafx.h"
#include "network/network.h"
#include "plugins/pluginsManager.h"
#include "plugins/pluginsApi.h"
#include "msdpNetwork.h"

extern luaT_State L;
extern PluginsManager* _plugins_manager;
extern MsdpNetwork* _msdp_network;

MsdpNetwork::MsdpNetwork() : m_state(false)
{
    m_to_send.setBufferSize(512);
}

void MsdpNetwork::process(Network *network)
{
    DataQueue *msdp_data = network->receive_msdp();
    if (msdp_data->getSize() > 0)
        translate(msdp_data);
    if (m_to_send.getSize() > 0)
    {
        network->sendplain((const tbyte*)m_to_send.getData(), m_to_send.getSize());
        m_to_send.clear();
    }
}

void MsdpNetwork::translate(DataQueue *msdp)
{    
    const tbyte* data = (const tbyte*)msdp->getData();
    int len = msdp->getSize();
    //OUTPUT_BYTES(data, len, len, "MSDP");

    while (len > 0)
    {
        if (len < 3 || data[0] != IAC || data[2] != MSDP)
        {
            assert(false);
            msdp->clear();
            return;
        }

        tbyte cmd = data[1];
        if (cmd == SB)
        {
            const tbyte *p = data;
            for (int i=0; i<len; ++i)
            {
                if (p[i]==SE && p[i-1]==IAC && p[i-2]!=IAC)
                {
                    run_plugins_msdp(p+3, i-4);
                    len = i+1;
                    break;
                }
            }
            msdp->truncate(len);
        }
        else if (cmd == DO)
        {
            m_state = true;
            msdp->truncate(3);
            _plugins_manager->processPluginsMethod("msdpon", 0);
        }
        else if (cmd == DONT)
        {
            m_state = false;
            msdp->truncate(3);
            _plugins_manager->processPluginsMethod("msdpoff", 0);
        }
        else
        {
            assert(false);
            msdp->clear();
            return;
        }
        data = (const tbyte*)msdp->getData();
        len = msdp->getSize();
    }
}

void MsdpNetwork::send_command(const char* cmd, const char* val)
{
    send_begin();
    send_param(MSDP_VAR, cmd);
    send_param(MSDP_VAL, val);
    send_end();
}

void MsdpNetwork::send_param(tbyte param, const char* param_text)
{
    m_to_send.write(&param, 1);
    m_to_send.write(param_text, strlen(param_text));
}

void MsdpNetwork::send_begin()
{
    tbyte begin[] = { IAC, SB, MSDP };
    m_to_send.write(begin, 3);
}

void MsdpNetwork::send_end()
{
    tbyte end[] = { IAC, SE };
    m_to_send.write(end, 2);
}

void MsdpNetwork::run_plugins_msdp(const tbyte* data, int len)
{
    OUTPUT_BYTES(data, len, len, "PLUGINS MSDP");

    /*const tbyte* data = (const tbyte*)msdp->getData();
    int len = msdp->getSize();

    const tbyte* p = data + 2;
    while (*p != )*/

    // translate msdp data into lua table
    lua_newtable(L);

    // run plugins
    _plugins_manager->processPluginsMethod("msdp", 1);
}

bool msdp_isoff()
{
   return (!_msdp_network->state()) ? true : false;
}

int msdpOffError(lua_State *L, const utf8* fname) 
{
    u8string error(fname);
    error.append(":MSDP is off");
    return pluginError(error.c_str()); 
}

int msdp_list(lua_State *L)
{
    if (msdp_isoff())
        return msdpOffError(L, "msdp.list");

    if (luaT_check(L, 1, LUA_TSTRING))
    {
        _msdp_network->send_command("LIST", lua_tostring(L, 1));
        return 0;
    }
    return pluginInvArgs(L, "msdp.list");
}

int msdp_report(lua_State *L)
{
    if (msdp_isoff())
        return msdpOffError(L, "msdp.report");

    if (luaT_check(L, 1, LUA_TSTRING))
    {
        _msdp_network->send_command("REPORT", lua_tostring(L, 1));
        return 0;
    }
    return pluginInvArgs(L, "msdp.report");
}

void reg_msdp(lua_State *L)
{
    lua_newtable(L);
    regFunction(L, "list", msdp_list);
    regFunction(L, "report", msdp_report);
    lua_setglobal(L, "msdp");
}
