#pragma once

enum MapCursorColor { RCC_NONE = 0, RCC_NORMAL, RCC_LOST };
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
    virtual MapCursorInterface* dublicate() = 0;
    virtual bool move(RoomDir dir) = 0;
};

typedef std::shared_ptr<MapCursorInterface> MapCursor;
