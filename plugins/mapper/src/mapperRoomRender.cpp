#include "stdafx.h"
#include "mapperRoomRender.h"
#include "maskDC.h"

MapperRoomRender::MapperRoomRender(int room_size, int corridor_size, int deflate_size) : m_size(room_size), m_csize(corridor_size), 
m_roomsizeDeflate(deflate_size), m_plist(NULL)
{
    assert(room_size > 0 && corridor_size > 0);
    if (room_size % 2 != corridor_size % 2)
         m_csize = corridor_size + 1;
    m_roomBkg.CreateSolidBrush(RGB(180,180,180));
    m_blackBkg.CreateSolidBrush(RGB(70,0,0));
    m_whiteBkg.CreateSolidBrush(RGB(255,255,255));
    m_exitBkg.CreateSolidBrush(RGB(0,0,255));

    m_white.CreatePen(PS_SOLID, 1, RGB(255,255,255));
    m_black.CreatePen(PS_SOLID, 1, RGB(0,0,0));
    m_bkg.CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    m_exit.CreatePen(PS_SOLID, 1, RGB(0,0,255));
    m_exitL.CreatePen(PS_SOLID, 1, RGB(0,255,255));
    m_yellow.CreatePen(PS_SOLID, 1, RGB(255,0,255));
    m_door.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

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

void MapperRoomRender::render(int dc_x, int dc_y, const Room *r, int type)
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

void MapperRoomRender::renderDummy(int dc_x, int dc_y)
{
   int x = dc_x; int y = dc_y;
   CRect bk( x, y, x+m_size, y+m_size );
   CRect p(1, 1, 1, 1);
   bk.DeflateRect(&p);
   fillBkg(bk.left,bk.top,bk.right-bk.left,bk.bottom-bk.top);
}

void MapperRoomRender::renderCursor(int dc_x, int dc_y, int color)
{
    RECT p = { dc_x-3, dc_y-3, dc_x+m_size+3, dc_y+m_size+3 };
    HPEN old = (color == 0) ? m_hdc.SelectPen(m_exit) : m_hdc.SelectPen(m_yellow);
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

void MapperRoomRender::renderHole(int x, int y, const Room *r)
{
   CRect rp(x, y, x + m_size, y + m_size);
   int cs = m_csize;        // corridor size to draw (included)
   int ds = 2;              // doors offset to draw (included) -  x= lefx - ds, len = cs + ds + ds
   int rs = (cs - m_roomsizeDeflate) * 2;
   rp.DeflateRect(cs, cs, rs, rs);

   // start points for corridors - top, bottom, left, right
   int offset = (rp.right - rp.left - cs) / 2;
   int leftx = rp.left + offset;
   int topy = rp.top + offset;
   POINT ct = { leftx, rp.top };
   POINT cb = { leftx, rp.bottom };
   POINT cl = { rp.left, topy };
   POINT cr = { rp.right, topy };

   int w = rp.right-rp.left + 1;
   int h = rp.bottom-rp.top + 1;

   HPEN old = m_hdc.SelectPen(m_white);
   bool skip_north_line = false;
   if (r->dirs[RD_NORTH].exist)
   {
       if (anotherZone(r, RD_NORTH))
       {
           int x = ct.x - 1;
           int y = ct.y;
           int len = cs + 4;
           renderLine(x,y,len,0);
           renderLine(x,y-1,len,0);
           renderLine(x,y-2,len,0);
       }
       else if (neighbor(r, RD_NORTH))
       {
           int x = ct.x;
           int x2 = ct.x + cs;
           int y = ct.y;
           int len = x - rp.left+1;
           renderLine(x, y, -len, 0);
           renderLine(x, y, 0, -len);
           renderLine(x2, y, len, 0);
           renderLine(x2, y, 0, -len);
           skip_north_line = true;
       }
       else
       {
           int x = ct.x + cs / 2;
           int y = ct.y;
           renderLine(x, y, 0, -3);
           renderLine(x + 1, y, 0, -3);
       }
   }
   if (!skip_north_line)
        { renderLine(rp.left,rp.top,w,0); }

   bool skip_south_line = false;
   if (r->dirs[RD_SOUTH].exist)
   {
       if (anotherZone(r, RD_SOUTH))
       {
           int x = cb.x - 1;
           int y = cb.y;
           int len = cs + 4;
           renderLine(x, y, len, 0);
           renderLine(x, y + 1, len, 0);
           renderLine(x, y + 2, len, 0);
       }
       else if (neighbor(r, RD_SOUTH))
       {
           int x = cb.x;
           int x2 = cb.x + cs;
           int y = cb.y;
           int len = x - rp.left + 1;
           renderLine(x, y, -len, 0);
           renderLine(x, y, 0, len);
           renderLine(x2, y, len, 0);
           renderLine(x2, y, 0, len);
           skip_south_line = true;
       }
       else
       {
           int x = cb.x + cs / 2;
           int y = cb.y;
           renderLine(x, y, 0, 3);
           renderLine(x + 1, y, 0, 3);
       }
   }
   
   if (!skip_south_line)
        { renderLine(rp.left, rp.bottom, w, 0);  }

   bool skip_west_line = false;
   if (r->dirs[RD_WEST].exist)
   {
       if (anotherZone(r, RD_WEST))
       {
           int x = cl.x;
           int y = cl.y - 1;
           int len = cs + 4;
           renderLine(x, y, 0, len);
           renderLine(x-1, y, 0, len);
           renderLine(x-2, y, 0, len);
       }
       else if (neighbor(r, RD_WEST))
       {
           int x = cl.x;
           int y = cl.y;
           int y2 = cl.y + cs;
           int len = y - rp.top + 1;
           renderLine(x, y, 0, -len);
           renderLine(x, y, -len, 0);
           renderLine(x, y2, 0, len);
           renderLine(x, y2, -len, 0);
           skip_west_line = true;
       }
       else
       {
           int x = cl.x;
           int y = cl.y + cs / 2;
           renderLine(x, y, -3, 0);
           renderLine(x, y+1, -3, 0);
       }
   }
   if (!skip_west_line)
        { renderLine(rp.left,rp.top,0,h);  }

   bool skip_east_line = false;
   if (r->dirs[RD_EAST].exist)
   {
       if (anotherZone(r, RD_EAST))
       {
           int x = cr.x;
           int y = cr.y - 1;
           int len = cs + 4;
           renderLine(x, y, 0, len);
           renderLine(x + 1, y, 0, len);
           renderLine(x + 2, y, 0, len);
       }
       else if (neighbor(r, RD_EAST))
       {
           int x = cr.x;
           int y = cr.y;
           int y2 = cr.y + cs;
           int len = y - rp.top + 1;
           renderLine(x, y, 0, -len);
           renderLine(x, y, len, 0);
           renderLine(x, y2, 0, len);
           renderLine(x, y2, len, 0);
           skip_east_line = true;
       } 
       else
       {
           int x = cr.x;
           int y = cr.y + cs / 2;
           renderLine(x, y, 3, 0);
           renderLine(x, y + 1, 3, 0);
       }
   }
   if (!skip_east_line)
   {
       renderLine(rp.right, rp.top, 0, h);
   }

   int de = cs + 1;
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

void MapperRoomRender::renderRoom(int x, int y, const Room *r)
{
   CRect rp( x, y, x+m_size, y+m_size );
   //CRect base(rp);
   int cs = m_csize;        // corridor size to draw (included)
   int ds = 2;              // doors offset to draw (included) -  x= lefx - ds, len = cs + ds + ds
   int rs = (cs- m_roomsizeDeflate) * 2;
   rp.DeflateRect(cs, cs, rs, rs);

   CRect bk(rp);
   bk.DeflateRect(2,2,0,0);

   HPEN old1 = m_hdc.SelectPen(m_black);
   rp.OffsetRect(1, 1);
   renderRect(rp.left,rp.top,rp.right-rp.left,rp.bottom-rp.top);
   rp.OffsetRect(-1, -1);
   m_hdc.SelectPen(m_white);
   renderRect(rp.left,rp.top,rp.right-rp.left,rp.bottom-rp.top);

   int offset = (rp.right - rp.left - cs) / 2;

   // start points for corridors - top, bottom, left, right
   int leftx = rp.left + offset;
   int topy = rp.top + offset;
   POINT ct = { leftx, rp.top };
   POINT cb = { leftx, rp.bottom };
   POINT cl = { rp.left, topy };
   POINT cr = { rp.right, topy };
  
   if (r->dirs[RD_NORTH].exist)
   {
       if (anotherZone(r, RD_NORTH))
       {
           int x0 = ct.x;
           int x = ct.x - ds;
           int y = ct.y - 1;
           int len = cs + ds + ds;
           m_hdc.SelectPen(m_exit);
           renderLine(x,y,len,0);
           renderLine(x,y+1,len,0);
           renderLine(x,y+2,len,0);
           m_hdc.SelectPen(m_exitL);
           renderLine(x0,y+1,cs,0);
       }
       else if (neighbor(r, RD_NORTH))
       {
           int x = ct.x;
           int x2 = x + cs;
           int y = ct.y;
           int len = -(cs+2);
          
           m_hdc.SelectPen(m_black);
           renderLine(x+1, y, 0, len);
           renderLine(x2, y, 0, len);
           m_hdc.SelectPen(m_white);
           renderLine(x, y, 0, len);
           renderLine(x2-1, y, 0, len);
           renderLine(x2, y, 1, 1);
           m_hdc.SelectPen(m_bkg);
           renderLine(x2-1, y+1, 1, 1);

           len = -(cs + 3);
           fillBkg(x + 2, y + 2, cs - 3, len); // затираем стенку
       }
       else
       {
           m_hdc.SelectPen(m_white);
           int x = ct.x + cs / 2;
           int y = ct.y;
           int dx = 0; int dy = 0;
           if (!calcDestOffset(dx, dy, r, RD_NORTH))
           {
               renderLine(x, y, 0, -3);
               renderLine(x+1, y, 0, -3);
           }
           else {
              dy = dy + rp.bottom-rp.top;
              renderArrow(x+dx, y+dy, -5, 5, 5, 4, 1, 0);
              renderLine(x, y, dx, dy);
              renderLine(x+1, y, dx, dy);
          }
       }
       if (r->dirs[RD_NORTH].door)
       {
          int x = ct.x - ds;
          int y = ct.y;
          int len = cs + ds + ds;
          m_hdc.SelectPen(m_door);
          renderLine(x, y, len, 0);
          m_hdc.SelectPen(m_black);
          renderLine(x,y+1,len,0);
       }
   }
   if (r->dirs[RD_SOUTH].exist)
   {
       if (anotherZone(r, RD_SOUTH))
       {
           int x0 = cb.x;
           int x = cb.x - ds;
           int y = cb.y + 1;
           int len = cs + ds + ds;
           m_hdc.SelectPen(m_exit);
           renderLine(x, y, len, 0);
           renderLine(x, y - 1, len, 0);
           renderLine(x, y - 2, len, 0);
           m_hdc.SelectPen(m_exitL);
           renderLine(x0, y - 1, cs, 0);
       }
       else if (neighbor(r, RD_SOUTH))
       {
           int x = cb.x;
           int x2 = x + cs;
           int y = cb.y;
           int len = cs + 2;
           m_hdc.SelectPen(m_black);
           renderLine(x + 1, y, 0, len);
           renderLine(x2, y, 0, len);
           m_hdc.SelectPen(m_white);
           renderLine(x, y, 0, len);
           renderLine(x2 - 1, y, 0, len);
           m_hdc.SelectPen(m_bkg);
           renderLine(x+1, y-1, 1, 1);

           len = cs + 3;
           fillBkg(x + 2, y - 2, cs-3, len); // затираем стенку
       }
       else 
       {
          m_hdc.SelectPen(m_white);
          int x = cb.x + cs / 2;
          int y = cb.y;
          int dx = 0; int dy = 0;
          if (!calcDestOffset(dx, dy, r, RD_SOUTH))
          {
              renderLine(x, y, 0, 3);
              renderLine(x+1, y, 0, 3);
          }
          else {
              dy = dy - (rp.bottom-rp.top);
              renderArrow(x+dx, y+dy, -5, -5, 5, -4, 1, 0);
              renderLine(x, y, dx, dy);
              renderLine(x+1, y, dx, dy);
          }
       }
       if (r->dirs[RD_SOUTH].door)
       {
           int x = cb.x - ds;
           int y = cb.y ;
           int len = cs + ds + ds;
           m_hdc.SelectPen(m_door);
           renderLine(x, y, len, 0);
           m_hdc.SelectPen(m_black);
           renderLine(x, y-1, len, 0);
        }
   }
   if (r->dirs[RD_WEST].exist)
   {
       if (anotherZone(r, RD_WEST))
       {
           int y0 = cl.y;
           int y = cl.y - ds;
           int x = cl.x + 1;
           int len = cs + ds + ds;
           m_hdc.SelectPen(m_exit);
           renderLine(x, y, 0, len);
           renderLine(x-1 , y, 0, len);
           renderLine(x-2, y, 0, len);
           m_hdc.SelectPen(m_exitL);
           renderLine(x-1, y0, 0, cs);
       }
       else if (neighbor(r, RD_WEST))
       {
           int x = cl.x;
           int y = cl.y;
           int y2 = cl.y + cs;
           int len = -(cs + 2);
           m_hdc.SelectPen(m_black);
           renderLine(x, y+1, len, 0);
           renderLine(x, y2, len, 0 );
           m_hdc.SelectPen(m_white);
           renderLine(x, y, len, 0);
           renderLine(x, y2-1, len, 0);
           renderLine(x, y2, 1, 1);
           m_hdc.SelectPen(m_bkg);
           renderLine(x + 1, y2 - 1, 1, 1);
           len = -(cs + 3);
           fillBkg(x+2, y + 2, len, cs - 3); // затираем стенку
       }
       else
       {
          m_hdc.SelectPen(m_white);
          int x = cl.x;
          int y = cl.y + cs / 2;
          int dx = 0; int dy = 0;
          if (!calcDestOffset(dx, dy, r, RD_WEST))
          {
              renderLine(x, y, -3, 0);
              renderLine(x, y+1, -3, 0);
          }
          else {
              dx = dx + rp.right-rp.left;
              renderArrow(x+dx, y+dy, 5, -5, 4, 5, -1, 0);
              renderLine(x, y, dx, dy);
              renderLine(x, y+1, dx, dy);
          }
       }
       if (r->dirs[RD_WEST].door)
       {
           int x = cl.x;
           int y = cl.y - ds;
           int len = cs + ds + ds;
           m_hdc.SelectPen(m_door);
           renderLine(x, y, 0, len);
           m_hdc.SelectPen(m_black);
           renderLine(x+1, y, 0, len);
       }
   }
   
   if (r->dirs[RD_EAST].exist)
   {
       if (anotherZone(r, RD_EAST)) 
       {
           int y0 = cr.y;
           int y = cr.y - ds;
           int x = cr.x - 1;
           int len = cs + ds + ds;
           m_hdc.SelectPen(m_exit);
           renderLine(x, y, 0, len);
           renderLine(x + 1, y, 0, len);
           renderLine(x + 2, y, 0, len);
           m_hdc.SelectPen(m_exitL);
           renderLine(x + 1, y0, 0, cs);
       }
       else if (neighbor(r, RD_EAST))
       {
           int x = cr.x;
           int y = cr.y;
           int y2 = cr.y + cs;
           int len = cs + 2;

           m_hdc.SelectPen(m_black);
           renderLine(x, y + 1, len, 0);
           renderLine(x, y2, len, 0);
           m_hdc.SelectPen(m_white);
           renderLine(x, y, len, 0);
           renderLine(x, y2 - 1, len, 0);
           m_hdc.SelectPen(m_bkg);
           renderLine(x-1 , y+1, 1, 1);
           len = cs + 3;
           fillBkg(x - 2, y + 2, len, cs - 3); // затираем стенку
       }
       else
       {
          m_hdc.SelectPen(m_white);
          int x = cr.x;
          int y = cr.y + cs / 2;
          int dx = 0; int dy = 0;
          if (!calcDestOffset(dx, dy, r, RD_EAST))
          {
              renderLine(x, y, 3, 0);
              renderLine(x, y+1, 3, 0);
          }
          else {
              dx = dx - (rp.right-rp.left);
              renderArrow(x+dx, y+dy, -5, -5, -4, 5, 0, 1);
              renderLine(x, y, dx, dy);
              renderLine(x, y+1, dx, dy);
          }
       }
       if (r->dirs[RD_EAST].door)
       {
           int x = cr.x;
           int y = cr.y - ds;
           int len = cs + ds + ds;
           m_hdc.SelectPen(m_door);
           renderLine(x, y, 0, len);
           m_hdc.SelectPen(m_black);
           renderLine(x - 1, y, 0, len);
       }
   }

   int bkw = bk.Width()-1; int bkh = bk.Height()-1;
   if (!r->selected) {
       if (!r->use_color)
           fillBkg(bk.left, bk.top, bkw, bkh );
       else
           fillColor(bk.left, bk.top, bkw, bkh, r->color);
   }
   else {
       fillColor(bk.left, bk.top, bkw, bkh, RGB(255, 255, 255));
   }
   
   if (r->icon > 0)
   {
       int icons_count = m_plist->GetImageCount();
       int icon_size = bk.Width();
       int x = bk.left + (bk.Width() - icon_size) / 2 - 1;
       int y = bk.top + (bk.Height() - icon_size) / 2 - 1;
       if (r->icon <= icons_count)
           m_plist->Draw(m_hdc, r->icon - 1, x, y, ILD_TRANSPARENT);
   }

   int de = cs + 1;  // size up/down corridor
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
   m_hdc.SelectPen(old1);
}

bool MapperRoomRender::anotherZone(const Room* r, int dir)
{
    const Room *r2 = r->dirs[dir].next_room;
    if (!r2) return false;
    return r->pos.zid != r2->pos.zid;
}

bool MapperRoomRender::bidirectionalExit(const Room* r, int dir)
{
    const Room *r2 = r->dirs[dir].next_room;
    if (!r2) return false;
    int revert_dir = revertDir(dir);
    const Room* back = r2->dirs[revert_dir].next_room;
    return (r == back);    
}

bool MapperRoomRender::neighbor(const Room* r, int dir)
{
   if (!bidirectionalExit(r, dir))
       return false;
   const Room *r2 = r->dirs[dir].next_room;
   if (dir == RD_NORTH || dir == RD_SOUTH)
   {
       if (r->pos.x == r2->pos.x) {
           if (dir == RD_NORTH) {
               return (r->pos.y == r2->pos.y+1) ? true : false;
           }
           return (r->pos.y == r2->pos.y-1) ? true : false;
       }
       return false;
   }
   if (dir == RD_WEST || dir == RD_EAST)
   {
       if (r->pos.y == r2->pos.y) {
           if (dir == RD_WEST) {
               return (r->pos.x == r2->pos.x+1) ? true : false;
           }
           return (r->pos.x == r2->pos.x-1) ? true : false;
       }
       return false;
   }
   assert(false);
   return false;
}

bool MapperRoomRender::calcDestOffset(int &dx, int &dy, const Room* r, int dir)
{
    const Room *r2 = r->dirs[dir].next_room;
    if (!r2)
        return false;
    if (r->pos.zid != r2->pos.zid || r->pos.z != r2->pos.z) {
        //assert(false);
        return false;
    }
    dx = (r2->pos.x - r->pos.x) * m_size;
    dy = (r2->pos.y - r->pos.y) * m_size;


    return true;
}

int MapperRoomRender::revertDir(int dir)
{
    RoomDirHelper dh;
    RoomDir d = dh.cast(dir);
    RoomDir rd = dh.revertDir(d);
    return dh.index(rd);
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

void MapperRoomRender::renderArrow(int x, int y, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3)
{
   m_hdc.MoveTo(x, y);
   m_hdc.LineTo(x+dx1, y+dy1);
   m_hdc.MoveTo(x+dx2, y+dy2);
   m_hdc.LineTo(x+dx3, y+dy3);
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
