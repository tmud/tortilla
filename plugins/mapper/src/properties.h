#pragma once

struct PropertiesMapper
{
    PropertiesMapper() { initAllDefault();  }
    int  zoneslist_width;
	tstring current_zone;
    bool center_mode;

    void initAllDefault()
    {
        zoneslist_width = -1;
		current_zone.clear();
        center_mode = true;
    }
};
