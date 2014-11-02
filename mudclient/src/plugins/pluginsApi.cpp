#include "stdafx.h"
#include "pluginsApi.h"
#include "api/api.h"
#include "../MainFrm.h"
#include "pluginSupport.h"
#include "plugin.h"
#include "../profiles/profilesPath.h"

#define CAN_DO if (!_wndMain.IsWindow()) return 0;
extern CMainFrame _wndMain;
extern Plugin* _cp;
ToolbarEx<CMainFrame>* _tbar;
LogicProcessorMethods* _lp;
PropertiesData* _pdata;
Palette256* _palette;
PropertiesManager* _pmanager;
PluginsIdTableControl m_idcontrol(PLUGING_MENUID_START, PLUGING_MENUID_END);
luaT_State L;

void initExternPtrs()
{
    _tbar = &_wndMain.m_toolBar;
    _lp = _wndMain.m_gameview.getMethods();
    _pdata = _wndMain.m_gameview.getPropData();
    _palette = _wndMain.m_gameview.getPalette();
    _pmanager = _wndMain.m_gameview.getPropManager();
}
//--------------------------------------------------------------------
UINT getId(int code, bool button) { return m_idcontrol.registerPlugin(_cp, code, button); }
UINT delCode(int code, bool button) { return m_idcontrol.unregisterByCode(_cp, code, button); }
UINT findId(int code, bool button) { return m_idcontrol.findId(_cp,code,button); }
void delId(UINT id) { m_idcontrol.unregisterById(_cp, id); }
void pluginsMenuCmd(UINT id) { m_idcontrol.runPluginCmd(id); }
void tmcLog(const tstring& msg) { _lp->tmcLog(msg); }
void pluginLog(const tstring& msg) { _lp->pluginLog(msg);  }
void pluginsUpdateActiveObjects(int type) { _lp->updateActiveObjects(type); }
//---------------------------------------------------------------------
wchar_t plugin_buffer[1024];
int pluginInvArgs(lua_State *L, const utf8* fname) 
{
    Utf8ToWide f(fname);
    swprintf(plugin_buffer, L"'%s'.%s: Неверный набор параметров (%d шт.)", _cp->get(Plugin::NAME), (const wchar_t*)f, lua_gettop(L));
    pluginLog(plugin_buffer);
    return 0; 
}

int pluginError(lua_State *L, const utf8* fname, const utf8* error)
{
    Utf8ToWide f(fname);
    Utf8ToWide e(error);
    swprintf(plugin_buffer, L"'%s'.%s: %s", _cp->get(Plugin::NAME), (const wchar_t*)f, (const wchar_t*)e);
    pluginLog(plugin_buffer);
    return 0;
}

int pluginError(lua_State *L, const utf8* error)
{
    Utf8ToWide e(error);
    swprintf(plugin_buffer, L"'%s': %s", _cp->get(Plugin::NAME), (const wchar_t*)e);
    pluginLog(plugin_buffer);
    return 0;
}

int pluginLog(lua_State *L, const utf8* msg)
{
    Utf8ToWide e(msg);
    swprintf(plugin_buffer, L"'%s': %s", _cp->get(Plugin::NAME), (const wchar_t*)e);
    pluginLog(plugin_buffer);
    return 0;
}
//---------------------------------------------------------------------
int addcommand(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        bool result = false;
        tstring cmd(luaT_towstring(L, 1));
        if (!cmd.empty())
            result = _lp->addSystemCommand(cmd);
        if (result)
            _cp->commands.push_back(cmd);
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "addCommand");
}

