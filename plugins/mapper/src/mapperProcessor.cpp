#include "stdafx.h"
#include "mapperProcessor.h"

void KeyPair::init(const tstring &b, const tstring& e)
{
    initkey(b, &begin);
    initkey(e, &end);
}

void KeyPair::initkey(const tstring& src, u8string* res)
{
    tstring keydata;
    const WCHAR *p = src.c_str();
    bool spec_sym = false;
    for (; *p; ++p)
    {
        if (*p == L'\\')
        {
            if (!spec_sym)
            {
                spec_sym = true;
                continue;
            }
            spec_sym = false;
        }
        if (!spec_sym)
        {
            keydata.append(p, 1);
            continue;
        }

        WCHAR s[2] = { *p, 0 };
        switch (*p) {
        case '$':
            s[0] = 0x1b;
            break;
        case 'n':
            s[0] = 0xa;
            break;
        case 'r':
            s[0] = 0xd;
            break;
        case 's':
            s[0] = 0x20;
            break;
        }
        keydata.append(s);
        spec_sym = false;
    }
    res->assign(TW2U(keydata.c_str()));
}

bool KeyPair::get(StreamTrigger &t, kpmode mode, tstring *result)
{
    int b = 0;
    if (mode != KeyPair::BEGIN)
    {
        b = t.find(0, begin.c_str());
        if (b == -1) return false;
        b = b + begin.length();
    }
    int e = t.datalen();
    if (mode != KeyPair::END)
    {
        e = t.find(b, end.c_str());
        if (e == -1) return false;
    }  
    // b - start pos, e - end pos.
    result->assign(TU2W(t.get(b, e-b)));
    return true;
}

MapperProcessor::MapperProcessor(PropertiesMapper *props) : 
m_propsData(props), m_pActions(NULL), m_lastDir(RD_UNKNOWN), m_pCurrentRoom(NULL)
{
    assert(m_propsData);
}

MapperProcessor::~MapperProcessor()
{
    auto_delete<Zone>(m_zones);
}

void MapperProcessor::setCallback(MapperActions* actions)
{
    m_pActions = actions;
}

void MapperProcessor::updateProps()
{
    PropertiesMapper *p = m_propsData;
    kp_name.init(p->begin_name, p->end_name);
    kp_key.init(p->begin_key, p->end_key);
    kp_descr.init(p->begin_descr, p->end_descr);
    kp_exits.init(p->begin_exits, p->end_exits);
    kp_prompt.init(p->begin_prompt, p->end_prompt);
    m_parser.create(kp_name.begin.c_str(), kp_exits.end.c_str(), 2048);
}

void MapperProcessor::processNetworkData(u8string& ndata)
{    
    int result = m_parser.stream(ndata.c_str());
    if (result <= 0)
        return;

    RoomData room;
    bool a = kp_name.get(m_parser, KeyPair::BEGIN, &room.name);
    bool b = (a) ? kp_descr.get(m_parser, KeyPair::ALL, &room.descr) : false;
    bool c = (b) ? kp_key.get(m_parser, KeyPair::ALL, &room.key) : false;
    bool d = (c) ? kp_exits.get(m_parser, KeyPair::END, &room.exits) : false;

    if (d)
    {
        OutputDebugString(room.name.c_str());
        OutputDebugString(L"\r\n");
        OutputDebugString(room.key.c_str());
        OutputDebugString(L"\r\n");
        OutputDebugString(room.exits.c_str());
        OutputDebugString(L"-----------------\r\n");
    }

    /*RoomData room;
    if (!m_parser.processNetworkData(ndata, &room))
    {
        if (m_prompt.processNetworkData(ndata))
            popDir();
        m_key_trimmer.processNetworkData(ndata);
        return;
    }
    popDir();
    processData(room);
    m_key_trimmer.processNetworkData(ndata);*/

}

