#pragma once
#include "clickpad.h"

struct ClickpadImageParams
{
    ClickpadImageParams() : atlas_x(0), atlas_y(0) {}
    int atlas_x;
    int atlas_y;
    std::wstring atlas_filename;
};

class ClickpadImage
{
    Image *m_image;
    ClickpadImageParams m_params;
public:
   ClickpadImage() : m_image(NULL) {}
   ~ClickpadImage() { delete m_image; }
   void create(Image* image, const std::wstring& filepath, int x, int y) 
   {
       delete m_image;
       m_image = image;
       m_params.atlas_x = x;
       m_params.atlas_y = y;
       m_params.atlas_filename = filepath;
   }
   void renderpushed(HDC dc, int x, int y, int w, int h)
   {
       if (m_image)
       {
           image_render_ex r;
           r.sw = m_image->width();
           r.sh = m_image->height();
           r.sx = r.sy = 0;
           r.w = w;
           r.h = h;
           m_image->render(dc, x+1, y+1, &r);
       }
   }
   void render(HDC dc, int x, int y, int w, int h)
   {
       if (m_image)
       {
           image_render_ex r;
           r.w = w;
           r.h = h;
           m_image->render(dc, x, y, &r);
       }
   }

   int width() const { return (m_image) ? m_image->width() : 0; }
   int height() const { return (m_image) ? m_image->height() : 0; }
   const ClickpadImageParams& params() const { return m_params; }
   bool empty() const { return (m_image) ? false : true; }
};
