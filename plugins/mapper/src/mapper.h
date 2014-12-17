#pragma once

#include "splitterEx.h"
#include "mapperObjects.h"
#include "mapperProcessor.h"
#include "mapperPrompt.h"
#include "mapperRender.h"
#include "mapperHashTable.h"
#include "mapperToolbar.h"
#include "mapperZoneControl.h"

class Mapper : public CWindowImpl<Mapper>
{
public:
    Mapper(PropertiesMapper *props);
    ~Mapper();
    void processNetworkData(const wchar_t* text, int text_len);
    void processCmd(const wchar_t* text, int text_len);
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
    void  findRooms(const RoomData& room, std::vector<Room*> *vr);    
    Room* addNewRoom(const RoomData& room);
    Zone* addNewZone();
    Room* createNewRoom(const RoomData& room);
    void  deleteRoom(Room* room);
    void  changeLevelOrZone(Room *old, Room* curr);
    void  checkExits(Room *room);
    int   revertDir(int dir);
    void  popDir();
    //Room* getNextRoom(Room *room, int dir);
    void  redrawPosition(Room *newpos);

private:
    PropertiesMapper *m_propsData;

    // Elements on the screen
    MapperToolbar m_toolbar;
    CSplitterWindowExT<true, 1, 3> m_vSplitter;
    MapperZoneControl m_zones_control;
    MapperRender m_view;
    int m_toolbar_height;

    MapperProcessor m_processor;
    MapperPrompt    m_prompt;
    MapperHashTable m_table;

    std::list<int> m_path;
    int m_lastDir;
    Room *m_pCurrentRoom;
    /*RoomCursor m_lastpos;

    std::vector<Zone*> m_zones;    
    ViewMapPosition m_viewpos;*/
};
