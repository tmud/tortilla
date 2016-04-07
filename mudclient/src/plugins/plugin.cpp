#include "stdafx.h"
#include "api/api.h"
#include "plugin.h"

typedef int (WINAPI *plugin_open)(lua_State *L);
extern luaT_State L;
extern wchar_t* plugin_buffer();
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
    if (isAlreadyLoaded(fname))
        return false;
    const wchar_t *e = wcsrchr(fname, L'.');
    if (!e) return false;
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
    Plugin *old = _cp;
    _cp = this;
    tstring ext(e+1);
    if (ext == L"lua")
        result = loadLuaPlugin(fname);
    else if (ext == L"dll")
        result = loadDllPlugin(fname);
    _cp = old;
    setOn(result);
    load_state = result;
    return result;
}

void Plugin::updateProps()
{
    for (int i = 0, e = dockpanes.size(); i < e; ++i)
    {
        CWindow wnd(*dockpanes[i]);
        wnd.Invalidate();
    }
    for (int i = 0, e = panels.size(); i < e; ++i)
    {
        CWindow wnd(*panels[i]);
        wnd.Invalidate();
    }
}

void pluginDeleteResources(Plugin *plugin);
void Plugin::setOn(bool on)
{
    if (!current_state && on) {
        error_state = false;
        for (int i = 0, e = dockpanes.size(); i < e; ++i)
            dockpanes[i]->resetRenderErrorState();
        for (int i = 0, e = panels.size(); i < e; ++i)
            panels[i]->resetRenderErrorState();
        if (!runMethod("init", 0, 0))
            error_state = true;
        current_state = true;
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

bool Plugin::runMethod(const char* method, int args, int results, bool *not_supported)
{
    lua_getglobal(L, module.c_str());
    if (!lua_istable(L, -1))
        { lua_pop(L, 1); return false; }
    lua_pushstring(L, method);
    lua_gettable(L, -2);
    if (!lua_isfunction(L, -1)) // not supported function in plugin
    {
        lua_pop(L, 2);
        if (not_supported) *not_supported = true;
        return true;
    }
    lua_insert(L, -(args + 2));
    lua_pop(L, 1);
    Plugin *old = _cp;
    _cp = this;
    if (lua_pcall(L, args, results, 0))
    {
        // error in call
        TA2W m(method);
        if (luaT_check(L, 1, LUA_TSTRING))
            pluginMethodError(m, lua_toerror(L));
        else
            pluginMethodError(m, L"неизвестная ошибка");
        _cp = old;
        lua_settop(L, 0);
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
    tchar path[MAX_PATH+1];
    GetCurrentDirectory(MAX_PATH, path);

    tstring plugin_file(path);
    plugin_file.append(L"\\plugins\\");
    plugin_file.append(fname);

    HMODULE hmod = LoadLibrary(plugin_file.c_str());
    if (!hmod)
        return false;

    bool loaded = false;
    plugin_open popen = (plugin_open)GetProcAddress(hmod, "plugin_open");
    if (popen)
    {
        int res = (popen)(L);
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

    if (luaL_loadfile(L, TW2A(plugin_file.c_str())))
    {
        pluginLoadError(lua_toerror(L));
        lua_pop(L, 1);
        return false;
    }
    if (lua_pcall(L, 0, 1, 0))
    {
        pluginLoadError(lua_toerror(L));
        lua_pop(L, 1);
        return false;
    }
    return initLoadedPlugin(fname);
}

bool Plugin::initLoadedPlugin(const wchar_t* fname)
{
    file = fname;
    const wchar_t *ext = wcsrchr(fname, L'.');
    filename.assign(fname, ext - fname);
    module = TW2A(filename.c_str());

    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_setglobal(L, module.c_str());
        tstring error(fname);
        error.append(L":plugin_open() did not return a table.");
        pluginLoadError(error.c_str());
        return false;
    }

    lua_setglobal(L, module.c_str());

    bool loaded = isLoadedPlugin(filename.c_str());
    if (loaded)
    {
        getparam("name", &name);
        getparam("version", &version);
        getparam("description", &description);
        if (name.empty() || version.empty())
            loaded = false;
    }
    if (!loaded)
    {
        lua_pushnil(L);
        lua_setglobal(L,module.c_str());
    }
    return loaded;
}

bool Plugin::isAlreadyLoaded(const wchar_t* filename)
{
    const wchar_t *e = wcsrchr(filename, L'.');
    if (!e) return false;
    tstring module_name(filename, e);
    tstring ext(e+1);
    if (isLoadedPlugin(module_name.c_str()))
    {
        swprintf(plugin_buffer(), L"Плагин %s не загружен, так как место _G['%s'] занято модулем или другим плагином.", filename, 
            module_name.c_str());
        pluginOut(plugin_buffer());
        return true;
    }
    return false;
}

bool Plugin::isLoadedPlugin(const wchar_t* module_name)
{
    TW2A name(module_name);
    lua_getglobal(L, name);
    bool loaded = lua_istable(L, -1);
    lua_pop(L, 1);
    return loaded;
}