int addmenu(lua_State *L)
{
    CAN_DO;
    int code = -1;
    bool params_ok = false;
    if (luaT_check(L, 1, LUA_TSTRING))
        params_ok = _tbar->addMenuItem(luaT_towstring(L, 1), -1, -1, NULL);
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
    {
        code = lua_tointeger(L, 2);
        params_ok = _tbar->addMenuItem(luaT_towstring(L, 1), -1, getId(code, false), NULL);
    }
    else if (luaT_check(L, 3, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER))
    {
        code = lua_tointeger(L, 2);
        params_ok = _tbar->addMenuItem(luaT_towstring(L, 1), lua_tointeger(L, 3), getId(code, false), NULL);
    }
    else if (luaT_check(L, 4, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        HBITMAP bmp = NULL;
        HMODULE module = _cp->getModule();
        if (module) bmp = LoadBitmap( module, MAKEINTRESOURCE(lua_tointeger(L, 4)) );
        code = lua_tointeger(L, 2);
        params_ok = _tbar->addMenuItem(luaT_towstring(L, 1), lua_tointeger(L, 3), getId(code, false), bmp);
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

int checkmenu(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        _tbar->checkMenuItem(findId(code, false), TRUE);
        _tbar->checkToolbarButton(findId(code, true), TRUE);
    }
    else { return pluginInvArgs(L, "checkMenu"); }
    return 0;
}

int uncheckmenu(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        _tbar->checkMenuItem(findId(code, false), FALSE);
        _tbar->checkToolbarButton(findId(code, true), FALSE);
    }
    else { return pluginInvArgs(L, "uncheckMenu"); }
    return 0;
}

int enablemenu(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        _tbar->enableMenuItem(findId(code, false), TRUE);
        _tbar->enableToolbarButton(findId(code, true), TRUE);
    }
    else { return pluginInvArgs(L, "enableMenu"); }
    return 0;
}

int disablemenu(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int code = lua_tointeger(L, -1);
        _tbar->enableMenuItem(findId(code, false), FALSE);
        _tbar->enableToolbarButton(findId(code, true), FALSE);
    }
    else { return pluginInvArgs(L, "disableMenu"); }
    return 0;
}

int addbutton(lua_State *L)
{
    CAN_DO;
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
            if (!_tbar->addToolbarButton(bmp, getId(code, true), hover.c_str()))
                delCode(code, true);
            else
                _cp->buttons.push_back(code);
        }
    }        
    return 0;
}

int addtoolbar(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TSTRING))
        _tbar->addToolbar(luaT_towstring(L, 1), 15);
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TNUMBER))
        _tbar->addToolbar(luaT_towstring(L, 1), lua_tointeger(L, 2));
    else { return pluginInvArgs(L, "addToolbar"); }
    tstring tbar_name(luaT_towstring(L, 1));
    _cp->toolbars.push_back(tbar_name);
    return 0;
}

int hidetoolbar(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TSTRING))
        _tbar->hideToolbar(luaT_towstring(L, 1));
    else { return pluginInvArgs(L, "hideToolbar"); }
    return 0;
}

int showtoolbar(lua_State *L)
{
    CAN_DO;
    if (luaT_check(L, 1, LUA_TSTRING))
        _tbar->showToolbar(luaT_towstring(L, 1));
    else { return pluginInvArgs(L, "showToolbar"); }
    return 0;
}

int getpath(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring filename(luaT_towstring(L, 1));
        ProfilePluginPath pp(_pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
        ProfileDirHelper dh;
        if (dh.makeDir(_pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME)))
        {
            luaT_pushwstring(L, pp);
            return 1;
        }
        else
        {
            return pluginError(L, "getPath", "Ошибка создания каталога для плагина");
        }
    }
    else { return pluginInvArgs(L, "getPath"); }
    lua_pushnil(L);
    return 1;
}

int getprofile(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        luaT_pushwstring(L, _pmanager->getProfileName().c_str());
        return 1;
    }    
    return pluginInvArgs(L, "getProfile");
}

int getparent(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        HWND hwnd = _wndMain.m_gameview;
        lua_pushunsigned(L, (DWORD)hwnd);
        return 1;
    }
    return pluginInvArgs(L, "getParent");
}