void MapperProcessor::processCmd(const tstring& cmd)
{
    if (cmd.empty())
        return;
    RoomDir dir = RD_UNKNOWN;
    if (cmd == m_propsData->north_cmd)
        dir = RD_NORTH;
    else if (cmd == m_propsData->south_cmd)
        dir = RD_SOUTH;
    else if (cmd == m_propsData->west_cmd)
        dir = RD_WEST;
    else if (cmd == m_propsData->east_cmd)
        dir = RD_EAST;
    else if (cmd == m_propsData->up_cmd)
        dir = RD_UP;
    else if (cmd == m_propsData->down_cmd)
        dir = RD_DOWN;
    if (dir != RD_UNKNOWN)
        m_path.push_back(dir);
}

void MapperProcessor::processData(const RoomData& room)
{
    std::vector<Room*> &vr = m_pos_rooms;
    vr.clear();
    m_table.findRooms(room, &vr);
    int count = vr.size();

    // нет ни одной подходящей комнаты 
    if (count == 0)
    {
        Room* new_room = createRoom(room);
        if (m_pCurrentRoom && m_lastDir != RD_UNKNOWN)      // есть местоположение и направление
        {
            RoomCursor cursor(m_pCurrentRoom, m_lastDir);
            if (!cursor.next())
            {
                cursor.setNext(new_room);
                if (setByDir(new_room))
                {
                    setCurrentRoom(new_room);
                    return;
                }
            }
        }

        // неизвестны комната и/или направление или это мультивыход
        Zone* zone = createZone();
        zone->getDefaultLevel()->set(0, 0, new_room);
        setCurrentRoom(new_room);
        return;
    }

    if (count == 1)
    {
        Room* next_room = vr[0];
        if (m_pCurrentRoom && m_lastDir != RD_UNKNOWN) // есть местоположение и направление
        {
            RoomCursor cursor(m_pCurrentRoom, m_lastDir);
            Room *next = cursor.next();
            if (next)
            {
                // даже если это не next_room - переходим по мультивыходу
                setCurrentRoom(next_room);
                return;
            }

            RoomCursor next_cursor(next_room, RoomCursor::revertDir(m_lastDir));
            Room *back = next_cursor.next();
            if (back == m_pCurrentRoom || cursor.isNeighbor(next_room))
            {
                // соединяем коридоры - туда/обратно
                cursor.setNext(next_room);
                next_cursor.setNext(m_pCurrentRoom);
                setCurrentRoom(next_room);
                return;
            }
            if (back)
            {
                // обратный коридор идет в другую комнату
                if (setByDir(next_room))
                    setCurrentRoom(next_room);
                return;
            }

            cursor.setNext(next_room);
            setCurrentRoom(next_room);
            return;
        }

        // неизвестны комната и/или направление
        setCurrentRoom(next_room);
        return;
    }

    /*Room* new_room = (cached) ? findRoomCached(room) : findRoom(room);
    if (!new_room)
    {
    if (m_lastDir == -1)
    {
    m_rpos.reset();
    m_pCurrentRoom = NULL;
    }
    else
    {
    if (m_pCurrentRoom)
    {
    m_rpos.reset();
    m_rpos.current_room = m_pCurrentRoom;
    }
    if (m_rpos.current_room)
    m_rpos.move(m_lastDir);
    }
    }
    else
    {
    if (m_pCurrentRoom)
    {
    RoomExit &e = m_pCurrentRoom->dirs[m_lastDir];
    if (!e.next_room)
    e.next_room = new_room;
    }

    m_rpos.reset();
    m_rpos.current_room = m_pCurrentRoom;
    m_rpos.new_room = new_room;
    }

    Room* croom = m_rpos.new_room ? m_rpos.new_room : m_rpos.current_room;
    m_viewpos.room = croom;
    m_viewpos.level = croom->level;
    redrawPosition();
    m_pCurrentRoom = new_room;*/
}

 // select zone/level to view  //todo save last zone at last session
    /*for (int i = 0, e = m_zones.size(); i < e; ++i)
    {
        Zone *zone = m_zones[i];
        if (zone->isEmpty())
            continue;
        m_pCurrentLevel = zone->getDefaultLevel();
        if (m_pCurrentLevel) break;
    }*/

