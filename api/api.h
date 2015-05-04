#pragma once

#include "base.h"

#ifndef API_EXPORTS
#pragma comment(lib, "api.lib")
#endif

#include <string>
#include <vector>
typedef char utf8;
typedef std::string u8string;

//lua api
#define LUAT_WINDOW     100
#define LUAT_VIEWDATA   101
#define LUAT_ACTIVEOBJS 102
#define LUAT_PANEL      103
#define LUAT_RENDER     104
#define LUAT_PEN        105
#define LUAT_BRUSH      106
#define LUAT_FONT       107
#define LUAT_PCRE       108
#define LUAT_LAST       108

bool  luaT_check(lua_State *L, int n, ...);
bool  luaT_run(lua_State *L, const utf8* func, const utf8* op, ...);
int   luaT_error(lua_State *L, const utf8* error_message);
void  luaT_log(lua_State *L, const utf8* log_message);
void* luaT_toobject(lua_State* L, int index);
void  luaT_pushobject(lua_State* L, void *object, int type);
bool  luaT_isobject(lua_State* L, int type, int index);
const utf8* luaT_typename(lua_State* L, int index);
void  luaT_showLuaStack(lua_State* L, const utf8* label);
void  luaT_showTableOnTop(lua_State* L, const utf8* label);
#define SS(L,n) luaT_showLuaStack(L,n)
#define ST(L,n) luaT_showTableOnTop(L,n)

namespace base {
    inline void addMenu(lua_State* L, const utf8* path, int pos, int id) {
        luaT_run(L, "addMenu", "sdd", path, pos, id); 
    }
    inline void addCommand(lua_State* L, const utf8* cmd) {
        luaT_run(L, "addCommand", "s", cmd);
    }
    inline void runCommand(lua_State* L, const utf8* cmd)  {
        luaT_run(L, "runCommand", "s", cmd);
    }
    inline void addButton(lua_State *L, int bmp, int id, const utf8* tooltip) {
        luaT_run(L, "addButton", "dds", bmp, id, tooltip);
    }
    inline void addToolbar(lua_State *L, const utf8* name, int button_size) {
        luaT_run(L, "addToolbar","sd", name, button_size);
    }
    inline void addToolbar(lua_State *L, const utf8* name) {
        luaT_run(L, "addToolbar","s", name);
    }
    inline void showToolbar(lua_State *L, const utf8* name) {
        luaT_run(L, "showToolbar","s", name);
    }
    inline void hideToolbar(lua_State *L, const utf8* name) {
        luaT_run(L, "hideToolbar","s", name);
    }
    inline void checkMenu(lua_State *L, int id) {
        luaT_run(L, "checkMenu","d", id);
    }
    inline void uncheckMenu(lua_State *L, int id) {
        luaT_run(L, "uncheckMenu","d", id);
    }
    inline void enableMenu(lua_State *L, int id) {
        luaT_run(L, "enableMenu","d", id);
    }
    inline void disableMenu(lua_State *L, int id) {
        luaT_run(L, "disableMenu","d", id);
    }
    inline void getPath(lua_State *L, const utf8* file, u8string* path) {
        luaT_run(L, "getPath", "s", file);
        path->assign(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    inline void getProfile(lua_State *L, u8string* profile) {
        luaT_run(L, "getProfile", "");
        profile->assign(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    inline HWND getParent(lua_State *L) {
        luaT_run(L, "getParent", "");
        HWND parent = (HWND)lua_tounsigned(L, -1);
        lua_pop(L, 1);
        return parent;
    }
    inline void saveTable(lua_State* L, const utf8* file) {
        if (lua_istable(L, -1))
            luaT_run(L, "saveTable", "rs", file);
    }
    inline bool loadTable(lua_State* L, const utf8* file) {
        luaT_run(L, "loadTable", "s", file);
        return lua_istable(L, -1) ? true : false;
    }
    // createWindow, createPanel, pcre -> classes below
    // log -> luaT_log
} // namespace base

class luaT_window
{
    lua_State *L;
    void *window;
public:
    luaT_window() : L(NULL), window(NULL) {}
	bool create(lua_State *pL, const utf8* caption, int width, int height, bool visible)
	{
		if (!pL)
			return false;
		L = pL;
		luaT_run(L, "createWindow", "sddb", caption, width, height, visible);
		void *wnd = luaT_toobject(L, -1);
		if (!wnd)
			return false;
		window = wnd;
		return true;
	}
    HWND hwnd()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "hwnd", "o");
        HWND hwnd = (HWND)lua_tounsigned(L, -1);
        lua_pop(L, 1);
        return hwnd;
    }
    HWND floathwnd()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "floathwnd", "o");
        HWND hwnd = (HWND)lua_tounsigned(L, -1);
        lua_pop(L, 1);
        return hwnd;
    }
    void hide()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "hide", "o");
    }
    void show()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "show", "o");
    }
    bool isVisible()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "isVisible", "o");
        bool result = (lua_toboolean(L, -1) == 0) ? false : true;
        lua_pop(L, 1);
        return result;
    }
    void dock(const utf8* side)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "dock", "os", side);
    }
    void undock()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "undock", "o");
    }
    void block(const utf8* side)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "block", "os", side);
    }
    void attach(HWND child)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "attach", "od", child);
    }
    void setBlocked(int width, int height)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "setBlocked", "odd", width, height);
    }
};

