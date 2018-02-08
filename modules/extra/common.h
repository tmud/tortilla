#pragma once

void regFunction(lua_State *L, const char* name, lua_CFunction f);
void tstring_tolower(tstring *str);
void tstring_trimleft(tstring *str);
void tstring_trimright(tstring *str);
void tstring_trim(tstring *str);
bool isOnlyDigits(const tstring& str);
void string_replace(std::string *str, const std::string& what, const std::string& forr);
