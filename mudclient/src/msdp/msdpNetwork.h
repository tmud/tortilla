#pragma once

class Network;
class MsdpNetwork
{
public:
    MsdpNetwork();
    ~MsdpNetwork();
    bool state() const { return m_state; }
    void processReceived(Network *network);
    void sendExist(Network *network);
    void send_varval(const utf8* var, const utf8* val);
    void send_varvals(const utf8* var, const std::vector<u8string>& vals);
    void report(Plugin* p, std::vector<u8string> *report);
    void unreport(Plugin* p, std::vector<u8string> *report);
    void loadPlugin(Plugin *p);
    void unloadPlugin(Plugin *p);
    void loadPlugins();
    void unloadPlugins();

private:
    void translate(DataQueue *msdp);
    void send_begin();
    void send_end();
    void send_param(tbyte param, const char* param_text);
    bool run_plugins_msdp(const tbyte* data, int len);
    struct cursor {
        const tbyte* p;
        const tbyte* e;
    };
    bool process_var(cursor& c);
    bool process_val(cursor& c);

    void releaseReports();

private:
    DataQueue m_to_send;
    bool m_state;
    typedef std::vector<Plugin*> PluginReport;
    typedef PluginReport::iterator PluginReportIterator;
    typedef std::map<u8string, PluginReport*>::iterator PluginIterator;
    std::map<u8string, PluginReport*> m_plugins_reports;    
};
