#pragma once

#include "common/tempThread.h"

class BeepTasks : public TempThread
{
    CRITICAL_SECTION m_cs;
    typedef std::pair<DWORD,DWORD> beep_data;
    std::deque<beep_data> m_tasks;
    HANDLE m_event;
public:
    BeepTasks() : m_event(NULL) 
    {
       InitializeCriticalSection(&m_cs); 
       m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
       run(); 
    }
    ~BeepTasks() 
    { 
        stop();
        wait();
        CloseHandle(m_event);
        DeleteCriticalSection(&m_cs); 
    } 
    void runTask(DWORD freq, DWORD duration)
    {
        EnterCriticalSection(&m_cs);
        beep_data bd(freq, duration);
        m_tasks.push_back(bd);
        LeaveCriticalSection(&m_cs);
        SetEvent(m_event);
    }
private:
    void threadProc()
    {
        while (!needStop())
        {
            bool tasks_exists = false;
            beep_data bd(0, 0);
            EnterCriticalSection(&m_cs);
            if (!m_tasks.empty())
            {
                bd = m_tasks[0];
                m_tasks.pop_front();
                tasks_exists = true;
            }
            LeaveCriticalSection(&m_cs);
            if (tasks_exists)
            {
                if (bd.first > 0 && bd.second > 0)
                    ::Beep(bd.first, bd.second);
            }
            else
            {
                WaitForSingleObject(m_event, INFINITE);
            }
        }
    }

    void threadStop()
    {
        SetEvent(m_event);
    }
};