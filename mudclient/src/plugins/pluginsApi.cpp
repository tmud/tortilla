#include "stdafx.h"
#include "accessors.h"
#include "pluginsApi.h"
#include "api/api.h"
#include "../MainFrm.h"
#include "pluginSupport.h"
#include "../profiles/profilesPath.h"
#include "plugins/pluginsParseData.h"
#include "highlightHelper.h"

#define CAN_DO if (!_wndMain.IsWindow()) return 0;
extern CMainFrame _wndMain;
extern Plugin* _cp;
PluginsIdTableControl m_idcontrol(PLUGING_MENUID_START, PLUGING_MENUID_END);
//--------------------------------------------------------------------
tstring _extra_plugin_name;
class CurrentPluginExtra
{  bool m_used;
public:
    CurrentPluginExtra() : m_used(false) {
       if (_cp || _extra_plugin_name.empty()) return;
       Plugin *p = tortilla::getPluginsManager()->findPlugin(_extra_plugin_name);
       if (p) { _cp = p; m_used = true; }
    }
    ~CurrentPluginExtra() {
        if (m_used) { _cp = NULL; _extra_plugin_name.clear(); }
    }
};
#define EXTRA_CP CurrentPluginExtra _cpgetextra;
//--------------------------------------------------------------------
ToolbarEx<CMainFrame>* tbar() { return &_wndMain.m_toolBar; }
LogicProcessorMethods* lp() { return _wndMain.m_gameview.getMethods(); }
UINT getId(int code, bool button) { return m_idcontrol.registerPlugin(_cp, code, button); }
UINT delCode(int code, bool button) { return m_idcontrol.unregisterByCode(_cp, code, button); }
UINT findId(int code, bool button) { return m_idcontrol.findId(_cp,code,button); }
void delId(UINT id) { m_idcontrol.unregisterById(_cp, id); }
void pluginsMenuCmd(UINT id) { m_idcontrol.runPluginCmd(id); }
void tmcLog(const tstring& msg) { lp()->tmcLog(msg); }
void pluginLogOut(const tstring& msg) { lp()->pluginLog(msg);  }
void pluginsUpdateActiveObjects(int type) { lp()->updateActiveObjects(type); }
void collectGarbage() { lua_gc(tortilla::getLua(), LUA_GCSTEP, 1); }
const tchar* plugin_name() { return _cp ? _cp->get(Plugin::FILE) : L"?"; }
//---------------------------------------------------------------------
MemoryBuffer pluginBuffer(16384*sizeof(tchar));
tchar* plugin_buffer() { return (tchar*)pluginBuffer.getData(); }
int pluginInvArgs(lua_State *L, const tchar* fname)
{
    luaT_push_args(L, TW2A(fname));
    tstring error(luaT_towstring(L, -1));
    tstring p( L"Некорректный тип в параметрах" ); // (_cp ? L"Некорректные параметры" : L"Параметры");
    swprintf(plugin_buffer(), L"%s: %s", p.c_str(), error.c_str());
    pluginLogOut(plugin_buffer());
    return 0;
}
int pluginInvArgsValues(lua_State *L, const tchar* fname)
{
    luaT_push_args(L, TW2A(fname));
    tstring error(luaT_towstring(L, -1));
    tstring p( L"Некорректное значение в параметрах" );
    swprintf(plugin_buffer(), L"%s: %s", p.c_str(), error.c_str());
    pluginLogOut(plugin_buffer());
    return 0;
}
int pluginNotDeclared(lua_State* L, const tchar* fname)
{
    luaT_push_args(L, TW2A(fname));
    tstring error(luaT_towstring(L, -1));
    tstring p( L"Вызов не из плагина" );
    swprintf(plugin_buffer(), L"%s: %s", p.c_str(), error.c_str());
    pluginLogOut(plugin_buffer());
    return 0;
}

int pluginLoadFail(lua_State *L, const tchar* fname, const tchar* file)
{
    swprintf(plugin_buffer(), L"%s: %s Ошибка загрузки файла: %s", plugin_name(), fname, file);
    pluginLogOut(plugin_buffer());
    return 0;
}

int pluginMethodError(const tchar* fname, const tchar* error)
{
    swprintf(plugin_buffer(), L"%s: Ошибка в методе %s: %s", plugin_name(), fname, error);
    pluginLogOut(plugin_buffer());
    return 0;
}

int pluginLog(const tchar* msg)
{
    swprintf(plugin_buffer(), L"%s: %s", plugin_name(), msg);
    pluginLogOut(plugin_buffer());
    return 0;
}

int pluginOut(const tchar* msg)
{
    pluginLogOut(msg);
    return 0;
}

void pluginLoadError(const tchar* msg)
{
    swprintf(plugin_buffer(), L"%s: Ошибка загрузки! %s", plugin_name(), msg);
    pluginLogOut(plugin_buffer());
}
//---------------------------------------------------------------------
int pluginName(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        _extra_plugin_name.assign(TU2W(lua_tostring(L, 1))); 
        return 0;
    }
    return pluginInvArgs(L, L"pluginName");
}
//---------------------------------------------------------------------
int addCommand(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TSTRING))
    {
        bool result = false;
        tstring cmd(luaT_towstring(L, 1));
        if (!cmd.empty())
            result = lp()->addSystemCommand(cmd);
        if (result)
            _cp->commands.push_back(cmd);
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"addCommand");
}

int runCommand(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring cmd(luaT_towstring(L, 1));
        lp()->processPluginCommand(cmd);
        return 0;
    }
    return pluginInvArgs(L, L"runCommand");
}

int setCommand(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring cmd(luaT_towstring(L, 1));
        _wndMain.m_gameview.setCommand(cmd);
        return 0;
    }
    return pluginInvArgs(L, L"setCommand");
}

int sendCommand(lua_State *L)
{
    EXTRA_CP;
    int n = lua_gettop(L);
    if (n > 1)
    {
        bool params_ok = false;
        tstring window, cmd;
        if (lua_isnil(L, 1) || lua_isstring(L, 1))
        {
            params_ok = true;
            if (lua_isstring(L, 1))
                window.assign(luaT_towstring(L, 1));
        }
        if (params_ok)
        {
            for (int i=2;i<=n;++i)
            {
                if (!lua_isstring(L, i)) { params_ok = false; break; }
                if (i != 2) cmd.append(L" ");
                cmd.append(luaT_towstring(L, i));
            }
        }
        if (params_ok)
        {
            sendCommandToWindow(_wndMain, window, cmd);
            return 0;
        }
    }
    return pluginInvArgs(L, L"sendCommand");
}

