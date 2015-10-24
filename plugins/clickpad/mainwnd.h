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
};

class ClickpadMainWnd : public CWindowImpl < ClickpadMainWnd >, public ClickpadSettings
{
public:
    DECLARE_WND_CLASS_EX(L"Clickpad", 0, COLOR_BTNFACE)
    ClickpadMainWnd();
    ~ClickpadMainWnd();
    void setEditMode(bool mode);
    void save(xml::node& node);
    void load(xml::node& node);
    void updated();
private:
    BEGIN_MSG_MAP(ClickpadMainWnd)
        MESSAGE_HANDLER(WM_USER, OnClickButton)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_USER+1, OnSetSize)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { onDestroy(); return 0; }        
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
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
    void showButton(int x, int y, bool show);
    void setWorkWindowSize();
    void setColumns(int count);
    int  getColumns() const;
    void setRows(int count);
    int  getRows() const;
    void setButtonSize(int size);
    int  getButtonSize() const;
    void showRows(int count);
    void showColumns(int count);
private:
    bool m_editmode;
    int m_button_size, m_rows, m_columns;
    std::vector<PadButton*> m_buttons;
    COLORREF m_backgroundColor;
};
