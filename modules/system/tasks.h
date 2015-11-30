#pragma once

#include "common/tempThread.h"

class Task
{
public:
    virtual ~Task() {}
    virtual void doTask() = 0;
};

class Tasks : public TempThread
{
    CRITICAL_SECTION m_cs;
    std::deque<Task*> m_tasks;
    HANDLE m_event;
public:
    Tasks() : m_event(NULL) 
    {
       InitializeCriticalSection(&m_cs); 
       m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
       run(); 
    }
    ~Tasks() 
    { 
        stop();
        wait();
        CloseHandle(m_event);
        DeleteCriticalSection(&m_cs); 
        std::for_each(m_tasks.begin(), m_tasks.end(), [](Task *o){ delete o;});
    } 
    void runTask(Task *t)
    {
        EnterCriticalSection(&m_cs);
        if (t)
            m_tasks.push_back(t);
        LeaveCriticalSection(&m_cs);
        SetEvent(m_event);
    }
private:
    void threadProc()
    {
        while (!needStop())
        {
            Task *t = NULL;
            EnterCriticalSection(&m_cs);
            if (!m_tasks.empty())
            {
                t = m_tasks[0];
                m_tasks.pop_front();
            }
            LeaveCriticalSection(&m_cs);
            if (t)
            {
                t->doTask();
                delete t;
            }
            else
            {
                WaitForSingleObject(m_event, 500);
            }
        }
    }
};