bool MapperProcessor::setByDir(Room *room)
{
    assert(m_pCurrentRoom && m_lastDir != -1);
    Zone *current_zone = m_pCurrentRoom->level->getZone();
    bool extend = false;
    switch (m_lastDir)
    {
    case RD_NORTH:          
        if (m_pCurrentRoom->y == 0)
            extend = true;
        break;
    case RD_SOUTH:
        if (m_pCurrentRoom->y == current_zone->getHeight()-1)
            extend = true;
        break;
    case RD_WEST:
        if (m_pCurrentRoom->x == 0)
            extend = true;
        break;
    case RD_EAST:
        if (m_pCurrentRoom->x == current_zone->getWidth()-1)
            extend = true;
        break;
    case RD_UP:
    {
        int level_index = m_pCurrentRoom->level->getLevel();
        int max_index = current_zone->getMaxLevel();
        if (level_index == max_index) extend = true;
    }
    break;
    case RD_DOWN:
    {
        int level_index = m_pCurrentRoom->level->getLevel();
        int min_level = current_zone->getMinLevel();
        if (level_index == min_level) extend = true;
    }
    break;
    }    
    if (extend)
        current_zone->extend(m_lastDir, 1);
    
    int x = m_pCurrentRoom->x;
    int y = m_pCurrentRoom->y;
    int z = m_pCurrentRoom->level->getLevel();
    switch (m_lastDir)
    {
    case RD_NORTH: y = y - 1; break;
    case RD_SOUTH: y = y + 1; break;
    case RD_WEST: x = x - 1; break;
    case RD_EAST: x = x + 1; break;
    case RD_UP: z = z + 1; break;
    case RD_DOWN: z = z - 1; break;
    }
    RoomsLevel *level = current_zone->getLevel(z);
    return level->set(x, y, room);
}

Zone* MapperProcessor::createZone()
{
    WCHAR buffer[20];
    for (int i = 1;; ++i)
    {
        swprintf(buffer, L"Новая зона %d", i);
        bool found = false;
        for (int j = 0, e=m_zones.size(); j<e; ++j)
        {
            const tstring& name = m_zones[j]->getName();
            if (!name.compare(buffer)) { found = true; break; }
        }
        if (!found) break;
    }
    Zone *new_zone = new Zone(buffer);
    m_zones.push_back(new_zone);
    if (m_pActions)
        m_pActions->addNewZone(new_zone);
    return new_zone;
}

void MapperProcessor::setCurrentRoom(Room *room)
{
    //m_pLastRoom = m_pCurrentRoom;
    m_pCurrentRoom = room;
    if (m_pActions)
    {
        if (room)
            m_pActions->setCurrentRoom(room);
        else
            m_pActions->lostPosition();
    }
}

Room* MapperProcessor::createRoom(const RoomData& room)
{
    Room *new_room = new Room();
    new_room->roomdata = room;

    // parse new_room->roomdata.exits to room->dirs
    const tstring& e = new_room->roomdata.exits;
    if (e.find(m_propsData->north_exit) != -1)
        new_room->dirs[RD_NORTH].exist = true;
    if (e.find(m_propsData->south_exit) != -1)
        new_room->dirs[RD_SOUTH].exist = true;
    if (e.find(m_propsData->west_exit) != -1)
        new_room->dirs[RD_WEST].exist = true;
    if (e.find(m_propsData->east_exit) != -1)
        new_room->dirs[RD_EAST].exist = true;
    if (e.find(m_propsData->up_exit) != -1)
        new_room->dirs[RD_UP].exist = true;
    if (e.find(m_propsData->down_exit) != -1)
        new_room->dirs[RD_DOWN].exist = true;

    m_table.addRoom(new_room);
    return new_room;
}

void MapperProcessor::deleteRoom(Room* room)
{
    m_table.deleteRoom(room);    
    for (int i=0, e=ROOM_DIRS_COUNT; i<e; ++i)
    {
        RoomCursor cursor(room, (RoomDir)i);
        if (cursor.isSameByRevert())
            cursor.delDirByRevert();
    }
    room->level->set(room->x, room->y, NULL);
    if (m_pCurrentRoom == room)
        m_pCurrentRoom = NULL;
    //if (m_pLastRoom == room)
    //    m_pLastRoom = NULL;
    delete room;
}

