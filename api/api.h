#pragma once

#include "base.h"

#ifndef API_EXPORTS
#pragma comment(lib, "api.lib")
#endif

#include <vector>
typedef char utf8;
typedef std::string u8string;

// utf8 <-> wide <-> ansi
typedef void* strbuf;
void* strbuf_ptr(strbuf b);
void  strbuf_destroy(strbuf b);
strbuf strbuf_new(int bytes);

strbuf convert_utf8_to_wide(const utf8* string);
strbuf convert_wide_to_utf8(const wchar_t* string);
strbuf convert_ansi_to_wide(const char* string);
strbuf convert_wide_to_ansi(const wchar_t* string);

//utf8 helpers functions
int utf8_symlen(const utf8* symbol);
int utf8_strlen(const utf8* string);
int utf8_sympos(const utf8* string, int index);
strbuf utf8_trim(const utf8* string);

class wstring_helper
{
    std::wstring &s;
public:
    wstring_helper(std::wstring & string) : s(string) {}
    void trim()
    {
        int pos = wcsspn(s.c_str(), L" ");
        if (pos != 0)
            s.assign(s.substr(pos));
        if (s.empty())
            return;
        int last = s.size() - 1;
        pos = last;
        while (s.at(pos) == L' ')
           pos--;
        if (pos != last)
           s.assign(s.substr(0, pos + 1));
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
#define LUAT_IMAGE      109
#define LUAT_TRIGGER    110
#define LUAT_VIEWSTRING 111
#define LUAT_LAST       111

class luaT_towstring 
{
    strbuf b;
public:
   luaT_towstring(lua_State *L, int index) : b(NULL) {
       if (!lua_isstring(L, index)) b = strbuf_new(2);
       else b = convert_utf8_to_wide(lua_tostring(L, index));   
   }
   ~luaT_towstring() { strbuf_destroy(b); }
    operator const wchar_t*() const { return (const wchar_t*)strbuf_ptr(b); }
};

class luaT_pushwstring
{
public:
    luaT_pushwstring(lua_State *L, const wchar_t* string) {
        strbuf b = convert_wide_to_utf8(string);
        lua_pushstring(L, (const utf8*)strbuf_ptr(b));
        strbuf_destroy(b);
    }
};

bool  luaT_check(lua_State *L, int n, ...);
bool  luaT_run(lua_State *L, const char* func, const char* op, ...);
int   luaT_error(lua_State *L, const wchar_t* error_message);
void* luaT_toobject(lua_State* L, int index);
void  luaT_pushobject(lua_State* L, void *object, int type);
bool  luaT_isobject(lua_State* L, int type, int index);
const char* luaT_typename(lua_State* L, int index);
bool  luaT_dostring(lua_State *L, const wchar_t* script_text);
void  luaT_showLuaStack(lua_State* L, const wchar_t* label);
void  luaT_showTableOnTop(lua_State* L, const wchar_t* label);
#define SS(L,n) luaT_showLuaStack(L,n)
#define ST(L,n) luaT_showTableOnTop(L,n)

namespace base {
    inline void addMenu(lua_State* L, const wchar_t* path, int id, int pos = -1, int bitmap = -1) {
        luaT_run(L, "addMenu", "sddd", path, id, pos, bitmap);
    }
    inline void addCommand(lua_State* L, const wchar_t* cmd) {
        luaT_run(L, "addCommand", "s", cmd);
    }
    inline void runCommand(lua_State* L, const wchar_t* cmd)  {
        luaT_run(L, "runCommand", "s", cmd);
    }
    inline void setCommand(lua_State* L, const wchar_t* cmd)  {
        luaT_run(L, "setCommand", "s", cmd);
    }
    inline void addButton(lua_State *L, int bmp, int id, const wchar_t* tooltip) {
        luaT_run(L, "addButton", "dds", bmp, id, tooltip);
    }
    inline void addToolbar(lua_State *L, const wchar_t* name, int button_size) {
        luaT_run(L, "addToolbar","sd", name, button_size);
    }
    inline void addToolbar(lua_State *L, const wchar_t* name) {
        luaT_run(L, "addToolbar","s", name);
    }
    inline void showToolbar(lua_State *L, const wchar_t* name) {
        luaT_run(L, "showToolbar","s", name);
    }
    inline void hideToolbar(lua_State *L, const wchar_t* name) {
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
    inline void getPath(lua_State *L, const wchar_t* file, std::wstring* path) {
        luaT_run(L, "getPath", "s", file);
        if (!lua_isstring(L, -1)) return;
        path->assign(luaT_towstring(L, -1));
        lua_pop(L, 1);
    }
    inline void getProfilePath(lua_State *L, std::wstring* path) {
        luaT_run(L, "getProfilePath", "");
        if (!lua_isstring(L, -1)) return;
        path->assign(luaT_towstring(L, -1));
        lua_pop(L, 1);
    }
    inline void getResource(lua_State *L, const wchar_t* file, std::wstring* path) {
        luaT_run(L, "getResource", "s", file);
        if (!lua_isstring(L, -1)) return;
        path->assign(luaT_towstring(L, -1));
        lua_pop(L, 1);
    }
    inline void getProfile(lua_State *L, std::wstring* profile) {
        luaT_run(L, "getProfile", "");
        if (!lua_isstring(L, -1)) return;
        profile->assign(luaT_towstring(L, -1));
        lua_pop(L, 1);
    }
    inline HWND getParent(lua_State *L) {
        luaT_run(L, "getParent", "");
        if (!lua_isnumber(L,1))
            return 0;
        HWND parent = (HWND)lua_tounsigned(L, -1);
        lua_pop(L, 1);
        return parent;
    }
    inline void flashWindow(lua_State *L) {
        luaT_run(L, "flashWindow", "");
    }
    inline void saveTable(lua_State* L, const wchar_t* file) {
        if (lua_istable(L, -1))
            luaT_run(L, "saveTable", "rs", file);
    }
    inline bool loadTable(lua_State* L, const wchar_t* file) {
        luaT_run(L, "loadTable", "s", file);
        return lua_istable(L, -1) ? true : false;
    }
    inline void pluginName(lua_State* L, const wchar_t* name) {
        luaT_run(L, "pluginName", "s", name);
    }
    inline void updateView(lua_State* L, int view, lua_CFunction f) {
        luaT_run(L, "updateView", "dF", view, f);
    }
    inline bool isViewVisible(lua_State* L, int view) {
        luaT_run(L, "isViewVisible", "d", view);
        int result = (lua_isboolean(L, -1)) ? lua_toboolean(L, -1) : 0;
        lua_pop(L, 1);
        return (result == 1);
    }
    inline void showView(lua_State* L, int view) {
        luaT_run(L, "showView", "d", view);
    }
    inline void hideView(lua_State* L, int view) {
        luaT_run(L, "hideView", "d", view);
    }
    inline bool getViewSize(lua_State* L, int view, int *width, int *height) {
        luaT_run(L, "getViewSize", "d", view);
        if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
        {
            if (width) *width = lua_tointeger(L, 1);
            if (height) *height = lua_tointeger(L, 2);
            return true;
        }
        return false;
    }
    inline void vprint(lua_State* L, int view, const wchar_t* message) {
        luaT_run(L, "vprint", "ds", view, message);
    }
    inline void print(lua_State* L, const wchar_t* message) {
        luaT_run(L, "print", "s", message);
    }
    inline void terminate(lua_State *L) {
        luaT_run(L, "terminate", "");
    }
    inline void log(lua_State *L, const wchar_t* message) {
        luaT_run(L, "log", "s", message);
    }
    inline bool translateColors(lua_State* L, const wchar_t* str, COLORREF* text, COLORREF *bgnd) {
        luaT_run(L, "translateColors", "s", str);
        if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
        {
            if (text) *text = lua_tounsigned(L, 1);
            if (bgnd) *bgnd = lua_tounsigned(L, 2);
            return true;
        }
        return false;
    }

    // createWindow, createPanel, pcre -> in classes below
} // namespace base

class luaT_window
{
    lua_State *L;
    void *window;
public:
    luaT_window() : L(NULL), window(NULL) {}
	bool create(lua_State *pL, const wchar_t* caption, int width, int height, bool visible)
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
	bool create(lua_State *pL, const wchar_t* caption, int width, int height)
	{
		if (!pL)
			return false;
		L = pL;
		luaT_run(L, "createWindow", "sdd", caption, width, height);
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
    void dock(const wchar_t* side)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "dock", "os", side);
    }
    void undock()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "undock", "o");
    }
    void block(const wchar_t* side)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "block", "os", side);
    }
    void attach(HWND child)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "attach", "od", child);
    }
    void setFixedSize(int width, int height)
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "setFixedSize", "odd", width, height);
    }
    SIZE getSize()
    {
        luaT_pushobject(L, window, LUAT_WINDOW);
        luaT_run(L, "getSize", "o");
        SIZE sz;
        sz.cx = lua_tointeger(L, -1);
        sz.cy = lua_tointeger(L, -2);
        lua_pop(L, 2);
        return sz;
    }
};

