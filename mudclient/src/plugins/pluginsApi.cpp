#include "stdafx.h"
#include "accessors.h"
#include "pluginsApi.h"
#include "api/api.h"
#include "../MainFrm.h"
#include "pluginSupport.h"
#include "../profiles/profilesPath.h"
#include "plugins/pluginsParseData.h"

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
void pluginLog(const tstring& msg) { lp()->pluginLog(msg);  }
void pluginsUpdateActiveObjects(int type) { lp()->updateActiveObjects(type); }
const wchar_t* lua_types_str[] = {L"nil", L"bool", L"lightud", L"number", L"string", L"table", L"function", L"userdata", L"thread"  };
void collectGarbage() { lua_gc(tortilla::getLua(), LUA_GCSTEP, 1); }
const wchar_t* unknown_plugin = L"?плагин?";
//---------------------------------------------------------------------
MemoryBuffer pluginBuffer(16384*sizeof(wchar_t));
wchar_t* plugin_buffer() { return (wchar_t*)pluginBuffer.getData(); }
int pluginInvArgs(lua_State *L, const utf8* fname)
{
    int n = lua_gettop(L);
    Utf8ToWide f(fname);
    swprintf(plugin_buffer(), L"'%s'.%s: Некорректные параметры(%d): ",
        _cp ? _cp->get(Plugin::FILE) : unknown_plugin, (const wchar_t*)f, n);
    tstring log(plugin_buffer());
    for (int i = 1; i <= n; ++i)
    {
        int t = lua_type(L, i);
        if (t >= 0 && t < LUA_NUMTAGS)
        {
            tstring type(lua_types_str[t]);
            if (t == LUA_TUSERDATA)
                type.assign(TU2W(luaT_typename(L, i)));
            log.append(type);
        }
        else
            log.append(L"unknown");
        if (i != n) log.append(L",");
    }
    pluginLog(log.c_str());
    return 0;
}

int pluginLoadFail(lua_State *L, const utf8* fname, const utf8* file)
{
    Utf8ToWide f(fname); Utf8ToWide fi(file);
    swprintf(plugin_buffer(), L"'%s'.%s: Ошибка загрузки файла: ",
        _cp ? _cp->get(Plugin::FILE) : unknown_plugin, (const wchar_t*)f, (const wchar_t*)fi);
    pluginLog(plugin_buffer());
    return 0;
}

int pluginError(const utf8* fname, const utf8* error)
{
    Utf8ToWide f(fname);
    Utf8ToWide e(error);
    swprintf(plugin_buffer(), L"'%s'.%s: %s", _cp ? _cp->get(Plugin::FILE) : unknown_plugin, (const wchar_t*)f, (const wchar_t*)e);
    pluginLog(plugin_buffer());
    return 0;
}

int pluginError(const utf8* error)
{
    Utf8ToWide e(error);
    swprintf(plugin_buffer(), L"'%s': %s", _cp ? _cp->get(Plugin::FILE) : unknown_plugin, (const wchar_t*)e);
    pluginLog(plugin_buffer());
    return 0;
}

int pluginLog(const utf8* msg)
{
    Utf8ToWide e(msg);
    swprintf(plugin_buffer(), L"'%s': %s", _cp ? _cp->get(Plugin::FILE) : unknown_plugin, (const wchar_t*)e);
    pluginLog(plugin_buffer());
    return 0;
}

void pluginLoadError(const wchar_t* msg, const wchar_t *fname)
{
    swprintf(plugin_buffer(), L"'%s': Ошибка загрузки! %s", fname, msg);
    pluginLog(plugin_buffer());
}
//---------------------------------------------------------------------
int pluginName(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        _extra_plugin_name.assign(TU2W(lua_tostring(L, 1))); 
        return 0;
    }
    return pluginInvArgs(L, "pluginName");
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
    return pluginInvArgs(L, "addCommand");
}

int runCommand(lua_State *L)
{
    EXTRA_CP;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring cmd(TU2W(lua_tostring(L, 1)));
        lp()->processPluginCommand(cmd);
        return 0;
    }
    return pluginInvArgs(L, "runCommand");
}

int addMenu(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (!_cp)
         return pluginInvArgs(L, "addMenu");
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
    else { return pluginInvArgs(L, "addMenu"); }
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
    return pluginInvArgs(L, "checkMenu");
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
    return pluginInvArgs(L, "uncheckMenu");
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
    return pluginInvArgs(L, "enableMenu");
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
    return pluginInvArgs(L, "disableMenu");
}

