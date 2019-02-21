#pragma once

class PluginsMouseHandler
{
public:
    PluginsMouseHandler(lua_State *pL);
    ~PluginsMouseHandler();
    bool event(UINT msg, WPARAM wparam, LPARAM lparam);
private:
    POINT point(LPARAM lparam);
    void click(UINT msg, POINT pos);
    void move(POINT pos);
    void leave();
    void wheel(const char* direction);
    void run(const char* method, int params);
    lua_State *L;
    lua_ref m_mousehandler_ref;
};
