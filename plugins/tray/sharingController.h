#pragma once

#include "sharedMemory.h"
#include "../common/tempThread.h"

struct SharingHeader {
  int counter_id;
  int messages;
};

struct SharingWindow {
  SharingWindow() : x(0), y(0), w(0), h(0) {}
  int x, y, w, h;
};

class SharingController
{
public:
    SharingController();
    ~SharingController();
    bool init();
    bool tryAddWindow(const SharingWindow& sw);
    void deleteWindow(const SharingWindow& sw);
    void updateWindow(const SharingWindow& sw, int newx, int newy);
private:
    SharingHeader* getHeader(SharedMemoryData *d);
    SharingWindow* getWindow(int index, SharedMemoryData *d);
private:
    SharedMemory m_shared_memory;
    int m_id;
};
