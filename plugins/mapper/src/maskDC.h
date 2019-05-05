#pragma once

class MaskDC : public CDC
{
public:
    // Data members
	HDC m_hDCOriginal;
    int m_width, m_height;
	CBitmap m_bmp;
	HBITMAP m_hBmpOld;

    MaskDC(HDC srcDC, int width, int height) : m_hDCOriginal(srcDC), m_width(width), m_height(height)
    {
        CreateCompatibleDC(m_hDCOriginal);
        ATLASSERT(m_hDC != NULL);

        // Create the monochrome DIB section with a black and white palette
		struct RGBBWBITMAPINFO
		{
			BITMAPINFOHEADER bmiHeader; 
			RGBQUAD bmiColors[2]; 
		};
		RGBBWBITMAPINFO rgbBWBitmapInfo = 
		{
			{ sizeof(BITMAPINFOHEADER), m_width, m_height, 1, 1, BI_RGB, 0, 0, 0, 0, 0 },
			{ { 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 } }
		};

        void* pbitsBW;
		m_bmp = CreateDIBSection(m_hDC, (LPBITMAPINFO)&rgbBWBitmapInfo, DIB_RGB_COLORS, &pbitsBW, NULL, 0);
		ATLASSERT(m_bmp.m_hBitmap != NULL);
        
        m_hBmpOld = SelectBitmap(m_bmp);                
    }

    void makeMask(int x, int y)
    {
        BitBlt(0, 0, m_width, m_height, m_hDCOriginal, x, y, SRCCOPY);
    }

    ~MaskDC()
    {
        SelectBitmap(m_hBmpOld);
    }
};
