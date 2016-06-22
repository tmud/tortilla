#pragma once

#include "propertiesPages/propertiesData.h"
#include "propertiesPages/propertiesManager.h"
#include "palette256.h"
#include "plugins/pluginsManager.h"
#include "varProcessor.h"

namespace tortilla {
void init();
CFont* getCurrentFont();
Palette256* getPalette();
PropertiesData* getProperties();
PluginsDataValues* pluginsData();
PropertiesManager* getPropertiesManager();
PluginsManager* getPluginsManager();
lua_State* getLua();
VarProcessor* getVars();
bool isPropertiesOpen();
} // namespace tortilla
