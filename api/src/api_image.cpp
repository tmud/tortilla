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

image image_cut(image img, int x, int y, int w, int h)
{
    return fimage_cut(img, x, y, w, h);
}

int image_width(image img)
{
    return fimage_width(img);
}

int image_height(image img)
{
    return fimage_height(img);
}

int image_render(image img, HDC dc, int x, int y, image_render_ex *p)
{
    if (!p)
        return fimage_render(dc, img, x, y, NULL);
    else
       return fimage_render(dc, img, x, y, (fimage_render_ex*)p);
}
