#include "stdafx.h"
#include "sendOrder.h"

SendOrder::SendOrder() : m_tryinitialize(false), m_initialized(false)
{   
}

SendOrder::~SendOrder()
{
}

bool SendOrder::initilize()
{
    if (m_initialized)
        return true;
    if (m_tryinitialize)
        return false;
    class SharedMemoryInitializer : public SharedMemoryHandler
    {
        void onInitSharedMemory(SharedMemoryData *d)
        {            
        }
    };
    const tchar* global_share_name = L"TortillaMudClientSharedMemory";
    const size_t global_share_size = 0;
    SharedMemoryInitializer smi;
    m_initialized = (!m_shared_memory.create(global_share_name, global_share_size, &smi));
    m_tryinitialize = true;
    return m_initialized;
}

void SendOrder::RegisterMe()
{
}

void SendOrder::UnregisterMe()
{
}