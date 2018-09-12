#pragma once

struct PropertiesMapper
{
    PropertiesMapper() { initAllDefault();  }
    int  zoneslist_width;
	tstring current_zone;

    void initAllDefault()
    {
        zoneslist_width = -1;
		current_zone.clear();
    }
};
