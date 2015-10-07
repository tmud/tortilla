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
   void render(HDC dc, int x, int y)
   {
       if (m_image)
            m_image->render(dc, x, y);
   }
   int width() const { return (m_image) ? m_image->width() : 0; }
   int height() const { return (m_image) ? m_image->height() : 0; }
   const ClickpadImageParams& params() const { return m_params; }
   bool empty() const { return (m_image) ? false : true; }
   
   /*
   void save(tstring *params) {
       if (!m_image) { params->clear(); return; }
       tchar buffer[32];
       wsprintf(buffer, L"%d,%d,", m_atlas_x, m_atlas_y);
       params->assign(buffer);
       params->append(m_atlas_filepath);
   }
   bool load(const tstring& params) {
       if (params.empty())
           return false;
       const tchar* p = params.c_str();
       const tchar *p1 = wcschr(p, L',');
       if (!p1) return false;
       const tchar *p2 = wcschr(p1+1, L',');
       if (!p2) return false;
       tstring ax(p, p1-p);
       tstring ay(p1+1, p2-p1-1);
       tstring path(p2+1);
       if (!s2i(ax, &m_atlas_x) || !s2i(ay, &m_atlas_y))
           return false;    
       m_atlas_filepath = path;

       tstring fpath;
       getImagesDir(&fpath);
       fpath.append(path);
       Image *image = new Image;
       if (!image->load(fpath.c_str(), 0))
        { delete image; return false; }

       m_image = image;
       return true;
   }*/
};
