#pragma once
#include "mapperObjects.h"
#include "mapperRender.h"
#include "properties.h"
#include "roomCursor.h"

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
    /*virtual void setCurrentRoom(Room *room) = 0;
    virtual void lostPosition() = 0;
    virtual void setPossibleRooms(const std::vector<Room*>& rooms) = 0;
    virtual void addNewZone(Zone *zone) = 0;*/
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
    void saveMaps(lua_State *L);
    void loadMaps(lua_State *L);
    void selectDefault() { //todo
    }

private:
    bool  recognizeRoom(u8string& ndata, RoomData *room);
    bool  recognizePrompt(u8string& ndata);
    void  popDir();
    void  processData(const RoomData& room);

private:
    friend class Table;
    PropertiesMapper *m_propsData;
    MapperActions *m_pActions;

    // Helpers to parse incoming data and recognize rooms and prompt
    StreamTrigger m_parser;
    StreamTrigger m_prompt;
    KeyPair kp_name;
    KeyPair kp_key;
    KeyPair kp_descr;
    KeyPair kp_exits;
    KeyPair kp_prompt;

    // Order for commands
    std::list<RoomDir> m_path;
    RoomDir m_lastDir;
    
    // Current position
    Room* m_pCurrentRoom;

    Table m_world;
};
