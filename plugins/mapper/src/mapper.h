#pragma once

#include "splitterEx.h"
#include "mapperObjects.h"
#include "mapperProcessor.h"
#include "mapperPrompt.h"
#include "mapperRender.h"
#include "mapperHashTable.h"
#include "mapperToolbar.h"
#include "mapperZoneControl.h"
#include "mapperRoomsCache.h"

class MapperDirCommand
{
    int dir; tstring main, rel;
    int main_size, rel_size;
public:
    MapperDirCommand(int d, const tstring& main_part, const tstring& rel_part) : dir(d), main(main_part), rel(rel_part) 
    {
        main_size = main.size(); rel_size = rel.size();
    }
    int check(const tstring& cmd) const;
};
typedef std::vector<MapperDirCommand> DirsVector;

class Mapper : public CWindowImpl<Mapper>
{
public:
    Mapper(PropertiesMapper *props);
    ~Mapper();
    void processNetworkData(const tchar* text, int text_len);
    void processCmd(const tstring& cmd);
    void updateProps();
    void saveMaps(lua_State *L);
    void loadMaps(lua_State *L);

    // operations
    void newZone(Room *room, RoomDir dir);

private:
    BEGIN_MSG_MAP(Mapper)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
      MESSAGE_HANDLER(WM_USER, OnUser)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { m_hWnd = NULL; return 0; }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    LRESULT OnUser(UINT, WPARAM, LPARAM, BOOL&){ onZoneChanged();  return 0; }
    void onCreate();
    void onSize();
    void onZoneChanged();

private:
    Zone* addNewZone();
    Room* findRoomCached(const RoomData& room);
    Room* findRoom(const RoomData& room);
    Room* addNewRoom(const RoomData& room);
    Room* createNewRoom(const RoomData& room);
    void  deleteRoom(Room* room);
    void  changeLevelOrZone(Room *old, Room* curr);
    void  checkExits(Room *room);
    int   revertDir(int dir);
    void  popDir();
    Room* getNextRoom(Room *room, int dir);
    void  redrawPosition();

private:
    // properties
    PropertiesMapper *m_propsData;

    // ui, windows and toolbars
    MapperToolbar m_toolbar;
    CSplitterWindowExT<true, 1, 4> m_vSplitter;
    MappeZoneControl m_zones_control;
    MapperRender m_view;
    int m_toolbar_height;

    // logic
    DirsVector m_dirs;
    MapperProcessor m_processor;
    MapperPrompt m_prompt;
    MapperHashTable m_table;
    //todo! MapperRoomsCache m_cache;

    std::deque<int> m_path;
    int m_lastDir;
    Room *m_pCurrentRoom;
    std::vector<Zone*> m_zones;
    RoomCursor m_rpos;
    ViewMapPosition m_viewpos;
};
