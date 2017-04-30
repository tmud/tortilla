#include "resource.h"
#include "padbutton.h"
#include "settingsDlg.h"
#include "selectImageDlg.h"

class ClickpadSettings
{
public:
    virtual void setColumns(int count) = 0;
    virtual int  getColumns() const = 0;
    virtual void setRows(int count) = 0;
    virtual int  getRows() const = 0;
    virtual void setButtonSize(int size) = 0;
    virtual int  getButtonSize() const = 0;
    virtual void getFont(LOGFONT* font) const = 0;
    virtual void setFont(LOGFONT font) = 0;
};

struct ButtonParams
{
    ButtonParams() : imagex(0), imagey(0), templ(false), update(0) {}
    std::wstring text;
    std::wstring cmd;
    std::wstring tooltip;
    std::wstring imagefile;
    int imagex, imagey;
    bool templ;
    enum { TEXT = 1, CMD = 2, TOOLTIP = 4, IMAGE = 8, IMAGEXY = 16, TEMPLATE = 32 };
    DWORD update;
};

class ClickpadMainWnd : public CWindowImpl < ClickpadMainWnd >, public ClickpadSettings, 
            public CToolTipDialog<ClickpadMainWnd>
{
public:
    DECLARE_WND_CLASS_EX(L"Clickpad", 0, COLOR_BTNFACE)
    ClickpadMainWnd();
    ~ClickpadMainWnd();
    void setEditMode(bool mode);
    void save(xml::node& node);
    void load(xml::node& node);
    void updated();

    bool showRowsColumns(int rows, int columns);
    bool setButton(int row, int column, const ButtonParams& p);
    bool updateButton(int row, int column, const ButtonParams& p);
    bool getButton(int row, int column, ButtonParams* p);
    bool clearButton(int row, int column);

private:
    BEGIN_MSG_MAP(ClickpadMainWnd)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_USER, OnClickButton)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_USER+1, OnSetSize)
        CHAIN_MSG_MAP(CToolTipDialog<ClickpadMainWnd>)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { onDestroy(); return 0; }        
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    LRESULT OnMouseMove(UINT, WPARAM wparam, LPARAM lparam, BOOL&) {
        return 0;
    }
    LRESULT OnClickButton(UINT, WPARAM wparam, LPARAM lparam, BOOL&) {
        onClickButton(LOWORD(wparam), HIWORD(wparam), (lparam==0) ? false : true);
        return 0;
    }
    LRESULT OnEraseBackground(UINT, WPARAM, LPARAM, BOOL&) { return 1; }
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&) { 
        CPaintDC dc(m_hWnd);
        onPaint(dc);
        return 0;
    }
    LRESULT OnSetSize(UINT, WPARAM, LPARAM, BOOL&) { setWorkWindowSize(); return 0; }
    void onCreate();
    void onDestroy();
    void onSize();
    void onClickButton(int x, int y, bool up);
    void onPaint(HDC dc);
private:
    PadButton* getButton(int x, int y);
    void showButton(PadButton*b, bool show);
    void updateTooltip(PadButton*b, bool show);
    void setWorkWindowSize();
    void setColumns(int count);
    int  getColumns() const;
    void setRows(int count);
    int  getRows() const;
    void setButtonSize(int size);
    int  getButtonSize() const;
    void showRows(int count);
    void showColumns(int count);
    void getFont(LOGFONT* font) const;
    void setFont(LOGFONT font);
    void initLogFont(LOGFONT *f);
    bool checkRowColumn(int row, int column);
private:
    bool m_editmode;
    int m_button_size, m_rows, m_columns;
    std::vector<PadButton*> m_buttons;
    COLORREF m_backgroundColor;
    LOGFONT m_logfont;
    CFont m_buttons_font;
};
