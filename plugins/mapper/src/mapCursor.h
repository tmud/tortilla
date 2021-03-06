﻿#pragma once

enum MapCursorColor { RCC_NONE = 0, RCC_NORMAL, RCC_LOST };
class MapCursorInterface;
typedef std::shared_ptr<MapCursorInterface> MapCursor;

class MapCursorInterface
{
public:
    virtual ~MapCursorInterface() {}
    virtual const Rooms3dCubeSize& size() const = 0;
    virtual const Rooms3dCubePos& pos() const = 0;
    virtual const Room* room(const Rooms3dCubePos& p) const = 0;
    virtual MapCursorColor color() const = 0;
    virtual const Rooms3dCube* zone() const = 0;
    virtual bool valid() const = 0;
    virtual MapCursor move(RoomDir dir) = 0;
};

class MapInstance;
class MapCursorImplementation : public MapCursorInterface
{
public:
    MapCursorImplementation(MapInstance* m, Room *r, MapCursorColor c) : map_ptr(m), room_ptr(r), ccolor(c)
    {
        assert(m);
        init();
    }
    ~MapCursorImplementation() {}
protected:
    const Rooms3dCubeSize& size() const;
    const Rooms3dCubePos& pos() const;
    const Room* room(const Rooms3dCubePos& p) const;
    MapCursorColor color() const;
    const Rooms3dCube* zone() const;
    bool valid() const;
    MapCursor move(RoomDir dir);
private:
    void init();
    MapInstance *map_ptr;
    Room *room_ptr;
    MapCursorColor ccolor;
    Rooms3dCube* map_zone;
    static Rooms3dCubeSize m_empty;
    static Rooms3dCubePos m_empty_pos;
};

class MapZoneCursorImplementation : public MapCursorInterface
{
public:
    MapZoneCursorImplementation(MapInstance* m, const Rooms3dCube* zone, int level) : map_ptr(m), map_zone(NULL)
    {
        assert(m);
        init(zone, level);
    }
    ~MapZoneCursorImplementation() {}
protected:
    const Rooms3dCubeSize& size() const;
    const Rooms3dCubePos& pos() const;
    const Room* room(const Rooms3dCubePos& p) const;
    MapCursorColor color() const;
    const Rooms3dCube* zone() const;
    bool valid() const;
    MapCursor move(RoomDir dir);
private:
    void init(const Rooms3dCube* zone, int level);
    MapInstance *map_ptr;
    const Rooms3dCube* map_zone;
    static Rooms3dCubeSize m_empty;
    Rooms3dCubePos m_zone_pos;
};

class MapNullCursorImplementation : public MapCursorInterface 
{
public:
    MapNullCursorImplementation() {}
    const Rooms3dCubeSize& size() const { return m_null_size; }
    const Rooms3dCubePos& pos() const { return m_null_pos; }
    const Room* room(const Rooms3dCubePos& p) const { return NULL; }
    MapCursorColor color() const { return RCC_NONE; }
    const Rooms3dCube* zone() const { return NULL; }
    bool valid() const { return false; }
    MapCursor move(RoomDir dir) { return std::make_shared<MapNullCursorImplementation>(); }
private:
    Rooms3dCubePos m_null_pos;
    Rooms3dCubeSize m_null_size;
};
