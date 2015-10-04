#pragma once

class ClickpadImage
{
public:
   ClickpadImage() : m_image(NULL), m_atlas_x(0), m_atlas_y(0) {}
   ~ClickpadImage() { delete m_image; }
   void create(Image* image, const tstring& filepath, int x, int y) {
       delete m_image;
       m_image = image;
       m_atlas_x = x;
       m_atlas_y = y;
       m_atlas_filepath = filepath;
   }
   void render(HDC dc, int x, int y) {
       if (m_image)
        m_image->render(dc, x, y);
   }
   int width() const { return (m_image) ? m_image->width() : 0; }
   int height() const { return (m_image) ? m_image->height() : 0; }
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
       return true;
   }

private:
   Image* m_image;
   tstring m_atlas_filepath;
   int m_atlas_x;
   int m_atlas_y;
};