void MapperProcessor::popDir()
{
    if (m_path.empty())
        m_lastDir = RD_UNKNOWN;
    else
    {
        m_lastDir = *m_path.begin();
        m_path.pop_front();
    }
}

/*void Mapper::findOrCreateRooms(const RoomData& room, std::vector<Room*> *vr)
{
    assert(vr->empty());
    if (m_lastDir == -1 || !m_pCurrentRoom)
    {
        m_table.findRooms(room, vr);
        if (vr->empty())
        {
            Room *new_room = addNewRoom(room);
            if (new_room)
                vr->push_back(new_room);
        }
        return;
    }

    Room* next = m_pCurrentRoom->dirs[m_lastDir].next_room;
    if (next)
    {
        if (room.dhash && room.equal(next->roomdata))
        {
            vr->push_back(next); return;
        }
        if (!room.dhash && room.similar(next->roomdata))
        {
            vr->push_back(next); return;
        }

        //todo new zone ? rooms conflict, multiexit ?
        Room *new_room = addNewRoom(room);
        if (new_room)
            vr->push_back(new_room);
        return;
    }

    int backDir = revertDir(m_lastDir);

    // check rooms like current and their back directions
    m_table.findRooms(m_pCurrentRoom->roomdata, vr);
    int size = vr->size();
    std::vector<Room*> like_current;
    for (int i = 0; i < size; ++i)
    {
        Room *candidate = vr->at(i);
        if (candidate == m_pCurrentRoom)
            continue;
        Room* next = candidate->dirs[m_lastDir].next_room;
        if (next && next->roomdata.equal(room))
        {
            Room* back = next->dirs[backDir].next_room;
            if (back && back->roomdata.equal(m_pCurrentRoom->roomdata))
                like_current.push_back(candidate);
        }
    }

    if (like_current.empty())
    {
        vr->clear();

        // check neighborhood room by coordinates
        RoomCursor cursor(m_pCurrentRoom);
        Room *next = cursor.move(m_lastDir);
        if (next && !next->dirs[backDir].next_room &&
            next->roomdata.equal(room))
        {
            m_pCurrentRoom->dirs[m_lastDir].next_room = next;
            next->dirs[backDir].next_room = m_pCurrentRoom;
            vr->push_back(next);
            return;
        }

        // create new room
        Room *new_room = addNewRoom(room);
        if (new_room) {
            vr->push_back(new_room);
            m_pCurrentRoom->dirs[m_lastDir].next_room = new_room;
        }
        return;
    }
    vr->swap(like_current);
}
*/
/*Room* Mapper::addNewRoom(const RoomData& room)
{
    if (!m_pCurrentRoom || m_lastDir == -1)
    {
    if (!m_pLastRoom || m_lastDir == -1)  // last known room
    {
    // new zone, level and room
    Zone *new_zone = addNewZone();
    m_zones_control.addNewZone(new_zone);
    RoomsLevel *level = new_zone->getLevelAlways(0);
    Room *new_room = createRoom(room);
    level->set(0, 0, new_room);
    return new_room;
    }
    else
    {
    Room *new_room = createRoom(room);
    if (!m_lastpos.setOffsetRoom(new_room))
    {
    m_table.deleteRoom(new_room);
    delete new_room;
    return NULL;
    }
    return new_room;
    }
    }*/

    /*
    // add another room in last direction
    Room *new_room = createNewRoom(room);
    RoomExit &e = m_pCurrentRoom->dirs[m_lastDir];
    if (e.next_room)
    {
    // exit with different rooms ? -> make new zone
    Zone *new_zone = addNewZone();
    m_zones_control.addNewZone(new_zone);
    RoomsLevel *level = new_zone->getLevel(0, true);
    level->addRoom(new_room, 0, 0);
    return new_room;
    }

    RoomCursor pos; pos.current_room = m_pCurrentRoom;
    pos.move(m_lastDir);
    pos.setOffsetRoom(new_room);
    e.next_room = new_room;
    new_room->dirs[revertDir(m_lastDir)].next_room = m_pCurrentRoom;
    return new_room;
    return NULL;
}*/