int addMenu(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (!_cp)
         return pluginNotDeclared(L, L"addMenu");
    int code = -1;
    bool params_ok = false;
    if (luaT_check(L, 1, LUA_TSTRING))
        params_ok = tbar()->addMenuItem(luaT_towstring(L, 1), -1, -1, NULL);
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
    {
        code = lua_tointeger(L, 2);
        params_ok = tbar()->addMenuItem(luaT_towstring(L, 1), -1, getId(code, false), NULL);
    }
    else if (luaT_check(L, 3, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER))
    {
        code = lua_tointeger(L, 2);
        params_ok = tbar()->addMenuItem(luaT_towstring(L, 1), lua_tointeger(L, 3), getId(code, false), NULL);
    }
    else if (luaT_check(L, 4, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        HBITMAP bmp = NULL;
        int bmp_id = lua_tointeger(L, 4);
        if (bmp_id > 0) {
            HMODULE module = _cp->getModule();
            if (module)
                bmp = LoadBitmap( module, MAKEINTRESOURCE(bmp_id) );
        }
        code = lua_tointeger(L, 2);
        params_ok = tbar()->addMenuItem(luaT_towstring(L, 1), lua_tointeger(L, 3), getId(code, false), bmp);
    }
    else { return pluginInvArgs(L, L"addMenu"); }
    if (!params_ok) { delCode(code, false); }
    else
    {
        tstring menu_id(luaT_towstring(L, 1));
        _cp->menus.push_back(menu_id);
    }
    return 0;
}

int checkMenu(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        tbar()->checkMenuItem(findId(code, false), TRUE);
        tbar()->checkToolbarButton(findId(code, true), TRUE);
        return 0;
    }
    return pluginInvArgs(L, L"checkMenu");
}

int uncheckMenu(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        tbar()->checkMenuItem(findId(code, false), FALSE);
        tbar()->checkToolbarButton(findId(code, true), FALSE);
        return 0;
    }
    return pluginInvArgs(L, L"uncheckMenu");
}

int enableMenu(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        tbar()->enableMenuItem(findId(code, false), TRUE);
        tbar()->enableToolbarButton(findId(code, true), TRUE);
        return 0;
    }
    return pluginInvArgs(L, L"enableMenu");
}

int disableMenu(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        tbar()->enableMenuItem(findId(code, false), FALSE);
        tbar()->enableToolbarButton(findId(code, true), FALSE);
        return 0;
    }
    return pluginInvArgs(L, L"disableMenu");
}

int addButton(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (!_cp) 
        return pluginNotDeclared(L, L"addButton");
    int image = -1, code = -1; tstring hover;
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
    {
        image = lua_tointeger(L, 1); code = lua_tointeger(L, 2);
    }
    else if (luaT_check(L, 3, LUA_TNUMBER, LUA_TNUMBER, LUA_TSTRING))
    {
        image = lua_tointeger(L, 1); code = lua_tointeger(L, 2); hover.assign(luaT_towstring(L, 3));
    }
    else { return pluginInvArgs(L, L"addButton"); }

    if (image > 0 && code > 0)
    {
        HMODULE module = _cp->getModule();
        HBITMAP bmp = NULL;
        if (module && (bmp = LoadBitmap(module, MAKEINTRESOURCE(image))))
        {
            if (!tbar()->addToolbarButton(bmp, getId(code, true), hover.c_str()))
                delCode(code, true);
            else
                _cp->buttons.push_back(code);
        }
    }
    return 0;
}

int addToolbar(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (!_cp)
        return pluginNotDeclared(L, L"addToolbar");
    if (luaT_check(L, 1, LUA_TSTRING))
        tbar()->addToolbar(luaT_towstring(L, 1), 15);
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
        tbar()->addToolbar(luaT_towstring(L, 1), lua_tointeger(L, 2));
    else { return pluginInvArgs(L, L"addToolbar"); }
    tstring tbar_name(luaT_towstring(L, 1));
    _cp->toolbars.push_back(tbar_name);
    return 0;
}

int hideToolbar(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TSTRING))
    {
        tbar()->hideToolbar(luaT_towstring(L, 1));
        return 0;
    }
    return pluginInvArgs(L, L"hideToolbar");
}

int showToolbar(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TSTRING))
    {
        tbar()->showToolbar(luaT_towstring(L, 1));
        return 0;
    }
    return pluginInvArgs(L, L"showToolbar");
}

int getPath(lua_State *L)
{
    EXTRA_CP;
    if (_cp && luaT_check(L, 1, LUA_TSTRING))
    {
        PropertiesManager *pmanager = tortilla::getPropertiesManager();
        tstring filename(luaT_towstring(L, 1));
        ProfilePluginPath pp(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
        ProfileDirHelper dh;
        if (dh.makeDir(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME)))
        {
            luaT_pushwstring(L, pp);
            return 1;
        }
        return pluginMethodError(L"getPath", L"Ошибка создания каталога для плагина");
    }
    return pluginInvArgs(L, L"getPath");
}

int getProfilePath(lua_State *L)
{
    EXTRA_CP;
    if (_cp && luaT_check(L, 0))
    {
        PropertiesManager *pmanager = tortilla::getPropertiesManager();
        tstring filename(pmanager->getProfileName());
        filename.append(L".xml");
        ProfilePluginPath pp(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
        ProfileDirHelper dh;
        if (dh.makeDir(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME)))
        {
            luaT_pushwstring(L, pp);
            return 1;
        }
        return pluginMethodError(L"getProfilePath", L"Ошибка создания каталога для плагина");
    }
    return pluginInvArgs(L, L"getProfilePath");
}

int getResource(lua_State* L)
{
    EXTRA_CP;
    if (!_cp)
        return pluginNotDeclared(L, L"getResource");

    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring filename(luaT_towstring(L, 1));
        tstring rd(L"resources");
        tstring pd(_cp->get(Plugin::FILENAME));
        ChangeDir cd;
        bool error = false;
        if (!cd.changeDir(rd))
        {
            CreateDirectory(rd.c_str(), NULL);
            if (!cd.changeDir(rd))
                error = true;
        }
        if (!error && !cd.changeDir(pd))
        {
            CreateDirectory(pd.c_str(), NULL);
            if (!cd.changeDir(pd))
                error = true;
        }
        if (!error)
        {
            tstring path(rd);
            path.append(L"\\");
            path.append(pd);
            path.append(L"\\");
            path.append(filename);
            lua_pushwstring(L, path.c_str());
            return 1;
        }
        return pluginMethodError(L"getResource", L"Ошибка создания каталога для плагина");
    }
    return pluginInvArgs(L, L"getResource");
}

int getProfile(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        PropertiesManager *pmanager = tortilla::getPropertiesManager();
        lua_pushwstring(L, pmanager->getProfileName().c_str());
        return 1;
    }
    return pluginInvArgs(L, L"getProfile");
}

