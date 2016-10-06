#pragma once

#include "sharedMemory.h"
#include "../common/tempThread.h"

struct SharingHeader {
  int counter_id;
  int messages;
};

struct SharingWindow {
  SharingWindow() : id(0), x(0), y(0), w(0), h(0) {}
  int id, x, y, w, h;
};

class SharingController
{
public:
    SharingController();
    ~SharingController();
    bool init();
    bool tryAddWindow(const SharingWindow& sw);
    void deleteWindow(const SharingWindow& sw);
    void deleteAll();
    void setPosition(const SharingWindow& sw, int oldx, int oldy);
private:
    //void threadProc();
    bool lock(SharedMemoryData *d);
    void unlock(SharedMemoryData *d);
    //void clear();
    SharingHeader* getHeader(SharedMemoryData *d);
    SharingWindow* getWindow(int index, SharedMemoryData *d);
    void deleteWindow(int index, SharedMemoryData *d);
    //bool addWindow(const SharingWindow& sw, SharedMemoryData *d);
private:
    SharedMemory m_shared_memory;
    int m_id;
};