int loadtable(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring filename(luaT_towstring(L, 1));
        ProfilePluginPath pp(_pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
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
            pluginError(L, "loadData", buffer);
            return 0;
        }
        lua_pop(L, 1);
       
        struct el { el(xml::node n, int l) : node(n), level(l) {} xml::node node; int level; };
        std::vector<el> stack;
        xml::request req(doc, "*");
        for (int i = 0, e = req.size(); i < e; ++i)
            { stack.push_back( el(req[i], 0) ); }

        lua_newtable(L);            // root (main) table 
        bool pop_stack_flag = false;
        int p = 0;
        int size = stack.size();
        while (p != size)
        {
            if (!pop_stack_flag)
                lua_newtable(L);
            pop_stack_flag = false;

            el s_el = stack[p++];
            xml::node n = s_el.node;
            std::string aname, avalue;
            for (int j = 0, atts = n.size(); j < atts; ++j)
            {
                n.getattrname(j, &aname);
                n.getattrvalue(j, &avalue);
                lua_pushstring(L, aname.c_str());
                lua_pushstring(L, avalue.c_str());
                lua_settable(L, -3);
            }

            lua_pushvalue(L, -1);
            std::string name;
            n.getname(&name);
            lua_pushstring(L, name.c_str());
            lua_insert(L, -2);
            lua_settable(L, -4);

            // insert subnodes
            {
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
                lua_pop(L, 1);
                pop_stack_flag = true;
            }
            else if (s_el2.level == s_el.level)
            {
                lua_pop(L, 1);
            }
        }
        doc.deletenode();
        return 1;
    }
    else { return pluginInvArgs(L, "loadData"); }
    return 0;
}

