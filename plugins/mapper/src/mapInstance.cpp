#include "stdafx.h"
#include "mapInstance.h"
#include "roomHasher.h"

MapInstance::MapInstance() : m_nextzone_id(1)
{
}

MapInstance::~MapInstance()
{
    std::for_each( zones.begin(),zones.end(),[](zonedata &p){delete p.zone;} );
    rooms_hash_table.clear();
}

MapCursor MapInstance::createCursor(Room *room, MapCursorColor color)
{
    return std::make_shared<MapCursorImplementation>(this, room, color);
}

MapCursor MapInstance::createZoneCursor(int zoneid)
{
    return std::make_shared<MapZoneCursorImplementation>(this, zoneid);
}

Room* MapInstance::findRoom(const tstring& hash)
{
    assert(!hash.empty());
    room_iterator rt = rooms_hash_table.find(hash);
    return (rt != rooms_hash_table.end()) ? rt->second : NULL;
}

Rooms3dCube* MapInstance::getZone(const Room *room)
{
    assert(room);
    const Rooms3dCubePos &pos = room->pos;
    Rooms3dCube* zone = findZone(pos.zid);
    if (!zone) {
        assert(false);
        return NULL;
    }
    assert( zone->getRoom(pos) == room );
    return zone;
}

bool MapInstance::addNewZoneAndRoom(const tstring& name, Room *room)
{
    if (!room || room->roomdata.vnum.empty() || findRoom(room->roomdata.vnum))
    {
        assert(false);
        return false;
    }
    Rooms3dCube* new_zone = new Rooms3dCube(zones.size(), getNewZoneName(name));
    Rooms3dCubePos p;
    new_zone->addRoom(p, room);
	zones.push_back(zonedata(new_zone));
    addRoomToHashTable(room);
    return true;
}

bool MapInstance::addNewRoom(Room* from, Room* newroom, RoomDir dir)
{
    if (!from || !newroom || dir == RD_UNKNOWN || newroom->roomdata.vnum.empty())
    {
        assert(false);
        return false;
    }

    if (isMultiExit(from, dir))
    {
        bool result = setRoomOnMap(from, newroom, dir);
        if (result)
            addRoomToHashTable(newroom);
        return result;
    }

    Room* next = getRoom(from, dir);
    if (!next)
    {
        bool result = setRoomOnMap(from, newroom, dir);
        if (result)
        {
            addLink(from, newroom, dir);
            addRoomToHashTable(newroom);
        }
        return result;
    }

    // ����������� �����������
    setMultiExit(from, dir);
    bool result = setRoomOnMap(from, newroom, dir);
    if (result)
        addRoomToHashTable(newroom);
    return result;
}

bool MapInstance::addLink(Room* from, Room* to, RoomDir dir)
{
    if (!from || !to || dir == RD_UNKNOWN) {
        assert(false); return false;
    }
    Room* current_next = from->dirs[dir].next_room;
    if (current_next && current_next != to) {
        assert(false); return false;
    }
    from->dirs[dir].next_room = to;
    return true;
}

bool MapInstance::migrateRoomsNewZone(const tstring& name, std::vector<Room*>& rooms)
{
    if (rooms.empty()) {
        assert(false);
        return false;
    }
    //migrate rooms
    Rooms3dCube* new_zone = new Rooms3dCube(zones.size(), getNewZoneName(name));
    for (Room *r : rooms) {
        Rooms3dCubePos p (r->pos);
        Rooms3dCube *z = getZone(r);
        if (z->detachRoom(p))
            new_zone->addRoom(p, r);
    }
    zones.push_back(new_zone);
    return true;
}

bool MapInstance::setRoomOnMap(Room* from,  Room* next, RoomDir dir)
{
    assert(from && next && dir != RD_UNKNOWN);
    Rooms3dCubePos pos = from->pos;
    if (!pos.move(dir))
        return false;
	Rooms3dCube* zone = zones[pos.zid].zone;
    Rooms3dCube::AR_STATUS s = zone->addRoom(pos, next);
    if (s == Rooms3dCube::AR_OK)
        return true;
    if (s == Rooms3dCube::AR_INVALIDROOM || s == Rooms3dCube::AR_FAIL) {
		assert(false);
        return false;
    }
     if (s != Rooms3dCube::AR_BUSY)
         return false;
    // create new zone
    if (!addNewZoneAndRoom(L"", next)) {
        return false;
    }
    return true;
}

