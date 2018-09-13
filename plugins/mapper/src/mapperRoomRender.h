#pragma once

#include "roomObjects.h"

class MapperRoomRender
{
public:
    MapperRoomRender(int room_size, int corridor_size);
    void setDC(HDC dc);
    void setIcons(CImageList *icons);
    void render(int dc_x, int dc_y, const Room *r, int type);
    void renderCursor(int dc_x, int dc_y, int color);
    void renderDummy(int dc_x, int dc_y);
private:
    void renderRoom(int x, int y, const Room *r);
    bool anotherZone(const Room* r, int dir);
    bool bidirectionalExit(const Room* r, int dir);
    bool neighbor(const Room* r, int dir);
    int  revertDir(int dir);    
    bool calcDestOffset(int &dx, int &dy, const Room* r, int dir);
    void renderShadow(int x, int y);
    void renderHole(int x, int y, const Room *r);    
    void renderRect(int x, int y, int dx, int dy);    
    void renderLine(int x, int y, int dx, int dy);
    void renderArrow(int x, int y, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3);
    void fillColor(int x, int y, int dx, int dy, COLORREF color);
    void fillBkg(int x, int y, int dx, int dy);
    void fillBlack(int x, int y, int dx, int dy);
    void fillWhite(int x, int y, int dx, int dy);
    void fillExit(int x, int y, int dx, int dy);
    void makeRect(int x, int y, int dx, int dy, RECT *rc);    

private:
    int m_size;
    int m_csize;
    CDCHandle m_dc;
    CDCHandle m_hdc;
    CBrush m_roomBkg, m_blackBkg, m_whiteBkg, m_exitBkg;
    CPen m_white, m_black, m_exit, m_exitL, m_yellow;
    CBrush m_shadowBrush;
    CImageList *m_plist;
};
