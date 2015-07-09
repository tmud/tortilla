#pragma once

class WaitCommands
{
    typedef std::pair<int, tstring> wait_cmd;
    std::vector<wait_cmd> m_commands;
public:
    void add(int timeout_msec, const tstring& cmd)
    {
        
    }
    void process()
    {
    
    }
};