class luaT_panel
{
    lua_State *L;
    void *panel;
public:
    luaT_panel() : L(NULL), panel(NULL) {}
    bool create(lua_State *pL, const utf8* side, int size)
    {
        if (!pL)
            return false;
        L = pL;
        luaT_run(L, "createPanel", "sd", side, size);
        void *p = luaT_toobject(L, -1);
        if (!p)
            return false;
        panel = p;
        return true;
    }
    void attach(HWND child)
    {
        luaT_pushobject(L, panel, LUAT_PANEL);
        luaT_run(L, "attach", "od", child);
    }
    HWND hwnd()
    {
        luaT_pushobject(L, panel, LUAT_PANEL);
        luaT_run(L, "hwnd", "o");
        HWND hwnd = (HWND)lua_tounsigned(L, -1);
        lua_pop(L, 1);
        return hwnd;
    }
};

class Pcre;
class luaT_ViewData
{
    lua_State *L;
    void *view_data;
public:
    enum vdparam { TEXTCOLOR = 0, BKGCOLOR, UNDERLINE, ITALIC, BLINK, REVERSE, EXTTEXTCOLOR, EXTBKGCOLOR };

    luaT_ViewData() : L(NULL), view_data(NULL) {}
    void init(lua_State *pL, void *viewdata) { L = pL; view_data = viewdata; }
    int size()             // count of all strings
    {
        runcmd("size");
        return intresult(); 
    }
    bool select(int index) // select string for operations
    {
        runcmdint("select", index);
        return boolresult();
    }
    int getIndex()
    {
        runcmd("getIndex");
        return intresult();
    }
    bool isFirst()
    {
        runcmd("isFirst");
        return boolresult(); 
    }
    bool isLast()
    {
        runcmd("isLast");
        return boolresult();
    }
    bool isGameCmd()
    {
        runcmd("isGameCmd");
        return boolresult();
    }
    bool isSystem()
    {
        runcmd("isSystem");
        return boolresult();
    }
    bool isPrompt()
    {
        runcmd("isPrompt");
        return boolresult();
    }
    void getPrompt(u8string *str)
    {
        runcmd("getPrompt");
        strresult(str);
    }
    void getText(u8string* str) 
    {
        runcmd("getText");
        strresult(str); 
    }
    int getTextLen()
    {
        runcmd("getTextLen");
        return intresult();
    }
    void getHash(u8string* str)
    {
        runcmd("getHash");
        strresult(str);
    }
    int blocks()            // count of blocks for selected string
    { 
        runcmd("blocks");
        return intresult(); 
    }
    void getBlockText(int block, u8string* str)
    {
        runcmdint("getBlockText", block);
        strresult(str);
    }
    bool get(int block, vdparam param, unsigned int *value)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "get", "odd", block, (int)param);
        bool result = false;
        if (lua_isnumber(L, -1)) { *value = lua_tounsigned(L, -1); result = true; }
        lua_pop(L, 1);
        return result;
    }
    bool set(int block, vdparam param, unsigned int value)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "set", "oddu", block, (int)param, value);
        return boolresult();
    }
    bool setBlockText(int block, const utf8* text)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "setBlockText", "ods", block, text);
        return boolresult();
    }
    bool copyBlock(int block, int dst_string, int dst_block)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "copyBlock", "oddd", block, dst_string, dst_block);
        return boolresult();
    }
    bool deleteBlock(int block)
    {
        runcmdint("deleteBlock", block);
        return boolresult();
    }
    bool deleteAllBlocks()
    {
        runcmd("deleteAllBlocks");
        return boolresult();
    }
    bool createString()
    {
        runcmd("createString");
        return boolresult();
    }
    bool createString(bool system, bool gamecmd)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "createString", "obb", system, gamecmd);
        return boolresult();
    }
    bool deleteString()
    {
        runcmd("deleteString");
        return boolresult();
    }
    bool find(Pcre *p)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_pushobject(L, p, LUAT_PCRE);
        luaT_run(L, "find", "ot");
        return boolresult();
    }

