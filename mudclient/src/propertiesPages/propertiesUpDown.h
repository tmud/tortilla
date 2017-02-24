#pragma once

#include "propertyList.h"
#include "propertiesData.h"

template<class T>
class propertiesUpDown 
{
    int m_group;
public:
    propertiesUpDown() : m_group(2) {}
    propertiesUpDown(int group) : m_group(group) {}
    void up(PropertyListCtrl &list, PropertiesValuesT<T>& values, bool mode);
    void down(PropertyListCtrl &list, PropertiesValuesT<T>& values, bool mode);
};
