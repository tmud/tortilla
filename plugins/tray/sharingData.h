#pragma once

struct SharingControl
{
};

struct SharingDataWindow
{
    std::vector<tstring> textlines;
    COLORREF background;
    COLORREF text;
    int showtime;
    RECT windowpos;
};

class SharedMemoryHandler
{
public: 
    virtual void onInitBlock() = 0;         // called for first for initializaion purposes
    virtual void onNewData() = 0;           // updated by new data
};

class SharedMemory
{
    SharedMemoryHandler* m_pHandler;
    HANDLE m_map_file;
    void*  m_pbuf;
    unsigned int m_size;
    std::wstring m_semaphore_name;
    HANDLE m_semaphore;

public:
    SharedMemory() : m_pHandler(NULL), m_map_file(NULL), m_pbuf(NULL), m_size(0), m_semaphore(NULL) {}
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
                return false;
             first_open = true;
         }

         m_pbuf = MapViewOfFile(m_map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);
         if (!m_pbuf)
         {
             CloseHandle(m_map_file);
             return false;
         }

         m_semaphore_name.assign(L"Global\\sem_");
         m_semaphore_name.append(global_name);
         m_semaphore = CreateSemaphore(NULL, 0, 1, m_semaphore_name.c_str());
         if (!m_semaphore)
         {
             close();
             return false;
         }
         if (first_open)
             m_pHandler->onInitBlock();
         return true;
   }

   unsigned int size() const {
       return m_size;
   }
 
   void* lock() {
        WaitForSingleObject(m_semaphore, INFINITE); 
        return m_pbuf;
   }

   void unlock() {
        ReleaseSemaphore(m_semaphore, 1, NULL);
   }

private:   
    void close() 
    {
        if (m_semaphore)
            CloseHandle(m_semaphore);
        m_semaphore = NULL;
        if (m_pbuf)
            UnmapViewOfFile(m_pbuf);
        m_pbuf = NULL;
        if (m_map_file)
            CloseHandle(m_map_file);
        m_map_file = NULL;
    }
};

class SharingManager : public SharedMemoryHandler
{
    const tchar *global_share_name = L"TortillaTray";
    const int share_size = 65536;
    SharedMemory m_shared_memory;
public:
    void onInitBlock() 
    {
        memset()
    }
    void onNewData() {}

    SharingManager() {}
    ~SharingManager() { }

    bool init()
    {
        return m_shared_memory.open(this, global_share_name, share_size);
    }

};
