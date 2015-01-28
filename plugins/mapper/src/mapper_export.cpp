#include "stdafx.h"
#include "../res/resource.h"
#include "properties.h"
#include "mapperSettings.h"
#include "mapper.h"

bool map_active = false;
PropertiesMapper m_props;
luaT_window m_parent_window;
Mapper* m_mapper_window = NULL;
//-------------------------------------------------------------------------
int get_name(lua_State *L) 
{
    lua_pushstring(L, "Карта(map)");
    return 1;
}

int get_description(lua_State *L) 
{
    lua_pushstring(L, "Отображает схему комнат и выходов. Показывает местоположение игрока.");
    return 1;
}

int get_version(lua_State *L)
{
    lua_pushstring(L, "1.0 dev");
    return 1;
}

int init(lua_State *L)
{
    luaT_run(L, "addMenu", "sddd", "Карта/Окно с картой", 1, 2, IDB_MAP);
    luaT_run(L, "addMenu", "s", "Карта/-");
    luaT_run(L, "addMenu", "sdd", "Карта/Настройка карты...", 2, 2);
    luaT_run(L, "addButton", "dds", IDB_MAP, 2, "Настройка карты");

    luaT_run(L, "getPath", "s", "config.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    m_props.initAllDefault();

    xml::node ld;
    if (ld.load(path.c_str()))
    {
        ld.get("darkroom/label", &m_props.dark_room);
        ld.get("name/begin", &m_props.begin_name);
        ld.get("name/end", &m_props.end_name);
        ld.get("descr/begin", &m_props.begin_descr);
        ld.get("descr/end", &m_props.end_descr);
        ld.get("exits/begin", &m_props.begin_exits);
        ld.get("exits/end", &m_props.end_exits);
        ld.get("prompt/begin", &m_props.begin_prompt);
        ld.get("prompt/end", &m_props.end_prompt);
        
        if (ld.move("dirs"))
        {
            ld.get("north", &m_props.north_exit);
            ld.get("south", &m_props.south_exit);
            ld.get("west", &m_props.west_exit);
            ld.get("east", &m_props.east_exit);
            ld.get("up", &m_props.up_exit);
            ld.get("down", &m_props.down_exit);
        }
        if (ld.move("/cmds"))
        {
            ld.get("north", &m_props.north_cmd);
            ld.get("south", &m_props.south_cmd);
            ld.get("west", &m_props.west_cmd);
            ld.get("east", &m_props.east_cmd);
            ld.get("up", &m_props.up_cmd);
            ld.get("down", &m_props.down_cmd);
        }
    }
    ld.deletenode();

	if (!m_parent_window.create(L, "Карта", 400, 400))
		return luaT_error(L, "Не удалось создать окно для карты");

    HWND parent = m_parent_window.hwnd();    
    map_active = m_parent_window.isvisible();

    m_mapper_window = new Mapper(&m_props);
    RECT rc; ::GetClientRect(parent, &rc);
    if (rc.right == 0) rc.right = 400; // requeires for splitter inside map window (if parent window hidden)
    HWND res = m_mapper_window->Create(parent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);  
    m_parent_window.attach(res);
 
    if (map_active)
        luaT_run(L, "checkMenu", "d", 1);

    m_mapper_window->loadMaps(L);
    return 0;
}

int release(lua_State *L)
{
    m_mapper_window->saveMaps(L);
           
    xml::node s("mapper");
    s.set("darkroom/label", m_props.dark_room);
    s.set("name/begin", m_props.begin_name);
    s.set("name/end", m_props.end_name);
    s.set("descr/begin", m_props.begin_descr);
    s.set("descr/end", m_props.end_descr);
    s.set("exits/begin", m_props.begin_exits);
    s.set("exits/end", m_props.end_exits);
    s.set("prompt/begin", m_props.begin_prompt);
    s.set("prompt/end", m_props.end_prompt);
    s.create("dirs");
    s.set("north", m_props.north_exit);
    s.set("south", m_props.south_exit);
    s.set("west", m_props.west_exit);
    s.set("east", m_props.east_exit);
    s.set("up", m_props.up_exit);
    s.set("down", m_props.down_exit);
    s.create("/cmds");
    s.set("north", m_props.north_cmd);
    s.set("south", m_props.south_cmd);
    s.set("west", m_props.west_cmd);
    s.set("east", m_props.east_cmd);
    s.set("up", m_props.up_cmd);
    s.set("down", m_props.down_cmd);
    s.move("/");

    luaT_run(L, "getPath", "s", "config.xml");
    u8string path(lua_tostring(L, -1));
    lua_pop(L, 1);

    if (!s.save(path.c_str()))
       return luaT_error(L, "Ошибка записи настроек карты: config.xml");
    s.deletenode();
    return 0;
}

int menucmd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    int id = lua_tointeger(L, 1);
    lua_pop(L, 1);
    if (id == 2)
    {
        PropertiesMapper props(m_props);
        MapperSettings ms(&props);
        if (ms.DoModal() == IDOK)
        {
            m_props = props;
            m_mapper_window->updateProps();
        }
    }
    if (id == 1)
    {
        if (map_active)
        {
            luaT_run(L, "uncheckMenu", "d", 1);
            m_parent_window.hide();
        }
        else
        {
            luaT_run(L, "checkMenu", "d", 1);
            m_parent_window.show();
        }
        map_active = !map_active;
    }
    return 0;
}

int closewindow(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    HWND hwnd = (HWND)lua_tointeger(L, 1);
    if (hwnd == m_parent_window.hwnd())
    {
        luaT_run(L, "uncheckMenu", "d", 1);
        m_parent_window.hide();
        map_active = false;
    }
    return 0;
}

int stream(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        const char *stream = lua_tostring(L, -1);
        const wchar_t *wstream = convert_utf8_to_wide (stream);
        m_mapper_window->processNetworkData(wstream, wcslen(wstream));
    }    
    return 1;
}

int gamecmd(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        const char *cmd = lua_tostring(L, -1);
        const wchar_t *wcmd = convert_utf8_to_wide(cmd);
        m_mapper_window->processCmd(wcmd, wcslen(wcmd));
    }
    return 1;
}

static const luaL_Reg mapper_methods[] = 
{
    {"name", get_name},
    {"description", get_description},
    {"version", get_version},
    {"init", init },
    {"release", release },
    {"menucmd", menucmd },
    {"closewindow", closewindow },
    {"streamdata", stream },
    {"gamecmd", gamecmd },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, mapper_methods);
    lua_setglobal(L, "mapper");
    return 0;
}

CAppModule _Module;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        _Module.Init(NULL, hModule);
        break;
    case DLL_PROCESS_DETACH:
        delete m_mapper_window;
        _Module.Term();        
        break;
    }
    return TRUE;
}