class luaT_panel
{
    lua_State *L;
    void *panel;
public:
    luaT_panel() : L(NULL), panel(NULL) {}
    bool create(lua_State *pL, const wchar_t* side, int size)
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
    void getPrompt(std::wstring *str)
    {
        runcmd("getPrompt");
        strresult(str);
    }
    void getText(std::wstring* str) 
    {
        runcmd("getText");
        strresult(str); 
    }
    int getTextLen()
    {
        runcmd("getTextLen");
        return intresult();
    }
    void getHash(std::wstring* str)
    {
        runcmd("getHash");
        strresult(str);
    }
    int blocks()            // count of blocks for selected string
    { 
        runcmd("blocks");
        return intresult(); 
    }
    void getBlockText(int block, std::wstring* str)
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
    bool setBlockText(int block, const wchar_t* text)
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
    bool find(Pcre *p, int from)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_pushobject(L, p, LUAT_PCRE);
        luaT_run(L, "find", "otd", from);
        return boolresult();
    }
    bool getBlockPos(int abspos, int *res_block, int *res_pos)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "find", "od", abspos);
        bool result = false;
        if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
        {
            *res_block = lua_tointeger(L, 1);
            *res_pos = lua_tointeger(L, 2);
            result = true;
        }
        lua_pop(L, 2);
        return result;
    }
    void setNext(bool next)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "setNext", "ob", next);
    }
    void setPrev(bool prev)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "setPrev", "ob", prev);
    }
    bool isNext()
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "isNext", "o");
        return boolresult();
    }
    bool isPrev()
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, "isPrev", "o");
        return boolresult();
    }

