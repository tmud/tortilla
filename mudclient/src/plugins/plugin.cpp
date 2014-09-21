#include "stdafx.h"
#include "api/api.h"
#include "plugin.h"

typedef int (WINAPI *plugin_open)(lua_State *L);
extern luaT_State L;
Plugin* _cp = NULL; // current plugin in lua methods

#include "pluginSupport.h"
#include "pluginsApi.h"

bool Plugin::isPlugin(const wchar_t* fname)
{
    const wchar_t *e = wcsrchr(fname, L'.');
    if (!e) return false;
    tstring ext(e+1);
    return (ext == L"lua" || ext == L"dll") ? true : false;
}

bool Plugin::loadPlugin(const wchar_t* fname)
{
    const wchar_t *e = wcsrchr(fname, L'.');
    if (!e)
        return false;
    bool result = false;
    tstring ext(e+1);
    if (ext == L"lua") 
        result = loadLuaPlugin(fname);
    else if (ext == L"dll")
        result = loadDllPlugin(fname);
    current_state = false;
    load_state = result;
    return result;
}

void Plugin::unloadPlugin()
{
    setOn(false);
    lua_pushnil(L);               // clear module in lua
    lua_setglobal(L, module.c_str());
    lua_gc(L, LUA_GCCOLLECT, 0);
    if (hModule)
        { FreeLibrary(hModule);  hModule = NULL; }
    load_state = false;
}

bool Plugin::reloadPlugin()
{
    if (file.empty())
        return false;    
    const wchar_t* fname = file.c_str();
    bool result = false;
    const wchar_t *e = wcsrchr(fname, L'.');
    if (!e) return false;
    tstring ext(e+1);
    if (ext == L"lua") 
        result = loadLuaPlugin(fname);
    else if (ext == L"dll")
        result = loadDllPlugin(fname);
    setOn(result);
    load_state = result;
    return result;
}

void pluginDeleteResources(Plugin *plugin);
void Plugin::setOn(bool on)
{
    if (!current_state && on) {
        current_state = true;
        runMethod("init", 0, 0);
    }
    else if (current_state && !on) {
        current_state = false;
        runMethod("release", 0, 0);
        pluginDeleteResources(this);
    }
}

void Plugin::menuCmd(int id)
{
    lua_pushinteger(L, id);
    runMethod("menucmd", 1, 0);
}

//close button on plugins pane
void Plugin::closeWindow(HWND wnd)
{
    lua_pushinteger(L, (int)wnd);
    runMethod("closewindow", 1, 0);
}

bool Plugin::runMethod(const char* method, int args, int results)
{
    lua_getglobal(L, module.c_str());
    if (!lua_istable(L, -1))
        { lua_pop(L, 1); return false; }
    lua_pushstring(L, method);
    lua_gettable(L, -2);
    if (!lua_isfunction(L, -1)) // not supported function in plugin
    {
        lua_pop(L, 2);
        return true;
    }
    lua_insert(L, -(args + 2));
    lua_pop(L, 1);
    Plugin *old = _cp;
    _cp = this;
    if (lua_pcall(L, args, results, 0))
    {
        // error in call
        if (luaT_check(L, 1, LUA_TSTRING))
            pluginError(L, method, lua_tostring(L, -1));
        else
            pluginError(L, method, "неизвестная ошибка");
        _cp = old;
        return false;
    }
    _cp = old;
    return true;
}

void Plugin::getparam(const char* state, tstring* value)
{
    if (!runMethod(state, 0, 1))
        return;
    if (!lua_isstring(L, -1))
        { lua_pop(L,1); value->clear(); return; }
    Utf8ToWide u2w;
    u2w.convert(lua_tostring(L, -1));
    lua_pop(L, 1);
    value->assign(u2w);
}

bool Plugin::loadDllPlugin(const wchar_t* fname)
{
    tstring plugin_file(L"plugins\\");
    plugin_file.append(fname);

    HMODULE hmod = LoadLibrary(plugin_file.c_str());
    if (!hmod)
        return false;

    bool loaded = false;
    plugin_open popen = (plugin_open)GetProcAddress(hmod, "plugin_open");
    if (popen)
    {
        popen(L);
        loaded = initLoadedPlugin(fname);
        if (loaded)
            hModule = hmod;
    }
    if (!loaded)
        FreeLibrary(hmod);
    return loaded;
}

bool Plugin::loadLuaPlugin(const wchar_t* fname)
{
    tstring plugin_file(L"plugins\\");
    plugin_file.append(fname);

    HANDLE hfile = CreateFile(plugin_file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hfile == INVALID_HANDLE_VALUE)
        return false;

    struct autoclose { HANDLE h;  
        autoclose(HANDLE file) : h(file) {}
        ~autoclose() { CloseHandle(h); }
    } _ac(hfile);

    DWORD high = 0;
    DWORD size = GetFileSize(hfile, &high);
    if (high != 0 || size < 3)
        return false;

    DWORD readed = 0;
    MemoryBuffer script(size+1);
    if (ReadFile(hfile, script.getData(), size, &readed, NULL))
    {
        unsigned char *p = (unsigned char *)script.getData();
        p[size] = 0;                                      // set EOF
        if (p[0] == 0xef && p[1] == 0xbb && p[2] == 0xbf) // check BOM
            p = p + 3;
        luaL_dostring(L, (const char*)p);
        return initLoadedPlugin(fname);
    }
    return false;
}

bool Plugin::initLoadedPlugin(const wchar_t* fname)
{
    file = fname;
    const wchar_t *ext = wcsrchr(fname, L'.');
    filename.assign(fname, ext - fname);
    module = convert_wide_to_ansi(filename.c_str());

    lua_getglobal(L, module.c_str());
    bool loaded = lua_istable(L, -1);
    lua_pop(L, 1);

    if (loaded)
    {
        getparam("name", &name);
        getparam("version", &version);
        getparam("description", &description);
        if (name.empty() || version.empty())
            loaded = false;
    }
    return loaded;
}