void MapInstance::addRoomToHashTable(Room* r)
{
    const tstring& vnum = r->roomdata.vnum;
    assert(r && !vnum.empty());
    room_iterator it = rooms_hash_table.find(vnum);
    if (it != rooms_hash_table.end()) {
        return;
    }
    rooms_hash_table[vnum] = r;
}

void MapInstance::removeRoomFromHashTable(Room *r)
{
    const tstring& vnum = r->roomdata.vnum;
    assert(r && !vnum.empty());
    room_iterator it = rooms_hash_table.find(vnum);
    if (it == rooms_hash_table.end()) {
        assert(false);
        return;
    }
    rooms_hash_table.erase(it);
}

tstring MapInstance::getNewZoneName(const tstring& templ)
{
    tstring zone_name(templ);
    if (zone_name.empty())
    {
        tchar buffer[32];
        while (true)
        {
            swprintf(buffer, L"����� ���� %d", m_nextzone_id++);
            if (!findZone(buffer))
                break;
        }
        zone_name.assign(buffer);
    }
    return zone_name;
}
//-------------------------------------------------------------------------------------
Rooms3dCube* MapInstance::findZone(int zid)
{
    zone_iterator zt = zones.begin(), zt_end = zones.end();
    for(; zt!=zt_end; ++zt) {
        Rooms3dCube* zone = zt->zone;
        if ( zone->id() == zid )
            return zone;
    }
    return NULL;
}

Rooms3dCube* MapInstance::findZone(const tstring& name)
{
    zone_iterator zt = zones.begin(), zt_end = zones.end();
    for(; zt!=zt_end; ++zt) {
        Rooms3dCube* zone = zt->zone;
        if ( zone->name() == name )
            return zone;
    }
    return NULL;
}

bool MapInstance::isMultiExit(Room* from, RoomDir dir)
{
    if (!from || dir == RD_UNKNOWN) {
        assert(false);
		return false;
    }
    return from->dirs[dir].multiexit;
}
bool MapInstance::setMultiExit(Room* from, RoomDir dir)
{
    if (!from || dir == RD_UNKNOWN) {
        assert(false);
		return false;
    }
    from->dirs[dir].multiexit = true;
    from->dirs[dir].next_room = NULL;
    return true;
}

Room* MapInstance::getRoom(Room* from, RoomDir dir)
{
    if (!from || dir == RD_UNKNOWN) {
        assert(false);
        return NULL;
    }
    return from->dirs[dir].next_room;
}


//-------------------------------------------------------------------------------------
/*void Mapper::newZone(Room *room, RoomDir dir)
{   
    RoomHelper rh(room);
    if (rh.isCycled())
    {
        MessageBox(L"���������� ������� ����� ���� ��-�� �����!", L"������", MB_OK | MB_ICONERROR);
        return;
    }

    std::vector<Room*> rooms;
    rh.getSubZone(dir, &rooms);
    if (rooms.empty())
    {
        MessageBox(L"��� ������ ��� �������� ����� ����!", L"������", MB_OK | MB_ICONERROR);
        return;
    }

    int min_x = -1, max_x = 0, min_y = -1, max_y = 0, min_z = -1, max_z = 0;
    for (int i = 0, e = rooms.size(); i < e; ++i)
    {
        Room *r = rooms[i];
        if (min_x == -1 || min_x > r->x) min_x = r->x;
        if (max_x < r->x) max_x = r->x;
        if (min_y == -1 || min_y > r->y) min_y = r->y;
        if (max_y < r->y) max_y = r->y;
        int z = r->level->getLevel();
        if (min_z == -1 || min_z > z) min_z = z;
        if (max_z < z) max_z = z;
    }
    
    Zone *new_zone = addNewZone();
    int levels = max_z - min_z + 1;
    for (int i = 0; i < levels; ++i)
       new_zone->getLevel(i, true);       
    new_zone->resizeLevels(max_x - min_x, max_y - min_y);
    
    for (int i = 0, e = rooms.size(); i < e; ++i)
    {
        Room *r = rooms[i];
        int x = r->x - min_x;
        int y = r->y - min_y;
        int z = r->level->getLevel() - min_z;            
        r->level->detachRoom(r->x, r->y);
        RoomsLevel *level = new_zone->getLevel(z, false);            
        level->addRoom(r, x, y);
    }

    m_zones_control.addNewZone(new_zone);
    m_view.Invalidate();
}*/

void tstring_replace(tstring *str, const tstring& what, const tstring& forr)
{
	size_t pos = 0;
	while ((pos = str->find(what, pos)) != tstring::npos)
	{
		str->replace(pos, what.length(), forr);
		pos += forr.length();
	}
}

