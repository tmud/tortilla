#include "stdafx.h"
#include "../api.h"
#include "fimage.h"

image image_load(const wchar_t* file, int extra_option)
{
    return fimage_load(file, extra_option);
}

void image_unload(image img)
{
    return fimage_unload(img);
}

int image_width(image img)
{
    return fimage_width(img);
}

int  image_height(image img)
{
    return fimage_height(img);
}

void image_render(image img, HDC dc, int x, int y)
{
    fimage_render(dc, img, x, y);
}

void  image_renderex(image img, HDC dc, int x, int y, int w, int h)
{
    fimage_renderex(dc, img, x, y, w, h);
}