private:
    void runcmd(const utf8* cmd)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, cmd, "o");
    }
    void runcmdint(const utf8* cmd, int p)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, cmd, "od", p);
    }
    bool boolresult()
    {
        int result = (lua_isboolean(L, -1)) ? lua_toboolean(L, -1) : 0;
        lua_pop(L, 1);
        return result ? true : false;
    }
    int intresult()
    {
        int result = (lua_isnumber(L, -1)) ? lua_tointeger(L, -1) : 0;
        lua_pop(L, 1);
        return result;
    }
    void strresult(u8string *res)
    {
        if (lua_isstring(L, -1)) res->assign(lua_tostring(L, -1));
        else res->clear();
        lua_pop(L, 1);
    }    
};

// active objects api
// supported types: aliases,actions,subs,antisubs,highlights,hotkeys,gags,vars,groups,timers,tabs
class luaT_ActiveObjects
{
    lua_State *L;
    u8string aotype;
public:
    enum { KEY = 0, VALUE, GROUP };
    luaT_ActiveObjects(lua_State *pL, const char* type) : L(pL), aotype(type)
    {
    }
    int size()
    {
        if (!getao()) return 0;
        luaT_run(L, "size", "o");
        return intresult();
    }
    bool select(int index)
    {
        if (!getao()) return false;
        luaT_run(L, "select", "od", index);
        return boolresult();
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (!getao()) return false;
        luaT_run(L, "add", "osss", key, value, group);
        return boolresult();
    }
    bool replace(const utf8* key, const utf8* value, const utf8* group)
    {
        if (!getao()) return false;
        luaT_run(L, "replace", "osss", key, value, group);
        return boolresult();
    }
    bool del()
    {
        if (!getao()) return false;
        luaT_run(L, "delete", "o");
        return boolresult();
    }
    int getIndex()
    {
        if (!getao()) return false;
        luaT_run(L, "getIndex", "o");
        return intresult();
    }
    bool setIndex(int index)
    {
        if (!getao()) return false;
        luaT_run(L, "setIndex", "od", index);
        return boolresult();
    }
    bool get(int param, u8string* value)
    {
        if (!value || !getao()) return false;
        luaT_run(L, "get", "od", param);
        return strresult(value);
    }
    bool set(int param, const utf8* value)
    {
        if (!value || !getao()) return false;
        luaT_run(L, "set", "ods", param, value);
        return boolresult();
    }
    bool update()
    {
        if (!getao()) return false;
        luaT_run(L, "update", "o");
        return true;
    }
private:
    bool getao()
    {
        lua_getglobal(L, aotype.c_str());
        if (!luaT_isobject(L, LUAT_ACTIVEOBJS, -1))
        {
            lua_pop(L, 1);
            return false;
        }
        return true;
    }
    bool boolresult()
    {
        int result = (lua_isboolean(L, -1)) ? lua_toboolean(L, -1) : 0;
        lua_pop(L, 1);
        return result ? true : false;
    }
    int intresult()
    {
        int result = (lua_isnumber(L, -1)) ? lua_tointeger(L, -1) : 0;
        lua_pop(L, 1);
        return result;
    }
    bool strresult(u8string *res)
    {
        bool result = false;
        if (lua_isstring(L, -1)) 
            { res->assign(lua_tostring(L, -1)); result = true; } 
        else res->clear();
        lua_pop(L, 1);
        return result;
    }
};

