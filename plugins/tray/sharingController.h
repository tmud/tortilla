#pragma once

#include "sharedMemory.h"
#include "sharingData.h"

class SharingController : public SharedMemoryHandler
{
public:
    SharingController();
    bool init();
    void regTray(const tstring& trayid);
    void unregTray(const tstring& trayid);
    void addMessage(const SharingDataMessage& msg);

private:
    bool write();
    bool read();
    bool reread(void *p, unsigned int size);

private:
    void onInitSharedMemory()
    {
        void *memory = m_shared_memory.lock();
        memset(memory, 0, m_shared_memory.size());
        m_shared_memory.unlock();
    }
    int m_shared_revision;
    SharedMemory m_shared_memory;
    SharingData m_shared_data;
};
