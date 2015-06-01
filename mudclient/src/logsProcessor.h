#pragma once
#include "common/tempThread.h"
#include "mudViewParser.h"
#include "propertiesPages/propertiesData.h"
#include "palette256.h"

class LogsProcessor : private TempThread
{
public:
    LogsProcessor();
    ~LogsProcessor();
    bool init();
    int  openLog(const tstring& filename, bool newlog);
    void closeLog(int id);
    void writeLog(int id, const parseData& pdata);
    tstring getLogFile(int id);
    void updateProps(PropertiesData *pdata);

private:
    void threadProc();
    void closeAllLogs();
    void saveAll();
    void prepare(int id);    
    void write(HANDLE file, const std::string &data);
    void closeReqLogs();
    void convertString(MudViewString* str, std::string* out);
    MudViewString* getFreeString();
    const char* color(COLORREF c);
    const char* color2(COLORREF c);

    WideToUtf8 m_converter;
    CriticalSection m_cs_logs;
    CriticalSection m_cs;
    Palette256     *m_palette;
    PropertiesData *m_propData;

    struct log
    {
        log() : hfile(INVALID_HANDLE_VALUE), close(false), newlog(false), append(false), opened(false) {}
        HANDLE hfile;
        tstring filename;
        bool close;
        bool newlog;
        bool append;
        bool opened;
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
    MemoryBuffer m_buffer;
    MemoryBuffer m_color_buffer;
    MemoryBuffer m_color_buffer2;
};
