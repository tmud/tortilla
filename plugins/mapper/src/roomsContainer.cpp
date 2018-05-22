#include "stdafx.h"
#include "roomsContainer.h"

Room* Rooms3dContainer::get(const Rooms3dCubePos& p) const
{
    Room** ptr = getp(p);
    if (!ptr) return NULL;
    return *ptr;
}

Room** Rooms3dContainer::getp(const Rooms3dCubePos& p) const
{
    if (!checkCoords(p))
        return NULL;
    int z = p.z - cube_size.minlevel;
    assert(z>=0 && z<(int)zone.size());
    level* l = zone[z];
    int x = p.x - cube_size.left;
    int y = p.y - cube_size.top;
    assert(y>=0 && y<(int)l->rooms.size());
    row* r = l->rooms[y];
    assert(x>=0 && x<(int)r->rr.size());
    return &r->rr[x];
}

void Rooms3dContainer::set(const Rooms3dCubePos& p, Room *room)
{
    if (!checkCoords(p))
    {
        //1. extends rows on all levels (y)
        extends_height(p);
        // 2. extends rows by new rooms (x)
        extends_width(p);
        // 3. extends levels (z)
        extends_levels(p);
    }
    Room** ptr = getp(p);    
    *ptr = room;
}

void Rooms3dContainer::extends_height(const Rooms3dCubePos& p)
{
    //extends rows on all levels (y)
    int count = 0;
    if (p.y < cube_size.top)
        count = cube_size.top - p.y;
    if (p.y > cube_size.bottom)
        count = cube_size.bottom - p.y;
    if (count == 0) return;

    for (int i=0; i<cube_size.levels(); ++i) 
    {
       level *l = zone[i];
       std::vector<row*>& v = l->rooms;
       if (count > 0) {
            for(int c = count; c>0; --c)
                v.insert(v.begin(), new row(deleterooms, cube_size.width()) );
            cube_size.top = p.y;
       } else {
            for(int c = -count; c>0; --c)
               v.push_back( new row(deleterooms, cube_size.width()) );
       }
    }
    if (count > 0)
        cube_size.top = p.y;
    else
        cube_size.bottom = p.y;
}

void Rooms3dContainer::extends_width(const Rooms3dCubePos& p)
{
    // extends rows by new rooms (x)
    int count = 0;
    if (p.x < cube_size.left)
        count = cube_size.left - p.x;
    if (p.x > cube_size.right)
        count = cube_size.right - p.x;
    if (count == 0) return;
    
    for (int i=0; i<cube_size.levels(); ++i)
    {
        level *l = zone[i];
        std::vector<row*>& v = l->rooms;
        for (int j=0, je=v.size(); j<je; ++j)
        {
            std::vector<Room*>& rr = v[j]->rr;
            if (count > 0)
              rr.insert(rr.begin(), count, NULL);              
            else
              rr.insert(rr.end(), -count, NULL);
        }
    }
    if (count > 0)
        cube_size.left = p.x;
    else
        cube_size.right = p.x;
}

void Rooms3dContainer::extends_levels(const Rooms3dCubePos& p)
{
    // extends levels (z)
    int count = 0;
    if (p.z < cube_size.minlevel)
        count = cube_size.minlevel - p.z;
    if (p.z > cube_size.maxlevel)
        count = cube_size.maxlevel - p.z;
    if (count == 0) return;
    bool insert_begin = (count > 0) ? true : false;
    if (count < 0) count = -count;
    for (;count > 0; --count)
    {
        level *l = new level;
        l->rooms.resize(cube_size.height(), NULL);
        for (int i=0;i<cube_size.height(); ++i)
            l->rooms[i] = new row(deleterooms, cube_size.width());
        if (insert_begin)
            zone.insert(zone.begin(), l);
        else
            zone.push_back(l);
    }
    if (insert_begin)
        cube_size.minlevel = p.z;
    else
        cube_size.maxlevel = p.z;
}