int addButton(lua_State *L)
{
    CAN_DO;
    EXTRA_CP;
    if (!_cp) 
        return pluginInvArgs(L, "addButton");
    int image = -1, code = -1; tstring hover;
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
    {
        image = lua_tointeger(L, 1); code = lua_tointeger(L, 2);
    }
    else if (luaT_check(L, 3, LUA_TNUMBER, LUA_TNUMBER, LUA_TSTRING))
    {
        image = lua_tointeger(L, 1); code = lua_tointeger(L, 2); hover.assign(luaT_towstring(L, 3));
    }
    else { return pluginInvArgs(L, "addButton"); }

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
        return pluginInvArgs(L, "addToolbar");
    if (luaT_check(L, 1, LUA_TSTRING))
        tbar()->addToolbar(luaT_towstring(L, 1), 15);
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
        tbar()->addToolbar(luaT_towstring(L, 1), lua_tointeger(L, 2));
    else { return pluginInvArgs(L, "addToolbar"); }
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
    return pluginInvArgs(L, "hideToolbar");
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
    return pluginInvArgs(L, "showToolbar");
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
        return pluginError("getPath", "Ошибка создания каталога для плагина");
    }
    return pluginInvArgs(L, "getPath");
}

int getResource(lua_State* L)
{
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
            tstring path(cd.getCurrentDir());
            path.append(rd);
            path.append(L"\\");
            path.append(pd);
            path.append(L"\\");
            path.append(filename);
            luaT_pushwstring(L, path.c_str());
            return 1;
        }
        return pluginError("getResource", "Ошибка создания каталога для плагина");
    }
    return pluginInvArgs(L, "getResource");
}

int getProfile(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        PropertiesManager *pmanager = tortilla::getPropertiesManager();
        luaT_pushwstring(L, pmanager->getProfileName().c_str());
        return 1;
    }
    return pluginInvArgs(L, "getProfile");
}

int getParent(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        HWND hwnd = _wndMain; //.m_gameview;
        lua_pushunsigned(L, (DWORD)hwnd);
        return 1;
    }
    return pluginInvArgs(L, "getParent");
}

