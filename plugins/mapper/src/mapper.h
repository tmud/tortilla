#pragma once

#include "splitterEx.h"
#include "mapperProcessor.h"
#include "mapperRender.h"
#include "mapperToolbar.h"
#include "mapperZoneControl.h"
#include "mapInstance.h"

class Mapper : public CWindowImpl<Mapper>
{
public:
    Mapper(PropertiesMapper *props);
    ~Mapper();
    void processNetworkData(const tchar* text, int text_len);
    void processCmd(const tstring& cmd);
    void updateProps();
private:
    BEGIN_MSG_MAP(Mapper)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
      //MESSAGE_HANDLER(WM_USER, OnUser)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { m_hWnd = NULL; return 0; }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    //LRESULT OnUser(UINT, WPARAM, LPARAM, BOOL&){ onZoneChanged();  return 0; }
    void onCreate();
    void onSize();
    //void onZoneChanged();
private:
    void popDir();
    void setExits(Room *room);
    void checkExit(Room *room, RoomDir dir, const tstring& exit);    
    void redrawPosition(MapCursor cursor);
private:
    // map
    MapInstance m_map;

    // properties
    PropertiesMapper *m_propsData;

    // ui, windows and toolbars    
    MapperToolbar m_toolbar;
    MapperRender m_view;
    ToolbarViewContainer m_container;
    CSplitterWindowExT<true, 1, 4> m_vSplitter;
    MappeZoneControl m_zones_control;    
    int m_toolbar_height;

    // logic    
    MapperDirsVector m_dirs;
    MapperProcessor m_processor;
    MapperPrompt m_prompt;
    MapperDarkRoom m_dark;
    std::deque<RoomDir> m_path;
    RoomDir m_lastDir;
    Room *m_pCurrentRoom;
};
