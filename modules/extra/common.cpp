#include "stdafx.h"
#include "common.h"

void regFunction(lua_State *L, const char* name, lua_CFunction f)
{
    lua_pushstring(L, name);
    lua_pushcfunction(L, f);
    lua_settable(L, -3);
}

void tstring_tolower(tstring *str)
{
    std::locale loc("");
    const std::ctype<wchar_t>& ct = std::use_facet<std::ctype<wchar_t> >(loc);
    std::transform(str->begin(), str->end(), str->begin(), std::bind1st(std::mem_fun(&std::ctype<wchar_t>::tolower), &ct));
}

bool isOnlySymbols(const tstring& str, const tstring& symbols)
{
    int pos = wcsspn(str.c_str(), symbols.c_str());
    return (pos != str.length()) ? false : true;
}

bool isOnlyDigits(const tstring& str)
{
   if (str.empty()) return false;
   return isOnlySymbols(str, L"0123456789");
}

void string_replace(std::string *str, const std::string& what, const std::string& forr)
{
    size_t pos = 0;
    while((pos = str->find(what, pos)) != std::string::npos)
    {
        str->replace(pos, what.length(), forr);
        pos += forr.length();
    }
}