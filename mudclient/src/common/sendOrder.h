#pragma once
#include "api/shared.h"

class SendOrder
{
public:
    SendOrder();
    ~SendOrder();
    bool initilize();
private:
    void RegisterMe();
    void UnregisterMe();

    bool m_tryinitialize;
    bool m_initialized;
    SharedMemory m_shared_memory;
};
