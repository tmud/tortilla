#pragma once

class Ticker
{
    DWORD m_ticker;
public:
    Ticker() { sync(); }
    void sync() { m_ticker = GetTickCount(); }
    DWORD getDiff() const
    {
        DWORD diff = -1;
        DWORD tick = GetTickCount();
        if (tick >= m_ticker)
            diff = tick - m_ticker;
        else
        {   // overflow 49.7 days (MSDN GetTickCount)
            diff = diff - m_ticker;
            diff = diff + tick + 1;
        }
        return diff;
    }
};

class TempDC
{
public:
    TempDC() : m_old_bitmap(NULL) {}
    ~TempDC() { clear(); }
    void create(HDC dc, SIZE sz)
    {
        clear();
        m_temp_dc.CreateCompatibleDC(dc);
        m_temp_bitmap.CreateCompatibleBitmap(dc, sz.cx, sz.cy);
        m_old_bitmap = m_temp_dc.SelectBitmap(m_temp_bitmap);
    }
    void destroy() { clear(); }
    operator HDC() const { return m_temp_dc; }

private:
    void clear() {
        if (m_temp_bitmap != NULL) {
            m_temp_dc.SelectBitmap(m_old_bitmap);
            m_temp_bitmap.DeleteObject();
            m_temp_dc.DeleteDC();
        }
        m_temp_bitmap = NULL;
    }
    CDC m_temp_dc;
    CBitmap m_temp_bitmap;
    HBITMAP m_old_bitmap;
};
