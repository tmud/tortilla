#include "stdafx.h"
#include "sharingController.h"

const tchar *global_share_name = L"TortillaTray";
const int global_share_size = 65536;

SharingController::SharingController() : m_shared_revision(-1) 
{
}

bool SharingController::init()
{
    if (!m_shared_memory.open(this, global_share_name, global_share_size))
        return false;
    return true;
}

void SharingController::regTray(const tstring& trayid)
{
    RegTrayCommand cmd;
    cmd.create(trayid);
}

void SharingController::unregTray(const tstring& trayid)
{
}

void SharingController::addMessage(const SharingDataMessage& msg)
{
}

bool SharingController::write()
{
    bool result = false;
    void *p = m_shared_memory.lock();
    if (reread(p, m_shared_memory.size()))
    {   
        m_shared_revision++;
        DataQueue d;
        Serialize s(d);
        s.write(m_shared_revision);
        m_shared_data.serialize(s);        
        if ((unsigned int)d.getSize() <= m_shared_memory.size())
        {
            memcpy(p, d.getData(), d.getSize());
            result = true;
        }
    }
    m_shared_memory.unlock();
    return result;
}

bool SharingController::read()
{
    void *p = m_shared_memory.lock();
    bool result = reread(p, m_shared_memory.size());
    m_shared_memory.unlock();
    return result;
}

bool SharingController::reread(void *p, unsigned int size)
{
    // check revision of shared memory
    DataQueue d;
    d.write(p, sizeof(int));

    int revision = 0;
    Serialize s(d);
    if (!s.read(revision))
        return false;
    if (m_shared_revision == revision)
        return true;

    DataQueue d1;
    d1.write(p, size);
    Serialize s1(d1);
    if (!m_shared_data.deserialize(s1))
    {
        m_shared_revision = -1;
        return false;
    }
    m_shared_revision = revision;
    return true;
}