// utf8 <-> wide <-> ansi
typedef void* strbuf;
void* strbuf_ptr(strbuf b);
void  strbuf_destroy(strbuf b);
strbuf convert_utf8_to_wide(const utf8* string);
strbuf convert_wide_to_utf8(const wchar_t* string);
strbuf convert_ansi_to_wide(const char* string);
strbuf convert_wide_to_ansi(const wchar_t* string);

//utf8 helpers functions
int utf8_symlen(const utf8* symbol);
int utf8_strlen(const utf8* string);
int utf8_sympos(const utf8* string, int index);
strbuf utf8_trim(const utf8* string);

//u8string wrapper
class U8
{   u8string& s;
public:
    U8(u8string& string) : s(string) {}
    operator u8string&() { return s; }
    operator u8string*() { return &s; }
    u8string at(int index) const {
        int pos = utf8_sympos(s.c_str(), index);
        if (pos == -1) return u8string();
        const utf8 *p = s.c_str()+pos;
        return u8string(p, utf8_symlen(p));
    }
    void trim() {
        strbuf b = utf8_trim(s.c_str()); 
        s.assign((const utf8*)strbuf_ptr(b));
        strbuf_destroy(b);
    }
};

class TU2W
{
    strbuf b;
public:
    TU2W(const utf8* string) { b = convert_utf8_to_wide(string); }
    ~TU2W() { strbuf_destroy(b); }
    operator const wchar_t*() const { return (const wchar_t*)strbuf_ptr(b); }
};

class TW2U
{
    strbuf b;
public:
    TW2U(const wchar_t* string) { b = convert_wide_to_utf8(string); }
    ~TW2U() { strbuf_destroy(b); }
    operator const utf8*() const { return (const utf8*)strbuf_ptr(b); }
};

class TA2W
{
    strbuf b;
public:
    TA2W(const char* string) { b = convert_ansi_to_wide(string); }
    ~TA2W() { strbuf_destroy(b); }
    operator const wchar_t*() const { return (const wchar_t*)strbuf_ptr(b); }
};

class TW2A
{
    strbuf b;
public:
    TW2A(const wchar_t* string) { b = convert_wide_to_ansi(string); }
    ~TW2A() { strbuf_destroy(b); }
    operator const char*() const { return (const char*)strbuf_ptr(b); }
};

class luaT_Props
{
    lua_State *L;
    const char* obj = "props";
public:
    luaT_Props(lua_State *pL) : L(pL) {}
    COLORREF paletteColor(int index)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "paletteColor", "td", index);
        return uintresult();
    }
    COLORREF backgroundColor()
    {
        lua_getglobal(L, obj);
        luaT_run(L, "backgroundColor", "t");
        return uintresult();
    }
    HFONT currentFont()
    {
        lua_getglobal(L, obj);
        luaT_run(L, "currentFontHandle", "t");
        if (!lua_isnumber(L, -1))
            return NULL;
        return (HFONT)uintresult();
    }
    void cmdPrefix(u8string* str)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "cmdPrefix", "t");
        strresult(str);
    }
    void cmdSeparator(u8string* str)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "cmdSeparator", "t");
        strresult(str);
    }
    void serverHost(u8string* str)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "serverHost", "t");
        strresult(str);
    }
    void serverPort(u8string* str)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "serverPort", "t");
        strresult(str);
    }
    bool connected()
    {
        lua_getglobal(L, obj);
        luaT_run(L, "connected", "t");
        return boolresult();
    }
    bool activated()
    {
        lua_getglobal(L, obj);
        luaT_run(L, "activated", "t");
        return boolresult();
    }

