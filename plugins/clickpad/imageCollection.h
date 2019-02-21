#pragma once

#include "clickpadImage.h"

struct BigImageData
{
    BigImageData() : image_size(0), image_border(0), image(NULL) {}
    void destroy() { delete image; image = NULL; image_size = image_border = 0; file_path.clear(); name.clear(); }
    int image_size;
    int image_border;
    std::wstring file_path;
    std::wstring name;
    Image* image;
};

class ImageCollection
{
public:
    ImageCollection();
    ~ImageCollection();
    void scanImages();
    int  getImagesCount() const;
    const BigImageData& getImage(int index) const;
    ClickpadImage* load(const std::wstring& params);
    ClickpadImage* load(const std::wstring& path, int x, int y);
    void save(ClickpadImage* image, std::wstring* params);
private:
    std::vector<BigImageData> m_files;
};