private:
    void runcmd(const char* cmd)
    {
        luaT_pushobject(L, view_data, LUAT_VIEWDATA);
        luaT_run(L, cmd, "o");
    }
    void runcmdint(const char* cmd, int p)
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
    void strresult(std::wstring *res)
    {
        if (lua_isstring(L, -1)) res->assign(luaT_towstring(L, -1));
        else res->clear();
        lua_pop(L, 1);
    }    
};

// active objects api
// supported types: aliases,actions,subs,antisubs,highlights,hotkeys,gags,vars,groups,timers,tabs
class luaT_ActiveObjects
{
    lua_State *L;
    std::string aotype;
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
    bool add(const wchar_t* key, const wchar_t* value, const wchar_t* group)
    {
        if (!getao()) return false;
        luaT_run(L, "add", "osss", key, value, group);
        return boolresult();
    }
    bool replace(const wchar_t* key, const wchar_t* value, const wchar_t* group)
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
    bool get(int param, std::wstring* value)
    {
        if (!value || !getao()) return false;
        luaT_run(L, "get", "od", param);
        return strresult(value);
    }
    bool set(int param, const wchar_t* value)
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
    bool strresult(std::wstring *res)
    {
        bool result = false;
        if (lua_isstring(L, -1)) 
            { res->assign(luaT_towstring(L, -1)); result = true; } 
        else res->clear();
        lua_pop(L, 1);
        return result;
    }
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
    void cmdPrefix(std::wstring* str)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "cmdPrefix", "t");
        strresult(str);
    }
    void cmdSeparator(std::wstring* str)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "cmdSeparator", "t");
        strresult(str);
    }
    void serverHost(std::wstring* str)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "serverHost", "t");
        strresult(str);
    }
    void serverPort(std::wstring* str)
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
    bool isPropertiesOpen()
    {
        lua_getglobal(L, obj);
        luaT_run(L, "isPropertiesOpen", "t");
        return boolresult();
    }
    int pluginsLogWindow()
    {
        lua_getglobal(L, obj);
        luaT_run(L, "pluginLogWindow", "t");
        if (lua_isboolean(L, -1)) 
            { lua_pop(L, 1); return -1; }
        if (lua_isnumber(L, -1))
            { int w = lua_tointeger(L, -1); lua_pop(L, 1); return w; }
        return -1;
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
    void strresult(std::wstring *res)
    {
        if (lua_isstring(L, -1)) res->assign(luaT_towstring(L, -1));
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
    void list(const std::wstring& listname)
    {
        lua_getglobal(L, obj);
        luaT_run(L, "list", "ts", listname.c_str());
    }
    void reset(const std::vector<std::wstring>& vars)
    {
        runf("reset", vars);
    }
    void send(const std::vector<std::wstring>& vars)
    {
        runf("send", vars);
    }
    void report(const std::vector<std::wstring>& vars)
    {
        runf("report", vars);
    }
    void unreport(const std::vector<std::wstring>& vars)
    {
        runf("unreport", vars);
    }
private:
    void runf(const char* fname, const std::vector<std::wstring>& t)
    {
        lua_getglobal(L, obj);
        pushtable(t);
        luaT_run(L, fname, "tt");
    }
    void pushtable(const std::vector<std::wstring>& t)
    {
        lua_newtable(L);
        for (int i=0,e=t.size();i<e;++i)
        {
            lua_pushinteger(L, i+1);
            luaT_pushwstring(L, t[i].c_str());
            lua_settable(L, -3);
        }
    }
};

// xml api
typedef void* xnode;
typedef void* xlist;
typedef strbuf xstringw; // use strbuf_destroy to free string memory

xnode xml_load(const wchar_t* filename);
int   xml_save(xnode node, const wchar_t* filename);
xnode xml_open(const wchar_t* name);
void  xml_delete(xnode node);
void  xml_set(xnode node, const wchar_t* name, const wchar_t* value);
xstringw xml_get_name(xnode node);
xstringw xml_get_attr(xnode node, const wchar_t* name);
void  xml_set_text(xnode node, const wchar_t* text);
xstringw xml_get_text(xnode node);
xnode xml_create_child(xnode node, const wchar_t* childname);
xlist xml_request(xnode node, const wchar_t* request);
int   xml_get_attr_count(xnode node);
xstringw xml_get_attr_name(xnode node, int index);
xstringw xml_get_attr_value(xnode node, int index);
xnode xml_move(xnode node, const wchar_t* path, int create);
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
        node(const wchar_t* rootnode) { m_Node = xml_open(rootnode); }
        node(xnode xml_node) : m_Node(xml_node) {}
        ~node() {}
        operator xnode() { return m_Node; }
        operator bool() const { return (m_Node) ? true : false; }
        bool load(const wchar_t *filename) { deletenode();  m_Node = xml_load(filename); return m_Node ? true : false; }
        bool save(const wchar_t *filename) { int result = xml_save(m_Node, filename); return result ? true : false; }
        void deletenode() { xml_delete(m_Node); m_Node = NULL; }
        void getname(std::wstring *name) { _getp(xml_get_name(m_Node), name); }
        bool get(const wchar_t* attribute, std::wstring* value)  { return _getp(xml_get_attr(m_Node, attribute), value); }
        bool get(const wchar_t* attribute, int* value) {
            std::wstring result;
            if (!get(attribute, &result)) return false;
            const wchar_t* num = result.c_str();
            if (wcsspn(num, L"-0123456789") != wcslen(num)) return false;
            *value = _wtoi(num);
            return true;
        }
        void set(const wchar_t* attribute, const wchar_t* value) { xml_set(m_Node, attribute, value); }
        void set(const wchar_t* attribute, int value) { wchar_t buffer[16]; _itow_s(value, buffer, 10); xml_set(m_Node, attribute, buffer); }
        void set(const wchar_t* attribute, const std::wstring& value) { xml_set(m_Node, attribute, value.c_str()); }
        void gettext(std::wstring *text) {  _getp(xml_get_text(m_Node), text); }
        void settext(const wchar_t* text) { xml_set_text(m_Node, text); }
        xml::node createsubnode(const wchar_t* name) { return xml::node(xml_create_child(m_Node, name)); }
        int  size() { return xml_get_attr_count(m_Node); }
        bool getattrname(int index, std::wstring* value) { return _getp(xml_get_attr_name(m_Node, index), value); }
        bool getattrvalue(int index, std::wstring* value) { return _getp(xml_get_attr_value(m_Node, index), value); }
        bool move(const wchar_t* path) { return _nmove(path, false); }
        bool create(const wchar_t* path) { return _nmove(path, true); }
    private:
        node(const char* rootnode) {} // blocked constructor (protect from ansi names strings)
        bool _getp(xstringw str, std::wstring* value)
        {
            if (!str) { value->clear(); return false; }
            const wchar_t *s = (const wchar_t*)strbuf_ptr(str);
            value->assign(s);
            strbuf_destroy(str);
            return true;
        }
        bool _nmove(const wchar_t* path, bool create)
        {
            xnode newnode = xml_move(m_Node, path, create ? 1 : 0);
            if (!newnode) return false;
            m_Node = newnode;
            return true;
        }
    private:
        xnode m_Node;
    };

    // xml XPath request helper
    class request
    {
    public:
        request(xnode node, const wchar_t *request_string)
        {
            m_NodeList = xml_request(node, request_string);
            m_ListSize = xml_list_size(m_NodeList);
        }
        request(xml::node& node, const wchar_t *request_string)
        {
            m_NodeList = xml_request(node, request_string);
            m_ListSize = xml_list_size(m_NodeList);
        }
        ~request() { xml_list_delete(m_NodeList);  }
        int   size() const { return m_ListSize; }
        bool  empty() const { return (m_ListSize==0) ? true : false; }
        xml::node operator[](int node_index) { return xml::node(xml_list_getnode(m_NodeList, node_index)); }
    private:
        request(const request& r) {}
        request& operator=(const request& r) {}
    private:
        xlist m_NodeList;
        int   m_ListSize;
    };
}

