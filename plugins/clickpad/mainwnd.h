#include "resource.h"
#include "padbutton.h"
#include "settingsDlg.h"

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
    HWND createSettingsDlg(HWND parent);
    void setEditMode(bool mode);
    void save(xml::node& node);
    void load(xml::node& node);

private:
    BEGIN_MSG_MAP(ClickpadMainWnd)
        MESSAGE_HANDLER(WM_USER, OnClickButton)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { onDestroy(); return 0; }        
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    LRESULT OnClickButton(UINT, WPARAM wparam, LPARAM lparam, BOOL&) {
        onClickButton(LOWORD(wparam), HIWORD(wparam), (lparam==0) ? false : true);
        return 0;
    }
    void onCreate();
    void onDestroy();
    void onSize();
    void onClickButton(int x, int y, bool up);

private:
    void createButton(int x, int y);
    void setWorkWindowSize();

private:
    void setColumns(int count);
    int  getColumns() const;
    void setRows(int count);
    int  getRows() const;
    void setButtonSize(int size);
    int  getButtonSize() const;
    void setRowsInArray(int count);
    void setColumnsInArray(int count);

private:
    SettingsDlg *m_settings_dlg;
    bool m_editmode;
    int m_button_size, m_rows, m_columns;
    std::vector<std::vector<PadButton*>> m_buttons;
};