int getParent(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        HWND hwnd = _wndMain; //.m_gameview;
        lua_pushunsigned(L, (DWORD)hwnd);
        return 1;
    }
    return pluginInvArgs(L, L"getParent");
}

class LuaEnviroment
{
    lua_State *L;
    std::vector<std::string> env_list;
public:
    LuaEnviroment(lua_State *pl) : L(pl)
    {
        lua_newtable(L);
    }
    void add(const char* global_func)
    {
        lua_pushstring(L, global_func);
        lua_getglobal(L, global_func);
        assert(lua_isfunction(L, -1) || lua_istable(L, -1));
        lua_settable(L, -3);
        env_list.push_back(global_func);
    }
    void clear()
    {
        assert(lua_istable(L, -1));
        if (!lua_istable(L, -1))
            return;
        for (int i=0,e=env_list.size();i<e;++i)
        {
            lua_pushstring(L, env_list[i].c_str());
            lua_pushnil(L);
            lua_settable(L, -3);
        }
    }
};

int loadTableLua(lua_State* L, const tstring& filename)
{
    const tchar* fname = filename.c_str();
    if (luaL_loadfile(L, TW2A(fname)))
    {
        pluginLoadError(lua_toerror(L));
        lua_pop(L, 1);
        return false;
    }
    // make empty eviroment and call script in them
    LuaEnviroment env(L);
    env.add("pairs");
    env.add("ipairs");
    env.add("log");
    env.add("createPcre");
    env.add("table");
    env.add("props");

    lua_insert(L, -2);
    lua_pushvalue(L, -2);
    lua_setupvalue(L, -2, 1); 
    if (lua_pcall(L, 0, 0, 0))
    {
        pluginLoadError(lua_toerror(L));
        lua_pop(L, 1);
        return false;
    }
    lua_pushstring(L, "_i");
    lua_gettable(L, -2);
    if (lua_istable(L, -1))
    {
        lua_pushnil(L);                     // first key
        while (lua_next(L, -2) != 0)
        {   //  value = -1, key = -2, srctable = -3, dsttable = -4
            lua_pushvalue(L, -2);
            lua_insert(L, -2);
            // value = -1, key = -2,-3, srctable = -4, dsttable = -5
            lua_settable(L, -5);
            // key = -1, srctable = -2, dsttable = -3
        }
        lua_pop(L, 1);
        lua_pushstring(L, "_i");
        lua_pushnil(L);
        lua_settable(L, -3);
    }
    else {
        lua_pop(L, 1);
    }
    //env.clear();
    return 1;
}

