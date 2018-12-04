#pragma once
#include "api/shared.h"

class SendOrder
{
public:
    SendOrder();
    ~SendOrder();

private:
    void RegisterMe();
    void UnregisterMe();

    SharedMemory m_shared;
};
