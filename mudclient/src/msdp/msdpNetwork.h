#pragma once

class Network;
class MsdpNetwork
{
public:
    MsdpNetwork();
    ~MsdpNetwork();
    bool state() const { return m_state; }    
    void translateReceived(DataQueue& msdp_data);
    DataQueue& getSendData();
    void send_varval(const char* var, const char* val);
    void send_varvals(const char* var, const std::vector<std::string>& vals);
    void report(Plugin* p, std::vector<std::string> *report);
    void unreport(Plugin* p, std::vector<std::string> *report);
    void loadPlugin(Plugin *p);
    void unloadPlugin(Plugin *p);
    void loadPlugins();
    void unloadPlugins();
    void setUtf8Encoding(bool flag);
private:
    void translate(DataQueue& msdp);
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
    typedef std::map<std::string, PluginReport*>::iterator PluginIterator;
    std::map<std::string, PluginReport*> m_plugins_reports;
    bool m_utf8_encoding;
};
