#pragma once

#pragma warning(disable: 4996)

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#pragma comment(lib, "lua.lib")

#include <stdlib.h>

class lua_pushwstring
{
public:
    lua_pushwstring(lua_State* L, const wchar_t* string) {
      int buffer_required = WideCharToMultiByte(CP_UTF8, 0, string, -1, NULL, 0, NULL, NULL);
      char *buffer = new char [buffer_required+1];
      WideCharToMultiByte(CP_UTF8, 0, string, -1, buffer, buffer_required, NULL, NULL);
      buffer[buffer_required] = 0;
      lua_pushstring(L, buffer);
      delete []buffer;
    }
};

class lua_towstring
{
    wchar_t *buffer;
public:
    lua_towstring(lua_State* L, int index) : buffer(NULL) {
      if (!lua_isstring(L, index)) return;
      const char* string = lua_tostring(L, index);
      int buffer_required = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
      buffer = new wchar_t[buffer_required+1];
      MultiByteToWideChar(CP_UTF8, 0, string, -1, buffer, buffer_required);
      buffer[buffer_required] = 0;
    }
    ~lua_towstring() { delete buffer; }
    operator const wchar_t*() const { return buffer; }
};

class wstring_to_int
{
    int result;
public:
    wstring_to_int(const wchar_t *string, bool *check = NULL) : result(0) {
        bool converted = false;
        if (string) {
          int len = wcslen(string);
          if (len > 0)
          {
              const wchar_t *p = string;
              if (*p == L'-') { p++; len--;}
              if (wcsspn(p, L"0123456789") == len)
                { result = _wtoi(string); converted = true; }
          }
        }
        if (check) *check = converted;
    }
    operator int() const { return result; }
};

class int_to_wstring 
{
    wchar_t buffer[16];
public:
    int_to_wstring(int index) { _itow(index, buffer, 10); }
    operator const wchar_t*() const { return buffer; }
};
