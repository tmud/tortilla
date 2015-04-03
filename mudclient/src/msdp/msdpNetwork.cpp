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
            for (int i=3; i<len; ++i)
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

bool MsdpNetwork::run_plugins_msdp(const tbyte* data, int len)
{
    //OUTPUT_BYTES(data, len, len, "PLUGINS MSDP");

    // translate msdp data into lua table
    lua_newtable(L);
    cursor c; c.p = data; c.e = data + len;
    while (c.p != c.e)
    {
        if (!process_var(c))
            { lua_pop(L, 1); return false; }
        if (!process_val(c))
            { lua_pop(L, 1); return false; }
    }

    // run plugins
    _plugins_manager->processPluginsMethod("msdp", 1);
    return true;
}

bool MsdpNetwork::process_var(cursor& c)
{
     const tbyte* p = c.p;
     if (p == c.e || *p != MSDP_VAR)
         return false;
     const tbyte* b = p+1;
     while (b != c.e && *b != MSDP_VAL)
     {
         if (*b < ' ') return false;
         b++;
     }
     if (b == c.e)
         return false;
     u8string var_name((const utf8*)(p+1), b-p-1);
     lua_pushstring(L, var_name.c_str());
     c.p = b;
     return true;
}

bool MsdpNetwork::process_val(cursor& c)
{
     const tbyte* p = c.p;
     if (p == c.e || *p != MSDP_VAL)
         return false;
     const tbyte* b = p+1;
     if (b == c.e)
     {
         lua_pushstring(L, "");
         lua_settable(L, -3);
         c.p = b;
         return true;
     }
     if (*b >= ' ')
     {
         while (b != c.e && *b >= ' ') b++;
         if (b == c.e)
             return false;
         if (*b != MSDP_VAR && *b != MSDP_VAL && *b != MSDP_ARRAY_CLOSE && *b != MSDP_TABLE_CLOSE)
             return false;
         u8string value((const utf8*)(p+1), b-p-1);
         lua_pushstring(L, value.c_str());
         lua_settable(L, -3);
         c.p = b;
         return true;
     }
     else if (*b == MSDP_ARRAY_OPEN)
     {
         c.p = b+1;
         if (c.p == c.e)
             return false;

         int index = 1;               // index for array's vars
         lua_newtable(L);             // table for array
         while(true)
         {
            lua_pushinteger(L, index++);
            if (!process_val(c))
                { lua_pop(L, 2); return false; }
            if (*c.p != MSDP_VAL && *c.p != MSDP_ARRAY_CLOSE)
                { lua_pop(L, 2); return false; }
            if (*c.p == MSDP_ARRAY_CLOSE)
            {
                lua_settable(L, -3);
                c.p++;
                return true;
            }
         }
     }
     else if (*b == MSDP_TABLE_OPEN)
     {
         c.p = b+1;
         if (c.p == c.e)
             return false;

          lua_newtable(L);             // table for table
          while (true)
          {
               if (!process_var(c))
                  { lua_pop(L, 1); return false; }
                if (!process_val(c))
                    { lua_pop(L, 2); return false; }
                if (*c.p != MSDP_VAR && *c.p != MSDP_TABLE_CLOSE)
                    { lua_pop(L, 2); return false; }
                if (*c.p == MSDP_TABLE_CLOSE)
                {
                    lua_settable(L, -3);
                    c.p++;
                    return true;
                }
          }
     }
     return false;
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