int loadTable(lua_State *L)
{
    EXTRA_CP;
    if (!_cp)
        return pluginNotDeclared(L, L"loadTable");        
    if (!luaT_check(L, 1, LUA_TSTRING))
        return pluginInvArgs(L, L"loadTable");

    tstring filename(luaT_towstring(L, 1));
    lua_pop(L, 1);

    PropertiesManager *pmanager = tortilla::getPropertiesManager();    
    ProfilePluginPath pp(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
    filename.assign(pp);

    DWORD fa = GetFileAttributes(filename.c_str());
    if (fa == INVALID_FILE_ATTRIBUTES || fa&FILE_ATTRIBUTE_DIRECTORY)
        return 0;

    tstring ext;
    size_t pos = filename.rfind(L".");
    if (pos != -1)
        ext.assign(filename.substr(pos+1));
    if (ext.find(L"xml") == tstring::npos)
    {
        return loadTableLua(L, filename);
    }

    xml::node doc;
    if (!doc.load(pp) )
    {
       swprintf(plugin_buffer(), L"Ошибка чтения: %s", filename);
       tstring tmp(plugin_buffer());
       pluginMethodError(L"loadTable", tmp.c_str());
       return 0;
    }

    struct el { el(xml::node n, int l) : node(n), level(l) {} xml::node node; int level; };
    std::vector<el> stack;
    xml::request req(doc, L"*");
    for (int i = 0, e = req.size(); i < e; ++i)
       { stack.push_back( el(req[i], 0) ); }

    lua_newtable(L);            // root (main) table 
    int p = 0; int size = stack.size();
    while (p != size)
    {
       el s_el = stack[p++];
       xml::node n = s_el.node;
       int array_index = 0;
       bool it_array = false;

       tstring name, val;
       n.getname(&name);
       if (name == L"array") 
       { //can be number index
         if (n.get(L"index", &array_index))
             it_array = true;
       }
       if (n.get(L"value", &val))
       {   // it is simple value
           if (it_array)
             lua_pushinteger(L, array_index);
           else
             luaT_pushwstring(L, name.c_str());
           int native = 0;
           if (n.get(L"native", &native) && native != 0)
           {
               if (val == L"false")
                   lua_pushboolean(L, 0);
               else if (val == L"true")
                   lua_pushboolean(L, 1);
               else {
                   bool ok = false;
                   wstring_to_int number(val.c_str(), &ok);
                   if (ok)
                       lua_pushinteger(L, number);
                   else
                       luaT_pushwstring(L, val.c_str());
               }
           }
           else {
            luaT_pushwstring(L, val.c_str());
           }
           lua_settable(L, -3);
       }
       else
       {   // it is table
           lua_newtable(L);
           if (it_array)
               lua_pushinteger(L, array_index);
           else
               luaT_pushwstring(L, name.c_str());
           lua_pushvalue(L, -2);
           lua_settable(L, -4);

           // insert subnodes
           int new_level = s_el.level + 1;
           xml::request req(n, L"*");
           std::vector<el> tmp;
           for (int i = 0, e = req.size(); i < e; ++i)
               tmp.push_back(el(req[i], new_level));
           if (!tmp.empty())
           {
               stack.insert(stack.begin() + p, tmp.begin(), tmp.end());
               size = stack.size();
           }
           else
               lua_pop(L, 1);
       }

       // goto next node
       if (p == size)
       {
           lua_settop(L, 1);
           break;
       }
       el s_el2 = stack[p];
       if (s_el2.level < s_el.level)  // pop from stack
       {
           int pop = s_el.level - s_el2.level;
           lua_pop(L, pop);
       }
   }
   doc.deletenode();
   return 1;
}

 // make lua file
class LuaRecorder
{
    tstring tabs;
    const size_t tabs_size = 2;
    tstring r;
    bool first_param;
    int root_index;
    int brackets_layer;
    bool index_mode;
public:
    LuaRecorder() : first_param(true), brackets_layer(0), index_mode(false) { r.reserve(4096);  root_index = 0; }
    const tstring& result() const { return r; }
    void index(const tstring& v, bool itstring) {
        beginparam();
        beginindex();
        addtabs(); 
        val(v, itstring);
    }
    void named(const tstring& k, const tstring& v, bool itstring) {
        endindex();
        beginparam();
        addtabs(); 
        r.append(k); r.append(L"="); 
        val(v, itstring);
        if (brackets_layer == 1) { first_param = true; endline(); }
    }

    void openbracket(const tstring& name) {
        if (brackets_layer++ == 0) return;
        endindex();
        beginparam();
        first_param = true;

        addtabs();
        if (!name.empty()) { r.append(name); r.append(L" = "); }
        r.append(L"{"); endline();
        tabs.append(tabs_size, L' ');
    }
    void closebracket() {
        if (--brackets_layer == 0) { return; }
        size_t len = tabs.size(); 
        if (len >= tabs_size) tabs.resize(len-tabs_size);
        endline();
        addtabs();
        r.append(L"}");
        if (brackets_layer == 1 ) { first_param = true; endline(); }
     }
private:
    void beginparam() {
      if (!first_param)
         endvar();
      first_param = false;
    }
    void beginindex() {
        if (brackets_layer == 1 && root_index == 0) { r.append(L"_i = {"); endline(); root_index = 1; }
    }
    void endindex() {
       if (root_index == 1) { r.append(L"}"); endline(); root_index = 2; first_param = true; }
    }
    void val(const tstring& v, bool itstring) 
    {
       if (!itstring) {  r.append(v); return; }
       tchar b[2] = { L'"', 0 }; 
       if (v.find(L"\"") != tstring::npos) { b[0] = L'\''; }
       r.append(b); r.append(v); r.append(b);
    }
    void endvar() { r.append(L","); endline(); }
    void endline() { r.append(L"\r\n"); }
    void addtabs() { r.append(tabs); }
};

int saveTable(lua_State *L)
{
    EXTRA_CP;
    if (!_cp)
        return pluginNotDeclared(L, L"saveTable");
    if (!luaT_check(L, 2, LUA_TTABLE, LUA_TSTRING))
        return pluginInvArgs(L, L"saveTable");

    tstring filename(luaT_towstring(L, 2));
    lua_pop(L, 1);

    PropertiesManager *pmanager = tortilla::getPropertiesManager();
    ProfilePluginPath pp(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
    const wchar_t *filepath = pp;

    int result = 0;
    ProfileDirHelper dh;
    if (!dh.makeDirEx(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename))
    {
       swprintf(plugin_buffer(), L"Ошибка записи: %s", filepath);
       tstring tmp(plugin_buffer());
       pluginMethodError(L"saveTable", tmp.c_str());
       return 0;
    }

    // recursive cycles in table
    struct saveDataNode
    {
        saveDataNode() : index(0) {}
        typedef std::pair<tstring,bool> value_with_type;
        typedef std::pair<tstring, value_with_type> value;
        typedef std::map<int, value_with_type> tarray;
        std::vector<value> attributes;
        std::vector<saveDataNode*> childnodes;
        tstring name;
        int index;
        tarray array;
    };

    saveDataNode *current = new saveDataNode();
    current->name = L"plugindata";

    std::vector<saveDataNode*> stack;
    std::vector<saveDataNode*> list;
    list.push_back(current);

    bool incorrect_data = false;

    lua_pushnil(L);                         // first key
    while (true)
    {
        bool pushed = false;
        while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
        {
            int value_type = lua_type(L, -1);
            int key_type = lua_type(L, -2);
            if (key_type != LUA_TSTRING && key_type != LUA_TNUMBER) 
            {
                lua_pop(L, 1);
                incorrect_data = true;
                continue; 
            }
            tstring v;
            if (value_type == LUA_TNUMBER || value_type == LUA_TSTRING) { v.assign(luaT_towstring(L, -1)); }            
            else if ( value_type == LUA_TBOOLEAN) {  v.assign(lua_toboolean(L, -1) ? L"true" : L"false"); }

            if (key_type == LUA_TNUMBER)
            {
                if (value_type == LUA_TNUMBER || value_type == LUA_TSTRING || value_type == LUA_TBOOLEAN)
                {
                    int index = lua_tointeger(L, -2);
                    current->array[index] = saveDataNode::value_with_type(v, (value_type == LUA_TSTRING));
                    lua_pop(L, 1);
                    continue;
                }
                else if (value_type != LUA_TTABLE) {
                    lua_pop(L, 1);
                    incorrect_data = true;
                    continue;
                }
            }
            if (value_type == LUA_TNUMBER || value_type == LUA_TSTRING || value_type == LUA_TBOOLEAN)
            {
                tstring a(luaT_towstring(L, -2));
                saveDataNode::value_with_type b(v, (value_type == LUA_TSTRING));
                current->attributes.push_back( saveDataNode::value( a, b ) );
            }
            else if (value_type == LUA_TTABLE)
            {
                saveDataNode* new_node = new saveDataNode();
                if (key_type == LUA_TNUMBER) {
                    new_node->index = lua_tointeger(L, -2);
                }
                else
                    new_node->name = luaT_towstring(L, -2);
                current->childnodes.push_back(new_node);
                stack.push_back(current);
                list.push_back(new_node);
                current = new_node;
                pushed = true;
                break;
            }
            else { incorrect_data = true; }
            lua_pop(L, 1); // remove 'value', keeps 'key' for next iteration 
        }
        if (pushed) // new iteration for new table (recursivly iteration)
        {
            lua_pushnil(L); 
            continue;
        }
        if (stack.empty())
            break;
        int last = stack.size() - 1;
        current = stack[last];
        stack.pop_back();
        lua_pop(L, 1);
    }
    // sorting
    for (int k = 0, ke = list.size(); k < ke; ++k)
    {
        saveDataNode* node = list[k];
        std::vector<saveDataNode::value>&a = node->attributes;
        int size = a.size() - 1;
        for (int i = 0, e = size - 1; i <= e; ++i) {
        for (int j = i + 1; j <= size; ++j) {
            if (a[i].first > a[j].first) { saveDataNode::value t(a[i]); a[i] = a[j]; a[j] = t; }
        }}
        std::vector<saveDataNode*>&n = node->childnodes;
        size = n.size() - 1;
        for (int i = 0, e = size - 1; i <= e; ++i) {
        for (int j = i + 1; j <= size; ++j) {
            if (n[i]->name > n[j]->name) { saveDataNode *t = n[i]; n[i] = n[j]; n[j] = t; }
        }}
    }

    tstring ext;
    size_t pos = filename.rfind(L".");
    if (pos != -1)
        ext.assign(filename.substr(pos+1));
    if (ext.find(L"xml") == tstring::npos)
    {
       LuaRecorder lr;
       std::vector<saveDataNode*> stack;
       stack.push_back(current);

        int index = 0;
        while (index < (int)stack.size())
        {
            saveDataNode* n = stack[index++];
            if (!n) {
                lr.closebracket();
                continue;
            }
            lr.openbracket(n->name);

            // save numeric indexes
            saveDataNode::tarray &ta = n->array;
            saveDataNode::tarray::const_iterator it = ta.begin(), it_end = ta.end();
            for(; it!=it_end; ++it)
                lr.index(it->second.first, it->second.second);

            // save named values
            std::vector<saveDataNode::value>::const_iterator at = n->attributes.begin(), at_end = n->attributes.end();
            for (;at != at_end; ++at)
            {
                lr.named(at->first, at->second.first, at->second.second);
            }
            // save subtables -> push in stack
            if (n->childnodes.empty())
            {
                lr.closebracket();
            }
            else 
            {
                std::vector<saveDataNode*>::const_iterator ct = n->childnodes.begin(), ct_end = n->childnodes.end();
                stack.insert(stack.begin()+index, ct, ct_end);
                size_t last = index+n->childnodes.size();
                stack.insert(stack.begin()+last, NULL);
            }
        }
        HANDLE hFile = CreateFile(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            unsigned char bom[3] = { 0xef, 0xbb, 0xbf };
            if (WriteFile(hFile, bom, 3, &written, NULL) && written == 3)
            {
                written = 0;
                TW2U data(lr.result().c_str());
                const utf8* r = data; DWORD len = strlen(r);
                if (WriteFile(hFile, r, len, &written, NULL) && written == len) { result = 1; }
            }
            CloseHandle(hFile);
        }
    }
    else
    {
        // make xml
        xml::node root(current->name.c_str());
        typedef std::pair<saveDataNode*, xml::node> _xmlstack;
        std::vector<_xmlstack> xmlstack;
        xmlstack.push_back(_xmlstack(current, root));

        int index = 0;
        while (index < (int)xmlstack.size())
        {
            _xmlstack &v = xmlstack[index++];
            xml::node node = v.second;
            std::vector<saveDataNode::value>&a = v.first->attributes;
            for (int i = 0, e = a.size(); i < e; ++i)
            {
                xml::node attr = node.createsubnode(a[i].first.c_str());
                attr.set(L"value", a[i].second.first.c_str());
                if (!a[i].second.second)
                    attr.set(L"native", 1);
            }
            saveDataNode::tarray &ta = v.first->array;
            saveDataNode::tarray::iterator it = ta.begin(), it_end = ta.end();
            for(; it!=it_end; ++it)
            {
                xml::node arr = node.createsubnode(L"array");
                arr.set(L"index", it->first);
                arr.set(L"value", it->second.first.c_str());
                if (!it->second.second)
                    arr.set(L"native", 1);
            }
            std::vector<saveDataNode*>&n = v.first->childnodes;
            for (int i = 0, e = n.size(); i < e; ++i)
            {
                tstring name(n[i]->name); bool index = false;
                if (name.empty() && n[i]->index > 0) {
                    name = L"array"; index = true;
                }
                xml::node new_node = node.createsubnode(name.c_str());
                if (index)
                    new_node.set(L"index", n[i]->index);
                xmlstack.push_back(_xmlstack(n[i], new_node));
            }
        }
        result = root.save(filepath);
        root.deletenode();
    }

    // delete temporary datalist
    for (int i = 0, e = list.size(); i < e; ++i)
       delete list[i];
    list.clear();

    if (incorrect_data)
        pluginMethodError(L"saveTable", L"Неверные данные в исходных данных.");

    if (!result)
    {
       swprintf(plugin_buffer(), L"Ошибка записи: %s", filepath);
       tstring tmp(plugin_buffer());
       pluginMethodError(L"saveTable", tmp.c_str());
    }
    return 0;
}
//----------------------------------------------------------------------------
void initVisible(lua_State *L, int index, OutputWindow *w)
{
    if (lua_gettop(L) == index)
      w->initVisible(lua_toboolean(L, index) ? true : false);
}

int createWindow(lua_State *L)
{
    EXTRA_CP;
    if (!_cp)
        return pluginNotDeclared(L, L"createWindow");
    PluginData &p = find_plugin();
    OutputWindow w;

    if (luaT_check(L, 1, LUA_TSTRING) || luaT_check(L, 2, LUA_TSTRING, LUA_TBOOLEAN))
    {
        tstring name( luaT_towstring(L, 1) );
        if (!p.findWindow(name, &w))
        {
            p.initDefaultPos(300, 300, &w);
            initVisible(L, 2, &w);
            w.name = name;
            p.windows.push_back(w);
        }
        else { initVisible(L, 2, &w); }
    }
    else if (luaT_check(L, 3, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER )|| 
             luaT_check(L, 4, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER, LUA_TBOOLEAN))
    {
        tstring name( luaT_towstring(L, 1) );
        if (!p.findWindow(name, &w))
        {
            int height = lua_tointeger(L, 3);
            int width = lua_tointeger(L, 2);
            p.initDefaultPos(width, height, &w);
            initVisible(L, 4, &w);
            w.name = name;
            p.windows.push_back(w);
        }
        else { initVisible(L, 4, &w); }
    }
    else {
        return pluginInvArgs(L, L"createWindow"); 
    }

    PluginsView *window =  _wndMain.m_gameview.createDockPane(w, _cp);
    if (window)
        _cp->dockpanes.push_back(window);
    luaT_pushobject(L, window, LUAT_WINDOW);
    return 1;
}

int pluginLog(lua_State *L)
{
    EXTRA_CP;
    int n = lua_gettop(L);
    if (n == 0)
        return pluginInvArgs(L, L"log");

    tstring log;
    for (int i = 1; i <= n; ++i)
    {
        tstring el;
        pluginFormatByType(L, i, &el);
        log.append(el);
    }
    pluginLog(log.c_str());
    return 0;
}

int terminatePlugin(lua_State *L)
{
    EXTRA_CP;
    if (!_cp)
        { assert(false); return 0; }

    int n = lua_gettop(L);
    tstring log;
    for (int i = 1; i <= n; ++i)
    {
        tstring el;
        pluginFormatByType(L, i, &el);
        log.append(el);
    }
    _cp->setErrorState();

    if (log.empty())
        log.assign(L"TERMINATE");
    lua_settop(L, 0);
    luaT_pushwstring(L, log.c_str());
    lua_error(L);
    return 0;
}

int updateView(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TFUNCTION))
    {
        int view = lua_tointeger(L, 1);
        if (view >= 0 && view <=OUTPUT_WINDOWS)
        {
            MudViewHandler *h = _wndMain.m_gameview.getHandler(view);
            parseData pd;
            mudViewStrings& src = h->get();
            pd.strings.swap(src);
            {
                PluginsParseData ppd(&pd, NULL);
                lua_insert(L, -2);
                lua_pop(L, 1);
                luaT_pushobject(L, &ppd, LUAT_VIEWDATA);
                if (lua_pcall(L, 1, 0, 0))
                { //error
                }
            }
            pd.strings.swap(src);
            h->update();
            return 0;
        }
        return pluginInvArgsValues(L, L"updateView");
    }
    return pluginInvArgs(L, L"updateView");
}

