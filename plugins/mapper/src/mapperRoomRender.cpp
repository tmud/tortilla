#include "stdafx.h"
#include "mapperRoomRender.h"
#include "maskDC.h"
#include "helpers.h"

MapperRoomRender::MapperRoomRender(int room_size, int corridor_size) : m_size(room_size), m_csize(corridor_size), m_plist(NULL)
{
    assert(room_size > 0 && corridor_size > 0);
    m_roomBkg.CreateSolidBrush(RGB(180,180,180));
    m_blackBkg.CreateSolidBrush(RGB(70,0,0));
    m_whiteBkg.CreateSolidBrush(RGB(255,255,255));
    m_exitBkg.CreateSolidBrush(RGB(0,0,255));

    m_white.CreatePen(PS_SOLID, 1, RGB(255,255,255));
    m_black.CreatePen(PS_SOLID, 1, RGB(0,0,0));
    m_exit.CreatePen(PS_SOLID, 1, RGB(0,0,255));
    m_exitL.CreatePen(PS_SOLID, 1, RGB(0,255,255));
    m_yellow.CreatePen(PS_SOLID, 1, RGB(255,0,255));

    CBitmap m_shadow;
    m_shadow.LoadBitmap(IDB_SHADOW);
    LOGBRUSH lb;
    lb.lbStyle = BS_PATTERN; lb.lbColor = 0; lb.lbHatch = (LONG_PTR)(HBITMAP)m_shadow;
    m_shadowBrush.CreateBrushIndirect(&lb);
}

void MapperRoomRender::setDC(HDC dc)
{
    m_dc.Attach(dc);
    m_dc.SetBkMode(TRANSPARENT);
    m_hdc = dc;
}

void MapperRoomRender::setIcons(CImageList *icons)
{
    m_plist = icons;
}

void MapperRoomRender::render(int dc_x, int dc_y, Room *r, int type)
{
    if (type == 0)
        renderRoom(dc_x, dc_y, r);
    else if (type == 1)
    {
        renderRoom(dc_x, dc_y, r);
        renderShadow(dc_x, dc_y);
    }
    else if (type == 2)
    {
        RECT pos = { dc_x,dc_y,dc_x+m_size,dc_y+m_size } ;
        CMemoryDC mdc(m_hdc, pos);
        m_hdc = mdc;
        renderHole(dc_x, dc_y, r);
        renderShadow(dc_x, dc_y);
        mdc.BitBlt(dc_x,dc_y,m_size,m_size,m_dc,dc_x,dc_y, SRCINVERT);
        m_hdc = m_dc;
    }
}

void MapperRoomRender::renderCursor(int dc_x, int dc_y, int type)
{
    RECT p = { dc_x-3, dc_y-3, dc_x+m_size+3, dc_y+m_size+3 };
    HPEN old = (type == 0) ? m_hdc.SelectPen(m_exit) : m_hdc.SelectPen(m_yellow);
    renderRect(dc_x-3, dc_y-3, m_size+6, m_size+6);
    renderRect(dc_x-2, dc_y-2, m_size+4, m_size+4);    
    m_hdc.SelectPen(old);
}

void MapperRoomRender::renderShadow(int x, int y)
{
    MaskDC m(m_hdc, m_size, m_size);
    m.makeMask(x, y);
    RECT rd = { x, y, x + m_size, y + m_size };
    CMemoryDC tdc(m_hdc, rd);
    tdc.FillRect(&rd, m_shadowBrush);
    tdc.BitBlt(x, y, m_size, m_size, m, 0, 0, SRCAND);
    tdc.BitBlt(x, y, m_size, m_size, m_hdc, x, y, SRCERASE);
}

