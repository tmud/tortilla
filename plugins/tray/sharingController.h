#pragma once

#include "sharedMemory.h"
#include "../common/tempThread.h"
#include "sharingData.h"

class SharingController
{
public:
    SharingController();
    ~SharingController();
    bool init();
    bool tryAddWindow(SharingWindow* sw, const RECT& working_area, int dh);
    bool tryMoveWindow(SharingWindow* sw);

    void deleteWindow(const SharingWindow* sw);
    //void updateWindow(const SharingWindow* sw, int newx, int newy);
    //bool getLastWindow(SharingWindow* sw); 
private:
    SharingHeader* getHeader(SharedMemoryData *d);
    SharingWindow* getWindow(int index, SharedMemoryData *d);
    int findWindow(const SharingWindow* sw, SharedMemoryData* memory);
private:
    SharedMemory m_shared_memory;
};
