#include "stdafx.h"
#include "mapper.h"
#include "roomObjects.h"
#include "debugHelpers.h"
//#include "roomMergeTool.h"
#include "newZoneNameDlg.h"
#include "mapTools.h"
#include "mapSmartTools.h"

Mapper::Mapper(PropertiesMapper *props, const tstring& mapsFolder) : m_propsData(props), 
m_lastDir(RD_UNKNOWN), m_pCurrentRoom(NULL), m_mapsFolder(mapsFolder)
{
}

Mapper::~Mapper()
{
}

void Mapper::processNetworkData(const tchar* text, int text_len)
{
    bool in_dark = false;
    RoomData room;
    if (!m_processor.processNetworkData(text, text_len, &room))
    {
        if (m_prompt.processNetworkData(text, text_len))
        {
            popDir();
			DEBUGOUT(L"------");
            if (m_dark.processNetworkData(text, text_len) && m_pCurrentRoom)
            {
                // move in dark to direction
                Room *next = m_pCurrentRoom->dirs[m_lastDir].next_room;
                //if (next)
                {
                    m_pCurrentRoom = next;
                    MapTools t(&m_map);
                    MapCursor cursor = t.createCursor(m_pCurrentRoom, RCC_LOST);
                    redrawPosition(cursor, false);
                }
            }
        }
		else
		{
			int x = 1;
		}
        return;
    }
    else
    {
		if (!m_prompt.processNetworkData(text, text_len))
		{
			int x = 1;

		}


        if (m_dark.processNetworkData(text, text_len))
            in_dark = true;
    }
	DEBUGOUT(L"------");
	popDir();
    DEBUGOUT(room.name);
    DEBUGOUT(room.vnum);
    //DEBUGOUT(room.exits);
	DEBUGOUT2(L"move: ", RoomDirHelper().getDirName(m_lastDir));
    

    MapTools t(&m_map);
    Room *new_room = t.findRoom(room.vnum);
    if (!new_room)
    {
        new_room = new Room();
        new_room->roomdata = room;
        setExits(new_room);
        if (!m_pCurrentRoom)
        {
            if (!t.createNewZone(new_room))
            {
                delete new_room;
                new_room = NULL;
            }
			else {
				updateZonesList();
			}
        }
        else
        {
            if (!t.addNewRoom(m_pCurrentRoom, new_room, m_lastDir))
            {
                delete new_room;
                new_room = NULL;
            }
        }
    }
    else
    {
         setExits(new_room);
         if (m_lastDir != RD_UNKNOWN && m_pCurrentRoom) {
            MapSmartTools st;
            st.addLink(m_pCurrentRoom, new_room, m_lastDir);
         }
    }
    m_pCurrentRoom = new_room;
    MapCursor cursor = t.createCursor(m_pCurrentRoom, RCC_NORMAL);
    redrawPosition(cursor, false);
}

void Mapper::processCmd(const tstring& cmd)
{
    RoomDir dir = RD_UNKNOWN;
    for (int i = 0, e = m_dirs.size(); i < e; ++i)
    {
        dir = m_dirs[i].check(cmd);
        if (dir != RD_UNKNOWN) { m_path.push_back(dir); break; }
    }

#ifdef _DEBUG
    tstring t(L"push: ");
	t.append(RoomDirHelper().getDirName(dir));
    DEBUGOUT(t);
#endif
}

void Mapper::processMsdp(const RoomData& rd)
{

}

void Mapper::popDir()
{
	if (m_path.empty())
	{
		m_lastDir = RD_UNKNOWN;
		DEBUGOUT(L"pop empty");
	}
    else {
        m_lastDir = *m_path.begin();
		m_path.pop_front();
#ifdef _DEBUG
		DEBUGOUT(L"pop");
		for (RoomDir d : m_path) {
			//DEBUGOUT(L"pop");
		}
#endif
    }
}

void Mapper::updateZonesList()
{
	Rooms3dCubeList zones;
	m_map.getZones(&zones);
	m_zones_control.updateList(zones);
}

void Mapper::checkExit(Room *room, RoomDir dir, const tstring& exit)
{
    /*const tstring& e = room->roomdata.exits; 
    if (e.find(exit) != -1) 
    {
        room->dirs[dir].exist = true;
        tstring door(L"(");
        door.append(exit);
        door.append(L")");
        if (e.find(door) != -1)
            room->dirs[dir].door = true;
    }*/
}

void Mapper::setExits(Room *room)
{
    // parse room->roomdata.exits to room->dirs
    checkExit(room,  RD_NORTH, m_propsData->north_exit);
    checkExit(room,  RD_SOUTH, m_propsData->south_exit);
    checkExit(room,  RD_WEST, m_propsData->west_exit);
    checkExit(room,  RD_EAST, m_propsData->east_exit);
    checkExit(room,  RD_UP, m_propsData->up_exit);
    checkExit(room,  RD_DOWN, m_propsData->down_exit);
}

void Mapper::updateProps()
{
    m_processor.updateProps(m_propsData);
    m_prompt.updateProps(m_propsData);
    m_dark.updateProps(m_propsData);
    InitDirVector h;
    h.make(RD_NORTH, m_propsData->north_cmd, m_dirs);
    h.make(RD_SOUTH, m_propsData->south_cmd, m_dirs);
    h.make(RD_WEST, m_propsData->west_cmd, m_dirs);
    h.make(RD_EAST, m_propsData->east_cmd, m_dirs);
    h.make(RD_UP, m_propsData->up_cmd, m_dirs);
    h.make(RD_DOWN, m_propsData->down_cmd, m_dirs);
}

void Mapper::saveProps()
{
    m_propsData->zoneslist_width = m_vSplitter.GetSplitterPos();
	int zone = m_zones_control.getCurrentZone();
	m_propsData->current_zone.clear();
	if (zone >= 0) {
		m_propsData->current_zone = m_zones_control.getZoneName(zone);
	}
	else {
		m_propsData->current_zone = L"";
	}
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

	m_path.clear();
	m_lastDir = RD_UNKNOWN;
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

void Mapper::redrawPosition(MapCursor cursor, bool resetScrolls)
{
    m_view.showPosition(cursor, resetScrolls);
    const Rooms3dCube* zone = cursor->zone();
    m_zones_control.setCurrentZone(zone);
}

void Mapper::redrawPositionByRoom(const Room *room)
{
    updateZonesList();
    MapCursorColor color = RCC_NONE;
    MapCursor current = m_view.getCurrentPosition();
    m_view.clearSelection();
    if (current->valid() && current->room(current->pos()) == room) 
        color = RCC_NORMAL;
    MapTools tools(&m_map);
    Room *r = tools.findRoom(room->hash());   
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
    updateProps();
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
    if (current->valid() && current->pos().zid == zone)
    {
        return redrawPosition(current, false);
    }
    MapTools t(&m_map);
    Rooms3dCube *ptr = m_map.findZone(zone);
    MapCursor cursor = t.createZoneCursor(ptr);
    redrawPosition(cursor, false);
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
