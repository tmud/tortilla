#pragma once

class WaitCommands
{
    typedef std::pair<int, tstring> wait_cmd;
    std::deque<wait_cmd> m_commands;
    Ticker m_ticker;
public:
    void add(int timeout_msec, const tstring& cmd)
    {
        assert (timeout_msec > 0);
        wait_cmd c(timeout_msec, cmd);
        // поиск позиции для вставки
        for (int i=0,e=m_commands.size();i<e;++i) 
        {
            if (timeout_msec <= m_commands[i].first) {
                m_commands.insert(m_commands.begin()+i, c);
                return; 
            }
        }
        m_commands.push_back(c);
    }
    void tick(std::vector<tstring>* commands)
    {
        int count_to_del = 0;
        int dt = m_ticker.getDiff();
        m_ticker.sync();
        for (int i=0,e=m_commands.size();i<e;++i) 
        {
            wait_cmd &c = m_commands[i];
            int timeout = c.first - dt;
            c.first = timeout;
            if (timeout <= 0) {
                commands->push_back(c.second);
                count_to_del++;
            }
        }
        if (count_to_del > 0)
            m_commands.erase(m_commands.begin(), m_commands.begin()+count_to_del);
    }
    void clear()
    {
        m_commands.clear();
    }
};
