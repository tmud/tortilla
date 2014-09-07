#pragma once

#include "../propertiesPages/propertyList.h"
#include "../propertiesPages/propertiesData.h"
#include "../logicProcessor.h"
#include "pluginsView.h"

class Plugin
{
public:
    Plugin() : empty(0), hModule(NULL), load_state(false), current_state(false) {}
    static bool isPlugin(const wchar_t* fname);
    bool loadPlugin(const wchar_t* fname);
    void unloadPlugin();
    bool reloadPlugin();
    void setOn(bool on);
    bool state() const { return current_state; }
    bool isloaded() const { return load_state; }
    void menuCmd(int id);
    HMODULE getModule() const { return hModule; }
    void closeWindow(HWND wnd);
    bool runMethod(const char* method, int args, int results);

    enum PluginState { FILE, FILENAME, NAME, VERSION, DESCRIPTION };
    const wchar_t* get(PluginState state) {
        switch(state) {
        case FILE: return file.c_str();
        case NAME: return name.c_str();
        case FILENAME: return filename.c_str();
        case VERSION: return version.c_str();
        case DESCRIPTION: return description.c_str();
        }
        return &empty; 
    }
    
    std::vector<PluginsView*> views;
    std::vector<tstring> menus;
    std::vector<int> buttons;
    std::vector<tstring> toolbars;
    std::vector<tstring> commands;

private:
    bool loadDllPlugin(const wchar_t* fname);
    bool loadLuaPlugin(const wchar_t* fname);
    bool initLoadedPlugin(const wchar_t* fname);
    void getparam(const char* state, tstring* value);
        
private:
    wchar_t empty;
    HMODULE hModule;            // dll module
    bool    load_state;
    bool    current_state;
    std::string module;         // name of module
    tstring file;
    tstring filename;           // name without extension
    tstring name;
    tstring version;
    tstring description;
};

typedef std::vector<Plugin*> PluginsList;
