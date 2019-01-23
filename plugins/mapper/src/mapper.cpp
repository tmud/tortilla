#include "stdafx.h"
#include "mapper.h"
#include "roomObjects.h"
#include "debugHelpers.h"
//#include "roomMergeTool.h"
#include "newZoneNameDlg.h"
#include "mapTools.h"
#include "mapSmartTools.h"

Mapper::Mapper(PropertiesMapper *props, const tstring& mapsFolder) : m_propsData(props), 
m_pCurrentRoom(NULL), m_mapsFolder(mapsFolder)
{
}

Mapper::~Mapper()
{
}

void Mapper::processMsdp(const RoomData& rd)
{
    RoomDirHelper dh;
    RoomDir movement = RD_UNKNOWN;
    if (m_pCurrentRoom)
    {
        const tstring& vn = m_pCurrentRoom->roomdata.vnum;
        for (const std::pair<tstring, tstring>& e : rd.exits) {
            if (e.second == vn) {
                RoomDir dir = dh.getDirFromMsdp(e.first);
                movement = dh.revertDir(dir);
                break;
            }
        }
    }
    auto fillExits = [](Room* room) 
    {
        RoomDirHelper dh;
        for (const std::pair<tstring, tstring>& p : room->roomdata.exits) {
            RoomDir dir = dh.getDirFromMsdp(p.first);
            if (dir == RD_UNKNOWN) {
                assert(false);
            }
            else {
                room->dirs[dir].exist = true;
            }
        }
    };
    Rooms3dCube *zone = m_map.findZone(rd.areaname);
    if (!zone)
        zone = m_map.createNewZone(rd.areaname);
    Room* room = zone->findRoom(rd.hash());
    if (!room)
    {
        Rooms3dCubePos p;
        if (m_pCurrentRoom)
        {
            if (movement != RD_UNKNOWN && m_pCurrentRoom->pos.zid == zone->id())
            {
                p = m_pCurrentRoom->pos;
                p.move(movement);
            }
        }
        Room *newroom = new Room();
        newroom->roomdata = rd;
        fillExits(newroom);
        Rooms3dCube::AR_STATUS result = (movement != RD_UNKNOWN) ?
            zone->addRoom(p, newroom) : zone->addRoomWithUnknownPosition(newroom);
        if (result != Rooms3dCube::AR_OK) {
            delete newroom;
            lostPosition();
            assert(false);
            return;
        }
        room = newroom;
    }
    else
    {
        assert(room->roomdata.vnum == rd.vnum);
        room->roomdata = rd;
        fillExits(room);
    }
    if (movement != RD_UNKNOWN)
    {
        m_pCurrentRoom->dirs[movement].next_room = room;
        RoomDir revertDir = dh.revertDir(movement);
        room->dirs[revertDir].next_room = m_pCurrentRoom;
    }
    m_pCurrentRoom = room;
    redrawPositionByRoom(m_pCurrentRoom);
}

void Mapper::updateZonesList()
{
	Rooms3dCubeList zones;
	m_map.getZones(&zones);
	m_zones_control.updateList(zones);
}

void Mapper::saveMaps()
{
    const tstring&dir = m_mapsFolder;
	m_map.saveMaps(dir);
}

void Mapper::loadMaps()
{
    m_view.clear();
    m_zones_control.deleteAllZones();

	m_pCurrentRoom = nullptr;

    const tstring&dir = m_mapsFolder;
    bool last_found = false;
    tstring &last = m_propsData->current_zone;

    m_map.loadMaps(dir);
	updateZonesList();

    Rooms3dCubeList zones;
	m_map.getZones(&zones);
	for (Rooms3dCube* zone : zones)
	{
        if (!last.empty() && last == zone->name()) {
            MapTools t(&m_map);
            MapCursor c = t.createZoneCursor(zone);
            redrawPosition(c, false);
            last_found = true;
            break;
        }
	}
    if (!last_found && !zones.empty()) {
        MapTools t(&m_map);
        MapCursor c = t.createZoneCursor(zones[0]);
        redrawPosition(c, false);
    }
}

void Mapper::lostPosition()
{
    m_pCurrentRoom = nullptr;
    redrawPositionByRoom(nullptr);
}

void Mapper::redrawPosition(MapCursor cursor, bool resetScrolls)
{
    m_view.showPosition(cursor, resetScrolls);
    const Rooms3dCube* zone = cursor->zone();
    m_zones_control.setCurrentZone(zone);
}

