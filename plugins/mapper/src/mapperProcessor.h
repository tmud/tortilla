#pragma once

#include "properties.h"
#include "mapperObjects.h"

class MapperKeyElement
{
public:
    MapperKeyElement();
    bool init(const tstring& macro);
    void reset();
    bool findData(const tchar* data, int datalen);

    void truncate() { key = 0; }
    int  getKey() const { return key; }
    int  getKeyLen() const { return keylen; }
    int  getAfterKey() const { return key + keylen; }
    bool isKeyFull() const {
        int size = keydata.size();
        return (size > 0 && keylen == size) ? true : false;
    }

private:
    tstring keydata;
    int  key;
    int  keylen;
};

class MapperDataQueue
{
public:
    void write(const tchar* data, int datalen)  {    buffer.write(data, datalen * sizeof(tchar));  }
    void truncate(int datalen) {  buffer.truncate(datalen * sizeof(tchar)); }
    void clear() {  buffer.clear(); }
    int getDataLen() const  {  return buffer.getSize() / sizeof(WCHAR); } 
    const tchar* getData() const { return (tchar*)buffer.getData(); }
private:
    DataQueue buffer;
};

class MapperProcessor
{
public:
    MapperProcessor();
    void updateProps(PropertiesMapper *props);
    bool processNetworkData(const tchar* text, int textlen, RoomData* result);

private:
    bool searchData(const tchar* data, int datalen, RoomData* result);
    void checkBufferLimit();
    MapperDataQueue  m_network_buffer;
    MapperKeyElement bn;        // begin name
    MapperKeyElement bn2;
    MapperKeyElement en;        // end name
    MapperKeyElement bv;        // begin vnum
    MapperKeyElement ev;        // end vnum
    MapperKeyElement bd;        // begin description
    MapperKeyElement ed;        // end description
    MapperKeyElement be;        // begin exits
    MapperKeyElement ee;        // end exits
};
