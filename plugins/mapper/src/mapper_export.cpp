#include "stdafx.h"
#include "../res/resource.h"
#include "properties.h"
#include "mapperSettings.h"
#include "mapper.h"
#include "debugHelpers.h"

bool map_active = false;
PropertiesMapper m_props;
luaT_window m_parent_window;
Mapper* m_mapper_window = NULL;
//-------------------------------------------------------------------------
int get_name(lua_State *L) 
{
    luaT_pushwstring(L, L"Карта");
    return 1;
}

int get_description(lua_State *L) 
{    
    luaT_pushwstring(L, 
        L"Отображает схему комнат и выходов. Показывает местоположение игрока.\r\n"
        L"Предназначен для мадов с 6 стандартными выходами.\r\n"
        L"Требует для работы наличия VNUM комнаты в блоке текста с ее описанием,\r\n"
        L"или MSDP протокола, в котором есть информация о местоположении.\r\n"
        L"О настройке см. справку (#help mapper)."
        );
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.0");
    return 1;
}

int init(lua_State *L)
{
    DEBUGINIT(L);
	init_log(L);
    luaT_run(L, "addMenu", "sddd", L"Карта/Окно с картой", 1, 2, IDB_MAP);
    luaT_run(L, "addMenu", "s", L"Карта/-");
    luaT_run(L, "addMenu", "sdd", L"Карта/Настройка карты...", 2, 2);
    luaT_run(L, "addButton", "dds", IDB_MAP, 2, L"Настройка карты");

    m_props.initAllDefault();

	tstring error;

    tstring path;
    base::getPath(L, L"settings.xml", &path);
    xml::node p;
    if (p.load(path.c_str(), &error))
    {
        int usemsdp = 0;
        p.get(L"usemsdp/value", &usemsdp);
        m_props.use_msdp = (usemsdp == 1) ? true : false;
        int width = 0;
        p.get(L"zoneslist/width", &width);
        m_props.zoneslist_width = (width > 0) ? width : -1;
	} else {
		if (!error.empty())
			base::log(L, error.c_str());
	}
    p.deletenode();
    
    path.clear();
    base::getPath(L, L"config.xml", &path);

	error.clear();

    xml::node ld;
    if (ld.load(path.c_str(), &error))
    {
        ld.get(L"darkroom/label", &m_props.dark_room);
        ld.get(L"name/begin", &m_props.begin_name);
        ld.get(L"name/end", &m_props.end_name);
        ld.get(L"descr/begin", &m_props.begin_descr);
        ld.get(L"descr/end", &m_props.end_descr);
        ld.get(L"exits/begin", &m_props.begin_exits);
        ld.get(L"exits/end", &m_props.end_exits);
        ld.get(L"prompt/begin", &m_props.begin_prompt);
        ld.get(L"prompt/end", &m_props.end_prompt);
        ld.get(L"vnum/begin", &m_props.begin_vnum);
        ld.get(L"vnum/end", &m_props.end_vnum);

        if (ld.move(L"dirs"))
        {
            ld.get(L"north", &m_props.north_exit);
            ld.get(L"south", &m_props.south_exit);
            ld.get(L"west", &m_props.west_exit);
            ld.get(L"east", &m_props.east_exit);
            ld.get(L"up", &m_props.up_exit);
            ld.get(L"down", &m_props.down_exit);
        }
        if (ld.move(L"/cmds"))
        {
            ld.get(L"north", &m_props.north_cmd);
            ld.get(L"south", &m_props.south_cmd);
            ld.get(L"west", &m_props.west_cmd);
            ld.get(L"east", &m_props.east_cmd);
            ld.get(L"up", &m_props.up_cmd);
            ld.get(L"down", &m_props.down_cmd);
        }
    } else {
		if (!error.empty())
			base::log(L, error.c_str());
    }
    ld.deletenode();

	if (!m_parent_window.create(L, L"Карта", 400, 400))
		return luaT_error(L, L"Не удалось создать окно для карты");

    HWND parent = m_parent_window.hwnd();    
    map_active = m_parent_window.isVisible();

    tstring folder;
    base::getPath(L, L"", &folder);

    m_mapper_window = new Mapper(&m_props, folder);
    RECT rc; ::GetClientRect(parent, &rc);
    if (rc.right == 0) rc.right = 400; // requeires for splitter inside map window (if parent window hidden)
    HWND res = m_mapper_window->Create(parent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);  
    m_parent_window.attach(res);
    m_parent_window.block(L"left,right,top,bottom");
    if (map_active)
        luaT_run(L, "checkMenu", "d", 1);

	tstring dir;
	base::getPath(L, L"", &dir);
    m_mapper_window->loadMaps(dir);
    return 0;
}

int release(lua_State *L)
{
    m_mapper_window->saveProps();

	tstring dir;
	base::getPath(L, L"", &dir);
    m_mapper_window->saveMaps(dir);

    xml::node p(L"settings");
    p.set(L"usemsdp/value", m_props.use_msdp ? 1 : 0);
    p.set(L"zoneslist/width", m_props.zoneslist_width);
    
    tstring path;
    base::getPath(L, L"settings.xml", &path);
    if (!p.save(path.c_str()))
    {
        tstring error(L"Ошибка записи настроек пользователя: ");
        error.append(path);
        base::log(L, error.c_str());
    }
    p.deletenode();

    xml::node s(L"mapper");
    s.set(L"darkroom/label", m_props.dark_room);
    s.set(L"name/begin", m_props.begin_name);
    s.set(L"name/end", m_props.end_name);
    s.set(L"descr/begin", m_props.begin_descr);
    s.set(L"descr/end", m_props.end_descr);
    s.set(L"exits/begin", m_props.begin_exits);
    s.set(L"exits/end", m_props.end_exits);
    s.set(L"prompt/begin", m_props.begin_prompt);
    s.set(L"prompt/end", m_props.end_prompt);
    s.set(L"vnum/begin", m_props.begin_vnum);
    s.set(L"vnum/end", m_props.end_vnum);
    s.create(L"dirs");
    s.set(L"north", m_props.north_exit);
    s.set(L"south", m_props.south_exit);
    s.set(L"west", m_props.west_exit);
    s.set(L"east", m_props.east_exit);
    s.set(L"up", m_props.up_exit);
    s.set(L"down", m_props.down_exit);
    s.create(L"/cmds");
    s.set(L"north", m_props.north_cmd);
    s.set(L"south", m_props.south_cmd);
    s.set(L"west", m_props.west_cmd);
    s.set(L"east", m_props.east_cmd);
    s.set(L"up", m_props.up_cmd);
    s.set(L"down", m_props.down_cmd);
    s.move(L"/");

    path.clear();
    base::getPath(L, L"config.xml", &path);
    if (!s.save(path.c_str()))
    {
        tstring error(L"Ошибка записи настроек распознавания карты: ");
        error.append(path);
        base::log(L, error.c_str());
    }
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
        tstring stream ( luaT_towstring(L, -1) );
        m_mapper_window->processNetworkData(stream.c_str(), stream.length());
    }
    return 1;
}

int gamecmd(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TTABLE))
    {
        lua_len(L, -1);
        int len = lua_tointeger(L, -1);
        lua_pop(L, 1);
        if (len > 0)
        {
            lua_pushinteger(L, 1);
            lua_gettable(L, -2);
            tstring cmd(luaT_towstring(L, -1));
            lua_pop(L, 1);
            int pos = wcsspn(cmd.c_str(), L" ");
            if (pos != 0)
                cmd.assign(cmd.substr(pos));
            m_mapper_window->processCmd(cmd);
        }
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
    return 1;
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
