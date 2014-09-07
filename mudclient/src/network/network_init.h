#pragma once

class NetworkInit
{
public:
    NetworkInit() : m_initialized(false) 
    {             
    }
    ~NetworkInit() 
    {
        if (m_initialized)
            WSACleanup();    
    }

    bool init()
    {
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        if (WSAStartup(wVersionRequested, &wsaData) != 0) 
            return false;
        m_initialized = true;
        return true;
    }

private:
    bool m_initialized;
};
