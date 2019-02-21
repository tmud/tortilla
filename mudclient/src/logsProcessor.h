#pragma once
#include "../common/tempThread.h"
#include "mudViewParser.h"
#include "propertiesPages/propertiesData.h"
#include "logsFormatter.h"

class LogsProcessor : private TempThread<false>
{
public:
    LogsProcessor();
    ~LogsProcessor();
    bool init();
    int  openLog(tstring& filename, bool newlog, int owner);
    void closeLog(int id);
    void writeLog(int id, const parseData& pdata);
    tstring getLogFile(int id);
    void updateProps(PropertiesData *pdata);
private:
    void threadProc();
    void closeAllLogs();
    void saveAll();
    void closeReqLogs();
    MudViewString* getFreeString();
    CriticalSection m_cs_logs;
    CriticalSection m_cs;
    PropertiesData *m_propData;

    struct log
    {
        log() : ff(NULL), close(false), owner(-1) {}
        LogsFormatter* ff;
        bool close;
        int  owner;
    };
    std::vector<log*> m_logs;

    struct msg
    {
        int log;
        MudViewString *str;
    };
    std::vector<msg> m_msgs;
    std::vector<msg> m_towrite;
    std::vector<MudViewString*> m_free;
};
