#pragma once

class Palette256
{
public:
    Palette256(PropertiesData* data)
    {
        updateProps(data);
        for (tbyte r=0;r<6;++r) {
        for (tbyte g=0;g<6;++g) {
        for (tbyte b=0;b<6;++b) {
        int index = 16+r*36+g*6+b;
            xterm_colors[index] = RGB(r*50,g*50,b*50);
        }}}
        for (int i=232; i <= 255; ++i)
        {
            tbyte b = (tbyte)i;
            tbyte c = (b - 232) * 10 + 8;
            xterm_colors[i] = RGB(c, c, c);
        }
    }

    COLORREF getColor(tbyte color) const
    {
        return xterm_colors[color];
    }

    void updateProps(PropertiesData* data)
    {
        assert(data);
        for (tbyte i=0; i<=15; ++i)
            xterm_colors[i] = (data->osc_flags[i]) ? data->osc_colors[i] : data->colors[i];
    }

    void setColor(tbyte color, COLORREF value)
    {
        xterm_colors[color] = value;
    }

private:
    COLORREF xterm_colors[256];
};
