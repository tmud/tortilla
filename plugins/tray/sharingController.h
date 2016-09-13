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
    size_t onInitSharedMemory(void* buffer, size_t size);
    std::wstring m_id;
    int m_shared_revision;
    SharedMemory m_shared_memory;
    SharedData m_shared_data;
    //bool m_locked;
};
