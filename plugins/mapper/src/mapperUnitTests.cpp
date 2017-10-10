#include "stdafx.h"
#include "mapperUnitTests.h"
#include "roomObjects.h"

#ifdef DEBUG
void MapperUnitTests::runTests()
{
    testMap3dCubeCollapse();
}

void MapperUnitTests::testMap3dCubeCollapse()
{
    Rooms3dCube zone(1, L"testzone");
    const Rooms3dCubeSize &sz =  zone.size();
    assert (sz.width() == 1 && sz.height() == 1 && sz.levels() == 1);
    assert ( zone.testInvariant() );

    Rooms3dCubePos p;
    p.x = p.y = p.z = 0;
    zone.addRoom(p, new Room);
    const Rooms3dCubeSize &sz2 = zone.size();
    assert (sz2.width() == 1 && sz2.height() == 1 && sz2.levels() == 1);
    assert ( zone.testInvariant() );

    p.x = 1;
    zone.addRoom(p, new Room);
    p.y = 3;
    zone.addRoom(p, new Room);
    const Rooms3dCubeSize &sz3 = zone.size();
    assert (sz3.width() == 2 && sz3.height() == 4 && sz3.levels() == 1);
    assert ( zone.testInvariant() );

    p.z = 2;
    zone.addRoom(p, new Room);
    p.z = -1;
    zone.addRoom(p, new Room);
    const Rooms3dCubeSize &sz4 = zone.size();
    assert (sz4.width() == 2 && sz4.height() == 4 && sz4.levels() == 4);
    assert ( zone.testInvariant() );

    p.y = -3;
    p.x = -2;
    zone.addRoom(p, new Room);
    const Rooms3dCubeSize &sz5 = zone.size();
    assert (sz5.width() == 4 && sz5.height() == 7 && sz5.levels() == 4);
    assert ( zone.testInvariant() );

    p.z = -1;
    p.x = 1;
    p.y = 3;
    zone.deleteRoom(p);
    const Rooms3dCubeSize &sz6 = zone.size();
    assert (sz6.width() == 4 && sz6.height() == 7 && sz6.levels() == 4);
    assert ( zone.testInvariant() );

}
#endif