#pragma once
#include "clickpad.h"

struct ClickpadImageParams
{
    ClickpadImageParams() : atlas_x(0), atlas_y(0) {}
    int atlas_x;
    int atlas_y;
    tstring atlas_filename;
};

class ClickpadImage
{
    Image *m_image;
    ClickpadImageParams m_params;
public:
   ClickpadImage() : m_image(NULL) {}
   ~ClickpadImage() { delete m_image; }
   void create(Image* image, const tstring& filepath, int x, int y) 
   {
       delete m_image;
       m_image = image;
       m_params.atlas_x = x;
       m_params.atlas_y = y;
       m_params.atlas_filename = filepath;
   }
   void renderpushed(HDC dc, int x, int y)
   {
       if (m_image)
       {
           image_render_ex r;
           r.sw = m_image->width()-1;
           r.sh = m_image->height()-1;
           r.sx = r.sy = 1;
           r.w = r.sw;
           r.h = r.sh;
           m_image->render(dc, x+2, y+2, &r);
       }
   }
   void render(HDC dc, int x, int y)
   {
       if (m_image)
           m_image->render(dc, x, y);
   }

   int width() const { return (m_image) ? m_image->width() : 0; }
   int height() const { return (m_image) ? m_image->height() : 0; }
   const ClickpadImageParams& params() const { return m_params; }
   bool empty() const { return (m_image) ? false : true; }
};
