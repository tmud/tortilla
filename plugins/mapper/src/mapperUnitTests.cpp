#include "stdafx.h"
#include "mapperUnitTests.h"
#include "roomsZone.h"

#ifdef DEBUG
void MapperUnitTests::runTests()
{
    testMap3dCubeCollapse();
}

void MapperUnitTests::testMap3dCubeCollapse()
{
    Rooms3dContainer zone(true);
    const Rooms3dCubeSize &sz =  zone.getSize();
    assert (sz.width() == 1 && sz.height() == 1 && sz.levels() == 1);
    assert ( zone.testInvariant() );

    Rooms3dCubePos p;
    p.x = p.y = p.z = 0;
    zone.set(p, new Room);
    const Rooms3dCubeSize &sz2 = zone.getSize();
    assert (sz2.width() == 1 && sz2.height() == 1 && sz2.levels() == 1);
    assert ( zone.testInvariant() );

    p.x = 1;
    zone.set(p, new Room);
    p.y = 3;
    zone.set(p, new Room);
    const Rooms3dCubeSize &sz3 = zone.getSize();
    assert (sz3.width() == 2 && sz3.height() == 4 && sz3.levels() == 1);
    assert ( zone.testInvariant() );

    p.z = 2;
    zone.set(p, new Room);
    p.z = -1;
    zone.set(p, new Room);
    const Rooms3dCubeSize &sz4 = zone.getSize();
    assert (sz4.width() == 2 && sz4.height() == 4 && sz4.levels() == 4);
    assert ( zone.testInvariant() );

    p.y = -3;
    p.x = -2;
    zone.set(p, new Room);
    const Rooms3dCubeSize &sz5 = zone.getSize();
    assert (sz5.width() == 4 && sz5.height() == 7 && sz5.levels() == 4);
    assert ( zone.testInvariant() );

    p.z = -1;
    p.x = 1;
    p.y = 3;
    Room *r = zone.get(p);
    delete r;
    zone.set(p, NULL);
    zone.collapse();    
    const Rooms3dCubeSize &sz6 = zone.getSize();
    assert (sz6.width() == 4 && sz6.height() == 7 && sz6.levels() == 4);
    assert ( zone.testInvariant() );
}
#endif