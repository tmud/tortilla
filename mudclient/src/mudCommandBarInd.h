#pragma once

class MudCommandBarIndicator : public CWindowImpl<MudCommandBarIndicator, CWindow>
{
    tchar buffer[16];
    int buffer_len;
    bool m_status;
    CFont m_font;
    static CPen m_pen1, m_pen2;
    static CBrush m_brush1, m_brush2;
    int width,height;
public:
    DECLARE_WND_CLASS(NULL)
    MudCommandBarIndicator() : buffer_len(0), m_status(0), width(0),height(0) {}
    void setText(const tstring& text) {
        buffer_len = min(text.length(), 15);
        wcsncpy(buffer, text.c_str(), buffer_len);        
        calcWidth();
        Invalidate();
    }
    void setStatus(bool status) {
        if (m_status == status)
            return;
        m_status = status;
        Invalidate();
    }
    int getWidth() const {
        return width;
    }
    void setFont(HFONT font) {

        CFontHandle h(font);
        LOGFONT lf;
        h.GetLogFont(&lf);
        createFont(lf.lfFaceName);
        calcWidth();
        Invalidate();
    }
private:
    BEGIN_MSG_MAP(MudCommandBarIndicator)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBknd)
    END_MSG_MAP()
    void onCreate();
    void onPaint();
    void calcWidth();
    void createFont(const tstring& fontname);
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled) {  onCreate(); return 0; }
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL& bHandled) { onPaint(); return 0; }
    LRESULT OnEraseBknd(UINT, WPARAM, LPARAM, BOOL&) { return 1; }    
};
