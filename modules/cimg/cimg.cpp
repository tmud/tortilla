#include "stdafx.h"
#include "cimg/CImg.h"
using namespace cimg_library;

class Image
{
public:
    bool load()
    {
       // m_image.load("d:\\1.jpg");
    }

    void render()
    {
        //m_image.
        //CImgDisplay d(;
    }
private:
    CImg<unsigned char> m_image;
};


int cimg_load(lua_State *L)
{
    return 0;
}

static const luaL_Reg cimg_methods[] =
{
    { "load", cimg_load },    
    { NULL, NULL }
};

int luaopen_cimg(lua_State *L)
{
    luaL_newlib(L, cimg_methods);
    return 1;
}
