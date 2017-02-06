#include "stdafx.h"
#include "sharingController.h"

const wchar_t *global_share_name = L"TortillaTray";
const int global_share_size = sizeof(SharingHeader) + 100*sizeof(SharingWindow);

class SharedMemoryInitializer : public SharedMemoryHandler
{
    void onInitSharedMemory(SharedMemoryData *d)
    {
        SharingHeader sh;
        size_t datalen = sizeof(SharingHeader);
        sh.messages = 0;
        memcpy(d->data, &sh, datalen);
        d->data_size = datalen;
    }
};

SharingController::SharingController()
{
}

SharingController::~SharingController()
{
}

bool SharingController::init()
{
    SharedMemoryInitializer smi;
    if (!m_shared_memory.create(global_share_name, global_share_size, &smi))
        return false;
   /* SharedMemoryLocker l(&m_shared_memory);
    SharedMemoryData* m = l.memory();
    SharingHeader* sh = getHeader(m);*/
    return true;
}

bool SharingController::tryAddWindow(SharingWindow* sw, const RECT& working_area, int dh)
{
    SharedMemoryLocker l(&m_shared_memory);
    SharedMemoryData* m = l.memory();
    if (m->data_size+sizeof(SharingWindow) > m->max_size)
       return false;
    SharingHeader* h = getHeader(m);
    int count = h->messages;
    if (count > 0) {
        SharingWindow* last = getWindow(count-1, m);
        if ((last->y - sw->h - dh ) < working_area.top)
            return false;
        sw->y = last->y - sw->h - dh;
    }
    else
    {
        sw->y = working_area.bottom - sw->h - dh;
    }
    sw->x = working_area.right - sw->w;
    SharingWindow* w = getWindow(count, m);   
    *w = *sw;
    h->messages = count + 1;
    return true;
}

bool SharingController::tryMoveWindow(SharingWindow* sw,const RECT& working_area, int dh)
{
    SharedMemoryLocker l(&m_shared_memory);
    SharedMemoryData* m = l.memory();
    SharingHeader* h = getHeader(m);
    int count = h->messages;
    int index = -1;
    for (int i=0; i<count; ++i)
    {
        SharingWindow* w = getWindow(i, m);
        if (w->x == sw->x && w->y == sw->y && w->w == sw->w && w->h == sw->h) {
            index = i; break;
        }
    }
    if (index == -1)
    {
        //todo!
        SharingWindow* w = getWindow(0, m);

        return false;
    }

    SharingWindow* w = getWindow(index, m);
    if (index == 0) {
        int bottom = w->y + w->h;
        if (bottom == working_area.bottom-dh)
            return false;
        sw->y = working_area.bottom - sw->h - dh;
        *w = *sw;
        return true;
    }

    SharingWindow *prev = getWindow(index-1, m);
    int newy = prev->y - w->h - dh;
    if (newy == w->y)
        return false;
    sw->y = newy;
    *w = *sw;
    return true;    
}

bool SharingController::getLastPostion(SharingWindow* sw)
{
    SharedMemoryLocker l(&m_shared_memory);
    SharedMemoryData* m = l.memory();
    SharingHeader* h = getHeader(m);
    int count = h->messages;
    if (count == 0) return false;
    SharingWindow* lastw = getWindow(count-1, m);
    *sw = *lastw;
    return true;
}

void SharingController::deleteWindow(const SharingWindow* sw)
{
    SharedMemoryLocker l(&m_shared_memory);
    SharedMemoryData* m = l.memory();
    SharingHeader* h = getHeader(m);
    int count = h->messages;
    SharingWindow* w = getWindow(0, m);
    bool found = false;
    for (int i=0; i<count; ++i)
    {
        SharingWindow& c = w[i];
        if (c.x == sw->x && c.y == sw->y && c.w == sw->w && c.h == sw->h) {
            memcpy(&w[i], &w[i+1], sizeof(SharingWindow)*(count-i-1));
            h->messages = count - 1;
            found = true;
            break;
        }
    }
    if (!found) {
        assert(false);    
    }
}

/*void SharingController::updateWindow(const SharingWindow& sw, int newx, int newy)
{
    SharedMemoryLocker l(&m_shared_memory);
    SharedMemoryData* m = l.memory();
    SharingHeader* h = getHeader(m);
    int count = h->messages;
    SharingWindow* w = getWindow(0, m);
    for (int i=0; i<count; ++i)
    {
        SharingWindow& c = w[i];
        if (c.x == sw.x && c.y == sw.y && c.w == sw.w && c.h == sw.h) {
            c.x = newx; c.y = newy;
            break;
        }
    }
}*/

/*bool SharingController::getLastWindow(SharingWindow* sw)
{
    SharedMemoryLocker l(&m_shared_memory);
    SharedMemoryData* m = l.memory();
    SharingHeader* h = getHeader(m);
    int count = h->messages;
    if (count == 0)
        return false;
    SharingWindow* w = getWindow(count-1, m);
    *sw = *w;
    return true;
}*/

int SharingController::findWindow(const SharingWindow* sw, SharedMemoryData* memory)
{
    SharingHeader* h = getHeader(memory);
    int count = h->messages;
    SharingWindow* w = getWindow(0, memory);
    for (int i=0; i<count; ++i)
    {
        SharingWindow& c = w[i];
        if (c.x == sw->x && c.y == sw->y && c.w == sw->w && c.h == sw->h) {
            return i;
        }
    }
    return -1;
}

SharingHeader* SharingController::getHeader(SharedMemoryData *d)
{
    return (SharingHeader*)d->data;
}

SharingWindow* SharingController::getWindow(int index, SharedMemoryData* d)
{
    SharingHeader *h = getHeader(d);
    SharingWindow* w = (SharingWindow*)(h+1);
    return &w[index];
}
