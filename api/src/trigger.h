#pragma once
#include <assert.h>
#include "../api.h"
#include "../common/dataQueue.h"

class TriggerKeyElement
{
public:
    TriggerKeyElement();
    bool init(const utf8* macro);
    bool findData(const utf8* data, int datalen);
    void truncate() { find_pos = 0; }

    int  getBegin() const { return find_pos; }
    int  getEnd() const { return find_pos + find_len; }
    int  getLen() const { return find_len; }    
    bool isFullComparsion() const {
        int size = keydata.size();
        return (size > 0 && find_len == size) ? true : false;
    }
    bool isUsable() const { return (find_pos != -1) ? true : false; }
    int  getTemplateLen() const { return keydata.size(); }
    void reset();

private:    
    bool initfail();
    u8string keydata;
    int  find_pos;
    int  find_len;
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
    void checkBufferLimit();
    DataQueue m_data;
    TriggerKeyElement m_key_begin;
    TriggerKeyElement m_key_end;
    int m_data_maxlen;
    u8string m_find_buffer;
};
