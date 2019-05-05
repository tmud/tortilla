#include "stdafx.h"
#include "common.h"

float volume_ToFloat(int volume)
{
    float v = static_cast<float>(volume);
    v = v / 100.0f;
    v = v + 0.005f;     // компенсация погрешности окгругления
    if (v < 0) v = 0;
    if (v > 1) v = 1;
    return v;
}

int volume_toInt(float volume)
{
    int v = static_cast<int>(volume * 100);
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    return v;
}