//pcre api
typedef void* hpcre;
typedef strbuf hpcre_string; // use strbuf_destroy to free string memory
hpcre pcre_create(const wchar_t* regexp);
void  pcre_delete(hpcre handle);
bool  pcre_find(hpcre handle, const wchar_t* string);
bool  pcre_findall(hpcre handle, const wchar_t* string);
int   pcre_size(hpcre handle);
int   pcre_first(hpcre handle, int index);
int   pcre_last(hpcre handle, int index);
hpcre_string pcre_string(hpcre handle, int index);

// pcre helper
class Pcre
{
public:
    Pcre() : regexp(NULL) {}
    ~Pcre() { pcre_delete(regexp); }
    bool init(const wchar_t* rgxp)
    {
        pcre_delete(regexp);
        regexp = pcre_create(rgxp);
        return (regexp) ? true : false;
    }
    bool find(const wchar_t* string) { return pcre_find(regexp, string); }
    bool findall(const wchar_t* string) { return pcre_findall(regexp, string); }
    int  size() { return pcre_size(regexp); }
    int  first(int index) { return pcre_first(regexp, index); }
    int  last(int index) { return pcre_last(regexp, index); }
    bool get(int index, std::wstring *str) { 
        if (!str) return false;
        hpcre_string s = pcre_string(regexp, index);
        if (!s) return false;
        str->assign((const wchar_t*)strbuf_ptr(s));
        strbuf_destroy(s);
        return true;
    }
private:
    Pcre(const Pcre& p) {}
    Pcre& operator=(const Pcre& p) {}
private:
    hpcre regexp;
};