/*Zone* Mapper::addNewZone()
{
    ZoneParams zp;
    WCHAR buffer[32];
    for (int i = 1;; ++i)
    {
    swprintf(buffer, L"Новая зона %d", i);
    bool found = false;
    for (int j = 0, e = m_zones.size(); j < e; ++j)
    {
    m_zones[j]->getParams(&zp);
    if (!zp.name.compare(buffer)) { found = true; break; }
    }
    if (!found)
    break;
    }

    Zone *new_zone = new Zone(buffer);
    m_zones.push_back(new_zone);
    return new_zone;
    return NULL;
}*/
//-------------------------------------------------------------------------------------
/*void Mapper::newZone(Room *room, RoomDir dir)
{
RoomHelper rh(room);
if (rh.isCycled())
{
MessageBox(L"Невозможно создать новую зону из-за цикла!", L"Ошибка", MB_OK | MB_ICONERROR);
return;
}

std::vector<Room*> rooms;
rh.getSubZone(dir, &rooms);
if (rooms.empty())
{
MessageBox(L"Нет комнат для создания новой зоны!", L"Ошибка", MB_OK | MB_ICONERROR);
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

void MapperProcessor::saveMaps(lua_State *L)
{
}

void MapperProcessor::loadMaps(lua_State *L)
{
}

void MapperProcessor::selectDefault()
{
}


/*void Mapper::saveMaps(lua_State *L)
{
return; //todo
luaT_run(L, "getPath", "s", "");
tstring dir( convert_utf8_to_wide(lua_tostring(L, -1)) );
lua_pop(L, 1);

std::vector<tstring> todelete;
for (int i = 0, e = m_zones.size(); i < e; ++i)
{
Zone *zone = m_zones[i];
if (!zone->isChanged())
continue;
ZoneParams zp;
zone->getParams(&zp);

xml::node s("zone");
//s.set("width", zone->width());
//s.set("height", zone->height());
//s.set("name", zp.name);
for (int j = zp.minl; j <= zp.maxl; ++j)
{
RoomsLevel *level = zone->getLevel(j, false);
if (level->isEmpty()) continue;
xml::node l = s.createsubnode("level");
l.set("z", j);
for (int x = 0, xe = level->width(); x<xe; ++x) {
for (int y = 0, ye = level->height(); y<ye; ++y) {
Room *room = level->getRoom(x, y);
if (!room) continue;
xml::node r = l.createsubnode("room");
r.set("name", room->roomdata.name);
r.set("exits", room->roomdata.exits);
r.set("x", room->x);
r.set("y", room->y);
if (room->use_color)
{
char buffer[8]; COLORREF c = room->color;
sprintf(buffer, "%.2x%.2x%.2x", GetRValue(c), GetGValue(c), GetBValue(c));
r.set("color", buffer);
}
if (room->icon > 0)
r.set("icon", room->icon);
r.set("descr", room->roomdata.descr);

for (int dir = RD_NORTH; dir <= RD_DOWN; ++dir)
{
RoomExit &exit = room->dirs[dir];
if (!exit.exist)
continue;

xml::node e = r.createsubnode("exit");
e.set("dir", RoomDirName[dir]);

//u8string state;
Room* next_room = exit.next_room;
if (next_room)
{
e.set("x", next_room->x);
e.set("y", next_room->y);
e.set("z", next_room->level->getLevel());
Zone* zone0 = next_room->level->getZone();
if (zone != zone0)
{
ZoneParams zp0;
zone0->getParams(&zp0);
e.set("zone", zp0.name);
}
}
if (exit.door)
e.set("door", 1);
}
}}
}

for (int j = 0, je = todelete.size(); j < je; ++j)
{
if (todelete[j] == zp.name)
{ todelete.erase(todelete.begin() + j); break; }
}
if (zp.name != zp.original_name)
todelete.push_back(zp.original_name);

tstring path(dir);
path.append(zp.name);
path.append(L".map");
if ( !s.save(convert_wide_to_utf8(path.c_str())) )
{
u8string error("Ошибка записи файла с зоной:");
error.append(convert_wide_to_utf8(zp.name.c_str()));
luaT_log(L, error.c_str());
}
s.deletenode();
}

for (int j = 0, je = todelete.size(); j < je; ++j)
{
// delete old zone files
tstring file(dir);
file.append(todelete[j]);
file.append(L".map");
DeleteFile(file.c_str());
}
}

void Mapper::loadMaps(lua_State *L)
{
return; //todo
luaT_run(L, "getPath", "s", "");
tstring dir(convert_utf8_to_wide(lua_tostring(L, -1)));
lua_pop(L, 1);

tstring mask(dir);
mask.append(L"*.map");
std::vector<tstring> zones;
WIN32_FIND_DATA fd;
memset(&fd, 0, sizeof(WIN32_FIND_DATA));
HANDLE file = FindFirstFile(mask.c_str(), &fd);
if (file != INVALID_HANDLE_VALUE)
{
do {
if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
zones.push_back(fd.cFileName);
} while (::FindNextFile(file, &fd));
::FindClose(file);
}

for (int i = 0, e = zones.size(); i < e; ++i)
{
tstring file(zones[i]);
int pos = file.rfind(L".");
tstring name(file.substr(0, pos));
tstring filepath(dir);
filepath.append(file);

u8string fpath(convert_wide_to_utf8(filepath.c_str()) );
xml::node zn;
bool loaded = false;
Zone *zone = new Zone(name);
if (zn.load(fpath.c_str()))
{
loaded = true;
xml::request levels(zn, "level");
for (int j = 0, je = levels.size(); j < je; ++j)
{
int z = 0;
if (!levels[j].get("z", &z))
{ loaded = false;  break; }
RoomsLevel *level = zone->getLevel(z, true);
xml::request rooms(levels[j], "room");
for (int r = 0, re = rooms.size(); r < re; ++r)
{
RoomData rdata;
xml::node room = rooms[r];
int x = 0, y = 0;
if (!room.get("x", &x) || !room.get("y", &y) ||
!room.get("name", &rdata.name) || !room.get("descr", &rdata.descr) || !room.get("exits", &rdata.exits)
) { continue; }
rdata.calcHash();
Room *new_room = createNewRoom(rdata);
int icon = 0;
if (room.get("icon", &icon) && icon > 0)
new_room->icon = icon;
u8string color;
if (room.get("color", &color))
{
char *p = NULL;
COLORREF n = strtol(color.c_str(), &p, 16);
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
levels[j].get("z", &z);
RoomsLevel *level = zone->getLevel(z, false);
xml::request rooms(levels[j], "room");
for (int r = 0, re = rooms.size(); r < re; ++r)
{
int x = 0, y = 0;
if (!rooms[r].get("x", &x) || !rooms[r].get("y", &y))
continue;
Room* room = level->getRoom(x, y);
xml::request exits(rooms[r], "exit");
for (int e = 0, ee = exits.size(); e < ee; ++e)
{
xml::node exitnode = exits[e];
u8string exit_dir;
exitnode.get("dir", &exit_dir);
int dir = -1;
for (int d = RD_NORTH; d <= RD_DOWN; ++d) { if (!exit_dir.compare(RoomDirName[d])) { dir = d; break; }}
if (dir == -1)
continue;
RoomExit &exit = room->dirs[dir];
exit.exist = true;
int d = 0; if (exitnode.get("door", &d) && d == 1) exit.door = true;

if (!exitnode.get("x", &x) || !exitnode.get("y", &y) || !exitnode.get("z", &z))
continue;
RoomsLevel *next_level = NULL;
tstring zone_name;
if (exitnode.get("zone", &zone_name))
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
}

zn.deletenode();
if (!loaded)
{
delete zone;
u8string error("Ошибка загрузки зоны:");
error.append(fpath);
luaT_log(L, error.c_str());
continue;
}
}

m_viewpos.reset();
if (!zones.empty())
{
Zone *zone = m_zones[0];
m_viewpos.level = zone->getDefaultLevel();
}

}
*/