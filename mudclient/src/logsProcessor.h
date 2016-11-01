#pragma once
#include "../common/tempThread.h"
#include "mudViewParser.h"
#include "propertiesPages/propertiesData.h"
//#include "palette256.h"
#include "logsFormatter.h"

class LogsProcessor : private TempThread<false>
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
    void calcFileName(tstring& filename);

private:
    void threadProc();
    void closeAllLogs();
    void saveAll();
    //void prepare(int id);
    //void prepare_txt(int id);
    void write(HANDLE file, const std::string &data);
    void closeReqLogs();
    //void convertString(MudViewString* str, std::string* out);
    //void convertString_txt(MudViewString* str, std::string* out);
    //void getHeader(std::string* out);
    MudViewString* getFreeString();
    //const char* color(COLORREF c);
    //const char* color2(COLORREF c);

    //WideToUtf8 m_converter;
    CriticalSection m_cs_logs;
    CriticalSection m_cs;
    //Palette256     *m_palette;
    PropertiesData *m_propData;

    struct log
    {
        log() : ff(NULL), close(false) {} //, newlog(false), append(false), opened(false) {} //, htmlformat(true) {}
        LogsFormatter* ff;
        bool close;
        //bool newlog;
        //bool append;
        //bool opened;
        //bool htmlformat;
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
    
    /*MemoryBuffer m_buffer;
    MemoryBuffer m_color_buffer;
    MemoryBuffer m_color_buffer2;*/
};