void MapInstance::saveMaps(const tstring& dir)
{
	std::vector<tstring> files;
	tstring path(dir); path.append(L"*.map");
	WIN32_FIND_DATA fd;
	memset(&fd, 0, sizeof(WIN32_FIND_DATA));
	HANDLE file = FindFirstFile(path.c_str(), &fd);
	if (file != INVALID_HANDLE_VALUE)
	{
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(fd.cFileName);
		} while (::FindNextFile(file, &fd));
		::FindClose(file);
	}

	for (int i = 0, e = zones.size(); i < e; ++i)
	{
		zonedata z = zones[i];
		tstring fname(z.zone->name());
		fname.append(L".map");
		std::vector<tstring>::iterator it = std::find(files.begin(), files.end(), fname);
		if (it != files.end()) {
			files.erase(it);
		}
		Rooms3dCube *zone = z.zone;
		Rooms3dCubeHash h(zone);
		if (!z.hash.compare(h.getHash()))
			continue;
		const Rooms3dCubeSize &sz = zone->size();
		xml::node s(L"zone");		
		//s.set(L"name", zone->name().c_str());
		Rooms3dCubePos p;
		for (int z = sz.minlevel; z <= sz.maxlevel; ++z)  
		{
			p.z = z;
			for (int y = sz.top; y <= sz.bottom; ++y) 
			{
				p.y = y;
				for (int x = sz.left; x <= sz.right; ++x)
				{
					p.x = x;
					const Room* r = zone->getRoom(p);
					if (!r) continue;
					xml::node rn = s.createsubnode(L"room");
					rn.set(L"x", x); rn.set(L"y", y); rn.set(L"z", z);
					rn.set(L"name", r->roomdata.name);
					rn.set(L"vnum", r->roomdata.vnum);
					rn.set(L"exits", r->roomdata.exits);
					if (r->use_color)
						rn.set(L"color", r->color);
					if (r->icon > 0)
						rn.set(L"icon", r->icon);
					tstring d(r->roomdata.descr);
					tstring_replace(&d, L"\r", L"");
					tstring_replace(&d, L"\n", L"\\n");
					xml::node descr = rn.createsubnode(L"descr");
					descr.settext(d.c_str());
					xml::node exits = rn.createsubnode(L"exits");
					for (int rd = beginRoomDir; rd < endRoomDir; ++rd) 
					{
						const RoomExit& e = r->dirs[rd];
						if (!e.exist) continue;
						RoomDirHelper h;
						xml::node en = exits.createsubnode(L"exit");
						en.set(L"dir", h.getDirName(h.cast(rd)));
						if (e.next_room)
							en.set(L"vnum", e.next_room->roomdata.vnum);
						if (e.door)
							en.set(L"door", 1);
						if (e.multiexit)
							en.set(L"multi", 1);						
					}
				}
			}
		}
        tstring path(dir);
        path.append(fname);
        if ( !s.save( path.c_str()) )
        {
            tstring error(L"������ ������ ����� � �����:");
            error.append( path );
			log(error);
        }
        s.deletenode();
    }

    for (int j = 0, je = files.size(); j < je; ++j)
    {
        // delete old zone files
		tstring path(dir);
		path.append(files[j]);
        DeleteFile(path.c_str());
    }
}

