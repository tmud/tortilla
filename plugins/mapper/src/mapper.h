#pragma once

#include "splitterEx.h"
#include "mapperProcessor.h"
#include "mapperToolbar.h"
#include "mapperZoneControl.h"

class Mapper : public CWindowImpl<Mapper>, public MapperActions
{
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

private: // actions from processor
    void setCurrentRoom(Room *room) 
    {
        m_view.setCurrentRoom(room);
        Zone *zone = room->level->getZone();
        m_zones_control.zoneChanged(zone);    
    }    
    void lostPosition() { m_view.setCurrentRoom(NULL); }
    void setPossibleRooms(const std::vector<Room*>& rooms) {}
    void addNewZone(Zone *zone) { m_zones_control.addNewZone(zone); }

private:
    void setCurrentLevel(RoomsLevel *level) {}
    MapperToolbar m_toolbar;
    CSplitterWindowExT<true, 1, 3> m_vSplitter;
    MapperZoneControl m_zones_control;
    MapperRender m_view;
    int m_toolbar_height;
};
