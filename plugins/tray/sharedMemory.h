#pragma once

class SharedMemoryHandler
{
public:
    virtual void onInitSharedMemory(void* buffer, size_t size) = 0;         // called for initializaion purposes (for first process)
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
        void disable() { m_psm = NULL; }
    };
    friend class Autocleaner;

    HANDLE m_map_file;
    void*  m_pbuf;
    size_t m_size;
    size_t m_datasize;
    HANDLE m_mutex;
    HANDLE m_change_event;

public:
    SharedMemory() : m_map_file(NULL), m_pbuf(NULL), m_size(0), m_datasize(0), m_mutex(NULL), m_change_event(NULL) {}
    ~SharedMemory() { close(); }    
    bool create(SharedMemoryHandler* handler, const wchar_t* global_name, size_t size)
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
             m_map_file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, global_name);
             if (!m_map_file)
                return false;
             first_open = true;
         }
         m_pbuf = MapViewOfFile(m_map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);
         if (!m_pbuf)
             return false;
         if (first_open)
             handler->onInitSharedMemory(m_pbuf, size);
         m_size = size;
         ac.disable();
         return true;
   }

   bool write(void *data, size_t datalen)
   {
        if (datalen > m_size)
            return false;
        MutexAutolocker al(m_mutex);
        memcpy(m_pbuf, data, datalen);
        m_datasize = datalen;
        return true;
   }

   bool read(void* buffer, size_t bufferlen)
   {
        if (bufferlen < m_datasize)
            return false;
        MutexAutolocker al(m_mutex);
        memcpy(buffer, m_pbuf, m_datasize);
        return true;   
   }

   size_t size() const {
       return m_datasize;
   }
 
   void sendChangeEvent() {
        SetEvent(m_change_event);
   }

   bool waitChangeEvent(DWORD msSeconds)
   {
        DWORD result = WaitForSingleObject(m_change_event, msSeconds);
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
        if (m_pbuf)
            UnmapViewOfFile(m_pbuf);
        m_pbuf = NULL;
        if (m_map_file)
            CloseHandle(m_map_file);
        m_map_file = NULL;
    }
};