private:
    bool boolresult()
    {
        int result = (lua_isboolean(L, -1)) ? lua_toboolean(L, -1) : 0;
        lua_pop(L, 1);
        return result ? true : false;
    }
    unsigned int uintresult()
    {
        int result = (lua_isnumber(L, -1)) ? lua_tounsigned(L, -1) : 0;
        lua_pop(L, 1);
        return result;
    }
    void strresult(u8string *res)
    {
        if (lua_isstring(L, -1)) res->assign(lua_tostring(L, -1));
        else res->clear();
        lua_pop(L, 1);
    }
};

class luaT_Msdp
{
     lua_State *L;
      const char* obj = "msdp";
public:
    luaT_Msdp(lua_State *pL) : L(pL) {}
    void list(const u8string& listname)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "list", "ts", listname.c_str());
    }
    void reset(const std::vector<u8string>& vars)
    {
        runf("reset", vars);
    }
    void send(const std::vector<u8string>& vars)
    {
        runf("send", vars);
    }
    void report(const std::vector<u8string>& vars)
    {
        runf("report", vars);
    }
    void unreport(const std::vector<u8string>& vars)
    {
        runf("unreport", vars);
    }
private:
    void runf(const utf8* fname, const std::vector<u8string>& t)
    {
        lua_getglobal(L, obj);
        pushtable(t);
        luaT_run(L, fname, "tt");
    }
    void pushtable(const std::vector<u8string>& t)
    {
        lua_newtable(L);
        for (int i=0,e=t.size();i<e;++i)
        {
            lua_pushinteger(L, i+1);
            lua_pushstring(L, t[i].c_str());
            lua_settable(L, -3);
        }
    }
};

// xml api
typedef void* xnode;
typedef void* xlist;
typedef const utf8* xstring;

xnode xml_load(const utf8* filename);
int   xml_save(xnode node, const utf8* filename);
xnode xml_open(const utf8* name);
void  xml_delete(xnode node);
void  xml_set(xnode node, const utf8* name, const utf8* value);
xstring xml_get_name(xnode node);
xstring xml_get_attr(xnode node, const utf8* name);
void  xml_set_text(xnode node, const utf8* text);
xstring xml_get_text(xnode node);
xnode xml_create_child(xnode node, const utf8* childname);
xlist xml_request(xnode node, const utf8* request);
int   xml_get_attr_count(xnode node);
xstring xml_get_attr_name(xnode node, int index);
xstring xml_get_attr_value(xnode node, int index);
xnode xml_move(xnode node, const utf8* path, int create);
void  xml_list_delete(xlist list);
int   xml_list_size(xlist list);
xnode xml_list_getnode(xlist list, int index);

