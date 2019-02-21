#pragma once

#include "propertiesData.h"
#include "..\palette256.h"

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
   int font_width;
};
