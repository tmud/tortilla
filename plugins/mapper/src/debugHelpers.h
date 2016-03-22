#pragma once

#ifdef _DEBUG
void init_debug_helpers(lua_State *_l);
void debug_print(const tstring& text);
#define DEBUGINIT(L) init_debug_helpers(L)
#define DEBUGOUT(x) debug_print(x)
#else
#define DEBUGINIT(x)
#define DEBUGOUT(x)
#endif
