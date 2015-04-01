#include "stdafx.h"
#include "network/network.h"
#include "plugins/pluginsManager.h"
#include "msdpNetwork.h"

extern luaT_State L;
extern PluginsManager* _plugins_manager;

MsdpNetwork::MsdpNetwork()
{
}

void MsdpNetwork::process(Network *network)
{
    DataQueue *msdp_data = network->receive_msdp();
    if (msdp_data->getSize() > 0)
    {
        translate((const byte*)msdp_data->getData(), msdp_data->getSize());
        msdp_data->clear();
    }
    if (m_to_send.getSize() > 0)
    {
        network->sendplain((const tbyte*)m_to_send.getData(), m_to_send.getSize());
        m_to_send.clear();
    }
}

void MsdpNetwork::translate(const tbyte* data, int len)
{
    OUTPUT_BYTES(data, len, len, "MSDP");
    if (len < 3 || data[0] != IAC || data[2] != MSDP)
    {
        assert(false);
        return;
    }
    tbyte cmd = data[1];
    if (cmd == DO)
    {
        //send_command("LIST", "LISTS");
        send_command("LIST", "COMMANDS");
        send_command("LIST", "CONFIGURABLE_VARIABLES");
        //send_command("LIST", "REPORTABLE_VARIABLES");
        //send_command("LIST", "REPORTED_VARIABLES");
        //send_command("LIST", "SENDABLE_VARIABLES");
    }
    
    if (cmd == DONT) 
    {
    }

    if (cmd == SB)
    {
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