namespace xml
{
    // xml node helper
    class node 
    {
    public:
        node() : m_Node(NULL) {}
        node(const utf8* rootnode) { m_Node = xml_open(rootnode); }
        node(xnode xml_node) : m_Node(xml_node) {}
        ~node() {}
        operator xnode() { return m_Node; }
        operator bool() const { return (m_Node) ? true : false; }
        bool load(const utf8 *filename) { deletenode();  m_Node = xml_load(filename); return m_Node ? true : false; }
        bool save(const utf8 *filename) { int result = xml_save(m_Node, filename); return result ? true : false; }
        void deletenode() { xml_delete(m_Node); m_Node = NULL; }
        void getname(u8string *name) { name->assign(xml_get_name(m_Node)); }
        bool get(const utf8* attribute, u8string* value)  { return _getp(xml_get_attr(m_Node, attribute), value); }
        bool get(const utf8* attribute, int* value)
        {
            u8string result;
            if (!get(attribute, &result)) return false;
            const utf8* num = result.c_str();
            if (strspn(num, "-0123456789") != strlen(num)) return false;
            *value = atoi(num);
            return true;
        }
        bool get(const utf8* attribute, std::wstring* value)
        {
            u8string result;
            if (!get(attribute, &result)) return false;
            value->assign(TU2W(result.c_str()));
            return true;
        }
        void set(const utf8* attribute, const utf8* value) { xml_set(m_Node, attribute, value); }
        void set(const utf8* attribute, int value) { char buffer[16]; _itoa_s(value, buffer, 10); xml_set(m_Node, attribute, buffer); }
        void set(const utf8* attribute, const std::wstring& value) { xml_set(m_Node, attribute, TW2U(value.c_str())); }
        void gettext(u8string *text) {  text->assign(xml_get_text(m_Node)); }
        void settext(const utf8* text) { xml_set_text(m_Node, text); }
        xml::node createsubnode(const utf8* name) { return xml::node(xml_create_child(m_Node, name)); }
        int  size() { return xml_get_attr_count(m_Node); }
        bool getattrname(int index, u8string* value) { return _getp(xml_get_attr_name(m_Node, index), value); }
        bool getattrvalue(int index, u8string* value) { return _getp(xml_get_attr_value(m_Node, index), value); }
        bool move(const utf8* path) { return _nmove(path, false); }
        bool create(const utf8* path) { return _nmove(path, true); }

    private:
        bool _getp(xstring str, u8string* value)
        {
            if (!str) { value->clear(); return false; }
            value->assign(str);
            return true;
        }

        bool _nmove(const utf8* path, bool create)
        {
            xnode newnode = xml_move(m_Node, path, create ? 1 : 0);
            if (!newnode) return false;
            m_Node = newnode;
            return true;
        }
        xnode m_Node;
    };

    // xml XPath request helper
    class request
    {
    public:
        request(xnode node, const utf8 *request_string)
        {
            m_NodeList = xml_request(node, request_string);
            m_ListSize = xml_list_size(m_NodeList);
        }
        request(xml::node& node, const utf8 *request_string)
        {
            m_NodeList = xml_request(node, request_string); 
            m_ListSize = xml_list_size(m_NodeList); 
        }
        ~request() { xml_list_delete(m_NodeList);  }
        int   size() const { return m_ListSize; }
        bool  empty() const { return (m_ListSize==0) ? true : false; }
        xml::node operator[](int node_index) { return xml::node(xml_list_getnode(m_NodeList, node_index)); }
    private:
        xlist m_NodeList;
        int   m_ListSize;
    };
}

//pcre api
typedef void* pcre8;
pcre8 pcre_create(const utf8* regexp);
void  pcre_delete(pcre8 handle);
bool  pcre_find(pcre8 handle, const utf8* string);
bool  pcre_findall(pcre8 handle, const utf8* string);
int   pcre_size(pcre8 handle);
int   pcre_first(pcre8 handle, int index);
int   pcre_last(pcre8 handle, int index);
const utf8* pcre_string(pcre8 handle, int index);

// pcre helper
class Pcre
{
public:
    Pcre() : regexp(NULL) {}
    ~Pcre() { pcre_delete(regexp); }
    bool init(const utf8* _rgxp)
    {
        pcre_delete(regexp);
        regexp = pcre_create(_rgxp);
        return (regexp) ? true : false;
    }
    bool find(const utf8* string) { return pcre_find(regexp, string); }
    bool findall(const utf8* string) { return pcre_findall(regexp, string); }
    int  size() { return pcre_size(regexp); }
    int  first(int index) { return pcre_first(regexp, index); }
    int  last(int index) { return pcre_last(regexp, index); }
    void get(int index, u8string *str) { str->assign(pcre_string(regexp, index)); }
private:
    pcre8 regexp;
};
