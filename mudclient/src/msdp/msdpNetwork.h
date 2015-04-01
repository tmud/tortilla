#pragma once

class Network;
class MsdpNetwork
{
public:
    MsdpNetwork();
    void process(Network *network);

private:
    void send_command(const char* cmd, const char* val);    
    void send_param(tbyte param, const char* param_text);
    void send_begin();
    void send_end();
    void translate(const tbyte* data, int len);
    DataQueue m_to_send;
};