void Mapper::redrawPositionByRoom(const Room *room)
{
    updateZonesList();
    m_view.clearSelection();
    MapCursorColor color = (room) ? RCC_NORMAL : RCC_NONE;    
    MapTools tools(&m_map);
    Room *r = (room) ? tools.findRoom(room->roomdata.hash()) : nullptr;
    MapCursor c = tools.createCursor( r, color );
    redrawPosition(c, true);
}

void Mapper::onCreate()
{
    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    RECT rc;
    GetClientRect(&rc);
    m_vSplitter.Create(m_hWnd, rc);
    m_vSplitter.m_cxySplitBar = 3;

    RECT pane_left, pane_right;
    m_vSplitter.GetSplitterPaneRect(0, &pane_left); pane_left.right -= 3;
    m_vSplitter.GetSplitterPaneRect(1, &pane_right);

    m_zones_control.Create(m_vSplitter, pane_left, style);
    m_container.Create(m_vSplitter, pane_right, L"", style);

    m_toolbar.Create(m_container, rcDefault, style);
    m_toolbar.setControlWindow(m_hWnd, WM_USER+1);
    m_view.Create(m_container, rcDefault, NULL, style | WS_VSCROLL | WS_HSCROLL, WS_EX_STATICEDGE);
    m_view.setMenuHandler(m_hWnd);
    m_view.setMoveToolHandler(this);

    m_container.attach(32, m_toolbar, m_view);
    m_vSplitter.SetSplitterPanes(m_zones_control, m_container);

    m_zones_control.setNotifications(m_hWnd, WM_USER, WM_USER+2);
    m_vSplitter.SetSplitterRect();

    if (m_propsData->zoneslist_width > 0)
        m_vSplitter.SetSplitterPos(m_propsData->zoneslist_width);
    else
        m_vSplitter.SetDefaultSplitterPos();
}

void Mapper::onSize()
{
    RECT rc;
    GetClientRect(&rc);
    m_vSplitter.MoveWindow(&rc, FALSE);
    m_vSplitter.SetSplitterRect();
}

void Mapper::onZoneChanged()
{
    MapCursor current = m_view.getCurrentPosition();
    int zone = m_zones_control.getCurrentZone();    
    if (m_pCurrentRoom && m_pCurrentRoom->pos.zid == zone)
        return redrawPositionByRoom(m_pCurrentRoom);
    MapTools t(&m_map);
    Rooms3dCube *ptr = m_map.findZone(zone);
    MapCursor cursor = t.createZoneCursor(ptr);
    if (cursor->valid())
        return redrawPosition(cursor, false);
    assert(false);
    return redrawPosition(t.createNullCursor(), false);
}

void Mapper::onZoneDeleted()
{
    int zone = m_zones_control.getCurrentZone();
    assert(zone != -1);
    const tstring zone_name = m_zones_control.getZoneName(zone);
    if (zone_name.empty()) {
        assert(false);
        return;
    }
    tstring msg(L"Вы уверены, что хотите удалить зону '");
    msg.append(zone_name);
    msg.append(L"' ?\r\nОтменить удаление будет невозможно.");
    if (MessageBox(msg.c_str(), L"Карта", MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2) == IDYES) {        
        m_map.deleteZone(zone_name);
        updateZonesList();
        Rooms3dCubeList zones;
	    m_map.getZones(&zones);
        if (!zones.empty())
        {
            MapTools t(&m_map);
            MapCursor cursor = t.createZoneCursor(zones[0]);
            redrawPosition(cursor, false);
        }
    }
}

