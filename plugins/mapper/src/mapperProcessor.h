#pragma once

#include "mapperObjects.h"
#include "mapperParser.h"
#include "mapperPrompt.h"
#include "mapperHashTable.h"
#include "mapperRender.h"

class MapperActions
{
public:
    virtual void setCurrentRoom(Room *room) = 0;
    virtual void setCurrentLevel(RoomsLevel *level) = 0;
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
    void processNetworkData(const wchar_t* text, int text_len);
    void processCmd(const wchar_t* text, int text_len);
    void updateProps();
    void selectDefault();
    void saveMaps(lua_State *L);
    void loadMaps(lua_State *L);

private:
    void processData(const RoomData& room);
    Zone* createZone();
    Room* createRoom(const RoomData& room);
    void  deleteRoom(Room* room);
    int   revertDir(int dir);
    void  popDir();
    void  setCurrentRoom(Room *room);

private:
    PropertiesMapper *m_propsData;
    MapperActions* m_pActions;

    // Helper to parse incoming data and find rooms
    MapperParser    m_parser;
    MapperPrompt    m_prompt;
    MapperHashTable m_table;

    // Order for commands
    std::list<int> m_path;
    int m_lastDir;
    Room *m_pLastRoom;

    // Current position
    Room *m_pCurrentRoom;
    RoomsLevel *m_pCurrentLevel;

    // Zones list
    std::vector<Zone*> m_zones;
};
