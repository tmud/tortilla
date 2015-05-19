#include "stdafx.h"
#include "cimg/CImg.h"
using namespace cimg_library;

template<class T>
class IndexedResourceManager
{
public:
    template <class CompareFunctor> 
    int find(CompareFunctor c)
    {
        for (int i=0,e=m_elements.size();i<e;++i)
        {
            if ( c(m_elements[i]) )
                return i;
        }
        return -1;
    }
    int add(T* el)
    {
        if (!m_free_ids.empty()) {
            int last = m_free_ids.size() - 1;
            int id = m_free_ids[last];
            m_free_ids.pop_back();
            m_elements[id] = el;
            return id;
        }
        int id = m_elements.size();
        m_elements.push_back(el);
        return id;
    }
    bool del(int id)
    {
        int count = m_elements.size();
        if (id >= 0 && id < count)
        {
            T* obj = m_elements[id];
            delete obj;
            m_elements[id] = NULL;
            m_free_ids.push_back(id);
            return true;
        }
        return false;
    }
    T* get(int id) const
    {
        int count = m_elements.size();
        if (id >= 0 && id < count)
        {
            return m_elements[id];
        }
        return NULL;
    }

private:
    std::vector<T*> m_elements;
    std::vector<int> m_free_ids;
};

class CImgDisplayEx : public CImgDisplay
{
public:
    CImgDisplayEx() {}
    void attach_hwnd(HWND window) {
        _window = window;
    }
    HWND get_hwnd() const {
        return _window;
    }
};

typedef CImg<unsigned char> CImgEx;

class CImgLib
{
public:
    CImgLib() {}
    ~CImgLib() {}
    int createDisplay(HWND wnd)
    {
        if (::IsWindow(wnd))
            return -1;
        class functor
        { HWND hwnd;
          public: functor(HWND wnd) : hwnd(wnd) {}
            bool operator()(CImgDisplayEx *w) {
                return (w->get_hwnd() == hwnd ) ? true : false;
            }
        } f(wnd);        
        int id = m_displays.find(f);
        if (id == -1)
            id = m_displays.add(new CImgDisplayEx());
        return id;
    }

    bool destroyDisplay(int display)
    {        
        return m_displays.del(display);
    }

    CImgDisplayEx* getDisplay(int display)
    {
        return m_displays.get(display);
    }

    int loadImage(const char *file, u8string *error)
    {
        CImgEx *image = new CImgEx();
        try {
        image->load(file);
        }
        catch (CImgArgumentException& e)
        {
            delete image;
            if (error)
                error->assign(e.what());
            return -1;
        }
        return m_images.add(image);
    }

    bool delImage(int image)
    {
        return m_images.del(image);
    }

    CImgEx* getImage(int image)
    {
        return m_images.get(image);
    }

private:
    IndexedResourceManager<CImgDisplayEx> m_displays;
    IndexedResourceManager<CImgEx> m_images;

} m_cimg_lib;

int error(lua_State *L, const utf8* method, const utf8* msg)
{
    u8string text("Error! ");
    text.append(msg);
    text.append(" in method: 'cimg.");
    text.append(method);
    text.append("'.");
    lua_pushnil(L);
    lua_pushstring(L, text.c_str());
    return 2;
}

int error_invargs(lua_State *L, const utf8* method)
{
    return error(L, method, "Invalid arguments");
}

int cimg_createDisplay(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        HWND hwnd = reinterpret_cast<HWND>(lua_tounsigned(L, 1));
        int id = m_cimg_lib.createDisplay(hwnd);
        if (id != -1)
        {
            lua_pushinteger(L, id);   
            return 1;
        }
        return error(L, "createDisplay", "Incorrect window handle");
    }
    return error_invargs(L, "createDisplay");
}

int cimg_destroyDisplay(lua_State *L)
{   
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int id = lua_tointeger(L, 1);
        bool result = m_cimg_lib.destroyDisplay(id);
        if (!result)
            return error(L, "destroyDisplay", "Incorrect display id");
        lua_pushboolean(L, 1);
        return 1;
    }
    return error_invargs(L, "destroyDisplay");
}

int cimg_load(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        u8string errmsg;
        int id = m_cimg_lib.loadImage(lua_tostring(L, 1), &errmsg);
        if (id < 0)
            return error(L, "load", errmsg.c_str());
        lua_pushinteger(L, id);
        return 1;        
    }    
    return error_invargs(L, "load");
}

int cimg_unload(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int image = lua_tointeger(L, 1);
        bool result = m_cimg_lib.delImage(image);
        if (!result)
            return error(L, "unload", "Incorrect image id");
        lua_pushboolean(L, 1);
        return 1;
    }
    return error_invargs(L, "unload");
}

int cimg_render(lua_State *L)
{
    if (lua_gettop(L) == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        CImgDisplayEx *display = m_cimg_lib.getDisplay(lua_tointeger(L, 1));
        if (!display)
            return error(L, "render", "Incorrect display");
        CImgEx *image = m_cimg_lib.getImage(lua_tointeger(L, 2));
        if (!image)
            return error(L, "render", "Incorrect image");
        display->render(*image);
        lua_pushboolean(L, 1);
        return 1;
    }
    return error_invargs(L, "render");
}

static const luaL_Reg cimg_methods[] =
{
    { "createDisplay", cimg_createDisplay },
    { "destroyDisplay", cimg_destroyDisplay },
    { "load", cimg_load },
    { "unload", cimg_unload },
    { "render", cimg_render },
    { NULL, NULL }
};

int luaopen_cimg(lua_State *L)
{
    luaL_newlib(L, cimg_methods);
    return 1;
}
