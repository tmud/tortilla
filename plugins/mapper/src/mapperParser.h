#pragma once

#include "properties.h"
#include "mapperObjects.h"

/*class MapperKeyElement
{
public:
    MapperKeyElement();
    bool init(const tstring& macro);
    void reset();
    bool findData(const WCHAR* data, int datalen);

    void truncate() { key = 0; }
    int  getKey() const { return key; }
    int  getKeyLen() const { return keylen; }
    int  getAfterKey() const { return key + keylen; }
    bool isKeyFull() const {
        int size = keydata.size();
        return (size > 0 && keylen == size) ? true : false;
    }
    bool isKeyUsable() const { return (key != -1) ? true : false; }
    int  getKeyTemplateLen() const { return keydata.size(); }

private:
    tstring keydata;
    int  key;
    int  keylen;
};

class MapperDataQueue
{
public:
    void write(const WCHAR* data, int datalen)  { buffer.write(data, datalen * sizeof(WCHAR)); }
    void truncate(int datalen) { buffer.truncate(datalen * sizeof(WCHAR)); }
    void clear() { buffer.clear(); }    
    int getDataLen() const  { return buffer.getSize() / sizeof(WCHAR); }
    const WCHAR* getData() const { return (WCHAR*)buffer.getData(); }
private:
    DataQueue buffer;
};*/

/*class MapperNetworkData
{
public:
    MapperNetworkData(const utf8* data)
    {
        assert(data);
        int datalen = (wcslen(data) + 1) *sizeof(WCHAR);
        buffer.alloc(datalen);
        memcpy(buffer.getData(), data, datalen);
    }    
    const utf8* getData() const { return (utf8*)buffer.getData(); }
    void append(const WCHAR* data, int size) 
    {
        int cur_size = buffer.getSize()-1;
        int new_size = (cur_size + size + 1) * sizeof(WCHAR);
        buffer.keepalloc(new_size);
        WCHAR *p = (WCHAR*)buffer.getData() + cur_size;
        int datalen = size * sizeof(WCHAR);        
        memcpy(p, data, datalen);
        p[size] = 0;
    }
    void trimLeft(int size) 
    {
        assert(size <= getDataLen());
        buffer.alloc( (size+1)*sizeof(WCHAR) );
        WCHAR *p = (WCHAR*)buffer.getData();
        p[size] = 0;
    }
    void trimRight(int size)
    {
        assert(size <= getDataLen());
        WCHAR *p = (WCHAR*)buffer.getData() + size;
        int len = (getDataLen() - size + 1) * sizeof(WCHAR);
        memcpy(buffer.getData(), p, len);
        buffer.alloc(len);
    }

private:    
    DataQueue buffer;
};
*/

/*
class MapperParser
{
public:
    MapperParser();
    void updateProps(PropertiesMapper *props);
    bool processNetworkData(MapperNetworkData &ndata, RoomData* result);

private:


    bool searchData(const WCHAR* data, int datalen, RoomData* result);
    void checkBufferLimit();
    MapperDataQueue  m_network_buffer;
    MapperKeyElement bn;        // begin name
    MapperKeyElement bn2;
    MapperKeyElement en;        // end name
    MapperKeyElement bk;        // key begin
    MapperKeyElement ek;        // key end
    MapperKeyElement bd;        // begin description
    MapperKeyElement ed;        // end description
    MapperKeyElement be;        // begin exits
    MapperKeyElement ee;        // end exits
    tstring dark_cs;            // dark room compare string



};
*/