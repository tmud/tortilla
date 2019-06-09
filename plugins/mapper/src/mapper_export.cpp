#include "stdafx.h"
#include "../res/resource.h"
#include "properties.h"
#include "mapper.h"
#include "debugHelpers.h"
#include "mapperUnitTests.h"

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
        L"Отображает схему комнат и выходов, показывает местоположение игрока.\r\n"
        L"Предназначен для мадов с 6 стандартными выходами.\r\n"
        L"Требует для работы поддержки MSDP протокола со стороны мад-сервера\r\n"
        L"с данными о комнатах, переходах и местоположении игрока.\r\n"
        L"Это тестовая версия плагина, так что он может работать неправильно!"
        );
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"0.04");
    return 1;
}

int init(lua_State *L)
{
#ifdef _DEBUG
    MapperUnitTests t;
    t.runTests();
#endif

    DEBUGINIT(L);
	init_clientlog(L);
    luaT_run(L, "addButton", "dds", IDB_MAP, 1, L"Окно с картой");

    m_props.initAllDefault();
    m_props.dpi = base::getDpi(L);

	tstring error;
    tstring path;
	tstring current_zone;
    base::getPath(L, L"settings.xml", &path);
    xml::node p;
    if (p.load(path.c_str(), &error))
    {
        int center = 1;
        p.get(L"center/mode", &center);
        m_props.center_mode = (center != 0) ? 1 : 0;
		if (p.get(L"lastzone/name", &current_zone))
            m_props.current_zone = current_zone;

        DWORD screen_width = GetSystemMetrics(SM_CXSCREEN);
        DWORD screen_height = GetSystemMetrics(SM_CYSCREEN);
        int size = 0;
        xml::request d(p, L"zoneswnd/display");
        for (int i=0; i<d.size(); ++i)
        {
            int width = 0; int height = 0;
            if (d[i].get(L"width", &width) &&
                d[i].get(L"height", &height) &&
                width == screen_width &&
                height == screen_height)
            {
                d[i].get(L"size", &size);
                break;
            }
        }
        m_props.zoneslist_width = (size > 0) ? size : -1;
	} else {
		if (!error.empty())
			base::log(L, error.c_str());
	}
    p.deletenode();

	if (!m_parent_window.createDpi(L, L"Карта", 400, 400))
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

    m_mapper_window->loadMaps();
    m_mapper_window->setActiveMode(map_active);
    return 0;
}

int release(lua_State *L)
{
    m_mapper_window->saveProps();

    //todo! m_mapper_window->saveMaps();

    tstring error, path;
    base::getPath(L, L"settings.xml", &path);
    xml::node p;
    if (!p.load(path.c_str(), &error))
    {
        p = xml::node(L"settings");
    }
    p.set(L"lastzone/name", m_props.current_zone);
    p.set(L"center/mode", m_props.center_mode ? 1 : 0);
    xml::request d(p, L"zoneslist");
    for (int i = 0; i < d.size(); ++i)
        d[i].deletenode();

    DWORD screen_width = GetSystemMetrics(SM_CXSCREEN);
    DWORD screen_height = GetSystemMetrics(SM_CYSCREEN);
    xml::request zr(p, L"zoneswnd");
    xml::node z;
    if (zr.size() == 0)
    {
        xml::node zl = p.createsubnode(L"zoneswnd");
        z = zl.createsubnode(L"display");
        z.set(L"width", screen_width);
        z.set(L"height", screen_height);
    }
    else
    {
        xml::node zl = zr[0];
        xml::request displays(zl, L"display");
        int index = -1;
        for (int i = 0, e = displays.size(); i < e; ++i)
        {
            int width = 0; int height = 0;
            if (displays[i].get(L"width", &width) &&
                displays[i].get(L"height", &height) &&
                width == screen_width && 
                height == screen_height)
            {
                z = displays[i];
                index = i; break;
            }
        }
        if (index == -1)
        {
            z = zl.createsubnode(L"display");
            z.set(L"width", screen_width);
            z.set(L"height", screen_height);
        }
    }
    z.set(L"size", m_props.zoneslist_width);

    if (!p.save(path.c_str()))
    {
        tstring error(L"Ошибка записи настроек пользователя: ");
        error.append(path);
        base::log(L, error.c_str());
    }
    p.deletenode();
    return 0;
}

int menucmd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    int id = lua_tointeger(L, 1);
    lua_pop(L, 1);
    if (id == 1)
    {
        map_active = !map_active;
        m_mapper_window->setActiveMode(map_active);
        if (!map_active)
        {
            luaT_run(L, "uncheckMenu", "d", 1);
            m_parent_window.hide();
        }
        else
        {
            luaT_run(L, "checkMenu", "d", 1);
            m_parent_window.show();
        }
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
        m_mapper_window->setActiveMode(false);
        m_parent_window.hide();
        map_active = false;
    }
    return 0;
}

void msdpstate(lua_State *L, bool state)
{
    luaT_Msdp m(L);
    std::vector<std::wstring> reports1;
    std::vector<std::wstring> reports2;
    reports1.push_back(L"ROOM");
    //reports2.push_back(L"MOVEMENT");
    if (state)
    {
        m.report(reports1); //todo!
        //m.report(reports2);
    }
    else
    {
        m.unreport(reports1);
        //m.unreport(reports2);
    }
}

int msdpon(lua_State *L)
{
    msdpstate(L, true);
    return 0;
}

int msdpoff(lua_State *L)
{
    msdpstate(L, false);
    return 0;
}

bool popString(lua_State *L, const char* name, tstring* val)
{
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    val->assign(luaT_towstring(L, -1));
    lua_pop(L, 1);
    return (val->empty()) ? false : true;
}

void tstring_trimsymbols(tstring *str, const tstring& symbols)
{
    if (str->empty() || symbols.empty()) return;

    tstring newstr;
    const tchar *b = str->c_str();
    const tchar *e = b + str->length();
    const tchar *p = b + wcscspn(b, symbols.c_str());
    while (p != e)
    {
        newstr.append(b, p - b);
        b = p + 1;
        p = b + wcscspn(b, symbols.c_str());
    }
    newstr.append(b);
    str->swap(newstr);
}

int msdp(lua_State *L)
{
    if (!map_active)
        return 0;
    RoomData rd;
    bool inconsistent_data = true;
    if (luaT_check(L, 1, LUA_TTABLE))
    {
        lua_pushstring(L, "ROOM");
        lua_gettable(L, -2);
        if (lua_istable(L, -1))
        {
            if (popString(L, "AREA", &rd.areaname)
                && popString(L, "VNUM", &rd.vnum)
                && popString(L, "NAME", &rd.roomname)
               )
            {
                tstring_trimsymbols(&rd.areaname, L"\"'");

                tstring zone;
                popString(L, "ZONE", &zone);
                if (!zone.empty())
                {
                    tstring suffix(L" [");
                    suffix.append(zone);
                    suffix.append(L"]");
                    rd.areaname.append(suffix);
                }

                lua_pushstring(L, "EXITS");
                lua_gettable(L, -2);
                if (lua_istable(L, -1))
                {
                    lua_pushnil(L);
                    while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
                    {
                        tstring dir(luaT_towstring(L, -2));
                        tstring vnum(luaT_towstring(L, -1));
                        rd.exits[dir] = vnum;
                        lua_pop(L, 1);
                    }
                    inconsistent_data = false;
                }
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }
    if (inconsistent_data)
        return 0;
    m_mapper_window->processMsdp(rd);
    return 0;
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
    {"msdpon", msdpon },
    {"msdpoff", msdpoff },
    {"msdp", msdp },
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
