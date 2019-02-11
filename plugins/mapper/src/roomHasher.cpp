#include "stdafx.h"
#include "roomHasher.h"
#include "md5.h"

class MD5
{
public:
	void add(const tstring& str) 
	{
		TW2U s(str.c_str());
		crc.update(s);
	}
	void add(int val)
	{
		crc.update(&val, sizeof(int));
	}
	void add(COLORREF val)
	{
		crc.update(&val, sizeof(COLORREF));
	}
	void add(bool val)
	{
		int f = (val) ? 1 : 0;
		add(f);
	}
	tstring getCRC() {
		std::string crc(crc.digest().hex_str_value());
		TA2W c(crc.c_str());
		return tstring(c);
	}
private:
	md5 crc;
};

Rooms3dCubeHash::Rooms3dCubeHash(const Rooms3dCube* c)
{
	const Rooms3dCubeSize &sz = c->size();
	MD5 m;
	m.add(sz.left);
	m.add(sz.right);
	m.add(sz.top);
	m.add(sz.bottom);
	m.add(sz.minlevel);
	m.add(sz.maxlevel);
	m.add(c->name());
	Rooms3dCubePos p;
	for (int z = sz.minlevel; z <= sz.maxlevel; ++z) {
		p.z = z;
		for (int y = sz.top; y <= sz.bottom; ++y) {
			p.y = y;
			for (int x = sz.left; x <= sz.right; ++x) {
				p.x = x;
				const Room* r = c->getRoom(p);
				if (!r) continue;
				const RoomData &rd = r->roomdata;
				m.add(rd.roomname);
				m.add(rd.vnum);
				//m.add(rd.exits);
				//m.add(rd.descr);
				m.add(r->icon);
				m.add(r->use_color);
				m.add(r->color);
				for (int d = beginRoomDir; d <= endRoomDir; ++d) {
					const RoomExit &e = r->dirs[d];
					if (!e.exist) continue;
					m.add(e.door);
					m.add(e.multiexit);
					if (e.next_room) {
						const Room* next = e.next_room;
						m.add(next->roomdata.vnum);
					}
				}
			}			
		}		
	}
	m_hash = m.getCRC();
}
