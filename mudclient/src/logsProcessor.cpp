#include "stdafx.h"
#include "logsProcessor.h"

LogsProcessor::LogsProcessor() 
{
}

LogsProcessor::~LogsProcessor()
{
    closeAllLogs();
    stop();
    wait();
    std::for_each(m_free.begin(),m_free.end(),[](MudViewString* s) {delete s;});
}

bool LogsProcessor::init()
{
    return run();
}

int LogsProcessor::openLog(tstring& filename, bool newlog, int owner)
{
    bool htmlformat = true;
    tstring format(m_propData->logformat);
    if (format == L"txt")
        htmlformat = false;

    LogsFormatter *ff = NULL;
    if (htmlformat) ff = new LogsFormatterHtml(m_propData);
    else ff = new LogsFormatterTxt(m_propData);
    ff->normFilename(filename);

    int index = -1;
    CSectionLock _lock(m_cs_logs);
    for (int i=0,e=m_logs.size(); i<e; ++i)
        if (m_logs[i] && m_logs[i]->ff->getFilename() == filename) 
        {
            if (m_logs[i]->owner != owner)
            {
                delete ff;
                return -1;
            }
            index = i; break; 
        }
 
    LogsFormatter::PrepareMode pm = (newlog) ? LogsFormatter::PM_NEW : LogsFormatter::PM_APPEND;
    if (index != -1) 
    {
        delete ff;
        log *l = m_logs[index];
        if (!l->ff->open(filename, pm) || (newlog && !l->ff->prepare()) )
        {
            l->close = true;
            return -1;
        }
        l->close = false;
        return index;
    }

    if (!ff->open(filename, pm) || !ff->prepare())
    {
        delete ff;
        return -1;
    }

    log *l = new log;
    l->ff = ff;
    l->owner = owner;

    m_logs.push_back(l);
    index = m_logs.size()-1;
    return index;
}

void LogsProcessor::closeLog(int id)
{
    CSectionLock _lock(m_cs_logs);
    int last = m_logs.size() - 1;
    if (id >= 0 && id <= last)
    {
        log *l = m_logs[id];
        if (l)
            l->close = true;
    }
}

void LogsProcessor::writeLog(int id, const parseData& pdata)
{
    CSectionLock _lock(m_cs);
    int e=pdata.strings.size()-1;
    for (int i=0; i<=e; ++i)
    {
        MudViewString *src = pdata.strings[i];
        if (src->dropped && !src->show_dropped) continue;
        if (i == 0 && pdata.update_prev_string && !m_msgs.empty())
        {
            int last = m_msgs.size()-1;
            MudViewString* s = m_msgs[last].str;
            s->copy(src);
            continue;
        }
        MudViewString* s = getFreeString();
        s->copy(src);
        msg m; m.log = id; m.str = s;
        m_msgs.push_back(m);
    }
}

tstring LogsProcessor::getLogFile(int id)
{
    CSectionLock _lock(m_cs_logs);
    int last = m_logs.size() - 1;
    if (id >= 0 && id <= last)
    {
        log *l = m_logs[id];
        if (l)
            return l->ff->getFilename();
    }
    return L"";
}

void LogsProcessor::closeAllLogs()
{
    CSectionLock _lock(m_cs_logs);
    for (int i=0,e=m_logs.size(); i<e; ++i)
    {
        log *l = m_logs[i];
        if (l)
         l->close = true;
    }
}

void LogsProcessor::threadProc()
{
    while (!needStop())
    {
        {
            CSectionLock _lock(m_cs);
            if (!m_msgs.empty())
            {
                int last = m_msgs.size() - 1;
                msg m = m_msgs[last];
                if (!m.str->system)
                {
                  m_msgs.pop_back();
                  m_msgs.swap(m_towrite);
                  m_msgs.push_back(m);
                }
                else
                {
                   m_msgs.swap(m_towrite);
                }
            }
        }
        if (m_towrite.empty())
        {
            closeReqLogs();
            Sleep(250);
            continue;
        }
        saveAll();
        closeReqLogs();
    }
    {
        CSectionLock _lock(m_cs);
        if (!m_msgs.empty())
        {
            m_towrite.insert(m_towrite.end(), m_msgs.begin(), m_msgs.end());
            m_msgs.clear();
        }
    }
    saveAll();
    closeReqLogs();
}

void LogsProcessor::saveAll()
{
    std::set<int> updated;
    for (int i=0,e=m_towrite.size(); i<e; ++i)
    {
        int id = m_towrite[i].log;
        if (id == -1)
            continue;
        CSectionLock _lock(m_cs_logs);
        int lastid = m_logs.size()-1;
        if (id < 0 || id > lastid || !m_logs[id])
            continue;
        for (int j=i; j<e; ++j)
        {
            msg &m = m_towrite[j];
            if (m.log != id)
                continue;
            m.log = -1;
            if (m.str->dropped && !m.str->show_dropped)
                continue;
            m_logs[id]->ff->writeString(m.str);
            updated.insert(id);
        }
    }

    {
        CSectionLock _lock(m_cs_logs);
        std::set<int>::iterator it = updated.begin(), it_end = updated.end();
        for (; it!=it_end;++it){
            m_logs[*it]->ff->flushStrings();
        }
    }

    CSectionLock _lock(m_cs);
    for (int i=0,e=m_towrite.size(); i<e; ++i)
    {
        MudViewString *s = m_towrite[i].str;
        m_free.push_back(s);
    }
    m_towrite.clear();
}

void LogsProcessor::closeReqLogs()
{
    CSectionLock _lock(m_cs_logs);
    for (int i=0,e=m_logs.size(); i<e; ++i)
    {
        log *l = m_logs[i];
        if (l && l->close)
        {
            l->ff->close();
            delete l->ff;
            m_logs[i] = NULL;
            delete l;
        }
    }
}

void LogsProcessor::updateProps(PropertiesData *pdata)
{
    m_propData = pdata;
}

MudViewString* LogsProcessor::getFreeString()
{
    if (m_free.empty())
        return new MudViewString;
    int last = m_free.size() - 1;
    MudViewString *s = m_free[last];
    m_free.pop_back();
    return s;
}
