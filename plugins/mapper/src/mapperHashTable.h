#pragma once

struct Room;
struct RoomData;
class MapperHashTable
{
public:
    MapperHashTable();
    ~MapperHashTable();
    void addRoom(Room* room);
    void deleteRoom(Room* room);
    void findRooms(const RoomData& room, std::vector<Room*> *vr);

private:
    struct hash_element
    {
        hash_element() : room(NULL), next(NULL) {}
        void destroy()
        {
            hash_element *p = next;
            while (p) {
                p = p->next; delete next; next = p;
            }
        }
        void add(Room* new_room)
        {
            hash_element *h = new hash_element;
            h->room = new_room;
            if (!next)
                { next = h; return;  }
            hash_element *p = next;
            while (p->next)
                p = p->next;
            p->next = h;
        }
        bool del(Room *exist_room)
        {
            if (room == exist_room)
            {
                if (!next)
                    return true;
                hash_element *tmp = next;
                room = next->room;
                next = next->next;
                delete tmp;
                return false;
            }

            hash_element *p = this;
            while (p->next)
            {
                if (p->next->room == exist_room)
                {
                    hash_element *tmp = p->next;
                    p->next = tmp->next;
                    delete tmp;
                    return false;
                }
                p = p->next;
            }
            return false;
        }
        Room *room;
        hash_element *next;
    };
    std::map<uint, hash_element> rooms;
    typedef std::map<uint, hash_element>::iterator iterator;
};
