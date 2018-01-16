#include "stdafx.h"
#include "mapInstance.h"
#include "roomHasher.h"

MapInstance::MapInstance() : m_nextzone_id(1)
{
}

MapInstance::~MapInstance()
{
    clear();
}

Rooms3dCube* MapInstance::createNewZone()
{
    return createNewZone ( getNewZoneName(L"") );
}

Rooms3dCube* MapInstance::createNewZone(const tstring& name)
{
    int newzoneid = zones.size() + 1;
    Rooms3dCube* new_zone = new Rooms3dCube(newzoneid, name);
    zones.push_back(new_zone);
    return new_zone;
}

tstring MapInstance::getNewZoneName(const tstring& templ)
{
    tstring zone_name(templ);
    if (zone_name.empty())
    {
        tchar buffer[32];
        while (true)
        {
            swprintf(buffer, L"Новая зона %d", m_nextzone_id++);
            if (!findZone(buffer))
                break;
        }
        zone_name.assign(buffer);
    }
    return zone_name;
}

void MapInstance::deleteZone(const tstring& name)
{
    bool found = false;
    zone_iterator zt = zones.begin(), zt_end = zones.end();
    for(; zt!=zt_end; ++zt) {
        Rooms3dCube* zone = *zt;
        if ( zone->name() == name )
        {
            found = true;
            hashes_iterator ht = hashes.find(zone);
            if (ht != hashes.end()) {
                hashes.erase(ht);
            } else {
                assert(false);
            }
            zones.erase(zt);
            delete zone;
            break;
        }
    }
}
//-------------------------------------------------------------------------------------
Rooms3dCube* MapInstance::findZone(int zid)
{
    zone_iterator zt = zones.begin(), zt_end = zones.end();
    for(; zt!=zt_end; ++zt) {
        Rooms3dCube* zone = *zt;
        if ( zone->id() == zid )
            return zone;
    }
    return NULL;
}

Rooms3dCube* MapInstance::findZone(const tstring& name)
{
    zone_iterator zt = zones.begin(), zt_end = zones.end();
    for(; zt!=zt_end; ++zt) {
        Rooms3dCube* zone = *zt;
        if ( zone->name() == name )
            return zone;
    }
    return NULL;
}
//-------------------------------------------------------------------------------------
void tstring_replace(tstring *str, const tstring& what, const tstring& forr)
{
	size_t pos = 0;
	while ((pos = str->find(what, pos)) != tstring::npos)
	{
		str->replace(pos, what.length(), forr);
		pos += forr.length();
	}
}

tstring i2s(int value) {
    tchar buffer[16];
    _itow(value, buffer, 10);
    return tstring(buffer);
}

