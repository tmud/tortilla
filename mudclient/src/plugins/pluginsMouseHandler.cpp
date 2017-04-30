#include "stdafx.h"
#include "pluginsMouseHandler.h"
#include "pluginsApi.h"

PluginsMouseHandler::PluginsMouseHandler(lua_State *pL) : L(pL)
{
    assert(L);
    assert(lua_istable(L, -1));
    m_mousehandler_ref.createRef(L);
}

PluginsMouseHandler::~PluginsMouseHandler()
{
    m_mousehandler_ref.unref(L);
}

bool PluginsMouseHandler::event(UINT msg, WPARAM wparam, LPARAM lparam)
{
    bool handled = false;
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONUP:
      click( msg, point(lparam) );
      handled = true;
    break;
    case WM_MOUSEMOVE:
      move( point(lparam) );
      handled = true;
    break;
    case WM_MOUSELEAVE:
      leave();
      handled = true;
    break;
    case WM_MOUSEWHEEL:
    {  
        DWORD position = HIWORD(wparam);
        const char* direction = (position & 0x8000) ? "down" : "up";
        wheel(direction);
        handled = true;
        break;
    }
   }
   return handled;
}

void PluginsMouseHandler::wheel(const char* direction)
{
    lua_pushstring(L, direction);
    run("wheel", 1);
}

void PluginsMouseHandler::click(UINT msg, POINT pos)
{
    const char *e = NULL;
    switch(msg){
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_MBUTTONDOWN:
          e = "down";
      break;
      case WM_LBUTTONDBLCLK:
      case WM_RBUTTONDBLCLK:
      case WM_MBUTTONDBLCLK:
          e = "double";
      break;
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP:
          e = "up";
      break;
    }
    if (!e) {
        assert(false);
        return;
    }
    const char *b = NULL;
    switch(msg){
      case WM_LBUTTONDOWN:
      case WM_LBUTTONDBLCLK:
      case WM_LBUTTONUP:
          b = "left";
      break;      
      case WM_RBUTTONDOWN:
      case WM_RBUTTONDBLCLK:
      case WM_RBUTTONUP:
          b = "right";
      break;
      case WM_MBUTTONDOWN:
      case WM_MBUTTONDBLCLK:
      case WM_MBUTTONUP:
          b = "middle";
      break;
    }
    if (!b) {
        assert(false);
        return;
    }
    lua_pushstring(L, e);
    lua_pushinteger(L, pos.x);
    lua_pushinteger(L, pos.y);
    run(b, 3);
}

void PluginsMouseHandler::move(POINT pos)
{
    lua_pushinteger(L, pos.x);
    lua_pushinteger(L, pos.y);
    run("move", 2);
}

void PluginsMouseHandler::leave()
{
    run("leave", 0);
}

void PluginsMouseHandler::run(const char* method, int params)
{
    m_mousehandler_ref.pushValue(L);
    lua_pushstring(L, method);
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1))
    {
        lua_insert(L, -(params + 2));
        lua_pop(L, 1);
        if (lua_pcall(L, params, 0, 0))
        {
            // error in call
            TA2W m(method);
            if (luaT_check(L, 1, LUA_TSTRING))
                pluginMethodError(m, lua_toerror(L));
            else
                pluginMethodError(m, L"неизвестная ошибка");
            lua_settop(L, 0);
        }
    }
    else
    {
        lua_pop(L, params+2);
    }
}

POINT PluginsMouseHandler::point(LPARAM lparam) 
{
    POINT p; p.x = GET_X_LPARAM(lparam); p.y = GET_Y_LPARAM(lparam);
    return p;
}
