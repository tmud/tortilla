#pragma once

struct SharingHeader 
{
    int messages;
};

struct SharingWindow 
{
  SharingWindow() : x(0), y(0), w(0), h(0) {}
  int x, y, w, h;
};