bool Rooms3dContainer::emptyLevel(level *l)
{
    for (row *r : l->rooms) {
        for (Room* room : r->rr ) {
            if (room) return false;
        }
    }
    return true;
}

void Rooms3dContainer::collapse()
{
    int levels = zone.size();
    // collapse from top level
    while (levels > 1) {
         level *l = zone[0];
         if (!emptyLevel(l))
             break;
         zone.erase(zone.begin());
         delete l;
         levels = zone.size();
         cube_size.maxlevel -= 1;
    }
    // collapse from bottom level
    while (levels > 1) {
         int bottom = levels-1;
         level *l = zone[bottom];
         if (!emptyLevel(l))
             break;
         zone.erase(zone.begin()+bottom);
         delete l;
         levels = zone.size();
         cube_size.minlevel += 1;
    }
    // collapse from left
    while(true) {
        bool trim_left = true;
        for (level *l : zone) {
        if (!trim_left) break;
        for (row *r : l->rooms) {
            std::vector<Room*> &rr = r->rr;
            if (rr.size() < 2 || rr[0]) { trim_left = false; break; }
        }}
        if (!trim_left)
            break;
        for (level *l : zone) {
        for (row *r : l->rooms) {
            r->rr.erase(r->rr.begin());
        }}
        cube_size.left += 1;
    }
    // collapse from right
    while (true) {
        bool trim_right = true;
        for (level *l : zone) {
        if (!trim_right) break;
        for (row *r : l->rooms) {
            std::vector<Room*> &rr = r->rr;
            int last = rr.size()-1;
            if (last == 0 || rr[last]) { trim_right = false; break; }
        }}
        if (!trim_right)
            break;
        for (level *l : zone) {
        for (row *r : l->rooms) {
            std::vector<Room*> &rr = r->rr;
            int last = rr.size()-1;
            rr.erase(rr.begin()+last);
        }}
        cube_size.right -= 1;
    }

    // collapse from top
    while (true) {
        bool trim_top = true;
        for (level *l : zone) {
            if (l->rooms.size()==1 || !trim_top) { trim_top = false; break; }
            row *r = l->rooms[0];
            for (Room *room : r->rr) {
                if (room) { trim_top = false; break; }
            }
        }
        if (!trim_top)
            break;
        for (level *l : zone) {
             row *r = l->rooms[0];
             l->rooms.erase(l->rooms.begin());
             delete r;
        }
        cube_size.top += 1;
    }

    // collapse from bottom
    while (true) {
        bool trim_bottom = true;
        for (level *l : zone) {
            if (l->rooms.size()==1 || !trim_bottom) { trim_bottom = false; break; }
            int last = l->rooms.size()-1;
            row *r = l->rooms[last];
            for (Room *room : r->rr) {
                if (room) { trim_bottom = false; break; }
            }
        }
        if (!trim_bottom)
            break;
        for (level *l : zone) {
             int last = l->rooms.size()-1;
             row *r = l->rooms[last];
             l->rooms.pop_back();
             delete r;
        }
        cube_size.bottom -= 1;
    }
}


bool Rooms3dContainer::checkCoords(const Rooms3dCubePos& p) const
{
    if (p.z >= cube_size.minlevel && p.z <= cube_size.maxlevel)
    {
        if (p.x >= cube_size.left && p.x <= cube_size.right && p.y >= cube_size.top && p.y <= cube_size.bottom) 
        {
            return true;
        }
    }
    return false;
}

#ifdef _DEBUG
bool Rooms3dContainer::testInvariant()
{
    const Rooms3dCubeSize&  s = getSize();
    int levels_count = zone.size();
    if (levels_count != s.levels())
        return false;
    for (level *l : zone) {
        std::vector<row*>&r = l->rooms;
        int rows = r.size();
        if (rows != s.height())
            return false;        
        for (row *rv : r) {
            int columns = rv->rr.size();
            if (columns != s.width())
                return false;
        }
    }
    return true;
}
#endif