int getViewSize(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int view = lua_tointeger(L, 1);
        if (view >= 0 && view <=OUTPUT_WINDOWS)
        {
            MudViewHandler *h = _wndMain.m_gameview.getHandler(view);
            SIZE sz = h->getSizeInSymbols();
            lua_pushinteger(L, sz.cx);
            lua_pushinteger(L, sz.cy);
            return 2;
        }
        return pluginInvArgsValues(L, L"getViewSize");
    }
    return pluginInvArgs(L, L"getViewSize");
}

int isViewVisible(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int view = lua_tointeger(L, 1);
        if (view >= 1 && view <=OUTPUT_WINDOWS)
        {
            bool visible = _wndMain.m_gameview.isViewVisible(view);
            lua_pushboolean(L, visible ? 1 : 0);
            return 1;
        }
        return pluginInvArgsValues(L, L"isViewVisible");
    }
    return pluginInvArgs(L, L"isViewVisible");
}

int showView(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int view = lua_tointeger(L, 1);
        if (view >= 1 && view <=OUTPUT_WINDOWS)
        {
            _wndMain.m_gameview.showView(view, true);
            return 0;
        }
        return pluginInvArgsValues(L, L"showView");
    }
    return pluginInvArgs(L, L"showView");
}

int hideView(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int view = lua_tointeger(L, 1);
        if (view >= 1 && view <=OUTPUT_WINDOWS)
        {
            _wndMain.m_gameview.showView(view, false);
            return 0;
        }
        return pluginInvArgsValues(L, L"hideView");
    }
    return pluginInvArgs(L, L"hideView");
}

