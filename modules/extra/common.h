#pragma once

void regFunction(lua_State *L, const char* name, lua_CFunction f);
void tstring_tolower(tstring *str);