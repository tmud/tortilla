#pragma once

#include "sharedMemory.h"
#include "sharingData.h"

class SharingController : public SharedMemoryHandler
{
public:
    SharingController();
    ~SharingController();
    bool init();
    void release();
    bool pushCommand(SharingCommand* cmd);
    //bool addMessage(const SharingDataMessage& msg);

private:
    bool lock();
    bool unlock();
    bool write();
    bool read(void *p, unsigned int size);

private:
    void onInitSharedMemory(siz)
    {
        m_shared_memory.lock();
        memset(m_shared_memory.ptr(), 0, m_shared_memory.size());
        m_shared_memory.unlock();
    }
    std::wstring m_id;
    int m_shared_revision;
    SharedMemory m_shared_memory;
    SharingData m_shared_data;
    bool m_locked;
};