int flashWindow(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 0))
    {
        HWND alarmWnd = _wndMain;
        if (::IsWindow(alarmWnd))
        {
            FLASHWINFO fw;
            fw.cbSize = sizeof(FLASHWINFO);
            fw.hwnd = alarmWnd;
            fw.uCount = 5;
            fw.dwFlags = FLASHW_ALL;
            fw.dwTimeout = 0;
            ::FlashWindowEx(&fw);
        }
        return 0;
    }
    return pluginInvArgs(L, L"flashWindow");
}

int regUnloadFunction(lua_State *L);

int clearView(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int window = lua_tointeger(L, 1);
        if (window >= 0 && window <= OUTPUT_WINDOWS)
        {
            lp()->windowClear(window);
            return 0;
        }
        return pluginInvArgsValues(L, L"clearView");
    }
    return pluginInvArgs(L, L"clearView");
}

int print(lua_State *L)
{
    EXTRA_CP;
    std::vector<tstring> params;
    int n = lua_gettop(L);
    for (int i=1; i<=n; ++i)
    {
        if (!lua_isstring(L, i))
            return pluginInvArgs(L, L"print");
        tstring p(lua_towstring(L, i));
        params.push_back(p);
    }
    lp()->windowOutput(0, params);
    return 0;
}

int vprint(lua_State *L)
{
    EXTRA_CP;
    int view = -1;
    if (lua_isnumber(L, 1))
    {
        int n = lua_tointeger(L, 1);
        if (n>=0 && n<OUTPUT_WINDOWS)
            view = n;
    }
    if (view == -1)
        return pluginInvArgsValues(L, L"vprint");

    std::vector<tstring> params;
    int n = lua_gettop(L);
    for (int i=2; i<=n; ++i)
    {
        if (!lua_isstring(L, i))
            return pluginInvArgs(L, L"vprint");
        tstring p(luaT_towstring(L, i));
        params.push_back(p);
    }
    lp()->windowOutput(view, params);
    return 0;
}

int translateColors(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 3, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER))
    {
        tstring p(luaT_towstring(L, 1));
        HighlightHelper hh;
        if (!hh.checkText(&p))
            return 0;
        COLORREF text = lua_tounsigned(L, 2);
        COLORREF bgnd = lua_tounsigned(L, 3);
        PropertiesHighlight ph(text, bgnd);
        ph.convertFromString(p);
        lua_pushunsigned(L, ph.textcolor);
        lua_pushunsigned(L, ph.bkgcolor);
        return 2;
    }
    return pluginInvArgs(L, L"translateColors");
}

int getVersion(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 0))
    {
        lua_pushinteger(L, TORTILLA_VERSION_MAJOR);
        lua_pushinteger(L, TORTILLA_VERSION_MINOR);
        return 2;
    }
    return pluginInvArgs(L, L"getVersion");
}

int isGroupActive(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring group(luaT_towstring(L, 1));
        PropertiesData* pdata = tortilla::getProperties();
        for (int i = 0, e = pdata->groups.size(); i < e; ++i)
        {
            const property_value &v = pdata->groups.get(i);
            if (v.key == group)
            {
                int state = (v.value == L"1") ? 1 : 0;
                lua_pushboolean(L, state);
                return 1;
            }
        }
        return pluginInvArgsValues(L, L"isGroupActive");
    }
    return pluginInvArgs(L, L"isGroupActive");
}
//---------------------------------------------------------------------
// Metatables for all types
void reg_mt_window(lua_State *L);
void reg_mt_viewdata(lua_State *L);
void reg_mt_viewstring(lua_State *L);
void reg_activeobjects(lua_State *L);
void reg_string(lua_State *L);
void reg_props(lua_State *L);
void reg_mt_panels(lua_State *L);
void reg_mt_render(lua_State *L);
void reg_mt_pcre(lua_State *L);
void reg_msdp(lua_State *L);
void reg_mt_image(lua_State *L);
void reg_mt_trigger(lua_State *L);
//---------------------------------------------------------------------
bool initPluginsSystem()
{
    tortilla::init();
    lua_State*L = tortilla::getLua();
    if (!L)
        return false;

    luaopen_base(L);
    lua_pop(L, 1);
#ifdef _DEBUG
    luaopen_debug(L);
    lua_setglobal(L, "debug");
#endif
    luaopen_math(L);
    lua_setglobal(L, "math");
    luaopen_table(L);
    lua_setglobal(L, "table");
    reg_string(L);
    lua_register(L, "addCommand", addCommand);
    lua_register(L, "runCommand", runCommand);
    lua_register(L, "setCommand", setCommand);
    lua_register(L, "sendCommand", sendCommand);
    lua_register(L, "addMenu", addMenu);
    lua_register(L, "addButton", addButton);
    lua_register(L, "addToolbar", addToolbar);
    lua_register(L, "hideToolbar", hideToolbar);
    lua_register(L, "showToolbar", showToolbar);
    lua_register(L, "checkMenu", checkMenu);
    lua_register(L, "uncheckMenu", uncheckMenu);
    lua_register(L, "enableMenu", enableMenu);
    lua_register(L, "disableMenu", disableMenu);
    lua_register(L, "getPath", getPath);
    lua_register(L, "getProfilePath", getProfilePath);
    lua_register(L, "getProfile", getProfile);
    lua_register(L, "getResource", getResource);
    lua_register(L, "getParent", getParent);
    lua_register(L, "loadTable", loadTable);
    lua_register(L, "saveTable", saveTable);
    lua_register(L, "createWindow", createWindow);
    lua_register(L, "log", pluginLog);
    lua_register(L, "terminate", terminatePlugin);
    lua_register(L, "updateView", updateView);
    lua_register(L, "getViewSize", getViewSize);  
    lua_register(L, "isViewVisible", isViewVisible);
    lua_register(L, "showView", showView);
    lua_register(L, "hideView", hideView);
    lua_register(L, "flashWindow", flashWindow);
    lua_register(L, "pluginName", pluginName);
    lua_register(L, "regUnloadFunction", regUnloadFunction);
    lua_register(L, "print", print);
    lua_register(L, "vprint", vprint);
    lua_register(L, "clearView", clearView);
    lua_register(L, "translateColors", translateColors);
    lua_register(L, "getVersion", getVersion);
    lua_register(L, "isGroupActive", isGroupActive);

    reg_props(L);
    reg_activeobjects(L);
    reg_mt_window(L);
    reg_mt_viewdata(L);
    reg_mt_viewstring(L);
    reg_mt_panels(L);
    reg_mt_image(L);
    reg_mt_render(L);
    reg_mt_pcre(L);
    reg_msdp(L);
    reg_mt_trigger(L);
    return true;
}