int loadTable(lua_State *L)
{
    EXTRA_CP;
    if (!_cp || !luaT_check(L, 1, LUA_TSTRING))
        return pluginInvArgs(L, "loadData");

    PropertiesManager *pmanager = tortilla::getPropertiesManager();
    tstring filename(luaT_towstring(L, 1));
    ProfilePluginPath pp(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
    filename.assign(pp);

    DWORD fa = GetFileAttributes(filename.c_str());
    if (fa == INVALID_FILE_ATTRIBUTES || fa&FILE_ATTRIBUTE_DIRECTORY)
        return 0;
    xml::node doc;
    if (!doc.load(WideToUtf8(pp)) )
    {
       W2U f(filename);
       utf8 buffer[128];
       sprintf(buffer, "Ошибка чтения: %s", (const utf8*)f);
       pluginError("loadData", buffer);
       return 0;
    }
    lua_pop(L, 1);

    struct el { el(xml::node n, int l) : node(n), level(l) {} xml::node node; int level; };
    std::vector<el> stack;
    xml::request req(doc, "*");
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

       u8string name, val;
       n.getname(&name);
       if (name == "array") 
       { //can be number index
         if (n.get("index", &array_index))
             it_array = true;
       }
       if (n.get("value", &val))
       {   // it is simple value
           if (it_array)
             lua_pushinteger(L, array_index);
           else
             lua_pushstring(L, name.c_str());
           lua_pushstring(L, val.c_str());
           lua_settable(L, -3);
       }
       else
       {   // it is table
           lua_newtable(L);
           if (it_array)
               lua_pushinteger(L, array_index);
           else
               lua_pushstring(L, name.c_str());
           lua_pushvalue(L, -2);
           lua_settable(L, -4);

           // insert subnodes
           int new_level = s_el.level + 1;
           xml::request req(n, "*");
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
           lua_pop(L, 1);
   }
   doc.deletenode();
   return 1;
}

int saveTable(lua_State *L)
{
    EXTRA_CP;
    if (!_cp || !luaT_check(L, 2, LUA_TTABLE, LUA_TSTRING))
        return pluginInvArgs(L, "saveData");

    tstring filename(luaT_towstring(L, 2));
    lua_pop(L, 1);

    // recursive cycles in table
    struct saveDataNode
    {
        typedef std::pair<std::string, std::string> value;
        typedef std::map<int, std::string> tarray;
        std::vector<value> attributes;
        std::vector<saveDataNode*> childnodes;
        std::string name;
        tarray array;
    };

    saveDataNode *current = new saveDataNode();
    current->name = "plugindata";

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
            if (key_type == LUA_TNUMBER)
            {
                if (value_type == LUA_TNUMBER || value_type == LUA_TSTRING)
                {
                    int index = lua_tointeger(L, -2);
                    current->array[index] = lua_tostring(L, -1);
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
                current->attributes.push_back( saveDataNode::value( lua_tostring(L, -2), lua_tostring(L, -1)) );
            }
            else if (value_type == LUA_TTABLE)
            {
                saveDataNode* new_node = new saveDataNode();
                if (key_type == LUA_TNUMBER)
                    new_node->name = current->name;
                else
                    new_node->name = lua_tostring(L, -2);
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
            attr.set("value", a[i].second.c_str());
        }
        saveDataNode::tarray &ta = v.first->array;
        saveDataNode::tarray::iterator it = ta.begin(), it_end = ta.end();
        for(; it!=it_end; ++it)
        {
            xml::node arr = node.createsubnode("array");
            arr.set("index", it->first);
            arr.set("value", it->second.c_str());
        }
        std::vector<saveDataNode*>&n = v.first->childnodes;
        for (int i = 0, e = n.size(); i < e; ++i)
        {
            xml::node new_node = node.createsubnode(n[i]->name.c_str());
            xmlstack.push_back(_xmlstack(n[i], new_node));
        }
    }
    xmlstack.clear();

    // delete temporary datalist
    for (int i = 0, e = list.size(); i < e; ++i)
       delete list[i];
    list.clear();

    // save xml
    PropertiesManager *pmanager = tortilla::getPropertiesManager();
    ProfilePluginPath pp(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
    const wchar_t *filepath = pp;

    int result = 0;
    ProfileDirHelper dh;
    if (dh.makeDirEx(pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename))
    {
        result = root.save(WideToUtf8(filepath));
    }

    if (incorrect_data)
        pluginError("saveData", "Неверные данные в исходных данных.");

    if (!result)
    {
       W2U f(filepath);
       utf8 buffer[128];
       sprintf(buffer, "Ошибка записи: %s", (const utf8*)f);
       pluginError("saveData", buffer);
    }
    root.deletenode();
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
        return pluginInvArgs(L, "createWindow");
    PluginData &p = find_plugin();
    OutputWindow w;

    if (luaT_check(L, 1, LUA_TSTRING) || luaT_check(L, 2, LUA_TSTRING, LUA_TBOOLEAN))
    {
        tstring name( U2W(lua_tostring(L, 1)) );
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
        tstring name(U2W(lua_tostring(L, 1)));
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
        return pluginInvArgs(L, "createWindow"); 
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
        return pluginInvArgs(L, "log");

    u8string log;
    for (int i = 1; i <= n; ++i)
    {
        u8string el;
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
    u8string log;
    for (int i = 1; i <= n; ++i)
    {
        u8string el;
        pluginFormatByType(L, i, &el);
        log.append(el);
    }
    _cp->setErrorState();

    if (log.empty())
        log.assign("TERMINATE");
    lua_settop(L, 0);
    lua_pushstring(L, log.c_str());
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
                PluginsParseData ppd(&pd);
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
    }
    return pluginInvArgs(L, "updateView");
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
    }
    return pluginInvArgs(L, "getViewSize");
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
    return pluginInvArgs(L, "flashWindow");
}

int regUnloadFunction(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TFUNCTION))
    {
        lua_getglobal(L, "munloadf");
        if (!lua_istable(L, -1))
        {
            if (!lua_isnil(L, -1))
            {
                lua_pop(L, 1);
                lua_pushboolean(L, 0);
                return 1;
            }
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setglobal(L, "munloadf");
        }
        lua_len(L, -1);
        int index = lua_tointeger(L, -1) + 1;
        lua_pop(L, 1);
        lua_insert(L, -2);
        lua_pushinteger(L, index);
        lua_insert(L, -2);
        lua_settable(L, -3);
        lua_pop(L, 1);
        lua_pushboolean(L, 1);
        return 1;
    }
    lua_pushboolean(L, 0);
    return 1;
}
//---------------------------------------------------------------------
// Metatables for all types
void reg_mt_window(lua_State *L);
void reg_mt_viewdata(lua_State *L);
void reg_activeobjects(lua_State *L);
void reg_string(lua_State *L);
void reg_props(lua_State *L);
void reg_mt_panels(lua_State *L);
void reg_mt_render(lua_State *L);
void reg_mt_pcre(lua_State *L);
void reg_msdp(lua_State *L);
void reg_mt_image(lua_State *L);
//---------------------------------------------------------------------
bool initPluginsSystem()
{
    tortilla::init();
    lua_State*L = tortilla::getLua();    
    if (!L)
        return false;

    luaopen_base(L);
    lua_pop(L, 1);
    luaopen_math(L);
    lua_setglobal(L, "math");
    luaopen_table(L);
    lua_setglobal(L, "table");
    reg_string(L);
    lua_register(L, "addCommand", addCommand);
    lua_register(L, "runCommand", runCommand);
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
    lua_register(L, "flashWindow", flashWindow);
    lua_register(L, "pluginName", pluginName);
    lua_register(L, "regUnloadFunction", regUnloadFunction);

    reg_props(L);
    reg_activeobjects(L);
    reg_mt_window(L);
    reg_mt_viewdata(L);
    reg_mt_panels(L);
    reg_mt_image(L);
    reg_mt_render(L);
    reg_mt_pcre(L);
    reg_msdp(L);
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
    lua_pushnil(L);
    return 1;
}

