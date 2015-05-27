#pragma once

/*class ImagesCollection
{
public:
    ~ImagesCollection()
    {
        for (iterator it=m_images.begin(),it_end=m_images.end(); it!=it_end; ++it)
            delete it->second;    
    }
    Image *loadImage(const tstring& fname)
    {
        iterator it = m_images.find(fname);
        if (it != m_images.end())
            return it->second;
        Image *image = new Image();
        if (!image->load(fname))
        {
            delete image;
            return NULL;
        }
        m_images[fname] = image;
        return image;
    }

private:
    std::map<tstring, Image*> m_images;    
    typedef std::map<tstring, Image*>::iterator iterator;
};
*/