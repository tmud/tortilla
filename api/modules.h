#pragma once

class luaM_towstring
{
public:
    luaM_towstring(lua_State* L, int index) : buffer(NULL)
    {
        if (!lua_isstring(L, index))
            return;        
        const char* utf8_string = lua_tostring(L, index);        
        int symbols_count = MultiByteToWideChar(CP_UTF8, 0, utf8_string, -1, NULL, 0);
        int buffer_required = symbols_count + 1;
        buffer = new wchar_t[buffer_required];
        MultiByteToWideChar(CP_UTF8, 0, utf8_string, -1, buffer, buffer_required);
        buffer[symbols_count] = 0;
    }
    ~luaM_towstring()  { delete []buffer; }
    operator const wchar_t*() const { return (buffer) ? buffer : L""; }
private:
    wchar_t* buffer;
};

class luaM_pushwstring
{
public:
    luaM_pushwstring(lua_State *L, const wchar_t* wide_string)
    {
        int buffer_required = WideCharToMultiByte(CP_UTF8, 0, wide_string, -1, NULL, 0, NULL, NULL);
        char* buffer = new char [buffer_required+1];
        WideCharToMultiByte(CP_UTF8, 0, wide_string, -1, buffer, buffer_required, NULL, NULL);
        buffer[buffer_required] = 0;
        lua_pushstring(L, buffer);
        delete []buffer;
    }
};