int string_strstr(lua_State *L)
{
     if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING) ||
         luaT_check(L, 3, LUA_TSTRING, LUA_TSTRING, LUA_TNUMBER))
     {
         const utf8* s1 = lua_tostring(L, 1);
         if (lua_gettop(L) == 3)
         {
             int index = lua_tointeger(L, 3);
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
            int find_pos = u8string_len(tmp) + 1;
            lua_pushinteger(L, find_pos);
            return 1;
         }
     }
     lua_pushnil(L);
     return 1;
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
     lua_pushnil(L);
     return 1;
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
    }
    return pluginInvArgs(L, "props.paletteColor");
}

int props_backgroundColor(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        lua_pushunsigned(L, tortilla::getProperties()->bkgnd);
        return 1;
    }
    return pluginInvArgs(L, "props.backgroundColor");
}

int props_currentFont(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        luaT_pushobject(L, tortilla::getCurrentFont(), LUAT_FONT);
        return 1;
    }
    return pluginInvArgs(L, "props.currentFont");
}

int props_currentFontHandle(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        HFONT handle = tortilla::getCurrentFont()->m_hFont;
        lua_pushunsigned(L, (DWORD)handle);
        return 1;
    }
    return pluginInvArgs(L, "props.currentFontHandle");
}

int props_cmdPrefix(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        tchar prefix[2] = { tortilla::getProperties()->cmd_prefix, 0 };
        lua_pushstring(L, TW2U(prefix));
        return 1;
    }
    return pluginInvArgs(L, "props.cmdPrefix");
}

int props_cmdSeparator(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        tchar prefix[2] = { tortilla::getProperties()->cmd_separator, 0 };
        lua_pushstring(L, TW2U(prefix));
        return 1;
    }
    return pluginInvArgs(L, "props.cmdSeparator");
}

int props_serverHost(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        if (lp()->getConnectionState())
        {
            const NetworkConnectData *cdata = _wndMain.m_gameview.getConnectData();
            lua_pushstring(L, cdata->address.c_str());
        }
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, "props.serverHost");
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
        }
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, "props.serverPort");
}

int props_connected(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        lua_pushboolean(L, lp()->getConnectionState() ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "props.connected");
}

int props_activated(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        int state = _wndMain.m_gameview.activated() ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "props.activated");
}

int props_settingsWnd(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        int state = _wndMain.m_gameview.isSettingsWindowOpen() ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "props.settingsWnd");
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
    regFunction(L, "settingsWnd", props_settingsWnd);
    lua_setglobal(L, "props");
}
