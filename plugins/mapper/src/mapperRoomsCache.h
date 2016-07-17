#pragma once

/*class MapperRoomsCache
{
public:
    MapperRoomsCache() {}
    ~MapperRoomsCache() {}

    void addRoom(Room *room) 
    {
        assert(room);
        m_rooms.push_back(room);
    }

    void deleteRoom(Room *room) 
    {
        assert(room);
        int index = find(room);
        if (index != -1)
          m_rooms.erase(m_rooms.begin() + index);
    }

    bool isExistRoom(Room *room)
    {
        if (!room)
            return false;
        return (find(room) != -1) ? true : false;
    }

private:
    int find(Room *room)
    {
        for (int i=0,e=m_rooms.size(); i<e; ++i) 
        {
           if (m_rooms[i] == room) return i;
        }
        return -1;
    }

private:
    std::vector<Room*> m_rooms;

};*/