void MapInstance::loadMaps(const tstring& dir)
{
	std::vector<tstring> files;
    tstring mask(dir);
    mask.append(L"*.map");    
    WIN32_FIND_DATA fd;
    memset(&fd, 0, sizeof(WIN32_FIND_DATA));
    HANDLE file = FindFirstFile(mask.c_str(), &fd);
    if (file != INVALID_HANDLE_VALUE)
    {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(fd.cFileName);
        } while (::FindNextFile(file, &fd));
        ::FindClose(file);
    }
    
    for (int i = 0, e = files.size(); i < e; ++i)
    {
        tstring file(files[i]);
        int pos = file.rfind(L".");
        tstring name(file.substr(0, pos));
        tstring filepath(dir);
        filepath.append(file);

		int zid = 1;
        tstring error;
        xml::node zn;
        bool loaded = false;
		Rooms3dCube *zone = new Rooms3dCube(zid, name);
        if (zn.load(filepath.c_str(), &error))
        {
            loaded = true;
            xml::request rooms(zn, L"room");
            for (int j = 0, je = rooms.size(); j < je; ++j)
            {
				xml::node r = rooms[i];
				Room *newroom = new Room();
				RoomData &rd = newroom->roomdata;
				if (!r.get(L"vnum", &rd.vnum) {
					log("unknown room vnum at order: "); //todo
					continue;
				}
				int x = 0;  int y = 0; int z = 0;
				if (!r.get(L"x", &x) || !r.get(L"y", &y) || !r.get(L"z", &z)) {
					log("invalid coord for room with vnum=" + rd.vnum );
					continue;
				}



                int z = 0;
                if (!levels[j].get(L"z", &z))
                    { loaded = false;  break; }
                RoomsLevel *level = zone->getLevel(z, true);
                xml::request rooms(levels[j], L"room");
                for (int r = 0, re = rooms.size(); r < re; ++r)
                {
                    RoomData rdata;
                    xml::node room = rooms[r];
                    int x = 0, y = 0;
                    if (!room.get(L"x", &x) || !room.get(L"y", &y) || 
                        !room.get(L"name", &rdata.name) || !room.get(L"descr", &rdata.descr) || !room.get(L"exits", &rdata.exits)
                       ) { continue; }                    
                    rdata.calcHash();
                    Room *new_room = createNewRoom(rdata);
                    int icon = 0;
                    if (room.get(L"icon", &icon) && icon > 0)
                        new_room->icon = icon;
                    tstring color;
                    if (room.get(L"color", &color))
                    {
                        wchar_t *p = NULL;
                        COLORREF n = wcstol(color.c_str(), &p, 16);
                        if (*p == 0)
                            { new_room->color = n; new_room->use_color = 1; }
                    }
                    if (!level->addRoom(new_room, x, y))                    
                        { delete new_room; loaded = false; break; }
                }
                if (!loaded) break;
            }
                        
            // load exits (after loading all rooms)
            if (loaded){
            m_zones.push_back(zone);
            m_zones_control.addNewZone(zone);
            for (int j = 0, je = levels.size(); j < je; ++j)
            {
                int z = 0;
                levels[j].get(L"z", &z);
                RoomsLevel *level = zone->getLevel(z, false);
                xml::request rooms(levels[j], L"room");
                for (int r = 0, re = rooms.size(); r < re; ++r)
                {
                    int x = 0, y = 0;
                    if (!rooms[r].get(L"x", &x) || !rooms[r].get(L"y", &y))
                        continue;
                    Room* room = level->getRoom(x, y);
                    xml::request exits(rooms[r], L"exit");
                    for (int e = 0, ee = exits.size(); e < ee; ++e)
                    {
                        xml::node exitnode = exits[e];
                        tstring exit_dir;
                        exitnode.get(L"dir", &exit_dir);
                        int dir = -1;
                        for (int d = RD_NORTH; d <= RD_DOWN; ++d) { if (!exit_dir.compare(RoomDirName[d])) { dir = d; break; }}
                        if (dir == -1)
                            continue;
                        RoomExit &exit = room->dirs[dir];
                        exit.exist = true;
                        int d = 0; if (exitnode.get(L"door", &d) && d == 1) exit.door = true;

                        if (!exitnode.get(L"x", &x) || !exitnode.get(L"y", &y) || !exitnode.get(L"z", &z))
                            continue;
                        RoomsLevel *next_level = NULL;
                        tstring zone_name;
                        if (exitnode.get(L"zone", &zone_name))
                        {
                            if (zone_name.empty()) continue;
                            int index = -1;
                            ZoneParams zp;
                            for (int zi = 0, ze = m_zones.size(); zi < ze; ++zi)
                            {
                                m_zones[zi]->getParams(&zp);
                                if (zp.name == zone_name) { index = zi; break; }
                            }
                            if (index == -1) continue;
                            Zone *new_zone = m_zones[index];
                            next_level = new_zone->getLevel(z, false);
                        }
                        else {
                            next_level = zone->getLevel(z, false);
                        }
                        if (!next_level) continue;
                        exit.next_room = next_level->getRoom(x, y);
                    }
                }
            }}
        } else {
            base::log(L, error.c_str());
        }

        zn.deletenode();
        if (!loaded)
        {
            delete zone;
            tstring error(L"������ �������� ����:");
            error.append(filepath);
            base::log(L, error.c_str());
            continue;
        }             
    }

    m_viewpos.reset();
    if (!zones.empty())
    {
        Zone *zone = m_zones[0];
        m_viewpos.level = zone->getDefaultLevel();
    }
    redrawPosition();*/
}
