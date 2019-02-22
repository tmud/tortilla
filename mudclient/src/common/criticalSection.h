#pragma once

class CriticalSection
{
    friend class CSectionLock;
public:
    CriticalSection() { InitializeCriticalSection(&m_cs); }
    ~CriticalSection() { DeleteCriticalSection(&m_cs); }
private:
    CRITICAL_SECTION m_cs;    
};

class CSectionLock
{
public:
    CSectionLock(CriticalSection &pcs) : cs(pcs.m_cs) { EnterCriticalSection(&cs); }
    ~CSectionLock() { LeaveCriticalSection(&cs); }
private:
    CRITICAL_SECTION &cs;
};