void pluginDeleteResources(Plugin *plugin)
{
    // delete UI (menu, buttons, etc)
    Plugin *old = _cp;
    _cp = plugin;
    for (int i = 0, e = plugin->menus.size(); i < e; ++i)
    {
        std::vector<UINT> ids;
        tbar()->deleteMenuItem(plugin->menus[i].c_str(), &ids);
        for (int i = 0, e = ids.size(); i < e; ++i)
            delId(ids[i]);
    }
    plugin->menus.clear();
    for (int i = 0, e = plugin->buttons.size(); i < e; ++i)
        tbar()->deleteToolbarButton(delCode(plugin->buttons[i], true));
    plugin->buttons.clear();
    for (int i = 0, e = plugin->toolbars.size(); i<e; ++i)
        tbar()->deleteToolbar(plugin->toolbars[i].c_str());
    plugin->toolbars.clear();
    for (int i = 0, e = plugin->dockpanes.size(); i < e; ++i)
        _wndMain.m_gameview.deleteDockPane(plugin->dockpanes[i]);
    plugin->dockpanes.clear();
    for (int i = 0, e = plugin->panels.size(); i < e; ++i)
        _wndMain.m_gameview.deletePanel(plugin->panels[i]);
    plugin->panels.clear();
    // delete all system commands of plugin
    for (int i = 0, e = plugin->commands.size(); i < e; ++i)
        lp()->deleteSystemCommand(plugin->commands[i]);
    plugin->commands.clear();
    // delete all triggers
    for (int i = 0,  e = plugin->triggers.size(); i<e; ++i)
        delete plugin->triggers[i];
    plugin->triggers.clear();
    _cp = old;
}
//--------------------------------------------------------------------
int string_len(lua_State *L)
{
    int len = 0;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        u8string str(lua_tostring(L, 1));
        len = u8string_len(str);
    }
    lua_pushinteger(L, len);
    return 1;
}

int string_substr(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
    {
        u8string s(lua_tostring(L, 1));
        u8string_substr(&s, 0, lua_tointeger(L, 2));
        lua_pushstring(L, s.c_str());
        return 1;
    }
    if (luaT_check(L, 3, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER))
    {
        u8string s(lua_tostring(L, 1));
        int from = lua_tointeger(L, 2);
        if (from < 1)
            s.clear();
        else
            u8string_substr(&s, from-1, lua_tointeger(L, 3));
        lua_pushstring(L, s.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"string:substr");
}

int string_strstr_impl(lua_State *L, const tchar* fname)
{
     if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING) ||
         luaT_check(L, 3, LUA_TSTRING, LUA_TSTRING, LUA_TNUMBER))
     {
         const utf8* s1 = lua_tostring(L, 1);
         int index = 0;
         if (lua_gettop(L) == 3)
         {
             index = lua_tointeger(L, 3);
             if (index <= 0) {
                 lua_pushnil(L);
                 return 1;
             }
             index = index - 1;
             int pos = utf8_sympos(s1, index);
             if (pos == -1) {
                 lua_pushnil(L);
                 return 1;
             }
             s1 = s1 + pos;
         }
         const utf8* s2 = lua_tostring(L, 2);
         const utf8* pos = strstr(s1, s2);
         if (pos)
         {
            u8string tmp(s1, pos-s1);
            int find_pos = u8string_len(tmp)+1+index;
            lua_pushinteger(L, find_pos);
            return 1;
         }
         return 0;
     }
     return pluginInvArgs(L,fname);
}

int string_strstr(lua_State *L)
{
    return string_strstr_impl(L, L"string:strstr");
}

int string_find(lua_State *L)
{
    return string_strstr_impl(L, L"string:find");
}

int string_strall(lua_State *L)
{
     if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
     {
         const utf8* s1 = lua_tostring(L, 1);
         const utf8* s2 = lua_tostring(L, 2);
         int len = utf8_strlen(s2);
         if (len == -1) {
             lua_pushnil(L);
             return 1;
         }

         const utf8* b = s1;
         std::vector<int> result;
         const utf8* pos = strstr(s1, s2);
         while (pos)
         {
             u8string tmp(b, pos - b);
             int find_pos = utf8_strlen(tmp.c_str())+1;
             result.push_back(find_pos);
             s1 = pos + len;
             pos = strstr(s1, s2);
         }
         lua_newtable(L);
         for (int i=1,e=result.size();i<=e;++i)
         {
             lua_pushinteger(L, i);
             lua_pushinteger(L, result[i-1]);
             lua_settable(L, -3);
         }
         return 1;
     }
     return pluginInvArgs(L, L"string:strall");
}

int string_lower(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
         tstring s(luaT_towstring(L, 1));
         tstring_tolower(&s);
         luaT_pushwstring(L, s.c_str());
         return 1;
    }
    return pluginInvArgs(L, L"string:lower");
}

