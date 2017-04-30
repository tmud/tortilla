#include "stdafx.h"
#include "accessors.h"
#include "propertiesPages/propertiesData.h"
#include "MainFrm.h"

extern CMainFrame _wndMain;
MudGameView* _gameview = NULL;
luaT_State L;
VarProcessor m_vars;

void tortilla::init()
{
    _gameview = &_wndMain.m_gameview;
}

PropertiesData* tortilla::getProperties()
{
    return _gameview->getPropData();
}

PluginsDataValues* tortilla::pluginsData()
{
    PropertiesData* pdata = _gameview->getPropData();
    return pdata->displays.plugins_data();
}

CFont* tortilla::getCurrentFont()
{
    return _gameview->getStandardFont();
}

int tortilla::getCurrentFontHeight()
{
    CFont *f =  _gameview->getStandardFont();
    LOGFONT lf;
    f->GetLogFont(&lf);
    CDC dc(_gameview->GetDC());
    int height = -MulDiv(lf.lfHeight, GetDeviceCaps(dc, LOGPIXELSY), 72);
    return height;
}

Palette256* tortilla::getPalette()
{
    return _gameview->getPalette();
}

PropertiesManager* tortilla::getPropertiesManager()
{
    return _gameview->getPropManager();
}

PluginsManager* tortilla::getPluginsManager()
{
    return _gameview->getPluginsManager();
}

lua_State* tortilla::getLua()
{
    return L;
}

VarProcessor* tortilla::getVars()
{
    return &m_vars;
}

bool tortilla::isPropertiesOpen()
{
    return _gameview->isPropertiesOpen();
}
