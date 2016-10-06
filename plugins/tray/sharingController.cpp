#include "stdafx.h"
#include "sharingController.h"

const wchar_t *global_share_name = L"TortillaTray";
const int global_share_size = sizeof(SharingHeader) * 100*sizeof(SharingWindow);

class SharedMemoryInitializer : public SharedMemoryHandler
{
    void onInitSharedMemory(SharedMemoryData *d)
    {
        SharingHeader sh;
        size_t datalen = sizeof(SharingHeader);
        sh.counter_id = 1;
        sh.messages = 0;
        memcpy(d->data, &sh, datalen);
        d->data_size = datalen;
    }
};

SharingController::SharingController() : m_id(0) 
{
}

SharingController::~SharingController()
{
}

bool SharingController::init()
{
    // get id
    SharedMemoryData smd;
    if (!lock(&smd))
        return false;
    SharingHeader* sh = getHeader(&smd);
    m_id = sh->counter_id;
    sh->counter_id = m_id+1;
    unlock(&smd);
    return true;
}

bool SharingController::tryAddWindow(const SharingWindow& sw)
{
    SharedMemoryData smd;
    if (!lock(&smd)) 
        return false;
    if (smd.data_size+sizeof(SharingWindow) > smd.max_size)
    {
        unlock(&smd);
        return false;
    }
    SharingHeader* sh = getHeader(&smd);
    SharingWindow* w = (SharingWindow*)(sh+1);



    unlock(&smd);
}

void SharingController::deleteWindow(const SharingWindow& sw)
{
}

void SharingController::deleteAll()
{
}

void SharingController::setPosition(const SharingWindow& sw, int oldx, int oldy)
{
}

/*void SharingController::threadProc()
{
    SharedMemoryInitializer smi;
    if (!m_shared_memory.create(global_share_name, global_share_size, &smi))
        return;
    SharedMemoryData smd;

    // get id
    if (!lock(&smd)) 
        return;
    SharingHeader* sh = getHeader(&smd);
    m_id = sh->counter_id;
    sh->counter_id = m_id+1;
    unlock(&smd);

    SharingControllerCleaner c(this);

    if (!lock(&smd)) 
        return;

    unlock(&smd);
}*/

bool SharingController::lock(SharedMemoryData *d)
{
    return m_shared_memory.lock(d, 100);
}

void SharingController::unlock(SharedMemoryData *d)
{
    m_shared_memory.unlock(d->data_size);
}

SharingHeader* SharingController::getHeader(SharedMemoryData *d)
{
    return (SharingHeader*)d->data;
}

SharingWindow* SharingController::getWindow(int index, SharedMemoryData* d)
{
    SharingHeader *h = getHeader(d);
    if (index >= 0 && index < h->messages)
    {
        SharingWindow* w = (SharingWindow*)(h+1);
        return &w[index];
    }
    return NULL;
}

void SharingController::deleteWindow(int index, SharedMemoryData* d)
{
    SharingHeader *h = getHeader(d);
    if (index >= 0 && index < h->messages)
    {
        
    }
}

/*bool SharingController::addWindow(const SharingWindow& sw, SharedMemoryData* d)
{

}

void SharingController::clear()
{
    SharedMemoryData smd;
    if (!m_shared_memory.lock(&smd))
        return;



    m_shared_memory.unlock();
}
*/
