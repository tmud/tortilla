#include "stdafx.h"
#include "sharingController.h"

const tchar *global_share_name = L"TortillaTray";
const int global_share_size = 65536;

SharingController::SharingController() : m_shared_revision(-1), m_locked(false)
{
}

SharingController::~SharingController()
{
}

bool SharingController::init()
{
    if (!m_shared_memory.open(this, global_share_name, global_share_size))
        return false;
    {   // gen id
        tchar buffer[MAX_PATH];
        GetTempFileName(L"", L"", 0, buffer);
        m_id.assign(&buffer[1]);
    }
    return pushCommand(new RegTrayCommand(m_id));
}

void SharingController::release()
{
     pushCommand(new UnregTrayCommand(m_id));
}

bool SharingController::pushCommand(SharingCommand* cmd)
{
    bool result = false;
    if (cmd && lock())
    {
        m_shared_data.commands.push_back(cmd);
        result = unlock();
    }
    else {
        delete cmd;
    }
    return result;
}

bool SharingController::lock()
{
    m_shared_memory.lock();
    if (read(m_shared_memory.ptr(), m_shared_memory.size()))       
        return true;
    m_shared_memory.unlock();
    return false;
}

bool SharingController::unlock()
{
    bool result = write();
    m_shared_memory.unlock();
    if (result)
        m_shared_memory.sendChangeEvent();
    return result;
}

bool SharingController::write()
{
    m_shared_revision++;
    DataQueue d;
    Serialize s(d);
    s.write(m_shared_revision);
    m_shared_data.serialize(s);
    if ((unsigned int)d.getSize() <= m_shared_memory.size())
    {
        memcpy(m_shared_memory.ptr(), d.getData(), d.getSize());
        return true;
    }
    return false;
}

bool SharingController::read(void *p, unsigned int size)
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
