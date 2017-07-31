#pragma once

void init_clientlog(lua_State *L);
void clientlog(const tstring& text);
void clientlog(const tchar* text);

