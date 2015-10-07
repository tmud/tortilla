#pragma once

#include "clickpadImage.h"

class ImageCollection
{
public:
    struct imdata {
        imdata() : image_size(0), image(NULL) {}
        int image_size;
        tstring file_path;
        tstring name;
        Image* image;
    };

    ImageCollection();
    ~ImageCollection();
    void scanImages();
    int  getImagesCount() const;
    const imdata& getImage(int index) const;
    
    ClickpadImage* load(const tstring& params);    
    void save(ClickpadImage* image, tstring* params);

private:
    std::vector<imdata> m_files;
};