void MapperRoomRender::renderHole(int x, int y, Room *r)
{
   CRect rp(x, y, x+m_size, y+m_size);
   int cs = m_csize;
   int rs = (cs-2) * 2;
   int de = (m_size-3*m_csize) / 2 - 1;
   rp.DeflateRect(cs, cs, rs, rs);
   int w = rp.right-rp.left;
   int h = rp.bottom-rp.top;

   HPEN old = m_hdc.SelectPen(m_white);
   if (r->dirs[RD_NORTH].exist)
   {
       int x = rp.left + de; int y = rp.top;
       renderLine(x-de,y,de,0);
       renderLine(x+cs,y,de+2,0);
       renderLine(x,y,0,-cs-1);
       renderLine(x+cs,y,0,-cs-1);

       if (anotherZone(r, RD_NORTH))
       {
           int x = rp.left + cs;
           int y = rp.top - 3;
           renderLine(x,y,de+3,0);
           renderLine(x,y-1,de+3,0);
           renderLine(x,y-2,de+3,0);
       }
   }
   else  {  renderLine(rp.left,rp.top,w,0); }

   if (r->dirs[RD_SOUTH].exist)
   {
       int x = rp.left + de;
       int y = rp.bottom;
       renderLine(x-de,y,de,0);
       renderLine(x+cs,y,de+2,0);
       renderLine(x,y,0,cs+1);
       renderLine(x+cs,y,0,cs+1);

       if (anotherZone(r, RD_SOUTH))
       {
           int x = rp.left + cs;
           int y = rp.bottom + 3;
           renderLine(x,y,de+3,0);
           renderLine(x,y+1,de+3,0);
           renderLine(x,y+2,de+3,0);
       }
   }
   else { renderLine(rp.left,rp.bottom,w,0);  }

   if (r->dirs[RD_WEST].exist)
   {   
       int x = rp.left;
       int y = rp.top + de;
       renderLine(x,y-de,0,de);
       renderLine(x,y+cs,0,de+2);
       renderLine(x,y,-cs-1,0);
       renderLine(x,y+cs,-cs-1,0);

       if (anotherZone(r, RD_WEST))
       {
           int x = rp.left - 3;
           int y = rp.top + cs;
           renderLine(x,y,0,de+3);
           renderLine(x-1,y,0,de+3);
           renderLine(x-2,y,0,de+3);
       }
   }
   else { renderLine(rp.left,rp.top,0,h);  }

   if (r->dirs[RD_EAST].exist)
   {   
       int x = rp.right;
       int y = rp.top + de;
       renderLine(x,y-de,0,de);
       renderLine(x,y+cs,0,de+3);
       renderLine(x,y,cs+1,0);
       renderLine(x,y+cs,cs+1,0);

       if (anotherZone(r, RD_EAST))
       {
           int x = rp.right + 3;
           int y = rp.top + cs;
           renderLine(x,y,0,de+3);
           renderLine(x+1,y,0,de+3);
           renderLine(x+2,y,0,de+3);
       }
   }
   else { renderLine(rp.right,rp.bottom,0,-h);  }

   de = cs + 1;
   if (r->dirs[RD_UP].exist)
   {
       int x = rp.left + 3;
       int y = rp.top + 3;
       renderLine(x,y,0,de);
       renderLine(x,y,de,0);
   }
   if (r->dirs[RD_DOWN].exist)
   {
       int x = rp.right - 4;
       int y = rp.bottom - 4;
       renderLine(x,y,0,-de);
       renderLine(x,y,-de,0);
   }
   m_hdc.SelectPen(old);
}