bool hex2i(tstring v, int* value) {
    if (v.length() != 2) return false;
    int result = swscanf(v.c_str(), L"%x", value);
    return (result == 1);
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
		Rooms3dCube *zone = zones[i];
		tstring fname(zone->name());
		fname.append(L".map");
		std::vector<tstring>::iterator it = std::find(files.begin(), files.end(), fname);
		if (it != files.end()) {
			files.erase(it);
		}		
		hashes_iterator ht = hashes.find(zone);
		if (ht != hashes.end()) {
			tstring loaded_hash(ht->second);
			Rooms3dCubeHash h(zone);
			if (!loaded_hash.compare(h.getHash()))
				continue;
		}
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
					if (r->use_color) {
                        tchar buffer[10];
                        swprintf(buffer, L"%02x%02x%02x", GetRValue(r->color), GetGValue(r->color), GetBValue(r->color));
						rn.set(L"color", buffer);
                    }
					if (r->icon > 0)
						rn.set(L"icon", r->icon);
					tstring d(r->roomdata.descr);
					tstring_replace(&d, L"\r", L"\\r");
					tstring_replace(&d, L"\n", L"\\n");
                    rn.set(L"desc", d.c_str());
					for (int rd = beginRoomDir; rd <= endRoomDir; ++rd) 
					{
						const RoomExit& e = r->dirs[rd];
						if (!e.exist) continue;
						RoomDirHelper h;
						xml::node en = rn.createsubnode(L"exit");
						en.set(L"dir", h.getDirName(h.cast(rd)));
						if (e.next_room)
							en.set(L"vnum", e.next_room->roomdata.vnum);
						if (e.door || e.multiexit)
                        {
                            tstring extra;
                            if (e.door)
                                extra.append(L"door");
                            if (e.multiexit)
                            {
                                if (e.door)
                                    extra.append(L",");
                                extra.append(L"multiexit");
                            }
                            en.set(L"extra", extra.c_str());
                        }
                    }
				}
			}
		}
        tstring path(dir);
        path.append(fname);
        if ( !s.save( path.c_str()) )
        {
            tstring error(L"Ошибка записи файла с зоной:");
            error.append( path );
			clientlog(error);
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
    struct exitdata {
      Room *room;
      RoomDir dir;
      tstring vnum;
    };

    clear();
    std::vector<exitdata> exitsOnLoad;
    std::vector<Rooms3dCube*> newzones;

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

    int zid = 1;
    for (int i = 0, e = files.size(); i < e; ++i)
    {
        tstring file(files[i]);
        int pos = file.rfind(L".");
        tstring name(file.substr(0, pos));
        tstring filepath(dir);
        filepath.append(file);

        tstring error;
        xml::node zn;
		Rooms3dCube *zone = new Rooms3dCube(zid++, name);
        if (zn.load(filepath.c_str(), &error))
        {
            xml::request rooms(zn, L"room");
            for (int j = 0, je = rooms.size(); j < je; ++j)
            {
                tstring err;
				xml::node r = rooms[j];
				Room *newroom = new Room();
				RoomData &rd = newroom->roomdata;
				if (!r.get(L"vnum", &rd.vnum)) {
					err = L"не задан vnum комнаты (xmlnode index): " + i2s(j);
				}
				int x = 0;  int y = 0; int z = 0;
				if (err.empty() && (!r.get(L"x", &x) || !r.get(L"y", &y) || !r.get(L"z", &z))) {
					err = L"неполные координаты vnum=" + rd.vnum;
				}
                if (err.empty() && !r.get(L"name", &rd.name)) {
                    err = L"не задано имя(name) vnum=" + rd.vnum;
                }
                if (err.empty() && !r.get(L"exits", &rd.exits)) {
                    err = L"не заданы выходы(exits) vnum=" + rd.vnum;
                }

                tstring desc;
                if (err.empty() && !r.get(L"desc", &desc)) {
                    err = L"не задано описание(desc) vnum=" + rd.vnum;
                } else {
                    tstring_replace(&desc, L"\\r", L"\r" );
                    tstring_replace(&desc, L"\\n", L"\n" );
                    rd.descr = desc;
                }
                if (err.empty()) {
                    int icon = 0;
                    if (r.get(L"icon", &icon) && icon > 0 )
                        newroom->icon = icon;
                    tstring color;
                    if (r.get(L"color", &color)) {
                        int r = 0; int g = 0; int b = 0;
                        if (color.length() == 6 &&
                            hex2i(color.substr(0,2), &r) &&
                            hex2i(color.substr(2,2), &g) &&
                            hex2i(color.substr(4,2), &b))
                        {
                            newroom->color = RGB(r,g,b);
                            newroom->use_color =  1;
                        } else {
                              err = L"некорректный цвет vnum=" + rd.vnum;
                        }
                    }
                }
                if (err.empty()) {
                    xml::request exits(r, L"exit");
                    {
                        for (int k=0;k<exits.size();++k) {
                            tstring dir;
                            if (!exits[k].get(L"dir", &dir)) {
                               err = L"не указано направление выхода vnum=" + rd.vnum;
                               break;
                            }
                            RoomDirHelper h;
                            RoomDir roomdir = h.getDirByName(dir.c_str());
                            if (roomdir == RD_UNKNOWN) {
                                err = L"неизвестное направление выхода (" + dir + L") vnum=" + rd.vnum;
                                break;
                            }
                            RoomExit &re = newroom->dirs[h.index(roomdir)];
                            re.exist = true;
                            tstring vnum;
                            if (exits[k].get(L"vnum", &vnum)) {
                                exitdata ed; ed.room = newroom; ed.dir = roomdir; ed.vnum = vnum;
                                exitsOnLoad.push_back(ed);
                            }
                            tstring extra;
                            if (exits[k].get(L"extra", &extra)) {
                                if (extra.find(L"door") != tstring::npos)
                                    re.door = true;
                                if (extra.find(L"multiexit") != tstring::npos)
                                    re.multiexit = true;
                            }
                        }
                    }
                }
                if (err.empty()) {
                    Rooms3dCubePos pos;
                    pos.x = x; pos.y = y; pos.z = z;
                    Rooms3dCube::AR_STATUS s = zone->addRoom(pos, newroom);
                    if (s != Rooms3dCube::AR_OK ) {
                        err = L"ошибка загрузки комнаты в зону vnum=" + rd.vnum;
                    }
                }
                if (!err.empty()) {
                    exitsOnLoad.erase(std::remove_if(exitsOnLoad.begin(), exitsOnLoad.end(),
                        [&](exitdata &ed) { return (ed.room == newroom); }), exitsOnLoad.end());
                    delete newroom;
                    clientlog(err + L" в файле: " + filepath);
                } else {
                }
            }
        }
        zn.deletenode();
        newzones.push_back(zone);
    }

    RoomDirHelper h;
    std::vector<exitdata>::iterator it = exitsOnLoad.begin(), it_end = exitsOnLoad.end();
    for (; it != it_end; ++it) {
        Room *destroom = NULL;
        for (Rooms3dCube *z : newzones) {
           Room *r = z->findRoom(it->vnum);
           if (r) { destroom = r; break; }
        }
        if (!destroom) {
            assert(false);
            continue;
        }
        int index = h.index(it->dir);
        it->room->dirs[index].next_room = destroom;
    }

    std::vector<Rooms3dCube*>::iterator zt = newzones.begin(), zt_end = newzones.end();
    for (; zt != zt_end; ++zt) {
		Rooms3dCube* newzone = *zt;
        Rooms3dCubeHash h(newzone);
		hashes[newzone] = h.getHash();
        zones.push_back(newzone);
    }
}

void MapInstance::clearMaps()
{
    clear();
}

void MapInstance::clear()
{
    std::for_each( zones.begin(),zones.end(),[](Rooms3dCube* p){ delete p; } );
    zones.clear();
	hashes.clear();
}

void MapInstance::getZones(std::vector<Rooms3dCube*> *zones_list)
{
    zone_iterator it = zones.begin(), it_end = zones.end();
	for (; it != it_end; ++it) {
		zones_list->push_back(*it);
	}
}
