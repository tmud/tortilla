#pragma once
#include "roomObjects.h"
#include "dirObjects.h"
//#include "mapCursor.h"
//#include "mapTools.h"

class MapInstance
{
public:
    MapInstance();
    ~MapInstance();

    Rooms3dCube* findZone(int zid);
    Rooms3dCube* findZone(const tstring& name);
    Rooms3dCube* createNewZone();
    Rooms3dCube* createNewZone(const tstring& name);
    void getZones(Rooms3dCubeList* zones_list);

    void saveMaps(const tstring& dir);
    void loadMaps(const tstring& dir);
    void clearMaps();
    tstring  getNewZoneName(const tstring& templ);

    void deleteZone(const tstring& name);
private:
    void clear();
private:
    int m_nextzone_id;
	std::vector<Rooms3dCube*> zones;
	std::map<Rooms3dCube*, tstring> hashes;
	typedef std::vector<Rooms3dCube*>::iterator zone_iterator;
	typedef std::map<Rooms3dCube*, tstring>::iterator hashes_iterator;
};


