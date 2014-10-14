#pragma once

#include "propertiesData.h"
#include "..\palette256.h"

class PropertiesGlobal
{
public:
    PropertiesGlobal();
    ~PropertiesGlobal();
    bool get(const tstring& name, tstring *value);
    bool get(const tstring& name, int *value);
    void set(const tstring& name, const tstring& value);
    void set(const tstring& name, int value);

private:
    xml::node m_data;
    u8string m_path;
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
