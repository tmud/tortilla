#pragma once

void regFunction(lua_State *L, const char* name, lua_CFunction f);
void tstring_tolower(tstring *str);
bool isOnlyDigits(const tstring& str);
void string_replace(std::string *str, const std::string& what, const std::string& forr);
