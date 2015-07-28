#pragma once

class SharedMemoryHandler
{
public: 
    virtual void onInitSharedMemory() = 0;         // called for initializaion purposes (for first process)
};

class SharedMemory
{
    SharedMemoryHandler* m_pHandler;
    HANDLE m_map_file;
    void*  m_pbuf;
    unsigned int m_size;
    HANDLE m_mutex;
    HANDLE m_init_event;
    HANDLE m_change_event;

public:
    SharedMemory() : m_pHandler(NULL), m_map_file(NULL), m_pbuf(NULL), m_size(0), m_mutex(NULL), m_init_event(NULL), m_change_event(NULL) {}
    ~SharedMemory() { close(); }    
    bool open(SharedMemoryHandler* handler, const wchar_t* global_name, unsigned int size)
    {
         assert(handler && global_name && size > 0);
         m_pHandler = handler;
         bool first_open = false;
         m_map_file = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, global_name);
         if (!m_map_file)
         {
             m_map_file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, global_name); 
             if (!m_map_file)
                return close();
             first_open = true;
         }
         m_pbuf = MapViewOfFile(m_map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);
         if (!m_pbuf)
             return close();

         std::wstring mutex_name(global_name);
         mutex_name.append(L"_mutex");
         m_mutex = CreateMutex(NULL, FALSE, mutex_name.c_str());
         if (!m_mutex)
             return close();

         std::wstring initevent_name(global_name);
         initevent_name.append(L"_initevent");
         m_init_event = CreateEvent(NULL, TRUE, FALSE, initevent_name.c_str() );
         if (!m_init_event)
             return close();

         std::wstring changeevent_name(global_name);
         changeevent_name.append(L"_changeevent");
         m_change_event = CreateEvent(NULL, FALSE, FALSE, changeevent_name.c_str() );
         if (!m_change_event)
             return close();

         m_size = size;
         if (first_open)
         {
             m_pHandler->onInitSharedMemory();
             SetEvent(m_init_event);
         }
         else
         {
             WaitForSingleObject(m_init_event, INFINITE);
         }
         return true;
   }

   unsigned int size() const {
       return m_size;
   }
 
   void* lock() {
        WaitForSingleObject(m_mutex, INFINITE); 
        return m_pbuf;
   }

   void unlock() {
        ReleaseMutex(m_mutex);
   }

   void sendChangeEvent()
   {
        SetEvent(m_change_event);
   }

   bool waitChangeEvent(DWORD msSeconds)
   {
        DWORD result = WaitForSingleObject(m_change_event, msSeconds);
        return (result == WAIT_OBJECT_0) ? true : false;
   }

private:   
    bool close()
    {
        if (m_change_event)
            CloseHandle(m_change_event);
        m_change_event = NULL;
        if (m_init_event)
            CloseHandle(m_init_event);
        m_init_event = NULL;
        if (m_mutex)
            CloseHandle(m_mutex);
        m_mutex = NULL;
        if (m_pbuf)
            UnmapViewOfFile(m_pbuf);
        m_pbuf = NULL;
        if (m_map_file)
            CloseHandle(m_map_file);
        m_map_file = NULL;
        return false;
    }
};