// images support
typedef void* image;
image image_load(const wchar_t* file, int extra_option);
void  image_unload(image img);
image image_cut(image img, int x, int y, int w, int h);
int   image_width(image img);
int   image_height(image img);
int   image_getpixelcolor(image img, int x, int y, COLORREF *c); // return 0-false/not 0-true

struct image_render_ex {
  image_render_ex() : w(0), h(0), sx(0), sy(0), sw(0), sh(0) {}
  int w, h;                         // scaling (width/height of dest. rect); w/h=0 - default (no scale)
  int sx, sy;                       // source image position
  int sw, sh;                       // source image size
};
int image_render(image img, HDC dc, int x, int y, image_render_ex *p);  // 0-fail,not 0-ok

class Image
{
public:
    Image() : img(NULL) {}
    ~Image() { unload(); }
    bool load(const wchar_t* file, int option) {
        unload();
        img = image_load(file, option);
        return (img) ? true : false;
    }
    bool cut(const Image& from, int x, int y, int w, int h) {
        unload();
        img = image_cut(from.img, x, y, w, h);
        return (img) ? true : false;
    }
    void unload() { if (img) { image_unload(img); img = NULL; } }
    int width() const { return image_width(img); }
    int height() const { return image_height(img); }
    int render(HDC dc, int x, int y, image_render_ex *p = NULL) { return image_render(img, dc, x, y, p); }
    bool getcolor(int x, int y, COLORREF *c) { return (image_getpixelcolor(img, x, y, c) == 0) ? false : true; }
private:
    Image(const Image& op) {}
    Image& operator=(const Image& op) {}
private:
    image img;
};
