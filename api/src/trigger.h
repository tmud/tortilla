#pragma once
#include <assert.h>
#include "../api.h"
#include "../common/dataQueue.h"

typedef std::pair<int, int> findpos;
class TriggerKeyElement
{
public:
    TriggerKeyElement();
    bool init(const utf8* macro);
    bool findData(const utf8* data, int datalen);
    void truncate() { pos.first = 0; }

    int  getBegin() const { return pos.first; }
    int  getEnd() const { return pos.first + pos.second; }
    int  getLen() const { return pos.second; }    
    bool isFullComparsion() const {
        int size = keydata.size();
        return (size > 0 && pos.second == size) ? true : false;
    }
    bool isUsable() const { return (pos.first != -1) ? true : false; }
    int  getTemplateLen() const { return keydata.size(); }
    void reset();

private:
    bool initfail();
    u8string keydata;
    findpos pos;
};

class Trigger
{
public:
    Trigger();
    bool init(const utf8* begin, const utf8* end, int max_len);
    int  add(const utf8* data);
    int  find(const utf8* data);
    const utf8* get(int from, int len);  
    
private:
    void reset();
    typedef std::pair<const utf8*, int> block_data;
    block_data getdata();
    DataQueue m_data;
    TriggerKeyElement m_key_begin;
    TriggerKeyElement m_key_end;
    int m_data_maxlen;
    u8string m_find_buffer;
};
