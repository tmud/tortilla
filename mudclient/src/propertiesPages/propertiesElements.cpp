#include "stdafx.h"
#include "propertiesElements.h"

PropertiesElements::PropertiesElements(PropertiesData *data) : propData(data), palette(data),
font_height(1), font_width(1)
{
}

PropertiesElements::~PropertiesElements()
{
}

void PropertiesElements::updateProps(HWND parentWnd)
{
    if (!background_brush.IsNull())
          background_brush.DeleteObject();
    background_brush = CreateSolidBrush(propData->bkgnd);

    if (!standard_font.IsNull())
        standard_font.DeleteObject();
    LOGFONT logfont;
    propData->initLogFont(parentWnd, &logfont); 
    standard_font.CreateFontIndirect(&logfont);

    CDC dc(GetDC(parentWnd));
    HFONT oldfont = dc.SelectFont(standard_font);
    SIZE sz = {0,0};
    GetTextExtentPoint32(dc, L"a", 1, &sz);
    font_height = sz.cy;
    font_width = sz.cx;
    dc.SelectFont(oldfont);

    if (!underlined_font.IsNull())
        underlined_font.DeleteObject();
    logfont.lfUnderline = 1;
    underlined_font.CreateFontIndirect(&logfont);

    if (!italic_font.IsNull())
        italic_font.DeleteObject();
    logfont.lfUnderline = 0;
    if (logfont.lfItalic)
        logfont.lfItalic = 0;
    else
        logfont.lfItalic = 1;
    italic_font.CreateFontIndirect(&logfont);

    if (!italic_underlined_font.IsNull())
         italic_underlined_font.DeleteObject();
    logfont.lfUnderline = 1;
    italic_underlined_font.CreateFontIndirect(&logfont);

    palette.updateProps(propData);
}
