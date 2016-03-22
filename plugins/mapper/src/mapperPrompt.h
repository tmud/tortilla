#pragma once
#include "mapperProcessor.h"

class MapperPrompt
{
public:
    MapperPrompt();
    void updateProps(PropertiesMapper *props);
    bool processNetworkData(const WCHAR* text, int textlen);

private:
    MapperKeyElement bp;       // begin prompt
    MapperKeyElement ep;       // end prompt
    MapperDataQueue m_network_buffer;
};

