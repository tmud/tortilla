#pragma once

#include "splitterEx.h"
#include "mapperRender.h"
#include "mapperToolbar.h"
#include "mapperZoneControl.h"
#include "mapInstance.h"
#include "properties.h"

class Mapper : public CWindowImpl<Mapper>, public MapperRenderRoomMoveTool
{
public:
    Mapper(PropertiesMapper *props, const tstring& mapsFolder);
    ~Mapper();
    void processMsdp(const RoomData& rd);
	void saveMaps();
	void loadMaps();
private:
    BEGIN_MSG_MAP(Mapper)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
      MESSAGE_HANDLER(WM_USER, OnUser)
      MESSAGE_HANDLER(WM_USER+2, OnUser2)
      MESSAGE_HANDLER(WM_COMMAND, OnMenu)
      MESSAGE_HANDLER(WM_USER+1, OnToolbar)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { m_hWnd = NULL; return 0; }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    LRESULT OnUser(UINT, WPARAM, LPARAM, BOOL&){ onZoneChanged();  return 0; }
    LRESULT OnUser2(UINT, WPARAM, LPARAM, BOOL&){ onZoneDeleted();  return 0; }
    LRESULT OnToolbar(UINT, WPARAM wparam, LPARAM, BOOL&){ onToolbar(wparam);  return 0; }
    LRESULT OnMenu(UINT, WPARAM wparam, LPARAM, BOOL&){ onRenderContextMenu(LOWORD(wparam)); return 0; }
    void onCreate();
    void onSize();
    void onZoneChanged();
    void onZoneDeleted();
    void onRenderContextMenu(int id);  
    void onToolbar(int id);
private:
	void updateZonesList();
    void redrawPosition(MapCursor cursor, bool resetScrolls);
    void redrawPositionByRoom(const Room *room);
    void roomMoveTool(std::vector<const Room*>& rooms, int x, int y);
    void lostPosition();
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

    // logic
    //MapperDirsVector m_dirs;
    Room *m_pCurrentRoom;

    tstring m_mapsFolder;
};