void Mapper::onRenderContextMenu(int id)
{
    std::vector<const Room*> rooms;
    m_view.getSelectedRooms(&rooms);
    if (rooms.empty()) {
        assert(false);
        return;
    }

    bool multiselection = (rooms.size() == 1) ? false : true;

    RoomDirHelper dh;
    if (id == MENU_NEWZONE) 
    {
        NewZoneNameDlg dlg;
        if (dlg.DoModal() == IDCANCEL)
        {
            for (const Room* r : rooms) {
                r->selected = false;
            }
            m_view.Invalidate();
            return;
        }
        tstring newZoneName(dlg.getName());
        if (newZoneName.empty()) {
           newZoneName = m_map.getNewZoneName(L"");
        }
        MapMoveRoomsToNewZoneTool t(&m_map);
        bool result = t.makeNewZone(rooms, newZoneName);
        if (!result)
        {
            return;
        }
        redrawPositionByRoom(rooms[0]);
        return;
    }

    if (id >= MENU_NEWZONE_NORTH && id <= MENU_NEWZONE_DOWN)
    {
        assert(!multiselection);
        MapNewZoneTool t(&m_map);
        RoomDir dir = dh.cast(id - MENU_NEWZONE_NORTH);
        bool result = t.tryMakeNewZone(rooms[0], dir);
        if (!result)
        {
            MessageBox(L"Невозможно создать новую зону из-за замкнутости коридоров на данную комнату!", L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }
        // удаляем отметку, т.к. комната не идет в другую зону
        rooms[0]->selected = false;
#ifdef _DEBUG
        m_view.Invalidate();
#endif
        NewZoneNameDlg dlg;
        if (dlg.DoModal() == IDCANCEL)
        {
            t.unselectRooms();
            m_view.Invalidate();
            return;
        }
        tstring newZoneName(dlg.getName());
        if (newZoneName.empty()) {
           newZoneName = m_map.getNewZoneName(L"");
        }
        result = t.applyMakeNewZone(newZoneName);
        if (!result) {
            assert(false);
            return;
        }
        redrawPositionByRoom(rooms[0]);
        return;
    }

    if (id >= MENU_JOINZONE_NORTH && id <= MENU_JOINZONE_DOWN)
    {
        assert(!multiselection);
        RoomDir dir = dh.cast(id - MENU_JOINZONE_NORTH);
        MapConcatZonesInOne t(&m_map);
        bool result = t.tryConcatZones(rooms[0], dir);
        if (!result)
        {
            MessageBox(L"Склеить две зоны в одну не получилось!", L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }
        redrawPositionByRoom(rooms[0]);
        return;
    }

    if (id >= MENU_MOVEROOM_NORTH && id <= MENU_MOVEROOM_DOWN)
    {
        assert(!multiselection);
        RoomDir dir = dh.cast(id - MENU_MOVEROOM_NORTH);
        MapMoveRoomToolToAnotherZone t(&m_map);
        bool result = t.tryMoveRoom(rooms[0], dir);
        if (!result)
        {
            MessageBox(L"Невозможно переместить комнату в другую зону!", L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }
        redrawPositionByRoom(rooms[0]);
        return;
    }
}

void Mapper::onToolbar(int id)
{
    if (id == IDC_BUTTON_SAVEZONES) {
        saveMaps();
    }

    if (id == IDC_BUTTON_LOADZONES) {
        loadMaps();
    }

    if (id == IDC_BUTTON_CLEARZONES) {
        m_view.clear();
        m_map.clearMaps();
        m_zones_control.deleteAllZones();
        MapCursor cursor = m_view.getCurrentPosition();
        redrawPosition(cursor, false);
    }

    if (id == IDC_BUTTON_LEVEL_DOWN) {
        MapCursor c = m_view.getCurrentPosition();
        if (!c->valid())
            return;
        if (c->move(RD_DOWN))
            redrawPosition(c, false);
    }

    if (id == IDC_BUTTON_LEVEL_UP) {
        MapCursor c = m_view.getCurrentPosition();
        if (!c->valid())
            return;
        if (c->move(RD_UP))
            redrawPosition(c, false);
    }

    if (id == IDC_BUTTON_LEVEL0) {
        MapCursor c = m_view.getCurrentPosition();
        if (!c->valid())
            return;        
        const Rooms3dCubePos& pos = c->pos();
        Rooms3dCube *zone = m_map.findZone(pos.zid);
        if (!zone) {
            assert(false);
            return;
        }
        if (zone->setAsLevel0(pos.z)) {
           MapTools t(&m_map);
           Rooms3dCube *ptr = m_map.findZone(pos.zid);
           MapCursor cursor = t.createZoneCursor(ptr);
           redrawPosition(cursor, true);
        }
    }
}

void Mapper::roomMoveTool(std::vector<const Room*>& rooms, int x, int y)
{
    MapMoveRoomByMouse t(&m_map);
    bool result = t.tryMoveRooms(rooms, x, y);
    if (result) {
        MapCursor cursor = m_view.getCurrentPosition();
        redrawPosition(cursor, true);
    }
}
