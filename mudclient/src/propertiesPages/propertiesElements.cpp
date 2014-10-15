#include "stdafx.h"
#include "propertiesElements.h"

PropertiesGlobal::PropertiesGlobal() : welcome(1)
{
    {
        tstring path;
        tchar szPath[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath);
        path.assign(szPath);
        int last = path.length() - 1;
        if (path.at(last) != L'\\')
            path.append(L"\\");
        path.append(L"tortilla.xml");
        m_path.assign(WideToUtf8(path.c_str()));
    }

    xml::node gd;
    if (gd.load(m_path.c_str()))
    {
        loadValue(gd, "welcome", 0, 1, &welcome);
        gd.deletenode();
    }    
}

PropertiesGlobal::~PropertiesGlobal()
{
    xml::node gd("global");
    saveValue(gd, "welcome", welcome);
    saveString(gd, "version", TORTILLA_VERSION);
    gd.save(m_path.c_str());
    gd.deletenode();
}

void PropertiesGlobal::saveValue(xml::node parent, const utf8* name, int value)
{
    xml::node n = parent.createsubnode(name);
    n.set("value", value);
}

bool PropertiesGlobal::loadString(xml::node parent, const utf8* name, tstring* value)
{
    xml::request r(parent, name);
    if (r.size() == 0)
        return false;
    std::string v;
    if (!r[0].get("value", &v))
        return false;
    U2W u2w(v);
    value->assign(u2w);
    return true;
}

void PropertiesGlobal::saveString(xml::node parent, const utf8* name, const tstring& value)
{
    W2U w2u(value);
    xml::node n = parent.createsubnode(name);
    n.set("value", w2u);
}

bool PropertiesGlobal::loadValue(xml::node parent, const utf8* name, int min, int max, int *value)
{
    xml::request r(parent, name);
    if (r.size() == 0)
        return false;
    int v = 0;
    if (!r[0].get("value", &v))
        return false;
    if (v < min)
        v = min;
    if (v > max)
        v = max;
    *value = v;
    return true;
}

PropertiesElements::PropertiesElements(PropertiesData *data) : propData(data), palette(data), font_height(1)
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
    GetTextExtentPoint32(dc, L"W", 1, &sz);
    font_height = sz.cy;
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
