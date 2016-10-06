#pragma once

struct SharedMemoryData
{
    SharedMemoryData() : data(NULL), data_size(0), max_size(0) {}
    void *data;
    size_t data_size;
    size_t max_size;
};

class SharedMemoryHandler
{
public:
    virtual void onInitSharedMemory(SharedMemoryData *d) = 0;         // or initializaion purposes (for first process)
};

class SharedMemory
{
    class MutexAutolocker
    {
        HANDLE m_mutex;
    public:
        MutexAutolocker(HANDLE h) : m_mutex(h) { WaitForSingleObject(m_mutex, INFINITE); }
        ~MutexAutolocker() { ReleaseMutex(m_mutex); }
    };

    class Autocleaner
    {
        SharedMemory* m_psm;
    public:
        Autocleaner(SharedMemory* sm) : m_psm(sm) {}
        ~Autocleaner() { if (m_psm) m_psm->close(); }
        void turnoff() { m_psm = NULL; }
    };
    friend class Autocleaner;

    HANDLE m_map_file;
    void*  m_pbuf;
    size_t* m_datasize;
    size_t* m_maxsize;
    void*  m_pdata;
    HANDLE m_mutex;
    HANDLE m_change_event;

public:
    SharedMemory() : m_map_file(NULL), m_pbuf(NULL), m_datasize(NULL), m_maxsize(NULL), m_pdata(NULL), m_mutex(NULL), m_change_event(NULL) {}
    ~SharedMemory() { close(); }
    bool create(const wchar_t* global_name, size_t size, SharedMemoryHandler* handler = NULL)
    {
         assert(handler && global_name && size > 0);

         std::wstring mutex_name(global_name);
         mutex_name.append(L"_mutex");
         m_mutex = CreateMutex(NULL, FALSE, mutex_name.c_str());
         if (!m_mutex)
             return false;

         Autocleaner ac(this);
         MutexAutolocker al(m_mutex);

         std::wstring event_name(global_name);
         event_name.append(L"_event");
         m_change_event = CreateEvent(NULL, TRUE, FALSE, event_name.c_str());
         if (!m_change_event)
             return false;

         bool first_open = false;
         m_map_file = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, global_name);
         if (!m_map_file)
         {
             size_t header_size = sizeof(size_t) * 2;
             m_map_file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size+header_size, global_name);
             if (!m_map_file)
                return false;
             first_open = true;
         }
         m_pbuf = MapViewOfFile(m_map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);
         if (!m_pbuf)
             return false;

         m_maxsize = (size_t*)m_pbuf;
         m_datasize = m_maxsize+1;
         m_pdata = (void*)(m_maxsize+2);

         if (first_open)
         {
             char *data = (char*)(m_pbuf);
             *m_maxsize = size;
             memset(data, 0, size);
             if (handler)
             {
                 SharedMemoryData smd;
                 smd.data = m_pdata;
                 smd.data_size = 0;
                 smd.max_size = size;
                 handler->onInitSharedMemory(&smd);
                 *m_datasize = smd.data_size;
             }
         }
         ac.turnoff();
         SetEvent(m_change_event);
         return true;
   }

   bool lock(SharedMemoryData* d, DWORD wait_mseconds = 0)
   {
       if (wait_mseconds == 0)
           wait_mseconds = INFINITE;
       if (WaitForSingleObject(m_mutex, wait_mseconds) != WAIT_OBJECT_0)
           return false;
       d->data = m_pdata;
       d->data_size = *m_datasize;
       d->max_size = *m_maxsize;
       return true;
   } 

   void unlock(size_t new_datalen = 0)
   {
       if (new_datalen)
       { 
           *m_datasize = new_datalen;
            SetEvent(m_change_event);
       }
       ReleaseMutex(m_mutex);
   }

   bool wait(DWORD mseconds)
   {
        DWORD result = WaitForSingleObject(m_change_event, mseconds);
        return (result == WAIT_OBJECT_0) ? true : false;
   }

private:
    void close()
    {
        if (m_change_event)
            CloseHandle(m_change_event);
        m_change_event = NULL;
        if (m_mutex)
            CloseHandle(m_mutex);
        m_mutex = NULL;
        m_datasize = m_maxsize = NULL;
        m_pdata= NULL;
        if (m_pbuf)
            UnmapViewOfFile(m_pbuf);
        m_pbuf = NULL;
        if (m_map_file)
            CloseHandle(m_map_file);
        m_map_file = NULL;
    }
};
