#pragma once

class BassObjectEvents
{
public:
    virtual void playingEnd() = 0;
};

float volume_ToFloat(int volume);
int volume_toInt(float volume);
