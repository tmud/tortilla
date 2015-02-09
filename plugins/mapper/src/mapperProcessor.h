#pragma once

#include "mapperObjects.h"
#include "mapperParser.h"
#include "mapperPrompt.h"
#include "mapperKey.h"
#include "mapperHashTable.h"
#include "mapperRender.h"

class KeyPair
{
public:
    KeyPair() : afterend(0) {}
    enum kpmode { BEGIN = 0, END, ALL };
    void init(const tstring &b, const tstring& e);
    bool get(StreamTrigger &t, kpmode mode, tstring *result);
    u8string begin;
    u8string end;
    int afterend;
private:
    void initkey(const tstring& src, u8string* res);
};    

class MapperActions
{
public:
    virtual void setCurrentRoom(Room *room) = 0;
    virtual void lostPosition() = 0;
    virtual void setPossibleRooms(const std::vector<Room*>& rooms) = 0;
    virtual void addNewZone(Zone *zone) = 0;
};

class MapperProcessor
{
public:
    MapperProcessor(PropertiesMapper *props);
    ~MapperProcessor();
    void setCallback(MapperActions* actions);
    void updateProps();
    void processNetworkData(u8string& ndata);
    void processCmd(const tstring& cmd);    
    void selectDefault();
    void saveMaps(lua_State *L);
    void loadMaps(lua_State *L);

private:
    bool  recognizeRoom(u8string& ndata, RoomData *room);

    void  processData(const RoomData& room);
    bool  setByDir(Room *room);
    Zone* createZone();
    void  setCurrentRoom(Room *room);
    Room* createRoom(const RoomData& room);
    void  deleteRoom(Room* room);
    void  popDir();    

private:
    PropertiesMapper *m_propsData;
    MapperActions* m_pActions;

    StreamTrigger m_parser;
    StreamTrigger m_prompt;

    KeyPair kp_name;
    KeyPair kp_key;
    KeyPair kp_descr;
    KeyPair kp_exits;
    KeyPair kp_prompt;
    
    // Helper to parse incoming data and find rooms
    //MapperParser    m_parser;
    //MapperPrompt    m_prompt;
    //MapperKey       m_key_trimmer;

    MapperHashTable m_table;

    // Order for commands
    std::list<RoomDir> m_path;
    RoomDir m_lastDir;
    
    //Room *m_pLastRoom;

    // Current position + all possible rooms
    Room* m_pCurrentRoom;
    std::vector<Room*> m_pos_rooms;

    // Zones list
    std::vector<Zone*> m_zones;
};