void MapperRoomRender::renderRoom(int x, int y, Room *r)
{   
   CRect rp( x, y, x+m_size, y+m_size );
   CRect base(rp);
   int cs = m_csize;
   int rs = (cs-2) * 2;
   int de = (m_size-3*m_csize) / 2 - 1;
   rp.DeflateRect(cs, cs, rs, rs);

   CRect bk(rp);
   bk.DeflateRect(2,2,0,0);
   if (!r->use_color)
     fillBkg(bk.left,bk.top,bk.right-bk.left,bk.bottom-bk.top);
   else
     fillColor(bk.left, bk.top, bk.right - bk.left, bk.bottom - bk.top, r->color);

   if (m_plist && r->icon > 0)
   {
       int icons_count = m_plist->GetImageCount();
       if (r->icon <= icons_count)
            m_plist->Draw(m_hdc, r->icon - 1, bk.left, bk.top + 1, ILD_TRANSPARENT);
   }
   
   HPEN old = m_hdc.SelectPen(m_black);
   rp.OffsetRect(1, 1);
   renderRect(rp.left,rp.top,rp.right-rp.left,rp.bottom-rp.top);
   rp.OffsetRect(-1, -1);
   m_hdc.SelectPen(m_white);
   renderRect(rp.left,rp.top,rp.right-rp.left,rp.bottom-rp.top);

   if (r->dirs[RD_NORTH].exist)
   {
       int x = rp.left + de;
       int y = rp.top;
       fillBkg(x+2,y+2,cs-1,-cs-2);
       m_hdc.SelectPen(m_black);
       renderLine(x+1,y,0,-cs-1);
       renderLine(x+cs+1,y-1,0,-cs);
       m_hdc.SelectPen(m_white);
       renderLine(x,y,0,-cs-1);
       renderLine(x+cs,y,0,-cs-1);
       if (r->dirs[RD_NORTH].door)
       {
           int x = rp.left + cs;
           int y = rp.top - 2;
           m_hdc.SelectPen(m_white);
           renderLine(x,y, de+3,0);
           m_hdc.SelectPen(m_black);
           renderLine(x+3,y+1,cs-1,0);
       }
       if (anotherZone(r, RD_NORTH))
       {
           int x = rp.left + cs;
           int y = base.top;
           m_hdc.SelectPen(m_exit);
           renderLine(x,y,de+3,0);
           renderLine(x,y+1,de+3,0);
           renderLine(x,y+2,de+3,0);           
           m_hdc.SelectPen(m_exitL);
           renderLine(x+2,y+1,de-1,0);
       }
   }
   if (r->dirs[RD_SOUTH].exist)
   {
       int x = rp.left + de;
       int y = rp.bottom;
       fillBkg(x+1,y-1,cs-1,cs+2);
       m_hdc.SelectPen(m_black);
       renderLine(x+1,y,0,cs+1);
       renderLine(x+cs+1,y,0,cs+1);
       m_hdc.SelectPen(m_white);
       renderLine(x,y,0,cs+1);
       renderLine(x+cs,y,0,cs+1);

       if (r->dirs[RD_SOUTH].door)
       {
           int x = rp.left + cs;
           int y = rp.bottom + 2;
           m_hdc.SelectPen(m_white);
           renderLine(x,y, de+3,0);
           m_hdc.SelectPen(m_black);
           renderLine(x+3,y-1,cs-1,0);
       }
       if (anotherZone(r, RD_SOUTH))
       {
           int x = rp.left + cs;
           int y = base.bottom - 1;
           m_hdc.SelectPen(m_exit);
           renderLine(x,y,de+3,0);
           renderLine(x,y-1,de+3,0);
           renderLine(x,y-2,de+3,0);
           m_hdc.SelectPen(m_exitL);
           renderLine(x+2,y-1,de-1,0);
       }
   }

   if (r->dirs[RD_WEST].exist)
   {   
       int x = rp.left;
       int y = rp.top + de;
       fillBkg(x+2,y+2,-cs-2,cs-1);
       m_hdc.SelectPen(m_black);
       renderLine(x,y+1,-cs-1,0);
       renderLine(x-1,y+cs+1,-cs,0);
       m_hdc.SelectPen(m_white);
       renderLine(x,y,-cs-1,0);
       renderLine(x,y+cs,-cs-1,0);
       if (r->dirs[RD_WEST].door)
       {
           int x = rp.left - 2 ;
           int y = rp.top + cs;
           m_hdc.SelectPen(m_white);
           renderLine(x,y,0,de+3);
           m_hdc.SelectPen(m_black);
           renderLine(x+1,y+3,0,cs-1);
       }
       if (anotherZone(r, RD_WEST))
       {
           int x = base.left;
           int y = rp.top + cs;
           m_hdc.SelectPen(m_exit);
           renderLine(x,y,0,de+3);
           renderLine(x+1,y,0,de+3);
           renderLine(x+2,y,0,de+3);
           m_hdc.SelectPen(m_exitL);
           renderLine(x+1,y+2,0,de-1);
       }
   }

   if (r->dirs[RD_EAST].exist)
   {
       int x = rp.right;
       int y = rp.top + de;
       fillBkg(x-1,y+1,cs+2,cs-1);
       m_hdc.SelectPen(m_black);
       renderLine(x,y+1, cs+1,0);
       renderLine(x,y+cs+1,cs+1,0);
       m_hdc.SelectPen(m_white);
       renderLine(x,y,cs+1,0);
       renderLine(x,y+cs,cs+1,0);

       if (r->dirs[RD_EAST].door)
       {
           int x = rp.right + 2;
           int y = rp.top + cs;
           m_hdc.SelectPen(m_white);
           renderLine(x,y,0,de+3);
           m_hdc.SelectPen(m_black);
           renderLine(x-1,y+3,0,cs-1);
       }
       if (anotherZone(r, RD_EAST)) 
       {
           int x = base.right-1;
           int y = rp.top + cs;
           m_hdc.SelectPen(m_exit);
           renderLine(x,y,0,de+3);
           renderLine(x-1,y,0,de+3);
           renderLine(x-2,y,0,de+3);
           m_hdc.SelectPen(m_exitL);
           renderLine(x-1,y+2,0,de-1);          
       }
   }

   de = cs + 1;
   if (r->dirs[RD_UP].exist)
   {
       int x = rp.left + 3;
       int y = rp.top + 3;
       int x2 = x + de;
       int y2 = y + de;
       
       m_hdc.SelectPen(m_black);
       renderLine(x2, y, 0, de);
       renderLine(x2, y2, -de, 0);
       m_hdc.SelectPen(m_white);
       renderLine(x,y2,0,-de);
       renderLine(x,y,de+1,0);

       if (r->dirs[RD_UP].door)
       {
           fillWhite(x+1,y+1,de-1,de-1);
       }
       else if (!r->dirs[RD_UP].next_room) // unknown room
       {
           fillBlack(x+1,y+1,de-1,de-1);
       }
       if (anotherZone(r, RD_UP))
       {
           fillExit(x+1,y+1,de-1,de-1);
           m_hdc.SelectPen(m_exitL);
           renderLine(x2, y, 0, de);
           renderLine(x2, y2, -de, 0);
           renderLine(x,y2,0,-de);
           renderLine(x,y,de+1,0);
       }
   }

   if (r->dirs[RD_DOWN].exist)
   {
       int x = rp.right - 3;
       int y = rp.bottom - 3;
       int x2 = x - de;
       int y2 = y - de;

       m_hdc.SelectPen(m_black);
       renderLine(x2, y, 0, -de);
       renderLine(x2, y2, de, 0);
       m_hdc.SelectPen(m_white);
       renderLine(x,y2,0,de);
       renderLine(x,y,-de-1,0);

       if (r->dirs[RD_DOWN].door)
       {
           fillWhite(x,y,-de+1,-de+1);
       }       
       else if (!r->dirs[RD_DOWN].next_room) // unknown room
       {
          fillBlack(x,y,-de+1,-de+1); 
       }
       if (anotherZone(r, RD_DOWN)) 
       {
           fillExit(x,y,-de+1,-de+1);
           m_hdc.SelectPen(m_exitL);
           renderLine(x2, y, 0, -de);
           renderLine(x2, y2, de, 0);
           renderLine(x,y2,0,de);
           renderLine(x,y,-de-1,0);
       }
   }
   m_hdc.SelectPen(old);
}

