#pragma once

class Network;
class MsdpNetwork
{
public:
    MsdpNetwork();
    void process(Network *network);
    void send_command(const char* cmd, const char* val);
    bool state() const { return m_state; }

private:
    void translate(DataQueue *msdp);
    void send_param(tbyte param, const char* param_text);
    void send_begin();
    void send_end();
    bool run_plugins_msdp(const tbyte* data, int len);

    struct cursor {
        const tbyte* p;
        const tbyte* e;
    };
    bool process_var(cursor& c);
    bool process_val(cursor& c);

private:
    DataQueue m_to_send;
    bool m_state;
};
