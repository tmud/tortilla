#pragma once

class PluginsViewRender
{
public:
    PluginsViewRender(lua_State *pL, int index) : L(pL), m_render_func_index(index)
    , m_bkg_color(0)
    {
        assert(L && index > 0);
    }
    void render(HDC dc, int width, int height);
    void setBackground(COLORREF color);
    void clear(COLORREF color);
private:
    lua_State *L;
    int m_render_func_index;
    int m_width;
    int m_height;
    CDCHandle m_dc;
    COLORREF m_bkg_color;
};

void reg_pview_render_table(lua_State *L);
int  reg_pview_render(lua_State* L);

class PluginsView : public CWindowImpl<PluginsView>
{
    CWindow m_child_window;
    tstring m_plugin_name;
    PluginsViewRender *m_render;

public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND + 1)
    PluginsView(const tstring& plugin_name) : m_child_window(NULL), m_plugin_name(plugin_name), m_render(NULL) {}
    ~PluginsView() { delete m_render; }

    const tstring& getPluginName() const
    {
        return m_plugin_name;
    }

    void attachChild(HWND wnd)
    {
        m_child_window.Attach(wnd);
        RECT rc; GetClientRect(&rc);
        m_child_window.MoveWindow(&rc);
    }

    void setRender(lua_State *L, int render)
    {
        delete m_render;
        m_render = new PluginsViewRender(L, render);
    }

    void update()
    {
        if (m_child_window.IsWindow())
            Invalidate();
    }

    void setBackground(COLORREF color)
    {
        if (m_render)
            {  m_render->setBackground(color); Invalidate(); }
    }

private:
    BEGIN_MSG_MAP(PluginsView)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    END_MSG_MAP()
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&bHandled )
    {
        if (!m_child_window.IsWindow())
        {
            CPaintDC dc(m_hWnd);
            RECT rc; GetClientRect(&rc);            
            if (m_render)
                m_render->render(dc, rc.right, rc.bottom);
            else
                dc.FillSolidRect(&rc, 0);
            return 0;
        }
        bHandled = FALSE;
        return 0;
    }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&) 
    {
        if (m_child_window.IsWindow())
        {
            RECT rc; GetClientRect(&rc);
            m_child_window.MoveWindow(&rc);
        }
        return 0; 
    }
    LRESULT OnEraseBackground(UINT, WPARAM, LPARAM, BOOL&)
    {
        return 1; // handled, no background painting needed
    }
};