int savetable (lua_State *L)
{
    if (!luaT_check(L, 2, LUA_TTABLE, LUA_TSTRING))
        return pluginInvArgs(L, "saveData");

    tstring filename(luaT_towstring(L, 2));
    lua_pop(L, 1);
    
    // recursive cycles in table
    struct saveDataNode
    {
        typedef std::pair<std::string, std::string> value;         
        std::vector<value> attributes;
        std::vector<saveDataNode*> childnodes;
        std::string name;
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
                if (value_type != LUA_TTABLE) {
                    lua_pop(L, 1);
                    incorrect_data = true;
                    continue;
                }
                int x = 1;
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
            node.set(a[i].first.c_str(), a[i].second.c_str());
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
    ProfilePluginPath pp(_pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename);
    const wchar_t *filepath = pp;

    int result = 0;
    ProfileDirHelper dh;
    if (dh.makeDirEx(_pmanager->getProfileGroup(), _cp->get(Plugin::FILENAME), filename))
    {
        result = root.save(WideToUtf8(filepath));
    }

    if (incorrect_data)
        pluginError(L, "saveData", "Неверные данные в исходных данных.");

    if (!result)
    {
       W2U f(filepath);
       utf8 buffer[128];
       sprintf(buffer, "Ошибка записи: %s", (const utf8*)f);
       pluginError(L, "saveData", buffer);
    }
    root.deletenode();
    return 0;
}
//----------------------------------------------------------------------------
int find_plugin()
{
    tstring plugin_name(_cp->get(Plugin::FILE));
    int index = -1;
    for (int i = 0, e = _pdata->plugins.size(); i < e; ++i)
    {
        const PluginData &p = _pdata->plugins[i];
        if (p.name == plugin_name)
            return i;
    }
    return -1;
}

bool find_window(const tstring& window_name, OutputWindow *w)
{
    int plugin = find_plugin();
    if (plugin == -1)
        return false;
    const PluginData &p = _pdata->plugins[plugin];
    for (int j = 0, je = p.windows.size(); j < je; ++j)
    {
        if (p.windows[j].name == window_name)
        {
           *w = p.windows[j];
           return true;
        }
    }
    return false;      
}

int createwindow(lua_State *L)
{
    int index = find_plugin();
    if (index == -1)
    {
        PluginData pd; pd.name = _cp->get(Plugin::FILE);
        pd.state = _cp->state() ? 1 : 0;
        _pdata->plugins.push_back(pd);
        index = _pdata->plugins.size() - 1;
    }

    PluginData &p = _pdata->plugins[index];
    OutputWindow w;
    p.initDefaultPos(300, 300, &w);

    if (luaT_check(L, 1, LUA_TSTRING))
    {
        tstring name( U2W(lua_tostring(L, 1)) );
        if (!find_window(name, &w))
        {
            w.name = name;
            p.windows.push_back(w);
        }
    }
    else if (luaT_check(L, 3, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER ))
    {
        tstring name(U2W(lua_tostring(L, 1)));
        if (!find_window(name, &w))
        {
            int height = lua_tointeger(L, 3);
            int width = lua_tointeger(L, 2);
            p.initDefaultPos(width, height, &w);
            w.name = name;
            p.windows.push_back(w);
        }
    }
    else { return pluginInvArgs(L, "createWindow"); }
   
    PluginsView *window =  _wndMain.m_gameview.createDockPane(w, _cp->get(Plugin::FILE));
    if (window)
        _cp->views.push_back(window);

    luaT_pushobject(L, window, LUAT_WINDOW);
    return 1;
}

int pluginlog(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        pluginLog(L, lua_tostring(L, 1));
        return 0;
    }
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        pluginLog(L, lua_tostring(L, 1));
        return 0;
    }
    return pluginInvArgs(L, "log");    
}
//---------------------------------------------------------------------
// Metatables for all types
void reg_mt_window(lua_State *L);
void reg_mt_viewdata(lua_State *L);
void reg_activeobjects(lua_State *L);
void reg_string(lua_State *L);
//---------------------------------------------------------------------
bool initPluginsSystem()
{
    initExternPtrs();
    if (!L)
        return false;    

    luaopen_base(L);
    lua_pop(L, 1);
    luaopen_table(L);
    lua_setglobal(L, "table");
    reg_string(L);    
    lua_register(L, "addCommand", addcommand);
    lua_register(L, "addMenu", addmenu);
    lua_register(L, "addButton", addbutton);
    lua_register(L, "addToolbar", addtoolbar);
    lua_register(L, "hideToolbar", hidetoolbar);
    lua_register(L, "showToolbar", showtoolbar);
    lua_register(L, "checkMenu", checkmenu);
    lua_register(L, "uncheckMenu", uncheckmenu);
    lua_register(L, "enableMenu", enablemenu);
    lua_register(L, "disableMenu", disablemenu);
    lua_register(L, "getPath", getpath);
    lua_register(L, "getProfile", getprofile);
    lua_register(L, "getParent", getparent);    
    //todo lua_register(L, "loadTable", loadtable);
    //lua_register(L, "saveTable", savetable);
    lua_register(L, "createWindow", createwindow);
    lua_register(L, "log", pluginlog);
    reg_activeobjects(L);
    reg_mt_window(L);
    reg_mt_viewdata(L);
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
        _tbar->deleteMenuItem(plugin->menus[i].c_str(), &ids);
        for (int i = 0, e = ids.size(); i < e; ++i)
            delId(ids[i]);
    }
    plugin->menus.clear();
    for (int i = 0, e = plugin->buttons.size(); i < e; ++i)
        _tbar->deleteToolbarButton(delCode(plugin->buttons[i], true));
    plugin->buttons.clear();
    for (int i = 0, e = plugin->toolbars.size(); i<e; ++i)
        _tbar->deleteToolbar(plugin->toolbars[i].c_str());
    plugin->toolbars.clear();
    for (int i = 0, e = plugin->views.size(); i < e; ++i)
    {
        PluginsView *v = plugin->views[i];
        _wndMain.m_gameview.deleteDockPane(v);
    }
    plugin->views.clear();

    // delete all system commands of plugin
    for (int i = 0, e = plugin->commands.size(); i < e; ++i)
        _lp->deleteSystemCommand(plugin->commands[i]);
    plugin->commands.clear();
    _cp = old;
}

