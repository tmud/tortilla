#pragma once

struct PropertiesMapper
{
    PropertiesMapper() { initAllDefault(); dpi = 1.0f; }
    int  zoneslist_width;
	tstring current_zone;
    bool center_mode;
    float dpi;

    void initAllDefault()
    {
        zoneslist_width = -1;
		current_zone.clear();
        center_mode = true;
    }
};