bool MapperRoomRender::anotherZone(Room* r, int dir)
{
    Room *r2 = r->dirs[dir].next_room;
    if (!r2)
        return false;
    RoomHelper rh(r);
    return (rh.isSameZone(r2)) ? false : true;
}

void MapperRoomRender::renderRect(int x, int y, int dx, int dy)
{
   int x2 = x + dx - 1;
   int y2 = y + dy - 1;
   m_hdc.MoveTo(x, y);
   m_hdc.LineTo(x2, y);
   m_hdc.LineTo(x2, y2);
   m_hdc.LineTo(x, y2);
   m_hdc.LineTo(x, y);
}

void MapperRoomRender::renderLine(int x, int y, int dx, int dy)
{
   m_hdc.MoveTo(x, y);
   m_hdc.LineTo(x+dx, y+dy);
}

void MapperRoomRender::fillColor(int x, int y, int dx, int dy, COLORREF color)
{
    RECT p; makeRect(x, y, dx, dy, &p);
    m_hdc.FillSolidRect(&p, color);
}

void MapperRoomRender::fillBkg(int x, int y, int dx, int dy)
{   
    RECT p; makeRect(x,y,dx,dy,&p);
    m_hdc.FillRect(&p, m_roomBkg);
}

void MapperRoomRender::fillBlack(int x, int y, int dx, int dy)
{
    RECT p; makeRect(x,y,dx,dy,&p);
    m_hdc.FillRect(&p, m_blackBkg);
}

void MapperRoomRender::fillWhite(int x, int y, int dx, int dy)
{
    RECT p; makeRect(x,y,dx,dy,&p);
    m_hdc.FillRect(&p, m_whiteBkg);
}

void MapperRoomRender::fillExit(int x, int y, int dx, int dy)
{
    RECT p; makeRect(x,y,dx,dy,&p);
    m_hdc.FillRect(&p, m_exitBkg);
}

void MapperRoomRender::makeRect(int x, int y, int dx, int dy, RECT *rc)
{
    RECT p = { x, y, x+dx, y+dy };
    if (p.left > p.right) 
        { int t = p.left; p.left = p.right; p.right = t; }
    if (p.top > p.bottom)
        { int t = p.top; p.top = p.bottom; p.bottom = t; }
    *rc = p;
}
