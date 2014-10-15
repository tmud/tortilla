#pragma once

#include "propertiesData.h"
#include "..\palette256.h"

struct PropertiesGlobal
{
    int welcome;
public:
    PropertiesGlobal();
    ~PropertiesGlobal();
private:
    u8string m_path;
    bool loadValue(xml::node parent, const utf8* name, int min, int max, int *value);
    void saveValue(xml::node parent, const utf8* name, int value);
    bool loadString(xml::node parent, const utf8* name, tstring* value);
    void saveString(xml::node parent, const utf8* name, const tstring& value);
};

class PropertiesElements
{
public:
    PropertiesElements(PropertiesData* data);
    ~PropertiesElements();
    void updateProps(HWND parentWnd);

public:
   PropertiesData *propData;
   CBrush background_brush;
   CFont standard_font;
   CFont underlined_font;
   CFont italic_font;
   CFont italic_underlined_font;
   Palette256 palette;  
   int font_height;
   PropertiesGlobal global;
};