Rooms3dContainerMirror::Rooms3dContainerMirror(const Rooms3dContainer& src) : rooms(false)
{  
    const Rooms3dCubeSize &sz = src.getSize();
    for (int l=sz.minlevel; l<=sz.maxlevel; ++l)
    {
        Rooms3dCubePos p; p.z = l;
        for (int y = sz.top; y<=sz.bottom; ++y)
        {
            p.y = y;
            for (int x = sz.left; x<=sz.right; ++x)
            {
                p.x = x;
                Room *r = src.get(p);
                rooms.set(p, r);
            }        
        }
    }
    rooms.collapse();
    const Rooms3dCubeSize &sz2 =rooms.getSize();
    assert(sz2.left == sz.left && sz.right == sz2.right && sz.top == sz2.top && sz.bottom == sz2.bottom 
        && sz.maxlevel == sz2.maxlevel && sz.minlevel == sz2.minlevel);
}

bool Rooms3dContainerEx::setAsLevel0(int level)
{
    Rooms3dCubeSize &cube_size = rooms.cube_size;
    if (level >= cube_size.minlevel && level <= cube_size.maxlevel) 
    {
        int index = level - cube_size.minlevel;
        cube_size.minlevel = -index;
        cube_size.maxlevel = cube_size.minlevel + rooms.zone.size() - 1;
        correctPositions();
        return true;
    }
    return false;
}

void Rooms3dContainerEx::collapse() 
{ 
     rooms.collapse();           
     correctPositions();
}

void Rooms3dContainerEx::correctPositions() 
{   
    normalizePositions();

    Rooms3dCubeSize &cube_size = rooms.cube_size;
    std::vector<level*> zone = rooms.zone;

    // correct rooms positions
    Rooms3dCubePos c;
    c.z = cube_size.minlevel;     
    for (level *l : zone) {
        c.y = cube_size.top;
        for (row *r : l->rooms){
            c.x = cube_size.left;
            for (Room *room : r->rr) {
                if (room) {
                    int zid = room->pos.zid;
                    room->pos = c;
                    room->pos.zid = zid;
                }
                c.x++;
            }
            c.y++;
        }
        c.z++;
    }
}

void Rooms3dContainerEx::normalizePositions()
{
    Rooms3dCubeSize &cube_size = rooms.cube_size;
    std::vector<level*> zone = rooms.zone;

    // normalize cube size values (select center 0,0,0 if not exist)
     int min = cube_size.minlevel;
     int max = cube_size.maxlevel;

     if (min > 0 || 0 > max)  {
         // select base level
         int index = -1; int rooms_max = 0;
         for (int i=0,e=zone.size(); i<e; ++i) {
            level *l = zone[i];
            int count = countRooms(l);
            if (count > rooms_max) {
                index = i; rooms_max = count;
            }
            if (index == -1) {
                assert(false);
                return;
            }
         }
         cube_size.minlevel = -index;
         cube_size.maxlevel = cube_size.minlevel + (zone.size()-1);
     }
     min = cube_size.left; max = cube_size.right;
     if (min > 0 || 0 > max)  {
         int count = (max - min)+1;
         int center = count / 2;
         cube_size.left = -center;
         cube_size.right = cube_size.left + (count - 1);
     }
     min = cube_size.top; max = cube_size.bottom;
     if (min > 0 || 0 > max)  {
         int count = (max - min)+1;
         int center = count / 2;
         cube_size.top = -center;
         cube_size.bottom = cube_size.top + (count - 1);
     }
}

int Rooms3dContainerEx::countRooms(level *l)
{
    int count = 0;
    for (row *r : l->rooms) {
        for (Room* room : r->rr ) {
            if (room) count++;
        }
    }
    return count;
}
