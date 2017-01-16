#pragma once

#include "propertyList.h"
#include "propertiesData.h"

namespace propertiesUpDown {
void up(PropertyListCtrl &list, PropertiesValues& values, bool mode);
void down(PropertyListCtrl &list, PropertiesValues& values, bool mode);
}