int string_upper(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
         tstring s(luaT_towstring(L, 1));
         tstring_toupper(&s);
         luaT_pushwstring(L, s.c_str());
         return 1;
    }
    return pluginInvArgs(L, L"string:upper");
}

int string_lfup(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
         tstring s(luaT_towstring(L, 1));
         if (s.empty())
             return 1;
         tstring_tolower(&s);
         tchar first_letter[2] = { s.at(0), 0 };
         tstring news(first_letter);
         tstring_toupper(&news);
         news.append(s.substr(1));
         luaT_pushwstring(L, news.c_str());
         return 1;
    }
    return pluginInvArgs(L, L"string:lfup");
}

int string_tokenize(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
    {
         tstring s(luaT_towstring(L, 1));
         tstring tokens(luaT_towstring(L, 2));
         lua_newtable(L);
         if (s.empty())
             return 1;
         Tokenizer t(s.c_str(), tokens.c_str());
         t.trimempty();
         for (int i=0,e=t.size();i<e;++i)
         {
             lua_pushinteger(L, i+1);
             luaT_pushwstring(L, t[i]);
             lua_settable(L, -3);
         }
         return 1;
    }
    return pluginInvArgs(L, L"string:tokenize");
}

int string_trim(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring s(luaT_towstring(L, 1));
        tstring_trim(&s);
        luaT_pushwstring(L, s.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"string:trim");
}

int string_only(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
    {
         tstring s(luaT_towstring(L, 1));
         tstring tokens(luaT_towstring(L, 2));
         int result = isOnlySymbols(s, tokens) ? 1 : 0;
         lua_pushboolean(L, result);
         return 1;
    }
    return pluginInvArgs(L, L"string:only");
}


int string_contain(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
    {
         tstring s(luaT_towstring(L, 1));
         tstring tokens(luaT_towstring(L, 2));
         int result = isExistSymbols(s, tokens) ? 1 : 0;
         lua_pushboolean(L, result);
         return 1;
    }
    return pluginInvArgs(L, L"string:contain");
}

int string_clone(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
         std::string s(lua_tostring(L, 1));
         lua_pushstring(L, s.c_str());
         return 1;
    }
    return pluginInvArgs(L, L"string:clone");
}

extern void regFunction(lua_State *L, const char* name, lua_CFunction f);
extern void regIndexMt(lua_State *L);
void reg_string(lua_State *L)
{
    lua_newtable(L);
    regFunction(L, "len", string_len);
    regFunction(L, "substr", string_substr);
    regFunction(L, "strstr", string_strstr);
    regFunction(L, "strall", string_strall);
    regFunction(L, "lower", string_lower);
    regFunction(L, "upper", string_upper);
    regFunction(L, "lfup", string_lfup);
    regFunction(L, "tokenize", string_tokenize);
    regFunction(L, "trim", string_trim);
    regFunction(L, "only", string_only);
    regFunction(L, "clone", string_clone);
    regFunction(L, "find", string_find);
    regFunction(L, "contain", string_contain);
    regIndexMt(L);

    // set metatable for lua string type
    lua_pushstring(L, "");
    lua_insert(L, -2);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}

int props_paletteColor(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int index = lua_tointeger(L, 1);
        if (index >= 0 && index <= 255)
        {
            lua_pushunsigned(L, tortilla::getPalette()->getColor(index));
            return 1;
        }
        return pluginInvArgsValues(L, L"props.paletteColor");
    }
    return pluginInvArgs(L, L"props.paletteColor");
}

int props_backgroundColor(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        lua_pushunsigned(L, tortilla::getProperties()->bkgnd);
        return 1;
    }
    return pluginInvArgs(L, L"props.backgroundColor");
}

int props_currentFont(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        luaT_pushobject(L, tortilla::getCurrentFont(), LUAT_FONT);
        return 1;
    }
    return pluginInvArgs(L, L"props.currentFont");
}

int props_currentFontHandle(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        HFONT handle = tortilla::getCurrentFont()->m_hFont;
        lua_pushunsigned(L, (DWORD)handle);
        return 1;
    }
    return pluginInvArgs(L, L"props.currentFontHandle");
}

int props_cmdPrefix(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        tchar prefix[2] = { tortilla::getProperties()->cmd_prefix, 0 };
        lua_pushstring(L, TW2U(prefix));
        return 1;
    }
    return pluginInvArgs(L, L"props.cmdPrefix");
}

int props_cmdSeparator(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        tchar prefix[2] = { tortilla::getProperties()->cmd_separator, 0 };
        lua_pushstring(L, TW2U(prefix));
        return 1;
    }
    return pluginInvArgs(L, L"props.cmdSeparator");
}

int props_serverHost(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        if (lp()->getConnectionState())
        {
            const NetworkConnectData *cdata = _wndMain.m_gameview.getConnectData();
            lua_pushstring(L, cdata->address.c_str());
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"props.serverHost");
}

int props_serverPort(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        if (lp()->getConnectionState())
        {
            const NetworkConnectData *cdata = _wndMain.m_gameview.getConnectData();
            WCHAR buffer[8];
            _itow(cdata->port, buffer, 10);
            lua_pushstring(L, TW2U(buffer));
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"props.serverPort");
}

int props_connected(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        lua_pushboolean(L, lp()->getConnectionState() ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"props.connected");
}

int props_activated(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        int state = _wndMain.m_gameview.activated() ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, L"props.activated");
}

int props_isPropertiesOpen(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        int state = _wndMain.m_gameview.isPropertiesOpen() ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, L"props.isPropertiesOpen");
}

int props_pluginsLogWindow(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        if (tortilla::getProperties()->plugins_logs)
        {
            int window = tortilla::getProperties()->plugins_logs_window;
            lua_pushinteger(L, window);
            return 1;
        }
        lua_pushboolean(L, 0);
        return 1;
    }
    return pluginInvArgs(L, L"props.pluginsLogWindow");
}

void reg_props(lua_State *L)
{
    lua_newtable(L);
    regFunction(L, "paletteColor", props_paletteColor);
    regFunction(L, "backgroundColor", props_backgroundColor);
    regFunction(L, "currentFont", props_currentFont);
    regFunction(L, "currentFontHandle", props_currentFontHandle);
    regFunction(L, "cmdPrefix", props_cmdPrefix);
    regFunction(L, "cmdSeparator", props_cmdSeparator);
    regFunction(L, "serverHost", props_serverHost);
    regFunction(L, "serverPort", props_serverPort);
    regFunction(L, "connected", props_connected);
    regFunction(L, "activated", props_activated);
    regFunction(L, "isPropertiesOpen", props_isPropertiesOpen);
    regFunction(L, "pluginsLogWindow", props_pluginsLogWindow);    
    lua_setglobal(L, "props");
}
